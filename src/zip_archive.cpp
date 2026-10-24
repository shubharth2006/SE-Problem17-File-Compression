#include "zip_archive.hpp"
#include "huffman.hpp"

#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace ziptool {
namespace {

constexpr std::uint32_t LOCAL_SIG = 0x04034b50;
constexpr std::uint32_t CENTRAL_SIG = 0x02014b50;
constexpr std::uint32_t END_SIG = 0x06054b50;
constexpr std::uint16_t ZIP_VERSION = 20;
constexpr std::uint16_t METHOD_STORE = 0;
constexpr std::uint16_t METHOD_DEFLATE = 8;

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

std::uint16_t read_u16(std::istream& in) {
    unsigned char b[2]{};
    in.read(reinterpret_cast<char*>(b), 2);
    if (!in) throw std::runtime_error("Unexpected end of archive.");
    return static_cast<std::uint16_t>(b[0] | (b[1] << 8));
}

std::uint32_t read_u32(std::istream& in) {
    unsigned char b[4]{};
    in.read(reinterpret_cast<char*>(b), 4);
    if (!in) throw std::runtime_error("Unexpected end of archive.");
    return static_cast<std::uint32_t>(b[0] | (b[1] << 8) |
                                      (b[2] << 16) | (b[3] << 24));
}

void ensure_u32(std::uint64_t value, const char* label) {
    if (value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error(std::string(label) +
                                 " exceeds the basic ZIP 32-bit format limit.");
    }
}

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file: " + path.string());

    in.seekg(0, std::ios::end);
    const auto size = in.tellg();
    if (size < 0) throw std::runtime_error("Cannot determine file size: " + path.string());
    in.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    if (!data.empty()) {
        in.read(reinterpret_cast<char*>(data.data()),
                static_cast<std::streamsize>(data.size()));
    }
    if (!in && !data.empty()) {
        throw std::runtime_error("Failed while reading: " + path.string());
    }
    return data;
}

struct Entry {
    std::string name;
    bool directory{false};
    std::vector<std::uint8_t> data;
    std::vector<std::uint8_t> compressed;
    std::uint32_t crc{0};
    std::uint32_t local_offset{0};
    std::uint16_t method{METHOD_STORE};
};

std::string zip_name(const std::filesystem::path& p) {
    auto s = p.generic_string();
    std::replace(s.begin(), s.end(), '\\', '/');
    while (!s.empty() && s.front() == '/') s.erase(s.begin());
    return s;
}

bool starts_with_path(const std::filesystem::path& child,
                      const std::filesystem::path& parent) {
    auto child_it = child.begin();
    auto parent_it = parent.begin();
    for (; parent_it != parent.end(); ++parent_it, ++child_it) {
        if (child_it == child.end() || *child_it != *parent_it) return false;
    }
    return true;
}

std::filesystem::path canonicalish(const std::filesystem::path& p) {
    std::error_code ec;
    auto result = std::filesystem::weakly_canonical(p, ec);
    if (ec) return std::filesystem::absolute(p).lexically_normal();
    return result.lexically_normal();
}

bool unsafe_entry_name(const std::string& name) {
    if (name.empty()) return true;
    if (name.front() == '/' || name.front() == '\\') return true;
    if (name.size() >= 2 &&
        std::isalpha(static_cast<unsigned char>(name[0])) &&
        name[1] == ':') return true;

    std::filesystem::path p(name);
    if (p.is_absolute() || p.has_root_name() || p.has_root_directory()) return true;
    for (const auto& part : p) {
        if (part == "..") return true;
    }
    return false;
}

std::vector<std::uint8_t> deflate_data(const std::vector<std::uint8_t>& input) {
    if (input.empty()) return {};

    z_stream stream{};
    if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS,
                     8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("Deflate initialization failed.");
    }

    const auto bound = deflateBound(&stream, static_cast<uLong>(input.size()));
    std::vector<std::uint8_t> output(bound);

    stream.next_in = const_cast<Bytef*>(
        reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(output.size());

    const int rc = deflate(&stream, Z_FINISH);
    const bool ok = (rc == Z_STREAM_END);
    const auto produced = stream.total_out;
    deflateEnd(&stream);

    if (!ok) throw std::runtime_error("Deflate compression failed.");
    output.resize(produced);
    return output;
}

