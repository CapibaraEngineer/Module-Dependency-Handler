# A Modern C++ Template for Projects with Premake

A Mininal template for C++23 projects with Premake, a easy starting point.
I built this mostly for personal use, but feel free to open an issue or pull request. I appreciate feedback.
The template is licensed under the Unlicense for easy use. See [LICENSE](LICENSE)

## Layout

```text
.
├── ProjectName/             # template app (rename via configure.sh)
│   ├── include/example.hpp  # example header (declares add())
│   ├── modules/example.cppm # example named module (dependency-free)
│   └── source/              # main.cpp + example.cpp
├── tests/                   # separate ProjectNameTests binary (no framework)
│   └── test_example.cpp  
├── premake/                   
│   ├── premake5.lua         # workspace + ProjectName + ProjectNameTests
│   └── common.lua           # shared settings (C++23, warnings, Debug/Release/Sanitize)
├── bin/                     # compiled binaries (ignored by git)
├── build/                   # generated makefiles + objects (ignored by git)
├── configure.sh             # rename workspace/projects, add new projects
├── cleanup_template.sh      # one-shot template strip (self-deletes)
├── build.sh                 # premake5 + ninja wrapper (clang default)
├── build_run.sh             # build, then run the startProject binary
├── run.sh                   # run the startProject binary
├── test.sh                  # build + run all *Tests binaries
└── clean.sh                 # remove build/ and bin/ contents
```

Source code split per project: `include/` public headers, `modules/` `.cppm` named modules, `source/` implementation + `main.cpp`. 
Shared test suite lives in top-level `tests/` so the app keeps a single `main()`.

## Prerequisites

- `premake5` 5.0.0-beta8+ (`premake5 --version`), see `premake/README.md`.
- Clang 17+ (default) or GCC 14+ with C++23 support, plus `ninja`
  (`--gmake` falls back to GNU make).
- Optional: `bear` for `compile_commands.json` (clangd),
  `clang-format`, `clang-tidy`.

## Quickstart
Once you copy the template yo ucan run two shell script for setting it up.

### `./configure.sh`
- Rename ProjectName;
- Set workspace and startproject
- Optionally add more projects (ConsoleApp/StaticLib/SharedLib)

### `./cleanup_template.sh`
Cleans the template stuff, README, examples, and then self deletes along with configure.sh
Run this after configure.sh

## Utilitary Scripts
The templates comes with 5 useful scripts 
### `./build.sh`
build the project, default to clang + ninja in Debug build.
run build.sh Release for release build, -cc=gcc for gcc, --gmake for gmake
### `./run.sh`
run the startProject binary (`./build.sh` first).
program args go after `--`: `./run.sh Release -- --some-app-flag`
### `./build_run.sh`
build, then run the startProject binary.
config and build options go before `--`, program args after it:
`./build_run.sh Release --cc=gcc -- --some-app-flag`
### `./test.sh`
Build and run tests in debug build, run ./test.sh Sanitize for Sanitize build
### `./clean.sh`
Empty build/ and bin/


## Build details

- Binaries are located in bin folder:
  `bin/ProjectName/<system>_<arch>/<Config>/[binary]`
- Workspace `location` is `build/`;
- Generated makefiles stay out of the source tree. objects to `build/obj/<Proj>/...` (see `common_settings()` in `premake/common.lua`).
- Every project gets: `C++23`, `warnings "Extra"`, `staticruntime "on"`,
  In Debug builds: (`DEBUG`, no optimize, full symbols)
  In Release builds (`NDEBUG`, Link Time Optimization + optimize, no symbols)
  In Sanitize builds (like Debug but has address sanitazation and undefine behaviour sanitization;
  needs the sanitizer runtimes, e.g. `libasan libubsan`),
  `-fmodules` on GCC/Clang (required for `.cppm`),
- Tests: `ProjectNameTests` is a second `ConsoleApp` compiling `tests/**` plus the shared non-`main` sources (`source/example.cpp`, `include/example.hpp`, `modules/example.cppm`).
  It is dependency-free — just `[PASS]/[FAIL]` lines and a non-zero exit on failure.
  Swap in Catch2/doctest later if you like.
- Modules note: `modules/example.cppm` deliberately avoids `import std;` (needs a prebuilt `std.gcm/pcm`).
  Once your toolchain provides it, you can  switch the global-fragment `#include` to `import std;`.
  Note: Premake's `gmake` backend currently ignores `.cppm` files (app/tests still build from `.cpp`/`.hpp`); VS/Ninja handle them.
  Validate the module alone with `g++ -std=c++23 -fmodules -fsyntax-only ProjectName/modules/example.cppm`.

## Adding projects

Re-run `./configure.sh` and answer `y` at `Create another project?`.
It creates `Name/{include,modules,source}/` and appends a block using `common_settings(rootDir)` for no duplicated per-config filters.

## Hygiene

- `.gitignore` covers `bin/`, `build/`, VS/make artifacts, `gcm.cache/`.
- `.editorconfig`, `.clang-format` (LLVM, tabs width 4, 100 col),
  `.clang-tidy` (diagnostic/analyzer/modernize/readability/performance/bugprone).
- `clang-format -i` / `clang-tidy` your sources before committing.

## License

Unlicense — public domain, see [LICENSE](LICENSE).
