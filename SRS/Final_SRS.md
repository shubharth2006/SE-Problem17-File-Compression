# Software Requirements Specification (SRS)

Project: File Compression Tool (Basic ZIP Implementation)  
Problem: 17  
Group: 15  
Version: 1.0  
Date: 06-10-2026  
Status: Draft

## 1. Introduction
### Purpose
This document is a Software Requirements Specification (SRS) for the File Compression Tool (Basic ZIP Implementation). It defines the functional and non-functional requirements, system interfaces, security requirements, and acceptance criteria of the system. This document serves as a reference for the development, testing, and evaluation of the File Compression Tool.

### Scope
The File Compression Tool is a software application that allows users to compress files and folders into ZIP archives and extract files from existing ZIP archives.

- Select one or more files or folders for compression.
- Create a ZIP archive from the selected files or folders.
- Specify the name and destination of the ZIP archive.
- Extract files from an existing ZIP archive.
- Preserve file names and directory structure where applicable.
- Display the progress and status of compression and extraction operations.
- Display appropriate success and error messages.
- Handle invalid files, corrupted ZIP archives, and file access errors.

The system is intended for basic local file compression and extraction. Internet-based storage, cloud services, and advanced archive management features are outside the scope of this project.

### Audience
- Developers
- Testers
- Users
- Project Evaluators

### Definitions
- ZIP: A file archive format used to store one or more files in compressed form.
- Compression: The process of reducing file size and storing files in an archive.
- Extraction: Retrieving files from a ZIP archive.
- GUI: Graphical User Interface.
- Input File: A file or folder selected by the user for compression.
- Output File: The ZIP archive created by the compression process.

## 2. Overall Description
### 2.1 Product Perspective
The File Compression Tool is a standalone local application that provides basic file compression and extraction functionality. It interacts with the user's local file system to read input files, create ZIP archives, and extract files from existing ZIP archives.

Main components:
- User Interface Module
- File System Module
- Compression Module
- Extraction Module
- Error Handling Module

### 2.2 Major Product Functions
- Select one or more files for compression.
- Select a folder for compression.
- Display selected files and their basic details.
- Create a ZIP archive from selected files or folders.
- Specify the ZIP archive name and destination.
- Compress multiple files into a single ZIP archive.
- Preserve relative directory structure.
- Select an existing ZIP archive for extraction.
- Extract files to a selected destination.
- Preserve directory structure during extraction.
- Display operation progress/status.
- Display success messages.
- Display error messages.
- Detect invalid/corrupted ZIP archives.
- Ask for confirmation before overwriting existing files or archives.

### 2.3 User Roles and Characteristics
**User:** General computer user with basic knowledge of selecting files/folders and working with ZIP archives.

**Developer/Maintainer:** Responsible for implementation, testing and maintenance of compression, extraction, file handling and UI modules.

### 2.4 Operating Environment
The application operates in a standard desktop environment with access to the local file system, sufficient storage, and the required runtime/compiler environment. Internet access is not required for core operations.

### 2.5 Constraints
The tool is intended for basic ZIP compression and extraction. Performance depends on CPU, memory and storage. Original source files must not be modified or deleted during normal compression. Invalid/corrupted archives must be handled safely.

## 3. External Interface Requirements
### 3.1 User Interfaces
The GUI shall support file/folder selection, multiple-file selection, archive naming/destination, compression, extraction, file details, progress/status, success/error messages, and overwrite confirmation.

### 3.2 File System Interface
The system shall read/write selected files and folders, create ZIP archives, read existing ZIP archives, extract contents, preserve names/directories, check access permissions, and report access errors.

### 3.3 Software Interfaces
The application interacts with the operating system, file system module, compression module, extraction module and user interface module.

### 3.4 Communication Interfaces
The core application operates locally and requires no external network communication.

## 4. System Features
### 4.1 File Selection
The system shall allow users to select files and folders for compression.

### 4.2 ZIP File Compression
The system shall compress selected files/folders into a ZIP archive.

### 4.3 ZIP File Extraction
The system shall allow users to extract files from an existing ZIP archive.

### 4.4 Error Handling
The system shall identify common errors during compression and extraction and provide appropriate feedback.

### 4.5 Progress and Operation Status
The system shall display information about the status of compression and extraction operations.

## 5. Non-Functional Requirements
- ZIP-NF-001: Create a valid ZIP archive without corrupting original files.
- ZIP-NF-002: Complete compression/decompression within a reasonable time for files up to 100 MB under normal load.
- ZIP-NF-003: Preserve file contents after compression and decompression.
- ZIP-NF-004: Preserve file names and directory structure.
- ZIP-NF-005: Provide clear error messages without abnormal termination.
- ZIP-NF-006: Do not modify/delete original input files during compression.
- ZIP-NF-007: Handle empty, text and binary files without data loss.
- ZIP-NF-008: Provide a simple and understandable interface.
- ZIP-NF-009: Prevent unsafe extraction paths.
- ZIP-NF-010: Operate consistently in the supported environment without unexpected crashes.

## 5.1 Security
### 5.1.1 Security Objectives
1. Protect extracted files and the user's file system from unsafe archive entries.
2. Maintain file integrity during compression/decompression.
3. Validate files and archives to prevent malformed input from causing failures.
4. Protect original user files from unintended modification/deletion.

### 5.1.2 Security Requirements
- ZIP-SR-001: Validate input paths and file information before operations.
- ZIP-SR-002: Prevent archive entries from extracting outside the selected directory.
- ZIP-SR-003: Detect and safely handle corrupted/malformed archives.
- ZIP-SR-004: Do not modify or delete original input files during compression.
- ZIP-SR-005: Prevent extraction from overwriting existing files unless the user explicitly confirms.

## 6. Quality Attributes & Acceptance Tests
Quality attributes include performance, reliability, data integrity, usability, security, portability and maintainability. Acceptance requires high-priority requirements to be implemented and verified, critical tests to pass, extracted files to match originals, malformed archives to be handled without crashes, and the RTM to show requirement-to-test coverage.

## 7. System Models and Diagrams
### 7.1 UML Use-Case Diagrams
The two diagrams are stored as editable PlantUML source files:
- [Compression Use-Case](../diagrams/UML_Compression.puml)
- [Extraction Use-Case](../diagrams/UML_Extraction.puml)

The formatted SRS Word document contains the corresponding user-supplied diagrams as Figure 7.1 and Figure 7.2.

## 8. Requirements Traceability Matrix (RTM)
The RTM maps the 18 functional requirements, 10 non-functional requirements and 5 security requirements to their design references, modules and test cases. Status is currently **N (Not Implemented / Not Verified)** until the implementation is executed and tested.

### RTM Summary
| Category | Count |
|---|---:|
| Functional Requirements | 18 |
| Non-Functional Requirements | 10 |
| Security Requirements | 5 |
| Total Traced Requirements | 33 |

The complete row-level RTM is maintained in the formatted SRS report.
