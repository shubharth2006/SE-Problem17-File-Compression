# SRS — Problem 17

## Task 7 — UML Use-Case Diagrams
```mermaid
flowchart LR
    U[User] --> A((Select Input File))
    U --> B((Compress File))
    U --> C((View Compression Result))
    B -. <<include>> .-> D((Read File Data))
    B -. <<include>> .-> E((Build Huffman Tree))
    B -. <<include>> .-> F((Generate Huffman Codes))
    B -. <<include>> .-> G((Create Compressed File))
```

### Decompression
```mermaid
flowchart LR
    U[User] --> A((Select Compressed File))
    U --> B((Decompress File))
    U --> C((View Decompression Result))
    B -. <<include>> .-> D((Read Compression Header))
    B -. <<include>> .-> E((Reconstruct Huffman Tree))
    B -. <<include>> .-> F((Decode Compressed Data))
    B -. <<include>> .-> G((Restore Original File))
```

## Task 8 — RTM
The RTM maps functional requirements, non-functional requirements and security requirements to design modules and test cases.

See the formatted SRS Task 7 & 8 document in this folder.