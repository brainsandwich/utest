#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <concepts>
#include <filesystem>

// ------------------------------------------ HELPER MACROS

#define STR2(x) #x
#define STR(x) STR2(x)

namespace utest
{
    // ------------------------------------------ CONCEPTS

    template <typename Range>
    concept range_like = requires(const Range& r)
    {
        std::begin(r);
        std::end(r);
    };

    template <typename Iter>
    struct range
    {
        Iter first;
        Iter last;

        range(Iter first, Iter last)
            : first(first)
            , last(last) {}

        Iter begin() { return first; }
        Iter end() { return last; }
        Iter begin() const { return first; }
        Iter end() const { return last; }
    };

    template <range_like Range>
    using iterator_type_t = decltype(std::begin(std::declval<Range&>()));

    template <typename Iter> static constexpr auto make_range(Iter first, Iter last) { return range(first, last); }
    template <range_like Range> static constexpr auto make_range(const Range& range) { return range(std::begin(range), std::end(range)); }
    template <range_like Range> static constexpr auto make_range(Range& range) { return range(std::begin(range), std::end(range)); }

    // ------------------------------------------ STRING HELPERS

    template <typename T> inline std::string to_string(const T& value) { return std::to_string(value); }
    static inline std::string to_string(const char* const value) { return std::string(value); }

    template <range_like Range>
    static std::string to_string(const Range& range)
    {
        static constexpr const char* sep = ", ";
        std::string result;
        result.reserve(256);

        std::ptrdiff_t item_count = 0;
        for (auto it = std::begin(range); it != std::end(range); it++)
            item_count++;

        std::ptrdiff_t item_index = 0;
        for (auto it = std::begin(range); it != std::end(range); it++)
        {
            result += to_string(*it);
            if (item_index < item_count - 1)
            {
                result += sep;
            }
            item_index++;
        }
        return result;
    }

    template <range_like Range>
    static std::string join(const Range& range, std::string_view sep = ", ")
    {
        std::string result;
        result.reserve(256);
        for (auto it = std::begin(range); it != std::end(range); it++)
        {
            result += to_string(*it);
            result += sep;
        }
        if (result.size() > 3)
            result.resize(result.size() - sep.size());
        return result;
    }

    // ------------------------------------------ COMPARISON

    enum class comparison_type
    {
        equal,
        not_equal,
        greater_than,
        greater_equal,
        less_than,
        less_equal
    };

    template <typename Left, typename Right> static bool compare_equal(const Left& left, const Right& right) { return left == right; }
    template <typename Left, typename Right> static bool compare_not_equal(const Left& left, const Right& right) { return left != right; }
    template <typename Left, typename Right> static bool compare_greater_than(const Left& left, const Right& right) { return left > right; }
    template <typename Left, typename Right> static bool compare_greater_equal(const Left& left, const Right& right) { return left >= right; }
    template <typename Left, typename Right> static bool compare_less_than(const Left& left, const Right& right) { return left < right; }
    template <typename Left, typename Right> static bool compare_less_equal(const Left& left, const Right& right) { return left <= right; }

    template <comparison_type Comp, typename Left, typename Right>
    static bool compare(const Left& left, const Right& right)
    {
        switch (Comp)
        {
            case comparison_type::equal: return compare_equal(left, right);
            case comparison_type::not_equal: return compare_not_equal(left, right);
            case comparison_type::greater_than: return compare_greater_than(left, right);
            case comparison_type::greater_equal: return compare_greater_equal(left, right);
            case comparison_type::less_than: return compare_less_than(left, right);
            case comparison_type::less_equal: return compare_less_equal(left, right);
        }
        return false;
    }

