# Error Handling Test Checklist

Related Jira task: FCTG1-5 — Corrupted Archive and Error Handling

Run each case in a temporary directory using disposable files.
Record the actual output and pass/fail result after execution.

| Case | Action | Expected result | Status |
|---|---|---|---|
| Missing archive | List or extract a nonexistent ZIP | Clear error; no success message | Not run |
| Invalid archive | List or extract a plain text file renamed to .zip | Archive rejected with an error | Not run |
| Truncated archive | Extract a ZIP with its ending removed | Archive rejected; no success message | Not run |
| Corrupted data | Extract a ZIP whose file data was deliberately damaged | Corruption reported; no successful completion | Not run |
| Existing archive | Compress to an existing ZIP without --yes | Existing archive remains unchanged | Not run |
| Existing output | Extract over an existing file without --yes | Existing file remains unchanged | Not run |

## Evidence to record

- Application version or commit SHA
- Command executed
- Test input preparation
- Actual output and exit code
- Whether existing files remained unchanged

A checklist entry is not evidence of a passed test until executed.