# ModuleDependencyHandler

Module Dependency Handler is a CLI tool for hadling the pre-compilation part of C++ modules with Clang.

#### The problem
 with Clang you need to precompile the module. What this means is running a Clang command to generate a `.pcm` file.

For a single file this is fine, but for more files it get very hard, specially because you gotta do it at the right order.

CMake already does that for you, but other build systems are still quite not there yet. This tool is for the people who wants to use C++20 modules, and a build system that isn't Cmake.

#### What this this tool should do:
It scans the path you provide to find module file, then builds a graph so the tool can run Clang precompile in the right order.