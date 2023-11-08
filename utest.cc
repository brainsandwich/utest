#include "utest.h"

#include <fmt/format.h>
#include <fmt/color.h>

#define HLINE_BOLD "////////////////////////////////////"
#define HLINE      "------------------------------------"

namespace utest
{
    // ---------------------------------------- FIXTURE

    fixture::fixture()
    {
        suite::fixtures.push_back(this);
    }

    void fixture::setup()
    {
        if (suite::config::verbosity > verbosity::quiet)
            fmt::print("{}", fmt::format(fmt::emphasis::bold, HLINE_BOLD " Running '{}.{}' test cases", group(), name()));
            // printf(HLINE_BOLD " Running '%s.%s' test cases", group(), name());
    }
    void fixture::teardown()
    {
        if (printed_something)
        {
            if (errors == 0)
            {
                fmt::println("{}", fmt::format(fmt::fg(fmt::terminal_color::green)
                    , "\n" HLINE_BOLD " '{}.{}' tests passed [{}/{}]"
                    , group()
                    , name()
                    , (cases - errors), cases));
            }
            else
            {
                fmt::println("{}", fmt::format(fmt::fg(fmt::terminal_color::red)
                    , "\n" HLINE_BOLD " '{}.{}' tests failed [{}/{}] {} cases didn't pass"
                    , group()
                    , name()
                    , (cases - errors), cases
                    , errors));
            }
        } else {
            if (errors == 0)
            {
                fmt::println("{}", fmt::format(fmt::fg(fmt::terminal_color::green)
                    , " -> passed [{}/{}]", (cases - errors), cases));
            }
            else
            {
                fmt::println("{}", fmt::format(fmt::fg(fmt::terminal_color::red)
                    , " -> failed [{}/{}]", (cases - errors), cases));
            }
        }
    }

    void fixture::push_section(const char* name) { section_changed = true; sections.current++; sections.names[sections.current] = name; }
    void fixture::pop_section() { section_changed = true; sections.current--; }
    void fixture::add_case() { cases++; }

    void fixture::print_section() const
    {
        if (!section_changed)
            return;

        section_changed = false;
        const auto name_range = make_range(sections.names.begin(), sections.names.begin() + sections.current + 1);
        const auto sectionString = join(name_range, ".");
        fmt::println("\n> Section {}", sectionString.c_str());
        fmt::println(HLINE);
    }

    void fixture::print_case_header(bool success, const char* location) const
    {
        fmt::println("{}", fmt::format(fmt::fg(success ? fmt::terminal_color::black : fmt::terminal_color::red)
            , "[{}] -> {} {}"
            , caseindex
            , success ? "Success" : "Failure"
            , location));
    }

    void fixture::print_case_expression(const char* op, const char* left, const char* right)
    {
        fmt::println("\t~~ While evaluating:\n\t\t\"%s\"\n\t\t\t%s\n\t\t\"%s\"\n", left, op, right);
    }

    void fixture::print_case_evaluation(const char* left, const char* right)
    {
        fmt::println("\t~~ Left: %s\n\t~~ Right: %s"
            , left
            , right);
    }

    void fixture::add_result(bool success
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
                if (!printed_something)
                {
                    fmt::println("");
                    printed_something = true;
                }
                print_section();
                if (!success)
                    fmt::println("");
                print_case_header(success, location);
                if (!success || (suite::config::verbosity >= verbosity::everything))
                {
                    print_case_expression(op, left_expression, right_expression);
                    print_case_evaluation(left_evaluated, right_evaluated);
                    fmt::println("");
                }
            }
        }
        caseindex++;
    }

    // ---------------------------------------- SECTION

    section::section(const char* name) { suite::current->push_section(name); }
    section::~section() { suite::current->pop_section(); }
    section::operator bool() const { return true; }

    // ---------------------------------------- SUITE

    verbosity suite::config::verbosity = verbosity::failures;
    std::filesystem::path suite::config::source_root = {};
    std::vector<fixture*> suite::fixtures = {};
    fixture* suite::current = nullptr;

    std::string suite::ez_file(const char* filepath)
    {
        std::filesystem::path fp(filepath);
        return std::filesystem::relative(fp, config::source_root).string();
    }

    int suite::runall()
    {
        int numpassed = 0;
        int numtests = 0;
        int numcases = 0;
        int numerrors = 0;

        for (auto fixture: fixtures)
        {
            current = fixture;
            fixture->setup();
            fixture->run();
            fixture->teardown();
            numtests++;
            numcases += fixture->cases;
            numerrors += fixture->errors;
            if (fixture->errors == 0)
                numpassed++;
        }

        fmt::println(HLINE_BOLD " {} tests ({} passed), {} cases ({} passed)"
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