std::vector<std::uint8_t> inflate_data(const std::vector<std::uint8_t>& input,
                                       std::size_t expected_size) {
    std::vector<std::uint8_t> output(expected_size);
    if (expected_size == 0) return output;

    z_stream stream{};
    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Inflate initialization failed.");
    }

    stream.next_in = const_cast<Bytef*>(
        reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(output.size());

    const int rc = inflate(&stream, Z_FINISH);
    const bool ok = (rc == Z_STREAM_END &&
                     stream.total_out == expected_size);
    inflateEnd(&stream);

    if (!ok) throw std::runtime_error(
        "Archive entry failed DEFLATE decompression.");
    return output;
}

void collect_path(const std::filesystem::path& path,
                  const std::filesystem::path& base,
                  std::vector<Entry>& entries,
                  std::set<std::string>& names) {
    std::error_code ec;
    const auto status = std::filesystem::status(path, ec);
    if (ec || !std::filesystem::exists(status)) {
        throw std::runtime_error(
            "Input does not exist or is inaccessible: " + path.string());
    }

    if (std::filesystem::is_directory(status)) {
        const auto relative = std::filesystem::relative(path, base, ec);
        if (ec) throw std::runtime_error(
            "Unable to determine relative path for: " + path.string());

        const auto dir_name = zip_name(relative) + "/";
        if (dir_name != "/" && names.insert(dir_name).second) {
            Entry e;
            e.name = dir_name;
            e.directory = true;
            entries.push_back(std::move(e));
        }

        for (const auto& item :
             std::filesystem::directory_iterator(
                 path,
                 std::filesystem::directory_options::skip_permission_denied,
                 ec)) {
            if (ec) throw std::runtime_error(
                "Unable to enumerate directory: " + path.string());
            collect_path(item.path(), base, entries, names);
        }
        return;
    }

    if (!std::filesystem::is_regular_file(status)) {
        throw std::runtime_error("Unsupported input type: " + path.string());
    }

    const auto relative = std::filesystem::relative(path, base, ec);
    if (ec) throw std::runtime_error(
        "Unable to determine relative path for: " + path.string());

    const auto entry_name = zip_name(relative);
    if (!names.insert(entry_name).second) {
        throw std::runtime_error(
            "Duplicate archive entry name: " + entry_name);
    }

    Entry e;
    e.name = entry_name;
    e.data = read_file(path);
    e.crc = ::crc32(0L, Z_NULL, 0);
    if (!e.data.empty()) {
        e.crc = ::crc32(e.crc, e.data.data(),
                        static_cast<uInt>(e.data.size()));
    }

    e.compressed = deflate_data(e.data);
    if (!e.data.empty() && e.compressed.size() < e.data.size()) {
        e.method = METHOD_DEFLATE;
    } else {
        e.method = METHOD_STORE;
        e.compressed = e.data;
    }

    entries.push_back(std::move(e));
}

std::vector<Entry> build_entries(const std::vector<InputPath>& inputs) {
    if (inputs.empty()) {
        throw std::runtime_error("At least one input file or folder is required.");
    }

    std::vector<Entry> entries;
    std::set<std::string> names;
    for (const auto& item : inputs) {
     std::error_code ec;
     const auto abs = std::filesystem::absolute(item.path, ec);

    if (ec) {
        throw std::runtime_error(
            "Invalid input path: " + item.path.string());
    }

    const auto status = std::filesystem::status(abs, ec);

    if (ec || !std::filesystem::exists(status)) {
        throw std::runtime_error(
            "Input does not exist or is inaccessible: " +
            item.path.string());
    }

    const auto base = abs.parent_path();
    collect_path(abs, base, entries, names);
}

return entries;

}

struct CentralRecord {
    std::string name;
    std::uint16_t method{};
    std::uint32_t crc{};
    std::uint32_t compressed_size{};
    std::uint32_t uncompressed_size{};
    std::uint32_t local_offset{};
    bool directory{false};
};

