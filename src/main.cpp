#include "zip_archive.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void print_usage() {
    std::cout
        << "File Compression Tool - basic ZIP implementation\n\n"
        << "Usage:\n"
        << "  file-compress compress <archive.zip> <file/folder> [more inputs...] [--yes]\n"
        << "  file-compress extract <archive.zip> <destination> [--yes]\n"
        << "  file-compress list <archive.zip>\n"
        << "  file-compress\n\n"
        << "ZIP entries use DEFLATE when it produces a smaller result. "
           "DEFLATE uses Huffman coding internally.\n";
}

bool has_flag(std::vector<std::string>& args, const std::string& flag) {
    for (auto it = args.begin(); it != args.end(); ++it) {
        if (*it == flag) {
            args.erase(it);
            return true;
        }
    }
    return false;
}

void interactive() {
    while (true) {
        std::cout << "\n1. Compress\n2. Extract\n3. List archive\n4. Exit\nChoose: ";

        int choice = 0;
        if (!(std::cin >> choice)) return;
        std::cin.ignore(1024, '\n');

        try {
            if (choice == 1) {
                std::string archive, line;
                std::cout << "Output archive (.zip): ";
                std::getline(std::cin, archive);
                std::cout << "Input path(s), separated by spaces: ";
                std::getline(std::cin, line);

                std::istringstream stream(line);
                std::vector<ziptool::InputPath> inputs;
                std::string item;
                while (stream >> item) inputs.push_back({item});

                std::cout << "Overwrite existing archive? (y/N): ";
                char confirm = 'N';
                std::cin >> confirm;
                std::cin.ignore(1024, '\n');

                ziptool::create_archive(
                    archive, inputs,
                    {confirm == 'y' || confirm == 'Y'});
            } else if (choice == 2) {
                std::string archive, destination;
                std::cout << "Archive: ";
                std::getline(std::cin, archive);
                std::cout << "Destination: ";
                std::getline(std::cin, destination);

                std::cout << "Overwrite existing files? (y/N): ";
                char confirm = 'N';
                std::cin >> confirm;
                std::cin.ignore(1024, '\n');

                ziptool::extract_archive(
                    archive, destination,
                    {confirm == 'y' || confirm == 'Y'});
            } else if (choice == 3) {
                std::string archive;
                std::cout << "Archive: ";
                std::getline(std::cin, archive);

                for (const auto& name : ziptool::list_archive(archive)) {
                    std::cout << "  " << name << '\n';
                }
            } else if (choice == 4) {
                return;
            } else {
                std::cout << "Invalid choice.\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << '\n';
        }
    }
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 1) {
        interactive();
        return 0;
    }

    std::vector<std::string> args(argv + 1, argv + argc);
    const bool overwrite = has_flag(args, "--yes");

    try {
        if (args.empty() || args[0] == "help" ||
            args[0] == "--help" || args[0] == "-h") {
            print_usage();
            return 0;
        }

        if (args[0] == "compress") {
            if (args.size() < 3) {
                print_usage();
                return 2;
            }

            const auto archive = args[1];
            std::vector<ziptool::InputPath> inputs;
            for (std::size_t i = 2; i < args.size(); ++i) {
                inputs.push_back({args[i]});
            }

            ziptool::create_archive(archive, inputs, {overwrite});
            return 0;
        }

        if (args[0] == "extract") {
            if (args.size() != 3) {
                print_usage();
                return 2;
            }

            ziptool::extract_archive(args[1], args[2], {overwrite});
            return 0;
        }

        if (args[0] == "list") {
            if (args.size() != 2) {
                print_usage();
                return 2;
            }

            for (const auto& name : ziptool::list_archive(args[1])) {
                std::cout << name << '\n';
            }
            return 0;
        }

        print_usage();
        return 2;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
