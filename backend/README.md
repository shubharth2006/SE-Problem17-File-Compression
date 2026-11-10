# Flask backend

The backend exposes the frontend contract:

- `POST /api/compress`: multipart field `files` (one or more files) and optional `archive_name` (defaults to `archive.zip`).
- `POST /api/extract`: multipart field `file` (or `archive`) containing one ZIP. Returns `extracted.zip`.
- `POST /api/list`: multipart field `file` (or `archive`) containing one ZIP. Returns `{ "files": [...] }`.

Errors use `{ "error": "..." }`. Uploads are limited to 100 MiB per request and 50 MiB per file by default. Override `MAX_CONTENT_LENGTH`, `MAX_FILE_SIZE`, `MAX_FILES`, and `COMPRESSION_BINARY` with environment variables.

## Run

Build the C++ executable first from the repository root:

```bash
cmake -S . -B build
cmake --build build --parallel
python -m pip install -r backend/requirements.txt
python -m backend.app
```

The frontend can use `http://127.0.0.1:5000`; CORS is enabled for development and can be restricted with `CORS_ORIGIN`.