std::vector<CentralRecord> read_central_directory(std::ifstream& in) {
    in.seekg(0, std::ios::end);
    const auto file_size = static_cast<std::uint64_t>(in.tellg());
    const auto search = std::min<std::uint64_t>(file_size, 65557);

    in.seekg(static_cast<std::streamoff>(file_size - search), std::ios::beg);
    std::vector<char> tail(static_cast<std::size_t>(search));
    in.read(tail.data(), static_cast<std::streamsize>(tail.size()));
    if (!in) throw std::runtime_error("Cannot read archive tail.");

    std::size_t eocd_pos = std::string::npos;
    for (std::size_t i = tail.size(); i-- > 3;) {
        if (static_cast<unsigned char>(tail[i - 3]) == 0x50 &&
            static_cast<unsigned char>(tail[i - 2]) == 0x4b &&
            static_cast<unsigned char>(tail[i - 1]) == 0x05 &&
            static_cast<unsigned char>(tail[i]) == 0x06) {
            eocd_pos = i - 3;
            break;
        }
    }
    if (eocd_pos == std::string::npos) {
        throw std::runtime_error(
            "Not a valid basic ZIP archive: end record not found.");
    }

    auto read16_tail = [&](std::size_t pos) {
        return static_cast<std::uint16_t>(
            static_cast<unsigned char>(tail[pos]) |
            (static_cast<unsigned char>(tail[pos + 1]) << 8));
    };
    auto read32_tail = [&](std::size_t pos) {
        return static_cast<std::uint32_t>(
            static_cast<unsigned char>(tail[pos]) |
            (static_cast<unsigned char>(tail[pos + 1]) << 8) |
            (static_cast<unsigned char>(tail[pos + 2]) << 16) |
            (static_cast<unsigned char>(tail[pos + 3]) << 24));
    };

    const auto count = read16_tail(eocd_pos + 10);
    const auto central_size = read32_tail(eocd_pos + 12);
    const auto central_offset = read32_tail(eocd_pos + 16);

    if (static_cast<std::uint64_t>(central_offset) + central_size > file_size) {
        throw std::runtime_error(
            "Corrupted archive: central directory is outside the file.");
    }

    in.clear();
    in.seekg(static_cast<std::streamoff>(central_offset), std::ios::beg);

    std::vector<CentralRecord> records;
    records.reserve(count);

    for (std::uint16_t i = 0; i < count; ++i) {
        if (read_u32(in) != CENTRAL_SIG) {
            throw std::runtime_error(
                "Corrupted archive: bad central-directory entry.");
        }

        read_u16(in); // version made by
        read_u16(in); // version needed
        const auto flags = read_u16(in);
        if (flags != 0) {
            throw std::runtime_error(
                "Archive uses unsupported flags; only basic ZIP entries are supported.");
        }

        const auto method = read_u16(in);
        read_u16(in); // time
        read_u16(in); // date
        const auto crc = read_u32(in);
        const auto csize = read_u32(in);
        const auto usize = read_u32(in);
        const auto name_len = read_u16(in);
        const auto extra_len = read_u16(in);
        const auto comment_len = read_u16(in);
        read_u16(in); // disk start
        read_u16(in); // internal attrs
        read_u32(in); // external attrs
        const auto offset = read_u32(in);

        std::string name(name_len, '\0');
        if (name_len) in.read(name.data(), name_len);
        in.seekg(static_cast<std::streamoff>(extra_len + comment_len),
                 std::ios::cur);
        if (!in) {
            throw std::runtime_error(
                "Corrupted archive: truncated central-directory entry.");
        }

        records.push_back({
            name, method, crc, csize, usize, offset,
            !name.empty() && name.back() == '/'
        });
    }
    return records;
}

