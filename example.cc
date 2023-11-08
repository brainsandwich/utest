#include "utest.h"

#include <vector>
#include <array>

template <typename ... Args> constexpr auto make_array(Args&& ... args) { return std::array { args... }; }

// Create a new test category
test_define(example, basic)
{
    // Section for specific cases
    test_section("integers")
    {
        int value = 23;

        // All the operators you need !
        test_eq(value, 29);
        test_ne(value, 29);
        test_ge(value, 29);
        test_gt(value, 29);
        test_le(value, 29);
        test_lt(value, 29);
    }

    test_section("containers")
    {
        // The test operators compare element by element
        // when the used types are ranges (they have
        // std::begin() and std::end())
        const auto vector = std::vector { 1, 2, 3 };
        test_eq(vector, make_array(1, 2));
        test_eq(vector, make_array(1, 2, 3));
        test_eq(vector, make_array(1, 2, 1928));
    }
}

// utest includes fmtlib so you can use it too !
#include <fmt/format.h>

// Our custom type
struct MyType
{
    int integer;
    float number;
};

// utest uses standard comparison operators on each case
constexpr bool operator==(const MyType& left, const MyType& right)
{
    return left.integet == right.integer
        && left.number == right.number;
}

namespace utest
{
    template <>
    inline std::string to_string<MyType>(const MyType& m)
    {
        return fmt::format("[{}, {:.2f}]", m.integer, m.number);
    }
}

test_define(example, custom)
{
    MyType m { 123, 456.7f };
    test_eq(m, MyType { .integer = 123, .number = 456.7f });
}