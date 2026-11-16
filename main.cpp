#include "parser.h"
#include "searchhandler.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <json-file> <object.key.path>\n";
        return 2;
    }
    Searchhandler handler{argv[2]};
    if (!handler.valid_path()) {
        std::cerr << "Invalid path: use 1 to 64 nonempty keys separated by dots\n";
        return 2;
    }
    std::ifstream file{argv[1], std::ios::binary};
    if (!file) {
        std::cerr << "Cannot open JSON file: " << argv[1] << '\n';
        return 1;
    }
    std::string input;
    try {
        input.assign(std::istreambuf_iterator<char>{file},
                     std::istreambuf_iterator<char>{});
    } catch (const std::ios_base::failure&) {
        std::cerr << "Error reading JSON file\n";
        return 1;
    }
    if (file.bad()) {
        std::cerr << "Error reading JSON file\n";
        return 1;
    }
    Parser parser{input.data(), input.size(), handler};
    const auto result = parser.parse();
    if (!result) {
        std::cerr << "Parse error at byte " << result.error().offset_of_error
                  << ": " << result.error().error_descrip << '\n';
        return 1;
    }
    if (!handler.found()) {
        std::cerr << "No scalar value found at path: " << argv[2] << '\n';
        return 1;
    }
    std::cout << handler.result() << '\n';
}
