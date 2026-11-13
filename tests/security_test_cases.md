# Security Test Case Register

## Purpose and execution rules

This register describes security tests for the File Compression Tool. Test
inputs must be synthetic and confined to disposable temporary directories.
Do not use real sensitive files or shared/user directories. No case is
considered passed until it has been executed and its actual result and
evidence are recorded.

The current backend defaults are 50 MiB per file, 100 MiB per request,
100 files, and a 60-second subprocess timeout. These values are configurable.
Tests should record the effective configuration used for each run. The
compression API has no defined extension or MIME allowlist; arbitrary file
contents must not be treated as unsupported solely because of their
extension or MIME type.

Existing coverage is referenced where relevant to avoid duplicating automated
tests. The Actual result field is intentionally blank for every case.

## SEC-UPLOAD-01 — Upload validation and unsupported archive

- **Test objective:** Verify missing or unsafe uploads and invalid/unsupported
  archive content are rejected safely. Confirm that arbitrary file contents
  are not incorrectly rejected by compression based on extension or MIME type.
- **Test input:** Missing required multipart file; synthetic traversal
  filename; plain-text bytes submitted as an archive; synthetic ZIP using an
  unsupported compression method.
- **Test steps:**
  1. Submit a compress request with no file.
  2. Submit a compress request with a traversal-style uploaded filename.
  3. Submit the plain-text and unsupported-method archives to `/api/list` and
     `/api/extract`.
  4. Submit a synthetic ordinary file with an uncommon extension to
     `/api/compress`.
  5. Record status, response body, executable invocation, and any filesystem
     changes.
- **Expected result:** Missing/unsafe inputs and invalid or unsupported
  archives return an error without unsafe writes or a success-shaped response.
  An ordinary file is not rejected solely for its extension or MIME type.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** Partial checks for missing compression files,
  traversal filename, and archive-related error classification are in
  `backend/test_app.py`. Invalid ZIP handling is present in
  `tests/smoke_test.cpp`. Add only uncovered route-level cases.

## SEC-UPLOAD-02 — Empty uploads

- **Test objective:** Distinguish an empty file from an absent upload and
  verify the current empty-file behavior through the API.
- **Test input:** A zero-byte ordinary file; a request with no file; a
  zero-byte file supplied as the archive.
- **Test steps:**
  1. POST the zero-byte ordinary file to `/api/compress`.
  2. Submit the returned ZIP to `/api/list` and `/api/extract`.
  3. Separately submit a request with no required upload.
  4. Submit a zero-byte archive to `/api/list` and `/api/extract`.
- **Expected result:** Empty ordinary input is accepted and remains zero bytes
  after round-trip. Missing upload and zero-byte archive inputs are rejected
  with an error response.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** `backend/test_app.py` checks empty upload saving, and
  `tests/smoke_test.cpp` checks an empty file in C++ round-trip. Full route
  behavior is not covered by those checks.

## SEC-LIMIT-FILE-EXACT — File exactly at per-file limit

- **Test objective:** Verify acceptance at the configured per-file size
  boundary.
- **Test input:** A synthetic file exactly `MAX_FILE_SIZE` bytes (50 MiB by
  default); ensure the complete multipart request remains below
  `MAX_CONTENT_LENGTH`.
- **Test steps:**
  1. Record effective `MAX_FILE_SIZE` and `MAX_CONTENT_LENGTH`.
  2. Generate exactly the configured per-file number of synthetic bytes in
     the test temporary directory.
  3. POST the file to `/api/compress`.
- **Expected result:** The per-file size check accepts the file; the request
  succeeds if all other limits and executable prerequisites are satisfied.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** No exact per-file boundary case identified.

## SEC-LIMIT-FILE-OVER — File one byte above per-file limit

- **Test objective:** Verify rejection immediately above the configured
  per-file boundary.
