
#include "preCompile.hpp"

#include <filesystem>
#include <string>
#include <iostream>

[[nodiscard]] std::string buildClangCommand(const fs::path& outPath) {
    // temporay i'm going to do a proper implementation later. TODO!
    return std::string("clang++ -std=c++23 -fprebuilt-module-path=" + outPath.string() + " --precompile ");
}

void runClang(const std::vector<std::vector<std::shared_ptr<moduleRepresentation>>>& moduleDependencyGraph, const fs::path& outPath) {
    const std::string halfClangCommand = buildClangCommand(outPath);
    //Debug print
    for(const auto& level : moduleDependencyGraph) {
        for (const auto& modulea : level) {
            if(modulea->partitionName.has_value()) {
                std::cout << modulea->moduleName << ":" << modulea->partitionName.value() << " ";
            } else {
                std::cout << modulea->moduleName << " ";
            }
        }
        std::cout << "\n";
    }

    for(const auto& level : moduleDependencyGraph) {
        
        for(const auto& currentModule : level) {
            
            std::string fullClangCommand = halfClangCommand;
            fullClangCommand += currentModule->definitionFile.string();
            fullClangCommand += std::string(" -o ");
            fullClangCommand += std::string(outPath.string() + currentModule->definitionFile.stem().string() + ".pcm");
            std::cout << fullClangCommand << "\n";
            //Temporary solution. TODO: make it good
            std::system(fullClangCommand.c_str());
        }
    }
}