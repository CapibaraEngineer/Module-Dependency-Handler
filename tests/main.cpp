/*Tests
	Minimal testing framework
	testCase: a single of a test, tests a single function with a input and output
	testReturn: a collection of testCases of the same function
	printTestReturn() prints in a beatifully formatted way the result of a test
    printTestCase()) prints in a beatifully formatted way a single test case

    I just noticed this gets kinda stupid with function overloading. I think for those we should just do some function name mangling manually.

	Each funtions of the project is tested with its own test funntion

	Hey this shit is empty!
	Someday I will complete this, just add to the TODO
*/
/*

#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
#include <regex>

#include "parsing.hpp"

template<typename output, typename... input>
struct testCase {
    std::tuple<input...> testInput;
    output testExpectedOutput;
    output testAcutalOutput;

    [[nodiscard]] bool pass() const {
        return testExpectedOutput == testAcutalOutput;
    }
};

template<typename output, typename... input>
struct testReturn {
    std::string testedFunction;
    std::vector<testCase<output, input...>> cases;

    [[nodiscard]] bool allPassed() const {
        for(const auto& currentCase : cases) {
            if(not currentCase.pass()) {
                return false;
            }
        }
        return true;
    }
};

template<typename value>
void printTestValue(const value& valueToPrint) {
    if constexpr(requires(std::ostream& stream) { stream << valueToPrint; }) {
        std::cout << valueToPrint;
    } else if constexpr(requires { valueToPrint.has_value(); valueToPrint.value(); }) {
        if(valueToPrint.has_value()) {
            printTestValue(valueToPrint.value());
        } else {
            std::cout << "<empty>";
        }
    } else {
        std::cout << "<unprintable>";
    }
}

template<typename output, typename... input>
void printTestCase(const testCase<output, input...>& currentCase, size_t caseNumber = 0) {
    const bool passed = currentCase.pass();
    std::cout << "  Case " << caseNumber << ": " << (passed ? "PASS" : "FAIL") << '\n';

    if(not passed) {
        std::cout << "    Expected: ";
        printTestValue(currentCase.testExpectedOutput);
        std::cout << "\n    Actual:   ";
        printTestValue(currentCase.testAcutalOutput);
        std::cout << '\n';
    }
}

template<typename output, typename... input>
void printTestReturn(const testReturn<output, input...>& returnedTest) {
    const size_t passedCases = [&returnedTest] { //lambda bs so this variable is const
        size_t count = 0;
        for(const auto& currentCase : returnedTest.cases) {
            count += currentCase.pass();
        }
        return count;
    }();

    std::cout << returnedTest.testedFunction << ": "
              << (returnedTest.allPassed() ? "PASS" : "FAIL") << '\n';
    for(size_t i = 0; i < returnedTest.cases.size(); ++i) {
        printTestCase(returnedTest.cases.at(i), i + 1);
    }
    std::cout << "  " << passedCases << '/' << returnedTest.cases.size()
              << " cases passed\n";
}


testReturn<std::optional<std::string>, std::string, std::regex> testGetRegexMatch() {
    testReturn<std::optional<std::string>, std::string, std::regex> returnedTest;
    returnedTest.testedFunction = "getRegexMatch";

    static const std::vector<std::optional<std::string>> expectedReturns = {
        "core",
        "partition",
        std::nullopt,
        "",
    };

    static const std::vector<std::tuple<std::string, std::regex>> testInputs = {
        {"export module core;", std::regex(R"(export module (\w+);)")},
        {"export module:partition;", std::regex(R"(export (\w+):(\w+);)")},
        {"import core;", std::regex(R"(export module (\w+);)")},
        {"", std::regex(R"(()")},
    };

    //I had to make this a norma for loop to use 'i' to acess both vector. If there is a way to this with range base loop, it would be nice. TODO
    for(size_t i = 0; i < testInputs.size(); i++) {
        testCase<std::optional<std::string>, std::string, std::regex> currentCase;
        currentCase.testAcutalOutput = getRegexMatch(std::get<0>(testInputs.at(i)), std::get<1>(testInputs.at(i)));
        currentCase.testExpectedOutput = expectedReturns.at(i);
        returnedTest.cases.push_back(currentCase);
    }

    return returnedTest;
}

*/
int main() {
	return 0;
}
