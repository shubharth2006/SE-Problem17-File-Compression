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
    // FCTG1-5: Reject a missing ZIP archive.
    const fs::path missing_archive = root / "nonexistent.zip";

    bool missing_archive_rejected = false;
    try {
        ziptool::list_archive(missing_archive);
    } catch (const std::runtime_error&) {
        missing_archive_rejected = true;
    }

    if (!missing_archive_rejected) {
        std::cerr << "Missing ZIP archive rejection test failed.\n";
        return 1;
    }

    std::cout << "Missing ZIP archive rejection test passed.\n";
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
    // FCTG1-5: Extraction must also reject non-ZIP content.
    bool invalid_extraction_rejected = false;
    try {
        ziptool::extract_archive(
            invalid_archive, root / "invalid_output", {false});
    } catch (const std::runtime_error&) {
        invalid_extraction_rejected = true;
    }

    if (!invalid_extraction_rejected) {
        std::cerr << "Invalid ZIP extraction rejection test failed.\n";
        return 1;
    }
    std::cout << "Invalid ZIP extraction rejection test passed.\n";
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
    // FCTG1-5: Reject a truncated ZIP archive.
    const fs::path truncated_archive = root / "truncated.zip";

    // Copy the valid archive so the original remains unchanged.
    fs::copy_file(archive, truncated_archive,
                fs::copy_options::overwrite_existing);

    // Remove the final 10 bytes to damage the ZIP end record.
    const auto original_size = fs::file_size(truncated_archive);

    if (original_size <= 10) {
        std::cerr << "ZIP archive is too small for truncation test.\n";
        return 1;
    }

    fs::resize_file(truncated_archive, original_size - 10);

    // Verify that the truncated ZIP is rejected.
    bool truncated_archive_rejected = false;

    try {
        ziptool::list_archive(truncated_archive);
    } catch (const std::runtime_error&) {
        truncated_archive_rejected = true;
    }

    if (!truncated_archive_rejected) {
        std::cerr << "Truncated ZIP archive rejection test failed.\n";
        return 1;
    }

    std::cout << "Truncated ZIP archive rejection test passed.\n";
    fs::remove(truncated_archive);

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
    // FCTG1-4: Refusing archive overwrite must preserve its bytes.
    const std::string original_archive = read(archive);
    if (original_archive.empty()) {
        std::cerr << "Overwrite test requires a nonempty archive.\n";
        return 1;
    }

    bool archive_overwrite_rejected = false;
    try {
        ziptool::create_archive(
            archive, {{root / "input"}}, {false});
    } catch (const std::runtime_error&) {
        archive_overwrite_rejected = true;
    }

    if (!archive_overwrite_rejected ||
        read(archive) != original_archive) {
        std::cerr << "Archive overwrite protection test failed.\n";
        return 1;
    }

    // FCTG1-4: Refusing extraction overwrite must preserve user data.
    const fs::path protected_file = output / "input/readme.txt";
    const std::string user_content = "Existing user data: keep unchanged.\n";

    {
        std::ofstream file(
            protected_file, std::ios::binary | std::ios::trunc);
        file << user_content;
        file.close();

        if (!file) {
            std::cerr << "Failed to prepare overwrite test input.\n";
            return 1;
        }
    }

    bool extraction_overwrite_rejected = false;
    try {
        ziptool::extract_archive(archive, output, {false});
    } catch (const std::runtime_error&) {
        extraction_overwrite_rejected = true;
    }

    if (!extraction_overwrite_rejected ||
        !fs::is_regular_file(protected_file) ||
        read(protected_file) != user_content) {
        std::cerr << "Extraction overwrite protection test failed.\n";
        return 1;
    }

    std::cout << "Overwrite protection tests passed.\n";

    fs::remove_all(root);
    std::cout << "Smoke test passed.\n";
    return 0;
}
