"""HTTP API for the file-compression command-line program.

The API deliberately keeps ZIP parsing and extraction in the existing C++
implementation. Python is responsible only for HTTP handling and temporary
file management.
"""

from __future__ import annotations

import io
import os
import re
import subprocess
import tempfile
from pathlib import Path, PurePosixPath
from typing import Iterable
import zipfile

from flask import Flask, jsonify, request, send_file


DEFAULT_MAX_CONTENT_LENGTH = 100 * 1024 * 1024
DEFAULT_MAX_FILE_SIZE = 50 * 1024 * 1024
DEFAULT_MAX_FILES = 100
DEFAULT_TIMEOUT_SECONDS = 60
ZIP_NAME_RE = re.compile(r"^[^/\\][^/\\]*\.zip$", re.IGNORECASE)
INVALID_ZIP_MARKERS = (
    "not a valid basic zip archive",
    "corrupted archive",
    "archive uses unsupported",
    "archive contains an unsupported compression method",
    "archive entry failed deflate decompression",
    "crc check failed",
    "unsafe archive path rejected",
    "duplicate archive path rejected",
    "archive file conflicts",
    "archive contains an invalid entry name",
    "archive entry type does not match",
    "invalid local header",
    "entry data extends beyond the archive",
    "unexpected end of archive",
)


def _error(message: str, status: int):
    return jsonify(error=message), status


def _safe_upload_name(name: str) -> str:
    """Return a safe relative path for an uploaded file.

    Browsers may provide a relative path for directory uploads. We preserve
    those useful subdirectories, but reject absolute paths and traversal.
    """

    name = (name or "").replace("\\", "/")
    path = PurePosixPath(name)
    if not name or path.is_absolute() or "\x00" in name:
        raise ValueError("Invalid uploaded filename")
    parts = path.parts
    if not parts or any(part in {"", ".", ".."} for part in parts):
        raise ValueError("Invalid uploaded filename")
    if any(":" in part for part in parts):
        raise ValueError("Invalid uploaded filename")
    return "/".join(parts)


def _archive_name(value: str | None) -> str:
    value = (value or "archive.zip").strip()
    if not ZIP_NAME_RE.fullmatch(value) or Path(value).name != value:
        raise ValueError("Invalid ZIP filename")
    return value


def _file_size(file_storage) -> int:
    stream = file_storage.stream
    current = stream.tell()
    stream.seek(0, os.SEEK_END)
    size = stream.tell()
    stream.seek(current)
    return size


def _save_upload(file_storage, destination: Path, max_size: int) -> None:
    size = _file_size(file_storage)
    if size > max_size:
        raise ValueError("Uploaded file exceeds the size limit")
    destination.parent.mkdir(parents=True, exist_ok=True)
    file_storage.save(destination)


