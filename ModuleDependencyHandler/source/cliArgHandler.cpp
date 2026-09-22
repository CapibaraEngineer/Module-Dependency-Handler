#include <functional>
#include <cctype>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "cliArgHandler.hpp"


void showHelpPage() {
    std::cout << "Module Dependency Handler\n"
              << "Usage: ModuleDependencyHandler -i <input-directory> [options]\n\n"
              << "Options:\n"
              << "  -h, -help, help          Show this help page\n"
              << "  -i <directory>           Directory to search for module files\n"
              << "  -o <directory>           Directory for generated output\n"
              << "                           Defaults to the input directory\n"
              << "  -file-types <types...>   File extensions to scan\n"
              << "                           Defaults to .cppm and .ixx\n";
}

[[nodiscard]] bool isValidDirectoryPath(const std::string& string) {
    if (string.empty()) {
        return false;
    }

    std::error_code error;
    const fs::path path{string};
    return fs::is_directory(path, error) and not error;
}

[[nodiscard]] bool isValidWord(const std::string& string) {
    static const std::map<std::string, bool> knownArguments{
        {"-h", true},
        {"-help", true},
        {"help", true},
        {"-i", true},
        {"-o", true},
        {"-file-types", true},
    };

    if (knownArguments.contains(string)) {
        return true;
    }

    if (string.size() < 2 or string.front() != '.') {
        return false;
    }

    for (size_t i = 1; i < string.size(); ++i) {
        const auto character = static_cast<unsigned char>(string.at(i));
        if (std::isalnum(character) == 0 and character != '_' and character != '-') {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool isKnownArgument(const std::string& string) {
    static const std::map<std::string, bool> knownArguments{
        {"-h", true},
        {"-help", true},
        {"help", true},
        {"-i", true},
        {"-o", true},
        {"-file-types", true},
    };

    try {
        return knownArguments.at(string);
    } catch (const std::out_of_range& e) {
        return false;
    }
    return false;
}

argResult handleArgs(const int argc, char** argv) {
    //I think this function is not good. TODO: Make it better

    argResult returnResult;
    //turn argv into vector to make it easier to use
    const std::vector<std::string> args{argv + 1, argv + argc}; 
    //if you think this is confusing. This is just a iterator from the first arugemnt in argv to the last, the + 1 is to ignore the program name

    if (args.empty()) {
        return {.resultType = argParsingResultType::ERROR_SHOW_HELP, .errorMessage = "No args provided"};
    }
    bool outputWasDefined = false;
    bool inputWasDefined = false;
    bool fileTypesWereDefined = false;
    for(size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args.at(i); 

        if(arg == "-h" or arg == "-help" or arg == "help") {
            return {.resultType = argParsingResultType::HELP_PAGE};
        }

        if(arg == "-i") {
            if(not isValidDirectoryPath(args.at(i + 1))) {
                const std::string& path = args.at(i+1);
                return {.resultType = argParsingResultType::ERROR, .errorMessage = std::string("Invalid path for -i \n Path: " + path)};
            }
            returnResult.searchPath = args.at(i + 1);
            ++i;
            inputWasDefined = true;
            continue;
        }

        if(arg == "-o") {
            if(not isValidDirectoryPath(args.at(i + 1))) {
                const std::string& path = args.at(i+1);
                return {.resultType = argParsingResultType::ERROR, .errorMessage = std::string("Invalid path for -o" + path)};
            }
            returnResult.outputPath = args.at(i + 1);
            ++i;
            outputWasDefined = true;
            continue;
        }

        if(arg == "-file-types") {
            for(size_t j = i+1; j < args.size(); j++) {
                if(not isValidWord(args.at(j))) {
                    return {.resultType = argParsingResultType::ERROR_SHOW_HELP, .errorMessage = std::string("Invalid word for -file-types \n word:" + args.at(j))};
                }
                if(isKnownArgument(args.at(j))) {
                    break;
                }
                returnResult.fileTypes.push_back(args.at(j));
                fileTypesWereDefined = true;
            }
        }
    }
    //errors
    if(not inputWasDefined) {
        return {.resultType = argParsingResultType::ERROR_SHOW_HELP, .errorMessage = "No search path given"};
    }

    //returns default values
    if(not fileTypesWereDefined) {
        returnResult.fileTypes = {".cppm", ".ixx"};
    }
    if(not outputWasDefined) {
        returnResult.outputPath = returnResult.searchPath;
    }

    returnResult.resultType = argParsingResultType::SUCESS;
    return returnResult;
}