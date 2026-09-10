// Minimal dependency-free test driver.
// No external framework: keeps the template usable offline.
// Each check prints [PASS]/[FAIL]; the process exits non-zero on failure.
#include "example.hpp"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool cond, const std::string& name) {
	if (cond) {
		std::cout << "[PASS] " << name << "\n";
	} else {
		std::cout << "[FAIL] " << name << "\n";
		++failures;
	}
}

} // namespace

int main() {
	check(add(2, 3) == 5, "add(2, 3) == 5");
	check(add(-1, 1) == 0, "add(-1, 1) == 0");
	check(add(0, 0) == 0, "add(0, 0) == 0");

	ExampleStruct e;
	check(e.x == 0 && e.c == 'a', "ExampleStruct defaults");

	if (failures == 0) {
		std::cout << "All tests passed.\n";
	} else {
		std::cout << failures << " test(s) failed.\n";
	}
	return failures == 0 ? 0 : 1;
}
