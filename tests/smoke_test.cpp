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

    // FCTG1-7: a truncated end record must be rejected, not partially parsed.
    const fs::path truncated_archive = root / "truncated.zip";
    fs::copy_file(archive, truncated_archive, fs::copy_options::overwrite_existing);
    const auto archive_size = fs::file_size(truncated_archive);
    if (archive_size <= 10) {
        std::cerr << "Archive too small for truncation test.\\n";
        return 1;
    }
    fs::resize_file(truncated_archive, archive_size - 10);
    bool truncated_archive_rejected = false;
    try {
        ziptool::list_archive(truncated_archive);
    } catch (const std::runtime_error&) {
        truncated_archive_rejected = true;
    }
    if (!truncated_archive_rejected) {
        std::cerr << "Truncated archive validation test failed.\\n";
        return 1;
    }

    // FCTG1-7: altered central-directory CRC metadata must be rejected on extract.
    const fs::path corrupt_crc_archive = root / "corrupt_crc.zip";
    fs::copy_file(archive, corrupt_crc_archive,
                  fs::copy_options::overwrite_existing);
    {
        std::fstream file(corrupt_crc_archive,
                          std::ios::binary | std::ios::in | std::ios::out);
        if (!file) {
            std::cerr << "Could not open CRC corruption test archive.\\n";
            return 1;
        }
        std::vector<char> bytes(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        const char central_signature[] = {'P', 'K', 0x01, 0x02};
        bool crc_changed = false;
        auto search_from = bytes.begin();
        while (search_from != bytes.end()) {
            auto signature = std::search(
                search_from, bytes.end(),
                std::begin(central_signature), std::end(central_signature));
            if (signature == bytes.end()) break;
            const auto offset = static_cast<std::size_t>(
                std::distance(bytes.begin(), signature));
            if (bytes.size() - offset < 46) break;
            const auto name_len = static_cast<unsigned char>(bytes[offset + 28]) |
                (static_cast<unsigned char>(bytes[offset + 29]) << 8);
            const auto extra_len = static_cast<unsigned char>(bytes[offset + 30]) |
                (static_cast<unsigned char>(bytes[offset + 31]) << 8);
            const auto comment_len = static_cast<unsigned char>(bytes[offset + 32]) |
                (static_cast<unsigned char>(bytes[offset + 33]) << 8);
            if (bytes.size() - offset < 46u + name_len + extra_len + comment_len) break;
            const std::string entry_name(
                bytes.begin() + static_cast<std::ptrdiff_t>(offset + 46),
                bytes.begin() + static_cast<std::ptrdiff_t>(offset + 46 + name_len));
            if (!entry_name.empty() && entry_name.back() != '/') {
                bytes[offset + 16] ^= 0x01;
                crc_changed = true;
                break;
            }
            search_from = signature + 4;
        }
        if (!crc_changed) {
            std::cerr << "Could not locate a file-entry CRC for test.\\n";
            return 1;
        }
        file.clear();
        file.seekp(0, std::ios::beg);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) {
            std::cerr << "Could not write CRC corruption test archive.\\n";
            return 1;
        }
    }
    bool corrupt_crc_rejected = false;
    try {
        ziptool::extract_archive(corrupt_crc_archive,
                                 root / "corrupt_crc_output", {true});
    } catch (const std::runtime_error&) {
        corrupt_crc_rejected = true;
    }
    if (!corrupt_crc_rejected) {
        std::cerr << "Corrupted CRC metadata validation test failed.\\n";
        return 1;
    }
    std::cout << "Archive integrity validation tests passed.\\n";

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

