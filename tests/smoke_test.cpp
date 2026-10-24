#include <stdexcept>
#include "zip_archive.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    const fs::path root = fs::current_path() / "smoke_workspace";
    fs::remove_all(root);
        // Test 1: Missing input path must be rejected.
    bool missing_input_rejected = false;

    try {
        ziptool::create_archive(
            root / "missing.zip",
            {{root / "does_not_exist.txt"}},
            {true});
    } catch (const std::exception&) {
        missing_input_rejected = true;
    }

    if (!missing_input_rejected) {
        std::cerr << "Missing input validation test failed.\n";
        return 1;
    }

    // Test 2: Empty input list must be rejected.
    bool empty_inputs_rejected = false;

    try {
        ziptool::create_archive(root / "empty.zip", {}, {true});
    } catch (const std::exception&) {
        empty_inputs_rejected = true;
    }

    if (!empty_inputs_rejected) {
        std::cerr << "Empty input validation test failed.\n";
        return 1;
    }

    std::cout << "Input validation tests passed.\n";
    fs::create_directories(root / "input/subdir");

    {
        std::ofstream(root / "input/readme.txt")
            << "compression smoke test\n";
        std::ofstream(root / "input/subdir/data.bin", std::ios::binary)
            << "\x00\x01\x02\x03\x04\x05";
        std::ofstream(root / "input/empty.txt");
    }

    const fs::path archive = root / "sample.zip";
    const fs::path output = root / "output";

    ziptool::create_archive(archive, {{{root / "input"}}}, {true});
    const auto names = ziptool::list_archive(archive);

    if (names.size() < 4) {
        std::cerr << "Expected directory and file entries.\n";
        return 1;
    }

    ziptool::extract_archive(archive, output, {true});

    auto read = [](const fs::path& path) {
        std::ifstream in(path, std::ios::binary);
        return std::string(
            (std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
    };

    if (read(root / "input/readme.txt") !=
        read(output / "input/readme.txt")) return 1;

    if (read(root / "input/subdir/data.bin") !=
        read(output / "input/subdir/data.bin")) return 1;

    if (fs::file_size(root / "input/empty.txt") !=
        fs::file_size(output / "input/empty.txt")) return 1;

    fs::remove_all(root);
    std::cout << "Smoke test passed.\n";
    return 0;
}
