#include "huffman.hpp"

#include <array>
#include <functional>
#include <memory>
#include <queue>

namespace huffman {

namespace {
struct Compare {
    bool operator()(const Node* a, const Node* b) const {
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        return a->symbol > b->symbol;
    }
};
}

Analysis analyse(const std::vector<std::uint8_t>& data) {
    std::array<std::uint64_t, 256> freq{};
    for (auto byte : data) ++freq[byte];

    std::priority_queue<Node*, std::vector<Node*>, Compare> queue;
    std::vector<std::unique_ptr<Node>> owned;
    owned.reserve(511);

    for (int symbol = 0; symbol < 256; ++symbol) {
        if (freq[static_cast<std::size_t>(symbol)] == 0) continue;
        owned.push_back(std::make_unique<Node>());
        owned.back()->frequency = freq[static_cast<std::size_t>(symbol)];
        owned.back()->symbol = symbol;
        queue.push(owned.back().get());
    }

    Analysis result;
    result.unique_symbols = queue.size();
    if (data.empty() || queue.empty()) return result;

    if (queue.size() == 1) {
        result.average_code_length = 1.0;
        return result;
    }

    while (queue.size() > 1) {
        Node* left = queue.top(); queue.pop();
        Node* right = queue.top(); queue.pop();
        owned.push_back(std::make_unique<Node>());
        Node* parent = owned.back().get();
        parent->frequency = left->frequency + right->frequency;
        parent->left = left;
        parent->right = right;
        queue.push(parent);
    }

    Node* root = queue.top();
    std::uint64_t weighted_depth = 0;
    std::function<void(const Node*, std::uint64_t)> walk =
        [&](const Node* node, std::uint64_t depth) {
            if (!node) return;
            if (node->symbol >= 0) {
                weighted_depth += node->frequency * (depth == 0 ? 1 : depth);
                return;
            }
            walk(node->left, depth + 1);
            walk(node->right, depth + 1);
        };
    walk(root, 0);

    result.average_code_length =
        static_cast<double>(weighted_depth) /
        static_cast<double>(data.size());
    return result;
}

} // namespace huffman
