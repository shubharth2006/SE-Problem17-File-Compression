# File Compression Tool

## 1. What this project is

A Software Engineering course project (**Group 15, Problem 17**) for a **File Compression Tool (Basic ZIP Implementation)** using **C/C++** and **Huffman coding**.

## 2. The core idea, in one paragraph

The tool lets a user select files or folders, compress them into an archive, and later extract the contents. The compression side reads the input, builds frequency information, constructs a Huffman tree, generates codes, and writes the compressed data together with the information needed for extraction. During extraction, the stored information is read, the Huffman structure is reconstructed, and the original file contents are restored. The application is intended to run locally.

## 3. Main operations

**Compression:** Select input -> analyse data -> build Huffman tree -> generate codes -> encode -> create compressed file.

**Extraction:** Select archive -> read/validate stored information -> reconstruct Huffman tree -> decode -> restore files.

## 4. Documentation

All current submission documents and diagram sources are kept under [DOCS](DOCS/).

- Software Requirements Specification (SRS)
- Software Test Plan (STP)
- Software Architecture and Design Specification (SAD)
- UML and architecture diagram source files

## 5. Repository layout

```text
DOCS/
├── Software_Requirements_Specification_SRS.docx
├── Software_Test_Plan_STP.docx
├── Software_Architecture_and_Design_SAD.docx
└── diagrams/
    ├── use_case_compression.puml
    ├── use_case_extraction.puml
    ├── component_diagram.puml
    ├── seq_compression.puml
    └── seq_decompression.puml
```

The implementation can be added separately when the team is ready.
