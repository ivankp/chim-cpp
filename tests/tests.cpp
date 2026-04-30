#include "src/strings.hpp"

#include "test.hpp"

using strings::cat;

TEST(NoArgs) {
    auto s = cat();
    TEST_EQ(s, "");
    TEST_EQ(s.size(), 0);
}

TEST(OneArgString) {
    auto s = cat("hello");
    TEST_EQ(s, "hello");
    TEST_EQ(s.size(), 5);
}

TEST(OneArgChar) {
    auto s = cat('C');
    TEST_EQ(s, "C");
    TEST_EQ(s.size(), 1);
}

TEST(OneArgBool) {
    {
        auto s = cat(true);
        TEST_EQ(s, "true");
        TEST_EQ(s.size(), 4);
    } {
        auto s = cat(false);
        TEST_EQ(s, "false");
        TEST_EQ(s.size(), 5);
    }
}

TEST(OneArgNumbers) {
    {
        auto s = cat(1);
        TEST_EQ(s, "1");
        TEST_EQ(s.size(), 1);
    } {
        auto s = cat(1234567890);
        TEST_EQ(s, "1234567890");
        TEST_EQ(s.size(), 10);
    } {
        auto s = cat(2.71828);
        TEST_EQ(s, "2.71828");
        TEST_EQ(s.size(), 7);
    } {
        auto s = cat(-2.2250738585072020E-308);
        TEST_EQ(s, "-2.225073858507202e-308");
        TEST_EQ(s.size(), 23);
    }
}

TEST(MultipleArgs) {
    {
        auto s = cat("Hello", ',', ' ', "World", '!');
        TEST_EQ(s, "Hello, World!");
        TEST_EQ(s.size(), 13);
    } {
        auto s = cat(3.14, " is not equal to ", 4);
        TEST_EQ(s, "3.14 is not equal to 4");
        TEST_EQ(s.size(), 22);
    }
}
