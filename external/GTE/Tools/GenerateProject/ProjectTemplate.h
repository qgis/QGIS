// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <regex>
#include <string>

class Template
{
public:
    Template(std::string const& gt4RelativePath);
    virtual ~Template() = default;

    bool Execute(std::string const& projectName, std::string const& appType);

protected:
    bool CreateSource(
        std::string const& projectName,
        std::string const& sourceName,
        std::string const& text);

    bool CreateSolution(
        std::string const& projectName,
        std::string const& projectGUID,
        std::string const& graphicsAPI,
        std::string const& graphicsAPIGUID,
        std::string const& applicationsAPIGUID,
        std::string const& text);

    bool CreateProject(
        std::string const& projectName,
        std::string const& appType,
        std::string const& projectGUID,
        std::string const& graphicsAPI,
        std::string const& graphicsMacro,
        std::string const& linkLibrary,
        std::string const& graphicsAPIGUID,
        std::string const& applicationsAPIGUID,
        std::string const& text);

    bool CreateFilter(
        std::string const& projectName,
        std::string const& appType,
        std::string const& graphicsAPI,
        std::string const& text);

    bool CreateDX11System(
        std::string const& projectName,
        std::string const& appType);

    bool CreateGL45System(
        std::string const& projectName,
        std::string const& appType);

    static std::string GetGUIDString();

    virtual std::string GetMSVSVersion() const = 0;
    virtual std::string GetGTMathematicsGUID() const = 0;
    virtual std::string GetGTGraphicsGUID() const = 0;
    virtual std::string GetGTGraphicsDX11GUID() const = 0;
    virtual std::string GetGTGraphicsGL45GUID() const = 0;
    virtual std::string GetGTApplicationsDX11GUID() const = 0;
    virtual std::string GetGTApplicationsGL45GUID() const = 0;
    virtual std::string GetSolutionLines() const = 0;
    virtual std::string GetProjectLines() const = 0;
    virtual std::string GetFilterLines() const = 0;

    // The relative path from the generated project to the folder that
    // contains the GTE4 library projects.
    std::string mGT4RelativePath;

    // Matching patterns that are common across all versions.
    static std::regex const msGT4RelativePathPattern;
    static std::regex const msAppTypePattern;
    static std::regex const msSolutionGUIDPattern;
    static std::regex const msRequiredGUIDPattern;
    static std::regex const msProjectNamePattern;
    static std::regex const msProjectGUIDGPattern;
    static std::regex const msGTMathematicsPattern;
    static std::regex const msGTGraphicsPattern;
    static std::regex const msGTGraphicsAPIGUIDPattern;
    static std::regex const msGTApplicationsAPIGUIDPattern;
    static std::regex const msGraphicsAPIPattern;
    static std::regex const msGraphicsMacroPattern;
    static std::regex const msLinkLibraryPattern;

    // Libraries that are common across all versions.
    static std::string const msLinkLibraryDX11;
    static std::string const msLinkLibraryGL45;

    // Source code that is common across all versions.
    static std::string const msConsoleH;
    static std::string const msConsoleCPP;
    static std::string const msConsoleMainCPP;
    static std::string const msWindow2H;
    static std::string const msWindow2CPP;
    static std::string const msWindow2MainCPP;
    static std::string const msWindow3H;
    static std::string const msWindow3CPP;
    static std::string const msWindow3MainCPP;
};
