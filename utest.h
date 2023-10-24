#pragma once

#include <cstdio>
#include <string>
#include <array>
#include <concepts>
#include <filesystem>

// ------------------------------------------ HELPER MACROS

#define STR2(x) #x
#define STR(x) STR2(x)
#define HLINE_BOLD "////////////////////////////////////"
#define HLINE      "------------------------------------"

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
    inline std::string to_string(const char* const value) { return std::string(value); }

    template <range_like Range>
    std::string to_string(const Range& range)
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
    std::string join(const Range& range, std::string_view sep = ", ")
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
        passed,
        everything
    };
    static constexpr verbosity default_verbosity = verbosity::failures;
    static constexpr const char* default_sourceroot = "";

    struct fixture;

    struct suite
    {
        struct config
        {
            static verbosity verbosity;
            static std::filesystem::path source_root;
        };

        static fixture* begin;
        static fixture* end;
        static fixture* current;

        static int runall();
        static int run(int argc, char** argv);

        static std::string ez_file(const char* filepath)
        {
            std::filesystem::path fp(filepath);
            return std::filesystem::relative(fp, config::source_root).string();
        }
    };

    // ------------------------------------------ BASE TEST DEFINITION

    struct fixture
    {
        struct {
            std::array<const char*, 32> names = { "main" };
            int current = 0;
        } sections;

        mutable bool section_changed = true;
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
            if (suite::config::verbosity > verbosity::quiet)
                printf(HLINE_BOLD " Running '%s' test cases\n", name());
        }
        void teardown()
        {
            if (errors == 0)
                printf(HLINE_BOLD " '%s' tests passed [%d/%d]\n"
                    , name()
                    , (cases - errors), cases);
            else
                printf(HLINE_BOLD " '%s' tests failed [%d/%d] %d cases didn't pass\n"
                    , name()
                    , (cases - errors), cases
                    , errors);
        }

        void push_section(const char* name) { section_changed = true; sections.current++; sections.names[sections.current] = name; }
        void pop_section() { section_changed = true; sections.current--; }
        void add_case() { cases++; }

        void print_section() const
        {
            if (!section_changed)
                return;

            section_changed = false;
            const auto name_range = make_range(sections.names.begin(), sections.names.begin() + sections.current + 1);
            const auto sectionString = join(name_range, ".");
            printf("\n> Section '%s'\n", sectionString.c_str());
            printf(HLINE "\n");
        }

        void print_case_header(bool success, const char* location) const
        {
            printf("[%d] -> %s %s\n"
                , caseindex
                , success ? "Success" : "Failure"
                , location);
        }

        void print_case_expression(const char* op, const char* left, const char* right)
        {
            printf("\t~~ While evaluating:\n\t\t\"%s\"\n\t\t\t%s\n\t\t\"%s\"\n\n", left, op, right);
        }

        void print_case_evaluation(const char* left, const char* right)
        {
            printf("\t~~ Left: %s\n\t~~ Right: %s\n"
                , left
                , right);
        }

        void add_result(bool success
            , const char* location
            , const char* op
            , const char* left_expression, const char* right_expression
            , const char* left_evaluated, const char* right_evaluated)
        {
            if (!success)
                errors++;

            if (suite::config::verbosity > verbosity::quiet)
            {
                if (!success || (suite::config::verbosity >= verbosity::passed))
                {
                    print_section();
                    if (!success) printf("\n");
                    print_case_header(success, location);
                    if (!success || (suite::config::verbosity >= verbosity::everything))
                    {
                        print_case_expression(op, left_expression, right_expression);
                        print_case_evaluation(left_evaluated, right_evaluated);
                        printf("\n");
                    }
                }
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
    std::filesystem::path suite::config::source_root = default_sourceroot;
    fixture* suite::begin = nullptr;
    fixture* suite::end = nullptr;
    fixture* suite::current = nullptr;

    int suite::runall()
    {
        int numpassed = 0;
        int numtests = 0;
        int numcases = 0;
        int numerrors = 0;

        if (begin == end)
        {
            auto fixture = begin;
            fixture->setup();
            fixture->run();
            fixture->teardown();
            numtests++;
            numcases += fixture->cases;
            numerrors += fixture->errors;
            if (fixture->errors == 0)
                numpassed++;
        } else {
            for (auto fixture = begin; fixture != end; fixture = fixture->next_test)
            {
                fixture->setup();
                fixture->run();
                fixture->teardown();
                numtests++;
                numcases += fixture->cases;
                numerrors += fixture->errors;
                if (fixture->errors == 0)
                    numpassed++;
            }
        }

        printf(HLINE_BOLD " %d tests (%d passed), %d cases (%d passed)\n"
            , numtests, numpassed
            , numcases, numcases - numerrors);
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
                if (!strcmp(argv[i], "passed")) { suite::config::verbosity = verbosity::passed; }
                if (!strcmp(argv[i], "everything")) { suite::config::verbosity = verbosity::everything; }
            }

            if ((!strcmp(argv[i], "--source_root") || !strcmp(argv[i], "-s")) && i + 1 < argc)
            {
                i++;
                suite::config::source_root = argv[i];
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
#define __TEST_STR(value) ("(" + utest::to_string(value) + ")")
#define __TEST_END() }
#define __TEST_FILE() utest::suite::ez_file(__FILE__)
#define __TEST_LOCATION() std::string(__TEST_FILE() + std::string(":" STR(__LINE__)))

// ------------------------------------------ TEST MACROS, PUBLIC

#define test_op(left, right, op, invop)                             \
    __TEST_BEGIN();                                                 \
    __TEST_CURRENT.add_result(                                      \
          utest::compare(left, right,                               \
            [](const auto& l, const auto& r) { return l op r; })    \
        , __TEST_LOCATION().c_str()                                 \
        , STR(op), STR(left), STR(right)                            \
        , __TEST_STR(left).c_str(), __TEST_STR(right).c_str()       \
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
