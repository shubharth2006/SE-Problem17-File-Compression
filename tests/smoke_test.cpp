#include <stdexcept>
#include "zip_archive.hpp"

#include <algorithm>
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
    fs::create_directories(root / "input/empty/nested");
    // FCTG1-5: Reject plain text disguised as a ZIP archive.
    const fs::path invalid_archive = root / "invalid.zip";

    {
        std::ofstream file(invalid_archive, std::ios::binary);
        file << "This is plain text, not a ZIP archive.";
        file.close();

        if (!file) {
            std::cerr << "Failed to create invalid ZIP test input.\n";
            return 1;
        }
    }

    bool invalid_archive_rejected = false;
    try {
        ziptool::list_archive(invalid_archive);
    } catch (const std::runtime_error&) {
        invalid_archive_rejected = true;
    }

    if (!invalid_archive_rejected) {
        std::cerr << "Invalid ZIP rejection test failed.\n";
        return 1;
    }

    fs::remove(invalid_archive);
    std::cout << "Invalid ZIP rejection test passed.\n";

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

    if (names.size() < 6) {
        std::cerr << "Expected directory and file entries.\n";
        return 1;
    }

    if (std::find(names.begin(), names.end(), "input/subdir/data.bin") ==
            names.end() ||
        std::find(names.begin(), names.end(), "input/empty/nested/") ==
            names.end()) {
        std::cerr << "Archive did not preserve the original relative paths.\n";
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

    if (!fs::is_directory(output / "input/empty/nested")) {
        std::cerr << "Empty nested directory was not restored.\n";
        return 1;
    }

    fs::remove_all(root);
    std::cout << "Smoke test passed.\n";
    return 0;
}
