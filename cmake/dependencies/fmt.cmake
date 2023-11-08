include(CPM)

CPMAddPackage("https://github.com/fmtlib/fmt.git@10.0.0#10.0.0")

CPMAddPackage(
    NAME fmt
    GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
    GIT_TAG "10.1.1"
    VERSION "10"
)