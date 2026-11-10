from io import BytesIO
from pathlib import Path

import pytest

from backend.app import _archive_name, create_app


@pytest.fixture
def client(tmp_path):
    binary = tmp_path / "file-compress"
    binary.write_text("placeholder")
    return create_app({"TESTING": True, "COMPRESSION_BINARY": str(binary)}).test_client()


def test_compress_requires_files(client):
    response = client.post("/api/compress", data={"archive_name": "out.zip"})
    assert response.status_code == 400
    assert response.json == {"error": "At least one file is required"}


def test_archive_name_defaults_to_archive_zip():
    assert _archive_name(None) == "archive.zip"
    assert _archive_name("") == "archive.zip"


def test_oversized_request_returns_json_error():
    app = create_app({"TESTING": True, "MAX_CONTENT_LENGTH": 1})
    response = app.test_client().post(
        "/api/compress",
        data={"files": [(BytesIO(b"data"), "a.txt")]},
        content_type="multipart/form-data",
    )
    assert response.status_code == 413
    assert response.json == {"error": "Request exceeds the upload size limit"}


def test_rejects_traversal_filename(client):
    response = client.post(
        "/api/compress",
        data={
            "archive_name": "out.zip",
            "files": [(Path(__file__).open("rb"), "../escape.txt")],
        },
        content_type="multipart/form-data",
    )
    assert response.status_code == 400
    assert "Invalid uploaded filename" in response.json["error"]


def test_rejects_invalid_archive_name(client):
    response = client.post(
        "/api/compress",
        data={"archive_name": "../out.zip", "files": [(BytesIO(b"data"), "a.txt")]},
        content_type="multipart/form-data",
    )
    assert response.status_code == 400
    assert response.json == {"error": "Invalid ZIP filename"}


def test_missing_zip_is_json_error(client):
    response = client.post("/api/list", data={}, content_type="multipart/form-data")
    assert response.status_code == 400
    assert response.json == {"error": "A ZIP file is required"}