- **Test input:** A synthetic file of `MAX_FILE_SIZE + 1` bytes.
- **Test steps:**
  1. Record effective `MAX_FILE_SIZE` and configure the request so its total
     size remains below `MAX_CONTENT_LENGTH`.
  2. POST the file to `/api/compress`.
  3. Verify whether the executable was invoked and whether a download was
     returned.
- **Expected result:** The request is rejected with a client error indicating
  the file-size limit; the executable is not invoked and no archive is
  returned.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** No per-file over-limit case identified.

## SEC-LIMIT-COUNT — Maximum file count and one additional file

- **Test objective:** Verify the configured maximum file count and rejection
  of one additional file.
- **Test input:** `MAX_FILES` distinct small synthetic files, followed by
  `MAX_FILES + 1` distinct small synthetic files (100 files by default).
- **Test steps:**
  1. Record effective `MAX_FILES`, `MAX_FILE_SIZE`, and
     `MAX_CONTENT_LENGTH`.
  2. POST exactly `MAX_FILES` files to `/api/compress`.
  3. POST `MAX_FILES + 1` files in a separate request, keeping all other
     limits satisfied.
- **Expected result:** The maximum count is accepted if other conditions are
  valid; one additional file is rejected before executable invocation.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** No maximum-count boundary case identified.

## SEC-LIMIT-REQUEST — Request-size boundary

- **Test objective:** Verify the total request limit, including multipart
  framing.
- **Test input:** Multipart request bodies whose complete encoded size is
  exactly `MAX_CONTENT_LENGTH` and one byte above it (100 MiB by default).
- **Test steps:**
  1. Record effective `MAX_CONTENT_LENGTH`.
  2. Construct requests with controlled complete body lengths; account for
     multipart boundaries and headers.
  3. Submit each request to `/api/compress`.
- **Expected result:** An at-limit request is not rejected solely by the
  request-size cap. An over-limit request receives HTTP 413 and the documented
  JSON error response.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** `backend/test_app.py` tests an oversized request.
  Exact-boundary behavior is not covered.

## SEC-ZIP-CORRUPT — Corrupted archive through API

- **Test objective:** Verify that corrupted archives are rejected through the
  Flask list and extract routes.
- **Test input:** A synthetic valid ZIP with altered entry data or CRC.
- **Test steps:**
  1. Generate a valid test ZIP in a disposable directory.
  2. Create a separate corrupted copy by changing test data or CRC metadata.
  3. POST the corrupted copy to `/api/list` and `/api/extract`.
  4. Inspect responses and the isolated filesystem.
- **Expected result:** The routes return an invalid-archive error; no
  success-shaped response is returned and no unsafe output is written.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** C++ CRC corruption is tested in
  `tests/smoke_test.cpp`; Flask route behavior remains to be tested.

## SEC-ZIP-TRUNC — Truncated archive through API

- **Test objective:** Verify truncated archive rejection through the Flask
  list and extract routes.
- **Test input:** A synthetic valid ZIP truncated in its end record or entry
  data.
- **Test steps:**
  1. Generate a valid test ZIP in a disposable directory.
  2. Create a separate truncated copy.
  3. POST the truncated copy to `/api/list` and `/api/extract`.
  4. Inspect responses and confirm no writes escape the request workspace.
- **Expected result:** The archive is rejected without a success-shaped
  response or writes outside the isolated request workspace.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** C++ truncated ZIP rejection is tested in
  `tests/smoke_test.cpp`; Flask route behavior remains to be tested.

## SEC-ZIP-SLIP — Archive path traversal

- **Test objective:** Ensure traversal and absolute-path archive entries
  cannot write outside the extraction root.
- **Test input:** Synthetic ZIP entries using `../`, backslash traversal, and
  absolute or drive-style paths.
- **Test steps:**
  1. Generate each archive using only synthetic entries.
  2. Extract into a fresh temporary directory.
  3. Check that synthetic sentinel paths outside that directory remain absent
     or unchanged.
- **Expected result:** Unsafe entries are rejected and no outside write occurs.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** Traversal and absolute-path rejection are present in
  `tests/smoke_test.cpp`. Do not duplicate those C++ tests; add API-level
  coverage only if it is not covered by the real-executable integration
  case.

