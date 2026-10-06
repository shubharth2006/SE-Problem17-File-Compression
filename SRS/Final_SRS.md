# Software Requirements Specification (SRS)

Project: File Compression Tool (Basic ZIP Implementation)
Problem: 17
Group: 15
Version: 1.0
Date: 06-10-2026
Status: Draft

Revision history

Approvals

Table of Contents

## 1. Introduction
2. Overall description
3. External interfaces
4. System features (detailed)
5. Non-functional requirements (detailed)
6. Quality attributes & Acceptance tests
7. UML Use-Case Diagrams
8. Requirements Traceability Matrix (RTM)

## 1. Introduction

### Purpose

This document is a Software Requirements Specification (SRS) for the File Compression Tool (Basic ZIP Implementation). It defines the functional and non-functional requirements, system interfaces, security requirements, and acceptance criteria of the system. This document serves as a reference for the development, testing, and evaluation of the File Compression Tool.

### Scope

The File Compression Tool is a software application that allows users to compress files and folders into ZIP archives and extract files from existing ZIP archives.

Select one or more files or folders for compression.

Create a ZIP archive from the selected files or folders.

Specify the name and destination of the ZIP archive.

Extract files from an existing ZIP archive.

Preserve file names and directory structure where applicable.

Display the progress and status of compression and extraction  operations.

Display appropriate success and error messages.

Handle invalid files, corrupted ZIP archives, and file access errors.

The system is intended for basic local file compression and extraction. Internet-based storage, cloud services, and advanced archive management features are outside the scope of this project.

### Audience

This document is intended for:

Developers – to understand and implement the required system functionality.

Testers – to verify the functional and non-functional requirements.

Users – to understand the basic operations provided by the tool.

Project Evaluators – to evaluate the system against the specified requirements.

### Definitions and Abbreviations

ZIP: A file archive format used to store one or more files in a compressed form.

Compression: The process of reducing file size and storing files in an archive.

Extraction: The process of retrieving files from a ZIP archive. Archive: A file containing one or more files or folders stored together.

GUI: Graphical User Interface used to interact with the application.

Input File: A file or folder selected by the user for compression.

Output File: The ZIP archive created by the compression process.

## 2. Overall description

2.1 Product perspective

The File Compression Tool is a standalone local application that provides

basic file compression and extraction functionality. It interacts with the user's

local file system to read input files, create ZIP archives, and extract files from

existing ZIP archives.

The system consists of the following main components:

User Interface Module – allows users to select files, start operations, and view results.

File System Module – handles reading and writing of files and folders.

Compression Module – creates ZIP archives from selected files and folders.

Extraction Module – extracts files from existing ZIP archives.

Error Handling Module – handles invalid inputs, corrupted archives, and file access errors.

The system operates locally and does not require an Internet connection for its

core compression and extraction functionality.

2.2 Major product functions (detailed)

The major functions of the File Compression Tool are:

Select one or more files for compression.

Select a folder for compression.

Display selected files and their basic details.

Create a ZIP archive from selected files or folders.

Specify the ZIP archive name and destination.

Compress multiple files into a single ZIP archive.

Preserve the relative directory structure when a folder is compressed.

Select an existing ZIP archive for extraction.

Extract files from a ZIP archive to a selected destination.

Preserve the directory structure during extraction.

Display the progress or status of compression and extraction operations.

Display success messages after completed operations.

Display appropriate error messages for failed operations.

Detect invalid or corrupted ZIP archives.

Ask for confirmation before overwriting existing files or ZIP archives.

2.3 User roles and characteristics

User:
The primary user is a general computer user who wants to compress or extract files. The user is expected to have basic knowledge of selecting files and folders, choosing file locations, and working with ZIP archives.

Developer/Maintainer:
The developer or maintainer is responsible for implementing, testing, and maintaining the compression, extraction, file handling, and user interface modules.

2.4 Operating environment

The File Compression Tool shall operate in a standard desktop computing

environment.

The system requires:

A supported desktop operating system.

Access to the local file system.

Sufficient storage space for input files and generated ZIP archives.

The required software/runtime environment for executing the application.

The system shall operate locally and shall not require an Internet connection

for basic compression and extraction operations.

2.5 Constraints

The following constraints apply to the system:

The tool is intended for basic ZIP compression and extraction.

The system depends on operating system permissions for accessing files and folders.

Compression and extraction performance depends on available system resources such as CPU, memory, and storage.

The system shall not modify or delete original input files during normal compression operations.

