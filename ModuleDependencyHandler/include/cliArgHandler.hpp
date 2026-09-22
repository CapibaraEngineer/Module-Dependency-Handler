#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

enum class argParsingResultType : uint8_t {
    SUCESS,
    HELP_PAGE,
    ERROR,
    ERROR_SHOW_HELP,
};

//struct for the args hadling result. I think this is a easy way to give what has been found back to the main function
struct argResult {
    argParsingResultType resultType;
    std::string errorMessage;
    fs::path searchPath{};
    fs::path outputPath{}; //equal to searchPath if not provided
    std::vector<std::string> fileTypes;
};

void showHelpPage();
[[nodiscard]] bool isValidDirectoryPath(const std::string& string);
argResult handleArgs(int argc, char** argv);