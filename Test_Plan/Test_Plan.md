# Software Test Plan (STP) — Problem 17

**Project:** File Compression Tool (Basic ZIP Implementation)
**Group:** 15
**Language:** C / C++
**Algorithm:** Huffman Coding
**Version:** 1.0

## 1. Introduction
### Purpose
Defines the testing objectives, scope, strategy, environment, schedule, responsibilities and acceptance criteria for the File Compression Tool.

### Scope
Testing covers file selection, file reading, frequency calculation, Huffman tree construction, Huffman code generation, compression, compressed-file creation, decompression, decoding, restoration, statistics and error handling.

### References
Problem Statement 17, SRS v1.0, RTM and Architecture & Design Specification.

## 2. Test Items
- File Input Module
- Frequency Calculation Module
- Huffman Tree / Coding Module
- Compression / Encoding Module
- Compressed File Header Module
- Decompression / Decoding Module
- Statistics / User Interface
- Error Handling Module

## 3. Features to be Tested
| ID | Feature |
|---|---|
| FCT-F-001 | Select input file |
| FCT-F-002 | Read input file contents |
| FCT-F-003 | Calculate character/byte frequencies |
| FCT-F-004 | Create Huffman tree |
| FCT-F-005 | Generate Huffman codes |
| FCT-F-006 | Encode input data |
| FCT-F-007 | Store Huffman information |
| FCT-F-008 | Create compressed output |
| FCT-F-009 | Display file sizes |
| FCT-F-010 | Calculate compression ratio |
| FCT-F-011 | Select compressed file |
| FCT-F-012 | Reconstruct Huffman tree |
| FCT-F-013 | Decode compressed data |
| FCT-F-014 | Restore original file |
| FCT-F-015 | Handle invalid/corrupted files |

## 4. Features Not to be Tested
Advanced ZIP features not specified in Problem 17, operating-system internals, third-party compression utilities and hardware/storage reliability outside application file I/O.

## 5. Test Approach / Strategy
**Levels:** Unit, integration, system, regression and acceptance testing.

**Types:** Functional, negative/boundary, data-integrity, performance and usability/error-message testing.

**Entry criteria:** Stable build compiles and launches, test data is available, and the test environment is ready.

**Exit criteria:** All planned test cases executed, critical defects closed, high-priority acceptance criteria satisfied, and no unexplained data-integrity failures.

### 5.1 Security Validation
- Validate input and compressed-file metadata before processing.
- Reject malformed/corrupted compressed files safely.
- Verify decompression restores original data exactly.
- Verify failed I/O does not leave misleading output.
- Prevent unintended overwriting of the source file.

## 6. Test Environment
- Hardware: Standard student laptop/desktop with sufficient storage.
- Software: C/C++ compiler and project executable.
- Tools: VS Code/IDE, terminal, Git/GitHub, optional file-diff/hash utility.
- Test data: Text, CSV, source-code, binary, empty, repeated-character, mixed-content, large and corrupted files.

## 7. Test Schedule
| Milestone | Target |
|---|---|
| Test case design | 06-Oct-2026 |
| Environment/test-data setup | 06-Oct-2026 |
| Test execution | 06–07-Oct-2026 |
| Defect fixing/regression | 07-Oct-2026 |
| Final readiness | 07-Oct-2026 |

## 8. Test Deliverables
Test Plan, Test Cases, Test Data, Execution Logs, Defect Reports, Test Summary Report and updated RTM.

## 9. Roles and Responsibilities
| Role | Responsibility |
|---|---|
| QA/Test Lead | Plan and coordinate testing |
| Test Engineer | Design, execute and record cases |
| Developer | Fix defects and support debugging |
| Team Lead/Product Owner | Review acceptance results |

## 10. Risks and Mitigation
| Risk | Mitigation |
|---|---|
| Unstable build | Smoke-test each build |
| Wrong Huffman implementation | Unit-test small hand-verifiable inputs |
| Data loss during decoding | Byte-for-byte/hash comparison |
| Large-file resource usage | Test progressively larger files |
| Source overwritten | Use separate output paths |

## 11. Assumptions & Dependencies
Compiler/runtime, file-system access, documented compressed-file header format, and latest project source are available.

## 12. Suspension & Resumption
Suspend when the build cannot compile/launch, blocking defects prevent more than 30% of planned cases, or output is consistently corrupted. Resume after a stable build and resolution/approval of blocking issues.

## 13. Traceability
| Requirement | Test Case |
|---|---|
| FCT-F-001 | TC-FILE-01 |
| FCT-F-003 | TC-HUFF-01 |
| FCT-F-004 | TC-HUFF-02 |
| FCT-F-005 | TC-HUFF-03 |
| FCT-F-006 | TC-ENC-01 |
| FCT-F-008 | TC-COMP-01 |
| FCT-F-011 | TC-DEC-01 |
| FCT-F-012 | TC-DEC-02 |
| FCT-F-013 | TC-DEC-03 |
| FCT-F-014 | TC-DEC-04 |
| FCT-F-015 | TC-ERR-01 |
| FCT-NF-003 | TC-NF-03 |

## 14. Test Metrics
- Percentage of test cases executed
- Pass/fail percentage
- Open defects by severity
- Defect aging
- Requirement coverage
- Data-integrity failures

## 15. Approvals
QA/Test Lead, Developer/Dev Lead and Team Lead/Product Owner.