std::vector<std::uint8_t> read_entry_data(
    std::ifstream& in, const CentralRecord& record) {

    in.clear();
    in.seekg(static_cast<std::streamoff>(record.local_offset), std::ios::beg);
    if (read_u32(in) != LOCAL_SIG) {
        throw std::runtime_error("Corrupted archive: invalid local header.");
    }

    read_u16(in); // version
    const auto flags = read_u16(in);
    if (flags != 0) {
        throw std::runtime_error("Archive uses unsupported local-header flags.");
    }

    const auto method = read_u16(in);
    if (method != record.method) {
        throw std::runtime_error(
            "Corrupted archive: compression method mismatch.");
    }

    read_u16(in); // time
    read_u16(in); // date
    read_u32(in); // crc
    read_u32(in); // compressed size
    read_u32(in); // uncompressed size

    const auto name_len = read_u16(in);
    const auto extra_len = read_u16(in);
    in.seekg(static_cast<std::streamoff>(name_len + extra_len),
             std::ios::cur);
    if (!in) {
        throw std::runtime_error(
            "Corrupted archive: invalid local header lengths.");
    }

    std::vector<std::uint8_t> compressed(record.compressed_size);
    if (!compressed.empty()) {
        in.read(reinterpret_cast<char*>(compressed.data()),
                static_cast<std::streamsize>(compressed.size()));
    }
    if (!in && !compressed.empty()) {
        throw std::runtime_error(
            "Corrupted archive: entry data is truncated.");
    }

    std::vector<std::uint8_t> data;
    if (record.method == METHOD_STORE) {
        if (record.compressed_size != record.uncompressed_size) {
            throw std::runtime_error("Corrupted stored entry size.");
        }
        data = std::move(compressed);
    } else if (record.method == METHOD_DEFLATE) {
        data = inflate_data(compressed, record.uncompressed_size);
    } else {
        throw std::runtime_error(
            "Archive contains an unsupported compression method.");
    }

    auto actual_crc = ::crc32(0L, Z_NULL, 0);
    if (!data.empty()) {
        actual_crc = ::crc32(actual_crc, data.data(),
                             static_cast<uInt>(data.size()));
    }
    if (actual_crc != record.crc) {
        throw std::runtime_error(
            "CRC check failed for archive entry: " + record.name);
    }

    return data;
}

void write_local_entry(std::ofstream& out, Entry& entry) {
    const auto pos = static_cast<std::uint64_t>(out.tellp());
    ensure_u32(pos, "Archive offset");
    entry.local_offset = static_cast<std::uint32_t>(pos);

    write_u32(out, LOCAL_SIG);
    write_u16(out, ZIP_VERSION);
    write_u16(out, 0);
    write_u16(out, entry.directory ? METHOD_STORE : entry.method);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, entry.crc);
    write_u32(out, static_cast<std::uint32_t>(entry.compressed.size()));
    write_u32(out, static_cast<std::uint32_t>(entry.data.size()));
    write_u16(out, static_cast<std::uint16_t>(entry.name.size()));
    write_u16(out, 0);
    out.write(entry.name.data(), static_cast<std::streamsize>(entry.name.size()));
    if (!entry.compressed.empty()) {
        out.write(reinterpret_cast<const char*>(entry.compressed.data()),
                  static_cast<std::streamsize>(entry.compressed.size()));
    }
}

void write_central_entry(std::ofstream& out, const Entry& entry) {
    write_u32(out, CENTRAL_SIG);
    write_u16(out, ZIP_VERSION);
    write_u16(out, ZIP_VERSION);
    write_u16(out, 0);
    write_u16(out, entry.directory ? METHOD_STORE : entry.method);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, entry.crc);
    write_u32(out, static_cast<std::uint32_t>(entry.compressed.size()));
    write_u32(out, static_cast<std::uint32_t>(entry.data.size()));
    write_u16(out, static_cast<std::uint16_t>(entry.name.size()));
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, entry.directory ? 0x10u : 0u);
    write_u32(out, entry.local_offset);
    out.write(entry.name.data(), static_cast<std::streamsize>(entry.name.size()));
}

} // namespace