The system shall safely handle invalid or corrupted ZIP files.

The system shall preserve file names and directory structure where applicable.

The system shall operate within the supported file-size and system-resource limits.

## 3. External interface requirements

3.1 User interfaces

The File Compression Tool shall provide a simple and user-friendly graphical

user interface (GUI). The interface shall allow users to select files or folders for

compression and select existing ZIP files for extraction.

File and folder selection option.

Option to select multiple files for compression.

Option to specify the ZIP file name and destination location.

Compress option to create a ZIP archive.

Extract option to extract files from a ZIP archive.

Display of selected files and their basic details.

Progress or status information during compression and extraction.

Success messages after completing an operation.

Appropriate error messages when an operation fails.

Confirmation before overwriting an existing file.

3.2 File System Interface

The system shall interact with the local file system for reading, writing,

compressing, and extracting files.

Read files selected by the user.

Read files contained within selected folders.

Create ZIP archives at the specified destination.

Read existing ZIP archives.

Extract ZIP contents to the selected destination folder.

Preserve file names and directory structure where applicable.

Check whether input files and output locations are accessible.

Display an appropriate error message when a file or destination cannot be

accessed.

3.3 Software interfaces

The File Compression Tool shall interact with the following software

components:

Operating System: Provides access to files, folders, and storage.

File System Module: Handles reading and writing of files.

Compression Module: Handles creation of ZIP archives.

Extraction Module: Handles extraction of files from ZIP archives.

User Interface Module: Handles user input and displays operation status,

results, and error messages.

The software interfaces shall allow the different modules to communicate and perform compression and extraction operations correctly.

3.4 Communication Interfaces

The basic File Compression Tool shall operate locally and shall not require an Internet connection for compression or extraction. No external network communication is required for the core functionality of the system.

## 4. System features (detailed Functional requirements)

4.1 File Selection

Description: The system shall allow users to select files and folders that need to be compressed.

4.2  ZIP File Compression

Description: The system shall compress selected files and folders into a ZIP archive.

4.3 ZIP File Extraction

Description: The system shall allow users to extract files from an existing ZIP archive.

4.4  Progress and Operation Status

Description: The system shall identify common errors during compression and extraction and provide appropriate feedback to the user.

4.5  Progress and Operation Status

Description: The system shall provide information about the status of compression and extraction operations

## 5. Non-functional requirements (detailed)

NFRs below are measurable and tied to test plans. IDs ZIP-NF-###

5.1. Security

5.1.1 Security Objectives

The security objectives of the File Compression Tool are:

Protect extracted files and the user's file system by preventing unsafe archive entries from accessing locations outside the selected extraction directory.

Maintain data integrity by ensuring that files are not modified or corrupted during compression and decompression.

Validate input files and archives so that invalid or malformed ZIP files are handled safely without causing unexpected application failure.

Protect original user files by ensuring that the compression process does not unintentionally modify or delete the source files.

5.1.2 Security Requirements

## 6. Quality attributes & Acceptance tests

Exit criteria for acceptance

The File Compression Tool shall be considered acceptable when:

All high-priority functional and non-functional requirements have been implemented and verified.

All critical compression and extraction test cases pass.

Extracted files have the same contents as the original files.

Invalid and corrupted ZIP files are handled without application crashes.

No critical security issues are identified during testing.

The Requirements Traceability Matrix (RTM) shows that all requirements are mapped to corresponding test cases and verified successfully.

### Acceptance test suites

The following acceptance test suites shall be performed:

Compression Tests – Verify creation of valid ZIP archives.

Decompression Tests – Verify successful extraction of ZIP archives.

Data Integrity Tests – Compare original and extracted files.

Performance Tests – Measure compression and decompression time.

Error Handling Tests – Test invalid paths, missing files, and corrupted archives.

Security Tests – Test unsafe archive paths and malformed ZIP files.

Usability Tests – Verify that users can perform compression and extraction through the interface.

Portability Tests – Verify operation in the supported environment.

## 7. System models and diagrams

7.1 UML Use-Case Diagrams

The following use-case diagrams describe the two primary operations of the system: compression and extraction.

Figure 7.1: UML Use-Case Diagram – File Compression

Figure 7.2: UML Use-Case Diagram – File Extraction

## 8. Requirements Traceability Matrix (RTM)


## Requirement Tables

### Table 1

