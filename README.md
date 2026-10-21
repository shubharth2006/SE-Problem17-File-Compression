# File Compression Tool

Software Engineering mini-project (Group 15, Problem 17): a basic ZIP compression and extraction tool implemented in C++17.

## What it does

The application can:
- compress one or more files or folders into a ZIP archive
- preserve nested directories
- extract ZIP archives into a selected directory
- list archive entries
- show operation progress in the console
- prevent accidental overwrites unless confirmed
- reject malformed archives and unsafe extraction paths
- verify extracted data with CRC-32
- report basic Huffman coding statistics during compression

The ZIP writer uses standard ZIP local-file header, central-directory, and end-of-central-directory records. File entries use the standard DEFLATE method when it produces a smaller result. DEFLATE uses Huffman coding internally, while the project also exposes a small Huffman analysis module for the compression flow.

## Build

### Linux / macOS

Requires C++17, CMake 3.16+ and zlib.

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Windows

Use CMake with a C++17 compiler and a zlib installation. The same CMake targets are used:

```text
file-compress
smoke_test
```

## Run

```bash
file-compress compress sample.zip file1.txt folder/
file-compress list sample.zip
file-compress extract sample.zip extracted/
```

Add `--yes` to allow overwriting an existing archive or extracted file.

Running `file-compress` without arguments opens a simple interactive menu.

## Example

```text
$ file-compress compress project.zip input/
  input/readme.txt  [24 bytes, 6 symbols, avg Huffman code 2.5 bits]
  input/data.bin    [4096 bytes, ...]
Created: "project.zip"

$ file-compress extract project.zip output/
  Extracted input/readme.txt (50%)
  Extracted input/data.bin (100%)
Extraction completed: 2 files -> "output"
```

## Repository layout

```text
src/                  Core implementation
  main.cpp            Command-line interface
  zip_archive.*       ZIP read/write, compression, extraction and safety checks
  huffman.*           Huffman frequency/tree analysis
tests/                Round-trip smoke test
.github/workflows/    GitHub Actions build and test workflow
DOCS/                 SRS, SAD, STP and UML/architecture diagrams
```

## Notes

The implementation intentionally keeps the archive format limited to the basic ZIP features needed by the SRS. Large ZIP64 archives, encryption, and advanced archive metadata are outside the current project scope.
