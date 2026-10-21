#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace huffman {

struct Node {
    std::uint64_t frequency{};
    int symbol{-1};
    Node* left{nullptr};
    Node* right{nullptr};
};

struct Analysis {
    std::size_t unique_symbols{};
    double average_code_length{};
};

Analysis analyse(const std::vector<std::uint8_t>& data);

} // namespace huffman
