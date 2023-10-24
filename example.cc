#include "utest.h"

#include <vector>
#include <array>

template <typename ... Args> constexpr auto make_array(Args&& ... args) { return std::array { args... }; }

// Create a new test category
test_define(basic)
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