def _run(binary: Path, args: Iterable[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            [str(binary), *args],
            cwd=cwd,
            capture_output=True,
            text=True,
            timeout=DEFAULT_TIMEOUT_SECONDS,
            check=False,
        )
    except FileNotFoundError as exc:
        raise RuntimeError("Compression executable is not configured or built") from exc
    except subprocess.TimeoutExpired as exc:
        raise RuntimeError("Compression operation timed out") from exc


def _command_error(result: subprocess.CompletedProcess[str]) -> str:
    message = (result.stderr or result.stdout or "Compression operation failed").strip()
    if message.lower().startswith("error:"):
        message = message[6:].strip()
    return message[:500]


def _is_invalid_zip_error(detail: str) -> bool:
    detail = detail.lower()
    return any(marker in detail for marker in INVALID_ZIP_MARKERS)


def _successful(result: subprocess.CompletedProcess[str], operation: str) -> None:
    if result.returncode != 0:
        detail = _command_error(result)
        if operation in {"extract", "list"} and _is_invalid_zip_error(detail):
            raise ValueError("Invalid ZIP file")
        raise RuntimeError(detail)


def create_app(test_config: dict | None = None) -> Flask:
    app = Flask(__name__)
    app.config.from_mapping(
        MAX_CONTENT_LENGTH=int(os.getenv("MAX_CONTENT_LENGTH", DEFAULT_MAX_CONTENT_LENGTH)),
        MAX_FILE_SIZE=int(os.getenv("MAX_FILE_SIZE", DEFAULT_MAX_FILE_SIZE)),
        MAX_FILES=int(os.getenv("MAX_FILES", DEFAULT_MAX_FILES)),
        COMPRESSION_BINARY=os.getenv("COMPRESSION_BINARY", ""),
    )
    if test_config:
        app.config.update(test_config)

    def binary_path() -> Path:
        configured = app.config["COMPRESSION_BINARY"]
        if configured:
            return Path(configured).resolve()
        candidates = [
            Path(__file__).resolve().parent.parent / "build" / "file-compress",
            Path(__file__).resolve().parent.parent / "build" / "Debug" / "file-compress.exe",
        ]
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return candidates[0]

    @app.after_request
    def add_cors(response):
        response.headers["Access-Control-Allow-Origin"] = os.getenv("CORS_ORIGIN", "*")
        response.headers["Access-Control-Allow-Headers"] = "Content-Type"
        response.headers["Access-Control-Allow-Methods"] = "POST, OPTIONS"
        return response

    @app.errorhandler(413)
    def too_large(exc):
        return _error("Request exceeds the upload size limit", 413)

    @app.route("/api/compress", methods=["POST"])
    def compress():
        uploads = [item for item in request.files.getlist("files") if item.filename]
        if not uploads:
            return _error("At least one file is required", 400)
        if len(uploads) > app.config["MAX_FILES"]:
            return _error("Too many files", 400)
        try:
            archive_name = _archive_name(request.form.get("archive_name"))
            with tempfile.TemporaryDirectory(prefix="file-compress-") as raw_dir:
                workdir = Path(raw_dir)
                input_dir = workdir / "inputs"
                seen_names = set()
                for upload in uploads:
                    relative = _safe_upload_name(upload.filename)
                    if relative in seen_names:
                        return _error("Duplicate uploaded filename", 400)
                    seen_names.add(relative)
                    destination = input_dir / relative
                    _save_upload(upload, destination, app.config["MAX_FILE_SIZE"])
                input_paths = list(input_dir.iterdir())
                output = workdir / archive_name
                result = _run(binary_path(), ["compress", str(output), *(str(path) for path in input_paths)], workdir)
                _successful(result, "compress")
                return send_file(io.BytesIO(output.read_bytes()), as_attachment=True, download_name=archive_name, mimetype="application/zip")
        except ValueError as exc:
            return _error(str(exc), 400)
        except RuntimeError as exc:
            return _error(str(exc), 500)

    @app.route("/api/extract", methods=["POST"])
    def extract():
        upload = request.files.get("file") or request.files.get("archive")
        if upload is None or not upload.filename:
            return _error("A ZIP file is required", 400)
        try:
            with tempfile.TemporaryDirectory(prefix="file-extract-") as raw_dir:
                workdir = Path(raw_dir)
                archive = workdir / "input.zip"
                _save_upload(upload, archive, app.config["MAX_FILE_SIZE"])
                extracted = workdir / "extracted"
                result = _run(binary_path(), ["extract", str(archive), str(extracted)], workdir)
                _successful(result, "extract")
                output = workdir / "extracted.zip"
                children = list(extracted.iterdir()) if extracted.exists() else []
                if children:
                    result = _run(binary_path(), ["compress", str(output), *(str(path) for path in children)], workdir)
                    _successful(result, "compress")
                else:
                    with zipfile.ZipFile(output, "w"):
                        pass
                return send_file(io.BytesIO(output.read_bytes()), as_attachment=True, download_name="extracted.zip", mimetype="application/zip")
        except ValueError as exc:
            return _error(str(exc), 400)
        except RuntimeError as exc:
            return _error(str(exc), 500)

    @app.route("/api/list", methods=["POST"])
    def list_archive():
        upload = request.files.get("file") or request.files.get("archive")
        if upload is None or not upload.filename:
            return _error("A ZIP file is required", 400)
        try:
            with tempfile.TemporaryDirectory(prefix="file-list-") as raw_dir:
                archive = Path(raw_dir) / "input.zip"
                _save_upload(upload, archive, app.config["MAX_FILE_SIZE"])
                result = _run(binary_path(), ["list", str(archive)], archive.parent)
                _successful(result, "list")
                return jsonify(files=[line for line in result.stdout.splitlines() if line.strip()])
        except ValueError as exc:
            return _error(str(exc), 400)
        except RuntimeError as exc:
            return _error(str(exc), 500)

    return app


app = create_app()


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=int(os.getenv("PORT", "5000")))
