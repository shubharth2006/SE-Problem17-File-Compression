#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ziptool {

struct InputPath {
    std::filesystem::path path;
};

struct ArchiveOptions {
    bool overwrite{false};
};

struct ExtractionOptions {
    bool overwrite{false};
};

void create_archive(const std::filesystem::path& output,
                    const std::vector<InputPath>& inputs,
                    const ArchiveOptions& options = {});

void extract_archive(const std::filesystem::path& archive,
                     const std::filesystem::path& destination,
                     const ExtractionOptions& options = {});

std::vector<std::string> list_archive(const std::filesystem::path& archive);

} // namespace ziptool
