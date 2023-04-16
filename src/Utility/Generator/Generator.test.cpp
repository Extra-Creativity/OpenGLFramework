#include "Generator.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_get_random_seed.hpp>

using namespace OpenGLFramework::Coroutine;

Generator<char> TraverseLetters()
{
    for (char c = 'A'; c <= 'Z'; c++)
    {
        co_yield c;
    }
    co_return;
}

TEST_CASE("LetterTest")
{
    char c0 = 'A';
    for (auto c: TraverseLetters())
    {
        REQUIRE(c0 == c);
        c0++;
    }
}