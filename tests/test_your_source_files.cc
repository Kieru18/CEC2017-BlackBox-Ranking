#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

int add(int a, int b) {
    return a + b;
}

BOOST_AUTO_TEST_SUITE(TestSuiteAdd)

BOOST_AUTO_TEST_CASE(TestAddition) {
    BOOST_CHECK(add(2, 2) == 4);
    BOOST_CHECK(add(1, 1) != 3);
}

BOOST_AUTO_TEST_SUITE_END()