    template <comparison_type Comp, range_like Left, range_like Right>
    static bool compare(const Left& left, const Right& right)
    {
        auto l = std::begin(left);
        auto r = std::begin(right);
        while (true)
        {
            if (!compare<Comp>(*l, *r))
                return false;
            
            l++;
            r++;

            bool leftend = l == std::end(left);
            bool rightend = r == std::end(right);
            if (leftend != rightend)
                return false;
            if (leftend || rightend)
                break;
        }
        return true;
    }

    // ------------------------------------------ TEST SUITE AND CONFIG

    enum class verbosity
    {
        quiet,
        failures,
        passed,
        everything
    };

    struct fixture;

    struct suite
    {
        struct config
        {
            static verbosity verbosity;
            static std::filesystem::path source_root;
        };

        static std::vector<fixture*> fixtures;
        static fixture* current;

        static int runall();
        static int run(int argc, char** argv);
        static std::string ez_file(const char* filepath);
    };

    // ------------------------------------------ BASE TEST DEFINITION

    struct fixture
    {
        struct
        {
            std::array<const char*, 32> names = { "main" };
            int current = 0;
        } sections;

        mutable bool section_changed = true;
        bool printed_something = false;
        int cases = 0;
        int caseindex = 0;
        int errors = 0;
        fixture* next_test = nullptr;

        fixture();

        void setup();
        void teardown();

        void push_section(const char* name);
        void pop_section();
        void add_case();

        void print_section() const;
        void print_case_header(bool success, const char* location) const;
        void print_case_expression(const char* op, const char* left, const char* right);
        void print_case_evaluation(const char* left, const char* right);

        void add_result(
              bool success
            , const char* location
            , const char* op
            , const char* left_expression, const char* right_expression
            , const char* left_evaluated, const char* right_evaluated);

        virtual const char* name() const = 0;
        virtual const char* group() const = 0;
        virtual void run() = 0;
    };

    // ------------------------------------------ TEST SECTION

    struct section
    {
        section(const char* name);
        ~section();
        operator bool() const;
    };
}

// ------------------------------------------ TEST MACROS, DEFINITION

#define test_define(_group, _name)                                  \
    struct _group ## _ ## _name ## _fixture : utest::fixture        \
    {                                                               \
        void run() override;                                        \
        const char* name() const override { return STR(_name); }    \
        const char* group() const override { return STR(_group); }  \
    } _group ## _ ## _name ## _fixture_instance;                    \
    void _group ## _ ## _name ## _fixture::run()

// ------------------------------------------ TEST MACROS, PRIVATE

#define __TEST_CURRENT (*utest::suite::current)
#define __TEST_BEGIN() { __TEST_CURRENT.add_case()
#define __TEST_STR(value) ("(" + utest::to_string(value) + ")")
#define __TEST_END() }
#define __TEST_FILE() utest::suite::ez_file(__FILE__)
#define __TEST_LOCATION() std::string(__TEST_FILE() + std::string(":" STR(__LINE__)))

// ------------------------------------------ TEST MACROS, PUBLIC

#define test_op(left, right, opsymbol, opmode)                          \
    __TEST_BEGIN();                                                     \
    __TEST_CURRENT.add_result(                                          \
          utest::compare<opmode>(left, right)                           \
        , __TEST_LOCATION().c_str()                                     \
        , STR(opsymbol), STR(left), STR(right)                          \
        , __TEST_STR(left).c_str(), __TEST_STR(right).c_str()           \
    );                                                                  \
    __TEST_END()

#define test_eq(left, right) test_op(left, right, ==, utest::comparison_type::equal)
#define test_ne(left, right) test_op(left, right, !=, utest::comparison_type::not_equal)
#define test_gt(left, right) test_op(left, right, >, utest::comparison_type::greater_than)
#define test_ge(left, right) test_op(left, right, >=, utest::comparison_type::greater_equal)
#define test_lt(left, right) test_op(left, right, <, utest::comparison_type::less_than)
#define test_le(left, right) test_op(left, right, <=, utest::comparison_type::less_equal)

#define test_section(name) if (const auto s = utest::section(name))