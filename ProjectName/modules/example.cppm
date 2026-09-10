// C++23 named-module example.
//
// Kept dependency-free on purpose: `import std;` needs a prebuilt standard
// library module (gcm.cache/std.gcm) and extra setup, so the default build
// uses only the global fragment. See README.md for the `import std` notes.
module;

#include <iostream>

export module example;

export void print_example() {
	std::cout << "example module\n";
}
