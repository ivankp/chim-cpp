#include <vector>
#include <utility>
#include <iostream>

std::vector<std::pair<void(*)(), const char*>> allTests;

struct RegisterTest {
    RegisterTest(void(*f)(), const char* name) {
        allTests.emplace_back(f,name);
    }
};

int main() {
    using std::cout;
    using std::endl;

    for (const auto& test : allTests) {
        cout << "\033[34m[ Test   ]\033[0m " << test.second << "\n";
        try {
            (*test.first)();
            cout << "\033[32m[ PASSED ]\033[0m " << test.second << "\n";
        } catch (const std::exception& e) {
            cout << e.what() << endl;
            cout << "\033[31m[ FAILED ]\033[0m " << test.second << "\n";
        }
    }
}

constexpr bool Within(auto a, auto b, auto tol) {
    return ((1 - tol) * b < a) && (a < (1 + tol) * b);
}

#define STR(X) STR1(X)
#define STR1(X) #X

#define TEST(NAME) \
    void TEST_##NAME(); \
    RegisterTest RegisterTest_##NAME(&TEST_##NAME, STR(TEST_##NAME)); \
    void TEST_##NAME()

#define TEST_EQ(LHS, RHS) \
    if (!( (LHS) == (RHS) )) { \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #LHS " != " #RHS); \
    }

#define TEST_NE(LHS, RHS) \
    if (!( (LHS) != (RHS) )) { \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #LHS " == " #RHS); \
    }

#define TEST_TRUE(X) \
    if (!(X)) { \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #X " is false"); \
    }

#define TEST_FALSE(X) \
    if ((X)) { \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #X " is true"); \
    }

#define TEST_FAIL \
      throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": test failed");
