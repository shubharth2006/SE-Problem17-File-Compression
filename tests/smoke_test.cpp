#include <stdexcept>
#include "zip_archive.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

namespace fs = std::filesystem;


namespace {

void write_u16(std::ostream& out, std::uint16_t value) {
    const char bytes[2] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff)
    };
    out.write(bytes, 2);
}

void write_u32(std::ostream& out, std::uint32_t value) {
    const char bytes[4] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff)
    };
    out.write(bytes, 4);
}

void write_empty_zip(const fs::path& archive, const std::string& entry_name) {
    std::ofstream out(archive, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Could not create ZIP test archive.");

    write_u32(out, 0x04034b50);
    write_u16(out, 20);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, 0);
    write_u32(out, 0);
    write_u32(out, 0);
    write_u16(out, static_cast<std::uint16_t>(entry_name.size()));
    write_u16(out, 0);
    out.write(entry_name.data(), static_cast<std::streamsize>(entry_name.size()));

    const auto central_offset = static_cast<std::uint32_t>(out.tellp());
    write_u32(out, 0x02014b50);
    write_u16(out, 20);
    write_u16(out, 20);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, 0);
    write_u32(out, 0);
    write_u32(out, 0);
    write_u16(out, static_cast<std::uint16_t>(entry_name.size()));
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, 0);
    write_u32(out, 0);
    out.write(entry_name.data(), static_cast<std::streamsize>(entry_name.size()));

    const auto central_end = static_cast<std::uint32_t>(out.tellp());
    write_u32(out, 0x06054b50);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 1);
    write_u16(out, 1);
    write_u32(out, central_end - central_offset);
    write_u32(out, central_offset);
    write_u16(out, 0);

    if (!out) throw std::runtime_error("Could not finish ZIP test archive.");
}

} // namespace

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
        std::ofstream binary_file(
            root / "input/subdir/data.bin", std::ios::binary);

        binary_file.write("\x00\x01\x02\x03\x04\x05", 6);
        binary_file.close();

        if (!binary_file) {
            std::cerr << "Failed to create binary test input.\n";
            return 1;
        }
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
    // FCTG1-5: Detect corrupted ZIP file data using CRC-32.
    const fs::path crc_input = root / "crc_input.txt";
    const fs::path crc_archive = root / "crc_test.zip";
    const fs::path corrupted_archive = root / "corrupted_data.zip";

    const std::string payload = "Q7!z";

    // Create a small file with known contents.
    {
        std::ofstream file(crc_input, std::ios::binary);
        file << payload;

        if (!file) {
            std::cerr << "Failed to create CRC test input.\n";
            return 1;
        }
    }

    // Create a valid ZIP archive.
    ziptool::create_archive(crc_archive, {{crc_input}}, {true});

    // Read the ZIP archive into memory.
    std::ifstream archive_file(crc_archive, std::ios::binary);

    if (!archive_file) {
        std::cerr << "Cannot open CRC test archive.\n";
        return 1;
    }

    std::string archive_bytes(
        (std::istreambuf_iterator<char>(archive_file)),
        std::istreambuf_iterator<char>());

    archive_file.close();

    // Find the original file contents inside the ZIP.
    const auto position = archive_bytes.find(payload);

    if (position == std::string::npos) {
        std::cerr << "CRC test payload not found in ZIP.\n";
        return 1;
    }

    // Deliberately corrupt one byte.
    archive_bytes[position] ^= 0x01;

    // Save the corrupted ZIP as a separate file.
    {
        std::ofstream file(corrupted_archive, std::ios::binary);

        file.write(
            archive_bytes.data(),
            static_cast<std::streamsize>(archive_bytes.size()));

        if (!file) {
            std::cerr << "Failed to write corrupted ZIP archive.\n";
            return 1;
        }
    }

    // Attempt extraction and verify CRC mismatch detection.
    bool corruption_detected = false;

    try {
        ziptool::extract_archive(
            corrupted_archive,
            root / "corrupted_output",
            {true});
    } catch (const std::runtime_error& ex) {
        if (std::string(ex.what()).find("CRC check failed") !=
            std::string::npos) {
            corruption_detected = true;
        }
    }

    if (!corruption_detected) {
        std::cerr << "Corrupted ZIP data detection test failed.\n";
        return 1;
    }

    std::cout << "Corrupted ZIP data detection test passed.\n";
    // FCTG1-5: Reject an archive path that is a directory.
    const fs::path inaccessible_archive = root / "input";

    bool file_access_error_detected = false;

    try {
        ziptool::list_archive(inaccessible_archive);
    } catch (const std::runtime_error&) {
        file_access_error_detected = true;
    }

    if (!file_access_error_detected) {
        std::cerr << "File access error handling test failed.\n";
        return 1;
    }

    std::cout << "File access error handling test passed.\n";
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

    // Path traversal must be rejected on both POSIX and Windows-style separators.
    const auto traversal_output = root / "traversal_output";
    const auto expect_rejected = [&](const std::string& entry_name,
                                     const std::string& archive_name) {
        const auto malicious_archive = root / archive_name;
        write_empty_zip(malicious_archive, entry_name);
        try {
            ziptool::extract_archive(malicious_archive, traversal_output, {true});
        } catch (const std::runtime_error&) {
            return true;
        }
        return false;
    };

    if (!expect_rejected("../escaped.txt", "traversal.zip") ||
        !expect_rejected("..\\escaped_backslash.txt", "backslash_traversal.zip") ||
        !expect_rejected("/tmp/absolute.txt", "absolute_path.zip") ||
        !expect_rejected("C:/drive_path.txt", "drive_path.zip")) {
        std::cerr << "Unsafe archive path was not rejected.\n";
        return 1;
    }
    if (fs::exists(root / "escaped.txt") ||
        fs::exists(root / "escaped_backslash.txt")) {
        std::cerr << "Path traversal wrote outside the extraction directory.\n";
        return 1;
    }

    // Existing files stay untouched unless overwrite was explicitly enabled.
    const auto overwrite_source = root / "overwrite_source.txt";
    const auto overwrite_archive = root / "overwrite.zip";
    const auto overwrite_output = root / "overwrite_output";
    fs::create_directories(overwrite_output);
    {
        std::ofstream(overwrite_source) << "replacement data";
        std::ofstream(overwrite_output / overwrite_source.filename())
            << "keep this file";
    }
    ziptool::create_archive(overwrite_archive, {{overwrite_source}}, {true});

    bool overwrite_blocked = false;
    try {
        ziptool::extract_archive(overwrite_archive, overwrite_output, {false});
    } catch (const std::runtime_error&) {
        overwrite_blocked = true;
    }
    if (!overwrite_blocked ||
        read(overwrite_output / overwrite_source.filename()) != "keep this file") {
        std::cerr << "Existing file was not protected from an implicit overwrite.\n";
        return 1;
    }

    ziptool::extract_archive(overwrite_archive, overwrite_output, {true});
    if (read(overwrite_output / overwrite_source.filename()) != "replacement data") {
        std::cerr << "Explicit overwrite did not replace the existing file.\n";
        return 1;
    }

    // A directory symlink inside the destination must not lead extraction outside it.
    const auto symlink_output = root / "symlink_output";
    const auto outside = root / "outside";
    fs::create_directories(symlink_output);
    fs::create_directories(outside);
    bool symlink_supported = false;
    try {
        fs::create_directory_symlink(outside, symlink_output / "link");
        symlink_supported = true;
    } catch (const fs::filesystem_error&) {
        // Symlink creation may be unavailable on some Windows configurations.
    }

    if (symlink_supported) {
        const auto symlink_archive = root / "symlink_parent.zip";
        write_empty_zip(symlink_archive, "link/pwned.txt");
        bool symlink_path_rejected = false;
        try {
            ziptool::extract_archive(symlink_archive, symlink_output, {true});
        } catch (const std::runtime_error&) {
            symlink_path_rejected = true;
        }
        if (!symlink_path_rejected || fs::exists(outside / "pwned.txt")) {
            std::cerr << "Extraction followed a directory symlink.\n";
            return 1;
        }

        const auto protected_file = outside / "protected.txt";
        {
            std::ofstream(protected_file) << "outside data";
        }
        fs::create_symlink(protected_file, symlink_output / "protected.txt");

        const auto symlink_file_archive = root / "symlink_file.zip";
        write_empty_zip(symlink_file_archive, "protected.txt");
        bool symlink_file_rejected = false;
        try {
            ziptool::extract_archive(symlink_file_archive, symlink_output, {true});
        } catch (const std::runtime_error&) {
            symlink_file_rejected = true;
        }
        if (!symlink_file_rejected || read(protected_file) != "outside data") {
            std::cerr << "Extraction overwrote a file through a symlink.\n";
            return 1;
        }
    }

    fs::remove_all(root);
    std::cout << "Smoke test passed.\n";
    return 0;
}
