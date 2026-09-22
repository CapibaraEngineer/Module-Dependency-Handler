
#include "preCompile.hpp"

#include <string>
#include <iostream>

[[nodiscard]] std::string buildClangCommand() {
    // temporay i'm going to do a proper implementation later. TODO!
    return "clang++ -std=c++23 --precompile";
}

void runClang(const std::vector<std::vector<std::shared_ptr<moduleRepresentation>>>& moduleDependencyGraph) {
    const std::string halfClangCommand = buildClangCommand();
    for(const auto& level : moduleDependencyGraph) {
        for(const auto& currentModule : level) {
            std::string fullClangCommand = halfClangCommand;
            fullClangCommand += currentModule->definitionFile.filename().string();
            fullClangCommand += std::string(" -o ");
            fullClangCommand += std::string(currentModule->definitionFile.stem().string() + ".pcm");
            std::cout << fullClangCommand << "\n";
            //Temporary solution. TODO: make it good
            //std::system(fullClangCommand.c_str());
        }
    }
}