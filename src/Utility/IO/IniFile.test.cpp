#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "IniFile.h"

#include <stdio.h>
#include <filesystem>
using namespace OpenGLFramework::IOExtension;

static std::atomic<int> g_tempFileID = 0;
static std::string fileNamePrefix = "OPENGLFRAMEWORK_IOEXTENSION_INIFILE_temp";

TEST_CASE("WrongIni")
{
    SECTION("FileNotExist")
    {
        REQUIRE_THROWS_AS(
            []() {
                IniFile file{ fileNamePrefix + std::to_string(g_tempFileID++) };
                return file.rootSection.GetEntrySize() == 0;
            }(), std::runtime_error
        );
    }
    std::cout << std::endl;

    SECTION("FileFormatWrong")
    {
        std::string fileName = fileNamePrefix + std::to_string(g_tempFileID++);
        std::ofstream fout{ fileName };
        SECTION("NoKey")
        {
            fout << "[NoKeyTest]\n\t   = oops!";
            fout.close();
            REQUIRE_THROWS_AS(
                [&fileName]() {
                    IniFile file{ fileName };
                }(), FormatError);
        }
        std::cout << std::endl;
        SECTION("NoSectionName")
        {
            fout << "[  \t\t  ]";
            fout.close();
            REQUIRE_THROWS_AS(
                [&fileName]() {
                    IniFile file{ fileName };
                }(), FormatError);
        }
        std::cout << std::endl;
        SECTION("NoNameBetweenDotsInSection")
        {
            fout << "[..test]";
            fout.close();
            REQUIRE_THROWS_AS(
                [&fileName]() {
                    IniFile file{ fileName };
                }(), FormatError);
        }
        std::cout << std::endl;
        SECTION("NoEqualSignInEntry")
        {
            fout << "[test]\naaaa";
            fout.close();
            REQUIRE_THROWS_AS(
                [&fileName]() {
                    IniFile file{ fileName };
                }(), FormatError);
        }
    }
    std::cout << std::endl;
}

TEST_CASE("IniRequirements")
{
    SECTION("Simple")
    {
        std::string fileName = fileNamePrefix + std::to_string(g_tempFileID++);
        std::ofstream fout{ fileName };
        fout << "[\t grade  ]\n\t math=98\nchinese = 100\t ";
        fout.close();

        IniFile file{ fileName };
        auto section = file.rootSection.GetSubsection("grade");
        REQUIRE(section.has_value());

        auto& realSection = section->get();
        REQUIRE(realSection.GetEntrySize() == 2);
        REQUIRE(file.rootSection.GetSubsectionSize() == 1);

        auto mathGrade = realSection.GetEntry("math"),
            chineseGrade = realSection.GetEntry("chinese");
        REQUIRE(mathGrade.has_value());
        REQUIRE(chineseGrade.has_value());

        REQUIRE(mathGrade->get() == "98");
        REQUIRE(chineseGrade->get() == "100");
    }

    SECTION("CaseInsensitivity")
    {
        std::string fileName = fileNamePrefix + std::to_string(g_tempFileID++);
        std::ofstream fout{ fileName };
        fout << "[grade]\nMath=98\n[Grade]\nChinese=100";
        fout.close();

        IniFile file{ fileName };
        auto gradeSection = file.rootSection.GetSubsection("grade");
        REQUIRE(gradeSection.has_value());

        // section name insensitivity
        auto& realSection = gradeSection->get();
        REQUIRE(realSection.GetEntrySize() == 2);
        REQUIRE(file.rootSection.GetSubsectionSize() == 1);

        // entry name insensitivity
        auto mathGrade = realSection.GetEntry("math"),
            chineseGrade = realSection.GetEntry("chinese");
        REQUIRE(mathGrade.has_value());
        REQUIRE(chineseGrade.has_value());

        REQUIRE(mathGrade->get() == "98");
        REQUIRE(chineseGrade->get() == "100");
    }

    SECTION("Comment")
    {
        std::string fileName = fileNamePrefix + std::to_string(g_tempFileID++);
        std::ofstream fout{ fileName };
        fout << ";[grade]\n#Math=98\nChinese=100";
        fout.close();

        IniFile file{ fileName };
        REQUIRE(file.rootSection.GetEntrySize() == 1);
        REQUIRE(file.rootSection("chinese") == "100");
    }
}

TEST_CASE("IniVaryingRequirements")
{
    SECTION("Subsections")
    {
        std::string fileName = fileNamePrefix + std::to_string(g_tempFileID++);
        std::ofstream fout{ fileName };
        fout << "[dormitory]\nbed = 4\n"
                "[dormitory.GodLiu]\nGPA=4.3\n"
                "[dormitory.SageLiu]\nGPA=4.0\n"
                "[dormitory.Linus]\nGPA=4.0\n"
                "[dormitory.RubbishLiang]\nGPA=1.0\n";
        fout.close();

        IniFile file{ fileName };
        auto& dormSection = file.rootSection["dormitory"];
        REQUIRE(dormSection.GetSubsectionSize() == 4);
        REQUIRE(dormSection.GetSubsection("GodLiu").has_value());
        REQUIRE(dormSection.GetSubsection("SageLiu").has_value());
        REQUIRE(dormSection.GetSubsection("Linus").has_value());
        REQUIRE(dormSection.GetSubsection("RubbishLiang").has_value());

        REQUIRE(dormSection[IniFileNameNormalize("GodLiu")](
            IniFileNameNormalize("GPA")) == "4.3");
        REQUIRE(dormSection[IniFileNameNormalize("SageLiu")](
            IniFileNameNormalize("GPA")) == "4.0");
        REQUIRE(dormSection[IniFileNameNormalize("Linus")](
            IniFileNameNormalize("GPA")) == "4.0");
        REQUIRE(dormSection[IniFileNameNormalize("RubbishLiang")](
            IniFileNameNormalize("GPA")) == "1.0");

        REQUIRE(file.rootSection.GetSubsection("dormitory.GodLiu").has_value());
    };
}

int main()
{
    for (auto i = 0;; i++)
    {
        std::string fileName = fileNamePrefix + std::to_string(i);
        if (std::filesystem::exists(fileName))
            std::filesystem::remove(fileName);
        else
            break;
    }

    auto result = Catch::Session().run();
    for (auto i = 0; i < g_tempFileID; i++)
    {
        std::filesystem::remove(fileNamePrefix + std::to_string(i));
    }
    return result;
}