void create_archive(const std::filesystem::path& output,
                    const std::vector<InputPath>& inputs,
                    const ArchiveOptions& options) {

    std::error_code ec;
    if (std::filesystem::exists(output, ec) && !options.overwrite) {
        throw std::runtime_error(
            "Output archive already exists. Use --yes to overwrite: " +
            output.string());
    }

    std::cout << "Compression started: reading and compressing inputs...\n" << std::flush;
    auto entries = build_entries(inputs);
    ensure_u32(entries.size(), "Entry count");

    std::ofstream out(output, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error(
        "Cannot create archive: " + output.string());

    std::size_t file_count = 0;
    std::uint64_t total_input = 0;

    std::size_t processed_entries = 0;
    for (auto& entry : entries) {
        total_input += entry.data.size();
        ensure_u32(entry.data.size(), "File size");
        ensure_u32(entry.compressed.size(), "Compressed size");

        write_local_entry(out, entry);
        ++processed_entries;
        std::cout << "  Writing archive entries: " << (100 * processed_entries / entries.size()) << "%\n";

        if (!entry.directory) {
            ++file_count;
            const auto analysis = huffman::analyse(entry.data);
            std::cout << "  " << entry.name << "  ["
                      << entry.data.size() << " bytes, "
                      << analysis.unique_symbols
                      << " symbols, avg Huffman code "
                      << analysis.average_code_length << " bits]\n";
        }
    }

    const auto central_offset64 =
        static_cast<std::uint64_t>(out.tellp());
    ensure_u32(central_offset64, "Central directory offset");
    const auto central_offset =
        static_cast<std::uint32_t>(central_offset64);

    for (const auto& entry : entries) {
        write_central_entry(out, entry);
    }

    const auto central_end =
        static_cast<std::uint64_t>(out.tellp());
    ensure_u32(central_end - central_offset64,
               "Central directory size");
    const auto central_size =
        static_cast<std::uint32_t>(central_end - central_offset64);

    write_u32(out, END_SIG);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, static_cast<std::uint16_t>(entries.size()));
    write_u16(out, static_cast<std::uint16_t>(entries.size()));
    write_u32(out, central_size);
    write_u32(out, central_offset);
    write_u16(out, 0);

    out.flush();
    if (!out) throw std::runtime_error(
        "Failed while writing archive.");

    std::cout << "Created: " << output << "\n"
              << "Files: " << file_count
              << ", uncompressed bytes: " << total_input << "\n";
}

void extract_archive(const std::filesystem::path& archive,
                     const std::filesystem::path& destination,
                     const ExtractionOptions& options) {

    std::ifstream in(archive, std::ios::binary);
    if (!in) throw std::runtime_error(
        "Cannot open archive: " + archive.string());

    const auto records = read_central_directory(in);

    std::error_code ec;
    std::filesystem::create_directories(destination, ec);
    if (ec) throw std::runtime_error(
        "Cannot create extraction directory: " + destination.string());

    const auto safe_root = canonicalish(destination);
    const auto total_files = std::count_if(
        records.begin(), records.end(),
        [](const auto& record) { return !record.directory; });
    std::size_t extracted = 0;

    for (const auto& record : records) {
        if (unsafe_entry_name(record.name)) {
            throw std::runtime_error(
                "Unsafe archive path rejected: " + record.name);
        }

        const auto target =
            (safe_root / std::filesystem::path(record.name))
            .lexically_normal();

        if (!starts_with_path(target, safe_root)) {
            throw std::runtime_error(
                "Unsafe extraction path rejected: " + record.name);
        }

        if (record.directory) {
            std::filesystem::create_directories(target, ec);
            if (ec) throw std::runtime_error(
                "Cannot create directory: " + target.string());
            continue;
        }

        const auto parent = target.parent_path();
        std::filesystem::create_directories(parent, ec);
        if (ec) throw std::runtime_error(
            "Cannot create directory: " + parent.string());

        if (std::filesystem::exists(target, ec) && !options.overwrite) {
            throw std::runtime_error(
                "File already exists. Use --yes to overwrite: " +
                target.string());
        }

        auto data = read_entry_data(in, record);

        std::ofstream out(target, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error(
            "Cannot write extracted file: " + target.string());

        if (!data.empty()) {
            out.write(reinterpret_cast<const char*>(data.data()),
                      static_cast<std::streamsize>(data.size()));
        }
        if (!out) throw std::runtime_error(
            "Failed while writing extracted file: " + target.string());

        ++extracted;
        const double progress =
            total_files == 0 ? 100.0 :
            100.0 * static_cast<double>(extracted) /
            static_cast<double>(total_files);

        std::cout << "  Extracted " << record.name << " ("
                  << static_cast<int>(progress) << "%)\n";
    }

    std::cout << "Extraction completed: " << extracted
              << " files -> " << destination << "\n";
}

std::vector<std::string> list_archive(
    const std::filesystem::path& archive) {

    std::ifstream in(archive, std::ios::binary);
    if (!in) throw std::runtime_error(
        "Cannot open archive: " + archive.string());

    const auto records = read_central_directory(in);
    std::vector<std::string> result;
    result.reserve(records.size());

    for (const auto& record : records) {
        result.push_back(record.name);
    }
    return result;
}

} // namespace ziptool
