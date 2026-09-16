#pragma once

#include <filesystem>

namespace fs = std::filesystem;

//struct for the args hadling result. I think this is a easy way to give what has been found back to the main function
struct argResult {
    fs::path searchPath{};
    fs::path dumpPath{}; //equal to searchPath if not provided
};

argResult handleArg(int argc, char** argv);