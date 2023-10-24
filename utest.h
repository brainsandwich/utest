#pragma once

#include <cstdio>
#include <string>
#include <array>
#include <concepts>

// ------------------------------------------ HELPER MACROS

#define STR2(x) #x
#define STR(x) STR2(x)
#define HLINE "------------------------------------"

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

    template <typename Iter> constexpr auto make_range(Iter first, Iter last) { return range(first, last); }
    template <range_like Range> constexpr auto make_range(const Range& range) { return range(std::begin(range), std::end(range)); }
    template <range_like Range> constexpr auto make_range(Range& range) { return range(std::begin(range), std::end(range)); }

    // ------------------------------------------ STRING HELPERS

    template <typename T> inline std::string to_string(const T& value) { return std::to_string(value); }
    inline std::string to_string(const char*& value) { return std::string(value); }

    template <range_like Range>
    std::string to_string(const Range& range, std::string_view sep = ", ")
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

    template <typename Left, typename Right, typename Fn>
    bool compare(const Left& left, const Right& right, Fn&& predicate)
    {
        return predicate(left, right);
    }

    template <range_like Left, range_like Right, typename Fn>
    bool compare(const Left& left, const Right& right, Fn&& predicate)
    {
        auto l = std::begin(left);
        auto r = std::begin(right);
        while (true)
        {
            if (!compare(*l, *r, predicate))
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
        everything
    };
    static constexpr verbosity default_verbosity = verbosity::failures;

    struct fixture;

    struct suite
    {
        struct config
        {
            static verbosity verbosity;
        };

        static fixture* begin;
        static fixture* end;
        static fixture* current;

        static int runall();
        static int run(int argc, char** argv);
    };

    // ------------------------------------------ BASE TEST DEFINITION

    struct fixture
    {
        struct {
            std::array<const char*, 32> names = { "main" };
            int current = 0;
        } sections;

        bool sectionchanged = true;
        int cases = 0;
        int caseindex = 0;
        int errors = 0;
        fixture* next_test = nullptr;

        fixture()
        {
            if (!suite::begin || !suite::end)
            {
                suite::begin = this;
                suite::end = this;
                suite::current = this;
            } else {
                suite::end->next_test = this;
                suite::end = this;
            }
        }

        virtual const char* name() const = 0;
        virtual void run() = 0;

        void setup()
        {
            printf(HLINE "\n");
            printf("~~~~~~~ Running '%s' test cases\n", name());
        }
        void teardown()
        {
            printf("[%d/%d]", (cases - errors), cases);
            if (errors == 0)
                printf(" All cases passed\n");
            else
                printf(" %d cases failed !\n", errors);
            printf(HLINE "\n");
        }

        void print_section()
        {
            if (!sectionchanged)
                return;

            sectionchanged = false;
            const auto sectionString = to_string(make_range(sections.names.begin(), sections.names.begin() + sections.current + 1), ".");
            printf("> Section '%s'\n", sectionString.c_str());
        }
        void push_section(const char* name) { sectionchanged = true; sections.current++; sections.names[sections.current] = name; }
        void pop_section() { sectionchanged = true; sections.current--; }
        void add_case() { cases++; }
        void add_result(bool success, const char* location, const char* condexpr, const char* condmsg)
        {
            if (suite::config::verbosity != verbosity::quiet)
                print_section();

            if (success)
            {
                if (suite::config::verbosity == verbosity::everything)
                    printf("\t%d - %s : \"%s\" -> Success\n", caseindex, location, condexpr);
            } else {
                errors++;
                if (suite::config::verbosity != verbosity::quiet)
                    printf("\t%d - %s : \"%s\" -> Failure: \"%s\"\n"
                        , caseindex, location
                        , condexpr, condmsg);
            }
            caseindex++;
        }
    };

    // ------------------------------------------ TEST SECTION

    struct section
    {
        section(const char* name) { suite::current->push_section(name); }
        ~section() { suite::current->pop_section(); }
        operator bool() const { return true; }
    };

    // ------------------------------------------ TEST SUITE IMPL

    verbosity suite::config::verbosity = default_verbosity;
    fixture* suite::begin = nullptr;
    fixture* suite::end = nullptr;
    fixture* suite::current = nullptr;

    int suite::runall()
    {
        int numtests = 0;
        int numcases = 0;
        int numerrors = 0;

        printf(HLINE "\n");
        printf("Begin running unit tests\n");

        if (begin == end)
        {
            auto fixture = begin;
            fixture->setup();
            fixture->run();
            fixture->teardown();
            numtests++;
            numcases += fixture->cases;
            numerrors += fixture->errors;
        } else {
            for (auto fixture = begin; fixture != end; fixture = fixture->next_test)
            {
                fixture->setup();
                fixture->run();
                fixture->teardown();
                numtests++;
                numcases += fixture->cases;
                numerrors += fixture->errors;
            }
        }

        printf("%d test(s) ran\n", numtests);
        printf("[%d/%d] case(s) passed (%d errors)\n"
            , numcases - numerrors
            , numcases
            , numerrors);
        printf(HLINE "\n");
        return numerrors;
    }

    int suite::run(int argc, char** argv)
    {
        for (int i = 0; i < argc; i++)
        {
            if ((!strcmp(argv[i], "--verbosity") || !strcmp(argv[i], "-v")) && i + 1 < argc)
            {
                i++;
                if (!strcmp(argv[i], "quiet")) { suite::config::verbosity = verbosity::quiet; }
                if (!strcmp(argv[i], "failures")) { suite::config::verbosity = verbosity::failures; }
                if (!strcmp(argv[i], "everything")) { suite::config::verbosity = verbosity::everything; }
            }
        }
        return runall();
    }
}

// ------------------------------------------ TEST MACROS, DEFINITION

#define test_define(_name)                                          \
    struct _name ## _fixture : utest::fixture                       \
    {                                                               \
        void run() override;                                        \
        const char* name() const override { return STR(_name); }    \
    } _name ## _fixture_instance;                                   \
    void _name ## _fixture::run()

// ------------------------------------------ TEST MACROS, PRIVATE

#define __TEST_CURRENT (*utest::suite::current)
#define __TEST_BEGIN() { __TEST_CURRENT.add_case()
#define __TEST_STR(value) "(" + utest::to_string(value) + ")"
#define __TEST_CHECK(condition, location, condexpr, condmsg) { __TEST_CURRENT.add_result((condition), location, condexpr, condmsg);  }
#define __TEST_END() }

// ------------------------------------------ TEST MACROS, PUBLIC

#define test_op(left, right, op, invop)                             \
    __TEST_BEGIN();                                                 \
    __TEST_CHECK(                                                   \
          utest::compare(left, right,                               \
            [](const auto& l, const auto& r) { return l op r; })    \
        , __FILE__ ":" STR(__LINE__)                                \
        , STR(left op right)                                        \
        , (__TEST_STR(left) + std::string(" " STR(invop) " ") + __TEST_STR(right)).c_str() \
    );                                                              \
    __TEST_END()

#define test_eq(left, right) test_op(left, right, ==, !=)
#define test_ne(left, right) test_op(left, right, !=, ==)
#define test_gt(left, right) test_op(left, right, >, <=)
#define test_ge(left, right) test_op(left, right, >=, <)
#define test_lt(left, right) test_op(left, right, <, >=)
#define test_le(left, right) test_op(left, right, <=, >)

#define test_section(name) if (const auto s = utest::section(name))

#if !defined(UTEST_NODECLARE_MAIN)
    int main(int argc, char** argv)
    {
        return utest::suite::run(argc, argv);
    }
#endif