#include "IOExtension.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/catch_message.hpp>

#include <sstream>
#include <filesystem>
#include <fstream>
#include <atomic>

using namespace OpenGLFramework::IOExtension;
static std::atomic<int> g_tempFileID{0};

TEST_CASE("LogErrorTest")
{
    std::stringstream str;
    std::string errorStr = "To be or not to be, this is a question.";
    LogError(errorStr, str);
    std::string errorInfo = str.str();
    REQUIRE_THAT(errorInfo, Catch::Matchers::ContainsSubstring(errorStr));
    std::cout << "Check log error manually : " + errorStr << std::endl;
}

TEST_CASE("LogStatusTest")
{
    std::ifstream fin;
    SECTION("NormalStream")
    {
        REQUIRE_NOTHROW(LogStreamStatus(fin));
    }

    SECTION("FailStream")
    {
        fin.setstate(fin.failbit);
        REQUIRE_THROWS_AS(LogStreamStatus(fin), std::runtime_error);
    }

    SECTION("BadStream")
    {
        fin.setstate(fin.badbit);
        REQUIRE_THROWS_AS(LogStreamStatus(fin), std::runtime_error);
    }
}

TEST_CASE("ReadAllTest")
{
    std::string fileName = "OPENGLFRAMEWORK_IOEXTENSION_TEST_temp" + std::to_string(g_tempFileID++);
    std::cout << "Try to read wrongly.\n";
    REQUIRE(ReadAll(fileName).empty());

    std::string testStr = "No God Please No!";
    
    std::ofstream fout{ fileName };
    fout << testStr;
    fout.close();

    REQUIRE(testStr == ReadAll(fileName));

    std::filesystem::remove(fileName);
}