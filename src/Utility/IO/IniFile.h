#pragma once
#ifndef OPENGLFRAMEWORK_IOEXTENSION_INIFILE_H_
#define OPENGLFRAMEWORK_IOEXTENSION_INIFILE_H_

#include "IOExtension.h"
#include "../String/StringExtension.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <functional>
#include <utility>
#include <memory>
#include <optional>
#include <algorithm>
#include <cctype>
#include <atomic>

namespace OpenGLFramework::IOExtension {

class FormatError : public std::runtime_error
{
public:
    FormatError(std::string formatName) : 
        std::runtime_error{ "Wrong " + formatName + " format."}{};
};

inline std::string IniFileNameNormalize(std::string_view key) {
    using namespace OpenGLFramework::StringExtension;
    return StringAsciiToLower(Trim(key));
};

template<template<typename, typename> typename Container>
class Section
{
public:
    class SectionData;
    using NullableSectionRef 
        = std::optional<std::reference_wrapper<Section>>;
    using NullableConstSectionRef 
        = std::optional<std::reference_wrapper<const Section>>;
    using NullableEntryRef 
        = std::optional<std::reference_wrapper<std::string>>;
    using NullableConstEntryRef 
        = std::optional<std::reference_wrapper<const std::string>>;
private:
    std::unique_ptr<SectionData> data = std::make_unique<SectionData>();

public:
    Section* FindAlongPath(const std::string& normalizedKey);

    NullableSectionRef GetSubsection(const std::string& key)&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        Section* result = FindAlongPath(normalizedKey);
        return result ? NullableSectionRef{ *result } : std::nullopt;
    };

    NullableConstSectionRef GetSubsection(const std::string& key) const&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        Section* result = FindAlongPath(normalizedKey);
        return result ? NullableSectionRef{ *result } : std::nullopt;
    };

    std::optional<Section> GetSubsection(const std::string& key)&&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        Section* result = FindAlongPath(normalizedKey);
        return result ? NullableSectionRef{ *result } : std::nullopt;
    };

    NullableEntryRef GetEntry(const std::string& key)&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        auto it = data->entries.find(normalizedKey);
        return it == data->entries.end() ? std::nullopt : 
            NullableEntryRef{ it->second };
    };

    NullableConstEntryRef GetEntry(const std::string& key) const&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        auto it = data->entries.find(normalizedKey);
        return it == data->entries.end() ? std::nullopt :
            NullableConstEntryRef{ it->second };
    };

    std::optional<std::string> GetEntry(const std::string& key)&&
    {
        std::string normalizedKey = IniFileNameNormalize(key);
        auto it = data->entries.find(normalizedKey);
        return it == data->entries.end() ? std::nullopt :
            std::optional<std::string>(std::move(it->second));
    };

    Section& operator[](const std::string& key)&
    {
        return data->subsections[key];
    }

    const Section& operator[](const std::string& key) const&
    {
        return data->subsections[key];
    }

    Section operator[](const std::string& key)&&
    {
        return std::move(data->subsections[key]);
    }

    std::string& operator()(const std::string& key)&
    {
        return data->entries[key];
    }

    const std::string& operator()(const std::string& key) const&
    {
        return data->entries[key];
    }

    std::string operator()(const std::string& key)&&
    {
        return std::move(data->entries[key]);
    }

    const auto& GetRawSubsections() { return data->subsections; };
    const auto& GetRawEntries() { return data->entries; }
    size_t GetSubsectionSize() { return data->subsections.size(); };
    size_t GetEntrySize() { return data->entries.size(); }
};

template<template<typename, typename> typename Container>
class Section<Container>::SectionData
{
public:
    Container<std::string, Section> subsections;
    Container<std::string, std::string> entries;
};

template<template<typename, typename> typename Container>
auto Section<Container>::FindAlongPath(
    const std::string& normalizedKey)-> Section*
{
    decltype(data->subsections.find("")) it;
    Section* currSearchingSection = this;

    std::string subsectionName;
    subsectionName.reserve(16); // reduce re-allocate and copy.

    for (auto&& subsectionNameView : normalizedKey |
        std::views::split('.'))
    {
        if (subsectionNameView.empty())
        {
            return nullptr;
        }
        // In C++23, this can be changed to lazy_split('.') | 
        // std::ranges::to<std::string>().
        std::ranges::copy(subsectionNameView.begin(), subsectionNameView.end(),
            std::back_inserter(subsectionName));

        auto& candidateSections = currSearchingSection->data->subsections;
        it = candidateSections.find(subsectionName);
        if (it == candidateSections.end())
        {
            return nullptr;
        }
        currSearchingSection = &(it->second);
        subsectionName.clear();
    }
    return currSearchingSection;
}


