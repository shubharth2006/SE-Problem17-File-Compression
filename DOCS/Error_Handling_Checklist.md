# Error Handling Test Checklist

Related Jira task: FCTG1-5 — Corrupted Archive and Error Handling

The statuses below reflect executed automated checks. “Partially Passed” means the tests cover only part of the stated action or expected result.

| Case | Action | Expected result | Status |
|---|---|---|---|
| Missing archive | List or extract a nonexistent ZIP | Clear error; no success message | Partially Passed |
| Invalid archive | List or extract a plain text file renamed to .zip | Archive rejected with an error | Passed |
| Truncated archive | Extract a ZIP with its ending removed | Archive rejected; no success message | Partially Passed |
| Corrupted data | Extract a ZIP whose file data was deliberately damaged | Corruption reported; no successful completion | Passed |
| Existing archive | Compress to an existing ZIP without --yes | Existing archive remains unchanged | Passed |
| Existing output | Extract over an existing file without --yes | Existing file remains unchanged | Passed |

## Execution Evidence

The local CTest `build-coverage/Testing/Temporary/LastTest.log` records `smoke_test` as passed. The [GitHub Actions CI run for commit `dcb6923`](https://github.com/shubharth2006/SE-Problem17-File-Compression/actions/runs/43684124318) also completed successfully, including its C++ CTest and Flask API test steps. That CI result applies to the linked commit; the code blocks below identify what each test actually asserts.

- **Missing archive — Partially Passed:** `tests/smoke_test.cpp` calls `ziptool::list_archive` on a nonexistent ZIP and observes a `std::runtime_error`, so listing does not succeed. It does not inspect the error text for clarity or test extraction of the missing path.
- **Invalid archive — Passed:** `tests/smoke_test.cpp` writes plain text to `invalid.zip` and observes errors from both `list_archive` and `extract_archive`. `backend/test_integration.py::test_real_api_rejects_non_zip_content_for_list_and_extract` also checks HTTP 400 with `{"error":"Invalid ZIP file"}` for both routes.
- **Truncated archive — Partially Passed:** `tests/smoke_test.cpp` removes the final 10 bytes of a valid ZIP and observes rejection by `ziptool::list_archive`. The checklist calls for **extracting** that truncated ZIP; no inspected automated test performs that exact action.
- **Corrupted data — Passed:** `tests/smoke_test.cpp` changes a byte of archived file data, calls `ziptool::extract_archive`, and observes an exception containing `CRC check failed`; extraction does not complete successfully.
- **Existing archive — Passed:** `tests/smoke_test.cpp` calls `ziptool::create_archive` on an existing ZIP with overwrite disabled (`{false}`) and confirms the call is rejected and the archive bytes remain unchanged. This checks the core behavior represented by omitting `--yes`, though it does not exercise CLI argument parsing.
- **Existing output — Passed:** `tests/smoke_test.cpp` calls `ziptool::extract_archive` over an existing file with overwrite disabled (`{false}`) and confirms rejection and unchanged user content. This checks the core behavior represented by omitting `--yes`, though it does not exercise CLI argument parsing.

## Evidence to record

- Application version or commit SHA
- Command executed
- Test input preparation
- Actual output and exit code
- Whether existing files remained unchanged

A checklist entry is not evidence of a passed test until executed.