| Req ID | Requirement | Type | Priority | Acceptance Criteria / Test Case | Dependencies |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-001 | The system shall allow the user to select one or more files for compression. | Functional | High | Selected files are displayed correctly before compression. Test: TC-File-01 | File System |
| ZIP-F-002 | The system shall allow the user to select a folder for compression. | Functional | High | The selected folder and its contents are accepted for compression. Test: TC-File-02 | File System |
| ZIP-F-003 | The system shall display the names and basic details of selected files before compression. | Functional | Medium | Selected file names and sizes are displayed correctly. Test: TC-File-03 | User Interface |
| ZIP-F-004 | The system shall reject invalid or inaccessible input files and display an appropriate error message. | Functional | High | Invalid or inaccessible files are rejected with an appropriate message. Test: TC-File-04 | File System |

### Table 2

| Req ID | Requirement | Type | Priority | Acceptance Criteria / Test Case | Dependencies |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-005 | The system shall create a ZIP archive from the files selected by the user. | Functional | High | A valid ZIP archive containing the selected files is created. Test: TC-Comp-01 | Compression Module |
| ZIP-F-006 | The system shall allow the user to specify the name and destination location of the ZIP archive. | Functional | High | The ZIP file is created with the specified name and location. Test: TC-Comp-02 | File System |
| ZIP-F-007 | The system shall compress multiple selected files into a single ZIP archive. | Functional | High | All selected files are present in the generated ZIP archive. Test: TC-Comp-03 | Compression Module |
| ZIP-F-008 | The system shall preserve the relative directory structure when a folder is compressed. | Functional | High | The original directory structure is reproduced after extraction. Test: TC-Comp-04 | File System |
| ZIP-F-009 | The system shall notify the user when compression is successfully completed. | Functional | Medium | A success message is displayed after successful archive creation. Test: TC-Comp-05 | User Interface |

### Table 3

| Req ID | Requirement | Type | Priority | Acceptance Criteria / Test Case | Dependencies |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-010 | The system shall allow the user to select an existing ZIP archive for extraction. | Functional | High | A valid ZIP archive can be selected successfully. Test: TC-Ext-01 | File System |
| ZIP-F-011 | The system shall extract all files contained in a valid ZIP archive to a user-selected destination. | Functional | High | All archived files are extracted correctly. Test: TC-Ext-02 | Extraction Module |
| ZIP-F-012 | The system shall preserve the directory structure stored in the ZIP archive during extraction. | Functional | High | The directory structure stored in the archive is reproduced correctly. Test: TC-Ext-03 | Extraction Module |
| ZIP-F-013 | The system shall notify the user when extraction is successfully completed. | Functional | Medium | A success message is displayed after successful extraction. Test: TC-Ext-04 | User Interface |

### Table 4

| Req ID | Requirement | Type | Priority | Acceptance Criteria / Test Case | Dependencies |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-014 | The system shall detect an invalid or corrupted ZIP archive and display an appropriate error message. | Functional | High | The invalid or corrupted archive is rejected without crashing the application. Test: TC-Err-01 | Extraction Module |
| ZIP-F-015 | The system shall ask for confirmation before overwriting an existing file or ZIP archive. | Functional | High | The user receives an overwrite confirmation before an existing file is replaced. Test: TC-Err-02 | File System |

### Table 5

| Req ID | Requirement | Type | Priority | Acceptance Criteria / Test Case | Dependencies |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-016 | The system shall display the progress or status of an ongoing compression operation. | Functional | Medium | Operation status is displayed while compression is in progress. Test: TC-UI-01 | User Interface |
| ZIP-F-017 | The system shall display the progress or status of an ongoing extraction operation. | Functional | Medium | Operation status is displayed while extraction is in progress. Test: TC-UI-02 | User Interface |
| ZIP-F-018 | The system shall prevent conflicting compression and extraction operations from running simultaneously. | Functional | Medium | Conflicting operations are prevented or handled appropriately. Test: TC-UI-03 | Compression/Extra ction Module |

### Table 6