template<template<typename, typename> typename Container = std::unordered_map>
class IniFile
{
public:
    IniFile(const std::filesystem::path iniPath)
    {
        using OpenGLFramework::StringExtension::Trim;
        std::ifstream fin{ iniPath };
        if (!fin.is_open())
        {
            LogError("Cannot open file " + iniPath.string());
            throw std::runtime_error{ "Cannot open file." };
        }
        std::cout << "Reading .ini file from " << iniPath << "...\n";

        std::string currLine;
        std::reference_wrapper<Section<Container>> currSection = rootSection;
        bool needQuit = false;
        while (true)
        {
            if (needQuit)
                break;

            std::getline(fin, currLine);
            if (fin.eof())
                needQuit = true;
            else if(fin.fail())
                LogStreamStatus(fin);

            std::string_view validLineView = Trim(std::string_view{currLine});
            if (validLineView.empty())
                continue;

            ProcessValidLine_(validLineView, currSection);
        }
        return;
    }

    Section<Container> rootSection;
private:
    void ProcessValidLine_(std::string_view validLineView,
        std::reference_wrapper<Section<Container>>& currSection)
    {
        switch (validLineView[0])
        {
        case ';': // both of them
            [[fallthrough]];
        case '#': // are comments in .ini.
            break;
        case '[':
        {
            std::string sectionName = ProcessSectionValidLine_(validLineView);
            InsertSubsection_(sectionName, currSection);
            break;
        }
        default:
        {
            auto [entryKey, entryVal] = ProcessEntryValidLine_(validLineView);
            InsertEntry_(entryKey, std::move(entryVal), currSection);
            break;
        }
        }
        return;
    }

    std::string ProcessSectionValidLine_(std::string_view rawSectionLine)
    {
        using namespace OpenGLFramework::StringExtension;

        if (rawSectionLine.back() != ']') [[unlikely]]
        {
            LogError("No right bracket in section: " + 
                std::string{rawSectionLine});
            throw FormatError{".ini"};
        }

        std::string_view trimmedSectionName = Trim(std::string_view{
            std::next(rawSectionLine.begin()), std::prev(rawSectionLine.end())
        });
        if(trimmedSectionName.empty())
        {
            LogError("Blank between brackets.");
            throw FormatError{".ini"};
        }
        std::string validSectionName = StringAsciiToLower(trimmedSectionName);

        return validSectionName;
    };

    std::pair<std::string, std::string> ProcessEntryValidLine_(
        std::string_view rawEntryLine)
    {
        using namespace OpenGLFramework::StringExtension;

        auto equalPos = rawEntryLine.find('=');
        if (equalPos == std::string_view::npos)
        {
            LogError("No equal sign found in non-blank line.");
            throw FormatError{".ini"};
        }
        std::string_view trimmedKey = TrimEnd(rawEntryLine.substr(0, equalPos));
        if (trimmedKey.empty())
        {
            LogError("No key found before the first equal sign.");
            throw FormatError{".ini"};
        }
        std::string validKey = StringAsciiToLower(trimmedKey);

        std::string_view trimmedVal = TrimBegin(rawEntryLine.substr(equalPos + 1));
        // we allow value to be empty.

        return { std::move(validKey), std::string{ trimmedVal } };
    };

    // Adjust hint section to the hot one, and insert if it doesn't exist.
    void InsertSubsection_(const std::string& sectionFullName,
        std::reference_wrapper<Section<Container>>& currSection)
    {
        currSection = rootSection;
        std::string sectionPartialName;
        sectionPartialName.reserve(16);

        for (auto&& sectionPartialNameView : sectionFullName 
            | std::views::split('.'))
        {
            if (sectionPartialNameView.empty())
            {
                LogError("No valid name found between two dots in section " 
                    + sectionFullName);
                throw FormatError{ ".ini" };
            }

            std::ranges::copy(sectionPartialNameView.begin(),
                sectionPartialNameView.end(), 
                std::back_inserter(sectionPartialName));

            auto& underlyingSection = currSection.get();
            currSection = underlyingSection[sectionPartialName];

            sectionPartialName.clear();
        }
        return;
    }

    void InsertEntry_(const std::string& entryFullName, std::string value,
        std::reference_wrapper<Section<Container>> currSection)
    {
        auto& underlyingSection = currSection.get();
        underlyingSection(entryFullName) = std::move(value);
        return;
    };
};

} // namespace OpenGLFramework::IOExtension

#endif // !OPENGLFRAMEWORK_IOEXTENSION_INIFILE_H_
