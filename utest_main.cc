#define UTEST_DECLARE_MAIN
#include "utest.h"

int main(int argc, char** argv)
{
    return utest::suite::run(argc, argv);
}