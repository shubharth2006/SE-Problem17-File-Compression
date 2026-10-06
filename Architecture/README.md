# Architecture & Design — Problem 17

The project uses a modular layered architecture for a standalone C/C++ Huffman-based compression tool.

## Main components
File Manager, Frequency Analyzer, Huffman Tree Builder, Code Generator, Encoder/Compressor, Header Manager, Decoder/Decompressor, Statistics/UI, and Error Handling.

## Main flows
Compression: Select → Read → Frequency Analysis → Huffman Tree → Codes → Encode → Write.

Decompression: Select → Validate Header → Reconstruct Tree → Decode → Restore File.