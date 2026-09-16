/*Tests
	Minimal testing framework
	testCase: a single of a test, tests a single function with a single input and output
	testReturn: a collection of testCase of the same function
	printTestReturn() prints in a beatifully formatted way the result of a test

	Each funtions of the project is tested with its own test funntion

	Hey this shit is empty!
	Someday I will complete this, just add to the TODO
*/

#include <string>
#include <vector>

template<typename input, typename output>
struct testCase {
    input testInput;
    output testExpectedOutput;
    output testAcutalOutput;

    [[nodiscard]] bool pass() const {
        return testExpectedOutput == testAcutalOutput;
    }
};

template<typename input, typename output>
struct testReturn {
    std::string testedFunction;
    std::vector<testCase<input, output>> cases;

    [[nodiscard]] bool allPassed() const {
        for(const auto& currentCase : cases) {
            if(not currentCase.pass()) {
                return false;
            }
        }
        return true;
    }
};

void printTestReturn() {

}


int main() {
	return 0;
}