| Req ID | Requirement | Category | Priority | Acceptance Criteria / Measurement |
| --- | --- | --- | --- | --- |
| ZIP-NF-001 | The system shall compress a set of input files into a valid ZIP archive without corrupting the original files. | Performance / Reliability | High | Compression completes successfully and the generated archive can be opened and extracted by the tool. Test: TC-Perf-01 |
| ZIP-NF-002 | The system shall complete compression and decompression operations within a reasonable time for files up to 100 MB under normal system load. | Performance | High | Compression/decompression of a 100 MB test dataset shall complete without timeout or failure. Test: TC-Perf-02 |
| ZIP-NF-003 | The system shall preserve the contents of files after compression & decompression. | Data Integrity | High | Original and extracted files shall have identical contents using file comparison/hash verification. Test: TC-INT-01 |
| ZIP-NF-004 | The system shall preserve file names and directory structure when compressing and extracting files. | Reliability | High | Extracted files shall retain the original names and relative directory structure. Test: TC-INT-02 |
| ZIP-NF-005 | The system shall provide clear error messages when an invalid, corrupted, inaccessible, or unsupported input is provided. | Usability / Reliability | Medium | Appropriate error message shall be displayed without abnormal program termination. Test: TC-ERR-01 |
| ZIP-NF-006 | The system shall not modify or delete the original input files during compression. | Data Integrity | High | Original files shall remain unchanged after successful or failed compression. Test: TC-INT-03 |
| ZIP-NF-007 | The system shall handle empty files and files containing different types of data without data loss. | Reliability | Medium | Empty files and text/binary files shall be compressed and extracted successfully with identical contents. Test: TC-REL-01 |
| ZIP-NF-008 | The system shall provide a simple and understandable interface for selecting files, specifying the archive location, compressing, and extracting archives. | Usability | Medium | A user shall be able to perform compression and extraction without requiring additional technical instructions. Test: TC-UX-01 |
| ZIP-NF-009 | The system shall prevent unsafe file extraction paths such as paths attempting to access directories outside the selected extraction location. | Security | High | Malicious archive entries containing path traversal sequences shall be rejected or safely contained. Test: TC-SEC-01 |
| ZIP-NF-010 | The system shall operate consistently on the supported operating system/environment without unexpected crashes during normal operations. | Portability / Reliability | Medium | All defined compression and extraction test cases shall execute successfully in the supported environment. Test: TC-PORT-01 |

### Table 7

| Req ID | Requirement (shall...) | Type | Priority | Acceptance Criteria / Test Case Ref |
| --- | --- | --- | --- | --- |
| ZIP-SR-001 | The system shall validate the input path and file information before starting a compression or extraction operation. | Security | High | Invalid or inaccessible paths are rejected with an appropriate error message. Test: TC-SEC-01 |
| ZIP-SR-002 | The system shall prevent ZIP entries from extracting files outside the user-selected extraction directory. | Security | High | Path traversal entries such as ../ shall not be extracted outside the target directory. Test: TC-SEC-02 |
| ZIP-SR-003 | The system shall detect and safely handle corrupted or malformed ZIP archives without crashing. | Security / Reliability | High | Corrupted archives shall be rejected and an error message shall be displayed. Test: TC-SEC-03 |
| ZIP-SR-004 | The system shall not modify or delete the original input files during compression. | Data Integrity | High | Original files remain unchanged after compression. Test: TC-SEC-04 |
| ZIP-SR-005 | The system shall prevent extraction from overwriting an existing file unless the user explicitly confirms the overwrite. | Security / Data Integrity | High | Existing files are not replaced without explicit confirmation. Test: TC-SEC-05 |

### Table 8

| Quality Attribute | Requirement / Goal | Acceptance Test |
| --- | --- | --- |
| Performance | Compression and decompression shall complete within a reasonable time for the defined test datasets. | TC-Perf-01, TC-Perf-02 |
| Reliability | The system shall successfully compress and extract valid files without unexpected crashes. | TC-Rel-01 |
| Data Integrity | Extracted files shall have the same contents as the original files. | TC-INT-01, TC-INT-02 |
| Usability | Users shall be able to select files, create ZIP archives, and extract archives using a clear interface. | TC-UX-01 |
| Security | The system shall safely handle malformed archives and prevent unsafe extraction paths. | TC-SEC-01, TC-SEC-02, TC-SEC-03 |
| Portability | The application shall work correctly in the supported operating environment. | TC-PORT-01 |
| Maintainability | The implementation shall use modular components for compression, decompression, file handling, and error handling. | Code review / TC-MAINT-01 |

### Table 9