## SEC-COMP-SYMLINK-FILE — External file symlink during compression

- **Test objective:** Ensure compression does not read an outside file through
  a symlink located in the selected input tree.
- **Test input:** A synthetic input directory, an outside sentinel file with
  unique synthetic contents, and a symlink inside the input directory
  pointing to that file.
- **Test steps:**
  1. Create all inputs under a disposable temporary root.
  2. Create the symlink if supported by the test platform.
  3. Compress the input directory.
  4. Inspect archive entry names and contents.
- **Expected result:** The symlink is rejected or not followed according to
  the approved behavior; outside file contents are not included. If symlink
  creation is unavailable, record Blocked rather than Passed.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** Existing symlink tests cover extraction, not
  symlinks encountered during compression.

## SEC-COMP-SYMLINK-DIR — External directory symlink during compression

- **Test objective:** Ensure compression does not recurse into an external
  directory through a symlink in the selected input tree.
- **Test input:** A synthetic input directory, an outside temporary directory
  containing uniquely identifiable files, and a directory symlink inside the
  input directory.
- **Test steps:**
  1. Create all directories and files under a disposable temporary root.
  2. Create the directory symlink if supported by the test platform.
  3. Compress the input directory.
  4. Inspect all archive entries and contents.
- **Expected result:** The symlink is rejected or not followed according to
  the approved behavior; no outside entry or file content is archived. If
  symlink creation is unavailable, record Blocked rather than Passed.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** Existing symlink tests cover extraction, not
  symlinks encountered during compression.

## SEC-EXTRACT-RACE — Extraction path replacement race

- **Test objective:** Check that replacing a validated destination component
  with a symlink cannot redirect a write outside the extraction root.
- **Test input:** A synthetic archive, a private extraction root, and a
  synthetic outside sentinel in a separate temporary directory.
- **Test steps:**
  1. Use only a disposable temporary root and synthetic sentinel data.
  2. Use a controlled harness to attempt replacement between path validation
     and file creation.
  3. Inspect the extraction root and outside sentinel after the operation.
- **Expected result:** Extraction fails safely or remains within a protected
  staging area; the outside sentinel is unchanged. Do not run against
  uncontrolled or shared paths. A deterministic test hook may be needed for
  reliable automation.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** Existing extraction symlink tests cover pre-existing
  links, not replacement between validation and file open.

## SEC-TEMP-CLEANUP — Temporary-file cleanup after success and failure

- **Test objective:** Verify request-specific staging data is removed after
  successful and failed operations.
- **Test input:** Synthetic valid uploads and a synthetic invalid archive;
  isolated test temporary root.
- **Test steps:**
  1. Send successful compress, extract, and list requests.
  2. Trigger validation failure, invalid archive, executable failure, and
     timeout paths using controlled test configuration.
  3. Inspect the test temporary root after each response.
- **Expected result:** Each request's temporary directory and uploaded or
  extracted data are removed after the response, including handled failures.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** No explicit temporary-directory cleanup test
  identified.

## SEC-REAL-API — Flask integration with real C++ executable

- **Test objective:** Exercise the actual Flask-to-executable boundary rather
  than a placeholder executable.
- **Test input:** Built project executable; synthetic empty, text, and binary
  inputs; generated ZIPs for list and extract operations.
- **Test steps:**
  1. Build the project executable in the normal build directory.
  2. Configure `COMPRESSION_BINARY` to that executable.
  3. Call all three Flask API routes using a test client.
  4. Compare returned and extracted names and bytes with the synthetic
     inputs.
  5. Confirm all intermediate files remain in disposable temporary storage.
- **Expected result:** The real executable is invoked, response formats match
  the current route contract, and round-trip data is identical.
- **Actual result:**
- **Status:** Not Run
- **Existing coverage:** The current Flask test fixture uses a placeholder
  executable. No real-executable route integration test was identified.
