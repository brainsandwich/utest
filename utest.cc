#include "utest.h"

#include <fmt/format.h>
#include <fmt/color.h>

namespace utest
{
    // ---------------------------------------- FIXTURE

    fixture::fixture()
    {
        suite::fixtures.push_back(this);
    }

    void fixture::setup()
    {
        fmt::print("{}"
            , fmt::format(
                  fmt::fg(fmt::terminal_color::bright_blue)
                , "-- {}.{}"
                , group(), name()
            ));
    }
    void fixture::teardown()
    {
        if (!printed_something)
        {
            auto style = fmt::fg(errors == 0 ? fmt::terminal_color::green : fmt::terminal_color::bright_red);
            fmt::println(" -> {} {}"
                , fmt::format(style, "{}", errors == 0 ? "passed" : "failed")
                , fmt::format("[{}/{}]", (cases - errors), cases)
            );
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
        const auto name_range = make_range(sections.names.begin() + 1, sections.names.begin() + sections.current + 1);
        if (name_range.begin() == name_range.end())
            return;

        const auto sectionString = join(name_range, " > ");
        fmt::println("{}"
            , fmt::format(
                  fmt::fg(fmt::terminal_color::bright_blue)
                , "-- {}.{} > {}"
                , group(), name()
                , sectionString.c_str()
            ));
    }

    void fixture::print_case_header(bool success, const char* location) const
    {
        auto success_style = fmt::fg(success ? fmt::terminal_color::green : fmt::terminal_color::bright_red);
        auto location_style = fmt::fg(fmt::terminal_color::bright_black);
        fmt::println("{} {} -> {}"
            , fmt::format("[{}]", caseindex)
            , fmt::format(location_style, "{}", location)
            , fmt::format(success_style, "{}", success ? "success" : "failure")
        );
    }

    void fixture::print_case_expression(const char* op, const char* left, const char* right)
    {
        fmt::println("\t\twhile evaluating:\n\t\t\t\"{}\"\n\t\t\t\t{}\n\t\t\t\"{}\"\n", left, op, right);
    }

    void fixture::print_case_evaluation(const char* left, const char* right)
    {
        fmt::println("\t\tleft: {}\n\t\tright: {}"
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
                print_case_header(success, location);
                if (!success || (suite::config::verbosity >= verbosity::everything))
                {
                    print_case_expression(op, left_expression, right_expression);
                    print_case_evaluation(left_evaluated, right_evaluated);
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

        if (numpassed != numtests)
        {
            auto style = fmt::fg(fmt::terminal_color::bright_red);
            fmt::println(     "--------------------------");
            fmt::print(style, "-> some tests have failed: ");
            int pindex = 0;
            for (const auto& fixture: fixtures)
            {
                if (fixture->errors == 0)
                    continue;

                fmt::print("{}.{} ({})", fixture->group(), fixture->name(), fixture->errors);
                if ((numtests - numpassed) > (pindex + 2))
                    fmt::print(", ");
                else if ((numtests - numpassed) > (pindex + 1))
                    fmt::print(" & ");

                pindex++;
            }
            fmt::println("");
        }
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