| Req ID | Requirement | Section / Design Spec | Module | Test Case(s) | Status |
| --- | --- | --- | --- | --- | --- |
| ZIP-F-001 | Select one or more files for compression | 4.1 / DS-File-01 | File System | TC-File-01 | N |
| ZIP-F-002 | Select a folder for compression | 4.1 / DS-File-02 | File System | TC-File-02 | N |
| ZIP-F-003 | Display selected file names and basic details | 4.1 / DS-UI-01 | User Interface | TC-File-03 | N |
| ZIP-F-004 | Reject invalid or inaccessible input files | 4.1 / DS-File-03 | File System | TC-File-04 | N |
| ZIP-F-005 | Create a ZIP archive from selected files | 4.2 / DS-Comp-01 | Compression Module | TC-Comp-01 | N |
| ZIP-F-006 | Specify ZIP archive name and destination | 4.2 / DS-Comp-02 | File System | TC-Comp-02 | N |
| ZIP-F-007 | Compress multiple files into a single ZIP archive | 4.2 / DS-Comp-03 | Compression Module | TC-Comp-03 | N |
| ZIP-F-008 | Preserve relative directory structure during compression | 4.2 / DS-Comp-04 | File System | TC-Comp-04 | N |
| ZIP-F-009 | Notify user after successful compression | 4.2 / DS-UI-02 | User Interface | TC-Comp-05 | N |
| ZIP-F-010 | Select an existing ZIP archive for extraction | 4.3 / DS-Ext-01 | File System | TC-Ext-01 | N |
| ZIP-F-011 | Extract all files from a valid ZIP archive | 4.3 / DS-Ext-02 | Extraction Module | TC-Ext-02 | N |
| ZIP-F-012 | Preserve directory structure during extraction | 4.3 / DS-Ext-03 | Extraction Module | TC-Ext-03 | N |
| ZIP-F-013 | Notify user after successful extraction | 4.3 / DS-UI-03 | User Interface | TC-Ext-04 | N |
| ZIP-F-014 | Detect invalid or corrupted ZIP archives | 4.4 / DS-Err-01 | Extraction Module | TC-Err-01 | N |
| ZIP-F-015 | Ask for confirmation before overwriting files | 4.4 / DS-Err-02 | File System | TC-Err-02 | N |
| ZIP-F-016 | Display progress or status during compression | 4.5 / DS-UI-04 | User Interface | TC-UI-01 | N |
| ZIP-F-017 | Display progress or status during extraction | 4.5 / DS-UI-05 | User Interface | TC-UI-02 | N |
| ZIP-F-018 | Prevent conflicting compression and extraction operations | 4.5 / DS-Op-01 | Compression/Extraction Module | TC-UI-03 | N |
| ZIP-NF-001 | Valid ZIP archive and original-file protection | 5 / DS-Perf-01 | Compression Module | TC-Perf-01 | N |
| ZIP-NF-002 | 100 MB performance target | 5 / DS-Perf-02 | Compression / Extraction | TC-Perf-02 | N |
| ZIP-NF-003 | Preserve file contents | 5 / DS-Integrity-01 | Compression / Extraction | TC-INT-01 | N |
| ZIP-NF-004 | Preserve names and directory structure | 5 / DS-Integrity-02 | File System / Extraction | TC-INT-02 | N |
| ZIP-NF-005 | Clear error messages | 5 / DS-Error-01 | Error Handling | TC-ERR-01 | N |
| ZIP-NF-006 | Do not modify/delete source | 5 / DS-Integrity-03 | File System | TC-INT-03 | N |
| ZIP-NF-007 | Handle empty/text/binary files | 5 / DS-Reliability-01 | Compression / Extraction | TC-REL-01 | N |
| ZIP-NF-008 | Simple understandable interface | 5 / DS-UI-01 | User Interface | TC-UX-01 | N |
| ZIP-NF-009 | Prevent unsafe extraction paths | 5 / DS-Sec-01 | Extraction Module | TC-SEC-01 | N |
| ZIP-NF-010 | Supported environment stability | 5 / DS-Port-01 | Application | TC-PORT-01 | N |
| ZIP-SR-001 | Validate input path and file information | 5.1.2 / DS-Sec-01 | File System | TC-SEC-01 | N |
| ZIP-SR-002 | Prevent extraction outside target directory | 5.1.2 / DS-Sec-02 | Extraction Module | TC-SEC-02 | N |
| ZIP-SR-003 | Safely reject corrupted archives | 5.1.2 / DS-Sec-03 | Extraction Module | TC-SEC-03 | N |
| ZIP-SR-004 | Protect original input files | 5.1.2 / DS-Sec-04 | File System | TC-SEC-04 | N |
| ZIP-SR-005 | Require confirmation before overwrite during extraction | 5.1.2 / DS-Sec-05 | File System | TC-SEC-05 | N |