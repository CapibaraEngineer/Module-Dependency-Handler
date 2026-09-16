#include <functional>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "cliArgHandler.hpp"

std::map<std::string, std::function<void(std::string)>> argToFunction;

argResult handleArg(const int argc, char** argv) {
    if(argc == 0) {
        std::cerr << "No args provided, failing now. use --help for help";
        std::abort();
        return {};
    }
    //turn argv into vector to make it easier to use
    const std::vector<std::string> args{argv, argv + argc}; //if you think this is confusing. This is just ai terator from the first arugemnt in argv to the last

    for(const auto& arg : args) {
        std::cout << arg << "\n";
    }

    return {};
}