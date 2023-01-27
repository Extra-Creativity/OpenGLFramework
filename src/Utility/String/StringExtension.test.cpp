#include "StringExtension.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_get_random_seed.hpp>

#include <limits>
#include <random>

using namespace OpenGLFramework::StringExtension;

TEST_CASE("AsciiToLowerTest")
{
    auto lowerUpperDistance = 'a' - 'A';
    SECTION("CharAsciiToLower")
    {
        for (char ch = std::numeric_limits<char>::min();
            ch < std::numeric_limits<char>::max(); ch++)
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                REQUIRE(CharAsciiToLower(ch) == ch + lowerUpperDistance);
            }
            else
            {
                REQUIRE(CharAsciiToLower(ch) == ch);
            }
        }
    }

    SECTION("StringAsciiToLower", "AllUpperAlphabets")
    {
        std::string testStr, resultStr;
        std::u8string u8testStr, u8resultStr;
        for (char lowerCh = 'a', upperCh = 'A'; upperCh < 'Z';
            lowerCh++, upperCh++)
        {
            testStr.push_back(upperCh);
            resultStr.push_back(lowerCh);

            u8testStr.push_back(static_cast<char8_t>(upperCh));
            u8resultStr.push_back(static_cast<char8_t>(lowerCh));
        }
        REQUIRE(StringAsciiToLower<char>(testStr) == resultStr);
        REQUIRE(StringAsciiToLower<char8_t>(u8testStr) == u8resultStr);
    }

    SECTION("StringAsciiToLower", "Random")
    {
        std::default_random_engine engine{ Catch::getSeed() };
        std::uniform_int_distribution<int> distribution(
            std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
        const int testNum = 100, strLen = 300;
        std::string currStr(strLen, '\0'), resultStr(strLen, '\0');

        for (int _ = 0; _ < testNum; _++)
        {
            for (int i = 0; i < static_cast<int>(currStr.size()); i++)
            {
                char ch = distribution(engine);
                if (ch >= 'A' && ch <= 'Z')
                {
                    currStr[i] = ch;
                    resultStr[i] = ch + lowerUpperDistance;
                }
                else
                    currStr[i] = resultStr[i] = ch;
            }

            REQUIRE(StringAsciiToLower<char>(currStr) == resultStr);
        }
    }
}

TEST_CASE("Trim")
{
    std::string testStr = "   \t  Luke, I'm your father.\t    ";
    REQUIRE(Trim(testStr) == "Luke, I'm your father.");
    REQUIRE(TrimBegin(testStr) == "Luke, I'm your father.\t    ");
    REQUIRE(TrimEnd(testStr) == "   \t  Luke, I'm your father.");

    std::u8string u8testStr = u8"  \t  �Ҳ�����us���������ʥѧϰ�� . \t  ";
    REQUIRE(Trim(u8testStr) == u8"�Ҳ�����us���������ʥѧϰ�� .");
    REQUIRE(TrimBegin(u8testStr) == u8"�Ҳ�����us���������ʥѧϰ�� . \t  ");
    REQUIRE(TrimEnd(u8testStr) == u8"  \t  �Ҳ�����us���������ʥѧϰ�� .");
}