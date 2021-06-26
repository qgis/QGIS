// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include "ProjectTemplate.h"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <Rpc.h>

Template::Template(std::string const& gt4RelativePath)
    :
    mGT4RelativePath(gt4RelativePath)
{
}

bool Template::Execute(std::string const& projectName, std::string const& appType)
{
    if (appType == "c")
    {
        return CreateSource(projectName, projectName + "Console.h", msConsoleH)
            && CreateSource(projectName, projectName + "Console.cpp", msConsoleCPP)
            && CreateSource(projectName, projectName + "Main.cpp", msConsoleMainCPP)
            && CreateDX11System(projectName, "Console")
            && CreateGL45System(projectName, "Console");
    }
    else if (appType == "w2")
    {
        return CreateSource(projectName, projectName + "Window2.h", msWindow2H)
            && CreateSource(projectName, projectName + "Window2.cpp", msWindow2CPP)
            && CreateSource(projectName, projectName + "Main.cpp", msWindow2MainCPP)
            && CreateDX11System(projectName, "Window2")
            && CreateGL45System(projectName, "Window2");
    }
    else if (appType == "w3")
    {
        return CreateSource(projectName, projectName + "Window3.h", msWindow3H)
            && CreateSource(projectName, projectName + "Window3.cpp", msWindow3CPP)
            && CreateSource(projectName, projectName + "Main.cpp", msWindow3MainCPP)
            && CreateDX11System(projectName, "Window3")
            && CreateGL45System(projectName, "Window3");
    }
    else
    {
        return false;
    }
}

bool Template::CreateSource(
    std::string const& projectName,
    std::string const& sourceName,
    std::string const& text)
{
    std::ofstream outFile(sourceName);
    if (outFile)
    {
        std::string target = text;
        target = std::regex_replace(target, msProjectNamePattern, projectName);
        outFile << target << std::endl;
        outFile.close();
        return true;
    }
    return false;
}

bool Template::CreateSolution(
    std::string const& projectName,
    std::string const& projectGUID,
    std::string const& graphicsAPI,
    std::string const& graphicsAPIGUID,
    std::string const& applicationsAPIGUID,
    std::string const& text)
{
    std::string outputName = projectName + graphicsAPI + "." + GetMSVSVersion() + ".sln";
    std::ofstream outFile(outputName);
    if (outFile)
    {
        unsigned char ut8tag[3] = { 0xEF, 0xBB, 0xBF };
        for (int j = 0; j < 3; ++j)
        {
            outFile << ut8tag[j];
        }
        outFile << std::endl;

        std::string solutionGUID = GetGUIDString();
        std::string requiredGUID = GetGUIDString();

        std::string target = text;
        target = std::regex_replace(target, msGT4RelativePathPattern, mGT4RelativePath);
        target = std::regex_replace(target, msProjectNamePattern, projectName);
        target = std::regex_replace(target, msSolutionGUIDPattern, solutionGUID);
        target = std::regex_replace(target, msRequiredGUIDPattern, requiredGUID);
        target = std::regex_replace(target, msProjectGUIDGPattern, projectGUID);
        target = std::regex_replace(target, msGraphicsAPIPattern, graphicsAPI);
        target = std::regex_replace(target, msGTGraphicsAPIGUIDPattern, graphicsAPIGUID);
        target = std::regex_replace(target, msGTApplicationsAPIGUIDPattern, applicationsAPIGUID);
        target = std::regex_replace(target, msGTMathematicsPattern, GetGTMathematicsGUID());
        target = std::regex_replace(target, msGTGraphicsPattern, GetGTGraphicsGUID());
        outFile << target << std::endl;

        outFile.close();
        return true;
    }
    return false;
}

bool Template::CreateProject(
    std::string const& projectName,
    std::string const& appType,
    std::string const& projectGUID,
    std::string const& graphicsAPI,
    std::string const& graphicsMacro,
    std::string const& linkLibrary,
    std::string const& graphicsAPIGUID,
    std::string const& applicationsAPIGUID,
    std::string const& text)
{
    std::string outputName = projectName + graphicsAPI + "." + GetMSVSVersion() + ".vcxproj";
    std::ofstream outFile(outputName);
    if (outFile)
    {
        std::string target = text;
        target = std::regex_replace(target, msGT4RelativePathPattern, mGT4RelativePath);
        target = std::regex_replace(target, msAppTypePattern, appType);
        target = std::regex_replace(target, msProjectNamePattern, projectName);
        target = std::regex_replace(target, msProjectGUIDGPattern, projectGUID);
        target = std::regex_replace(target, msGraphicsAPIPattern, graphicsAPI);
        target = std::regex_replace(target, msGraphicsMacroPattern, graphicsMacro);
        target = std::regex_replace(target, msGTGraphicsAPIGUIDPattern, graphicsAPIGUID);
        target = std::regex_replace(target, msGTApplicationsAPIGUIDPattern, applicationsAPIGUID);
        target = std::regex_replace(target, msLinkLibraryPattern, linkLibrary);
        target = std::regex_replace(target, msGTGraphicsPattern, GetGTGraphicsGUID());
        outFile << target << std::endl;

        outFile.close();
        return true;
    }
    return false;
}

bool Template::CreateFilter(
    std::string const& projectName,
    std::string const& appType,
    std::string const& graphicsAPI,
    std::string const& text)
{
    std::string outputName = projectName + graphicsAPI + "." + GetMSVSVersion() + ".vcxproj.filters";
    std::ofstream outFile(outputName);
    if (outFile)
    {
        std::string target = text;
        target = std::regex_replace(target, msProjectNamePattern, projectName);
        target = std::regex_replace(target, msAppTypePattern, appType);
        outFile << target << std::endl;
        outFile.close();
        return true;
    }
    return false;
}

bool Template::CreateDX11System(
    std::string const& projectName,
    std::string const& appType)
{
    std::string graphicsAPI = "DX11";
    std::string graphicsMacro = "GTE_USE_DIRECTX";
    std::string projectGUID = GetGUIDString();

    bool success = CreateSolution(
        projectName,
        projectGUID,
        graphicsAPI,
        GetGTGraphicsDX11GUID(),
        GetGTApplicationsDX11GUID(),
        GetSolutionLines());
    if (!success)
    {
        return false;
    }

    success = CreateProject(
        projectName,
        appType,
        projectGUID,
        graphicsAPI,
        graphicsMacro,
        msLinkLibraryDX11,
        GetGTGraphicsDX11GUID(),
        GetGTApplicationsDX11GUID(),
        GetProjectLines());
    if (!success)
    {
        return false;
    }

    success = CreateFilter(
        projectName,
        appType,
        graphicsAPI,
        GetFilterLines());
    if (!success)
    {
        return false;
    }

    return true;
}

bool Template::CreateGL45System(
    std::string const& projectName,
    std::string const& appType)
{
    std::string graphicsAPI = "GL45";
    std::string graphicsMacro = "GTE_USE_OPENGL";
    std::string linkLibrary = msLinkLibraryGL45;
    std::string projectGUID = GetGUIDString();

    bool success = CreateSolution(
        projectName,
        projectGUID,
        graphicsAPI,
        GetGTGraphicsGL45GUID(),
        GetGTApplicationsGL45GUID(),
        GetSolutionLines());
    if (!success)
    {
        return false;
    }

    success = CreateProject(
        projectName,
        appType,
        projectGUID,
        graphicsAPI,
        graphicsMacro,
        msLinkLibraryGL45,
        GetGTGraphicsGL45GUID(),
        GetGTApplicationsGL45GUID(),
        GetProjectLines());
    if (!success)
    {
        return false;
    }

    success = CreateFilter(
        projectName,
        appType,
        graphicsAPI,
        GetFilterLines());
    if (!success)
    {
        return false;
    }

    return true;
}

std::string Template::GetGUIDString()
{
    UUID guid;
    RPC_STATUS status = UuidCreate(&guid);
    if (RPC_S_OK != status)
    {
        return "";
    }

    RPC_CSTR stringUuid = nullptr;
    status = UuidToString(&guid, &stringUuid);
    if (RPC_S_OK != status || nullptr == stringUuid)
    {
        return "";
    }

    std::string stringGuid(reinterpret_cast<char*>(stringUuid));
    RpcStringFree(&stringUuid);
    for (size_t i = 0; i < stringGuid.size(); ++i)
    {
        stringGuid[i] = static_cast<char>(std::toupper(stringGuid[i]));
    }
    return stringGuid;
}


std::regex const Template::msGT4RelativePathPattern("_GT4_RELATIVE_PATH_");
std::regex const Template::msAppTypePattern("_APPTYPE_");
std::regex const Template::msSolutionGUIDPattern("_SOLUTION_GUID_");
std::regex const Template::msRequiredGUIDPattern("_REQUIRED_GUID_");
std::regex const Template::msProjectNamePattern("_PROJECT_NAME_");
std::regex const Template::msProjectGUIDGPattern("_PROJECT_GUID_");
std::regex const Template::msGTMathematicsPattern("_GTMATHEMATICS_GUID_");
std::regex const Template::msGTGraphicsPattern("_GTGRAPHICS_GUID_");
std::regex const Template::msGTGraphicsAPIGUIDPattern("_GTGRAPHICSAPI_GUID_");
std::regex const Template::msGTApplicationsAPIGUIDPattern("_GTAPPLICATIONSAPI_GUID_");
std::regex const Template::msGraphicsAPIPattern("_GRAPHICS_API_");
std::regex const Template::msGraphicsMacroPattern("_GRAPHICS_MACRO_");
std::regex const Template::msLinkLibraryPattern("_LINK_LIBRARY_");

std::string const Template::msLinkLibraryDX11("d3d11.lib;d3dcompiler.lib;dxgi.lib;dxguid.lib;Windowscodecs.lib;");
std::string const Template::msLinkLibraryGL45("opengl32.lib;Windowscodecs.lib;");

std::string const Template::msConsoleH =
R"raw(#pragma once

#include <Applications/Console.h>
using namespace gte;

class _PROJECT_NAME_Console : public Console
{
public:
    _PROJECT_NAME_Console(Parameters& parameters);

    virtual void Execute() override;

private:
};)raw";

std::string const Template::msConsoleCPP =
R"raw(#include "_PROJECT_NAME_Console.h"

_PROJECT_NAME_Console::_PROJECT_NAME_Console(Parameters& parameters)
    :
    Console(parameters)
{
}

void _PROJECT_NAME_Console::Execute()
{
})raw";

std::string const Template::msConsoleMainCPP =
R"raw(#include "_PROJECT_NAME_Console.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Console::Parameters parameters(L"_PROJECT_NAME_Console");
    auto console = TheConsoleSystem.Create<_PROJECT_NAME_Console>(parameters);
    TheConsoleSystem.Execute(console);
    TheConsoleSystem.Destroy(console);
    return 0;
})raw";

std::string const Template::msWindow2H =
R"raw(#pragma once

#include <Applications/Window2.h>
using namespace gte;

class _PROJECT_NAME_Window2 : public Window2
{
public:
    _PROJECT_NAME_Window2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
};)raw";

std::string const Template::msWindow2CPP =
R"raw(#include "_PROJECT_NAME_Window2.h"

_PROJECT_NAME_Window2::_PROJECT_NAME_Window2(Parameters& parameters)
    :
    Window2(parameters)
{
    mDoFlip = true;
    OnDisplay();
}

void _PROJECT_NAME_Window2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
})raw";

std::string const Template::msWindow2MainCPP =
R"raw(#include "_PROJECT_NAME_Window2.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"_PROJECT_NAME_Window2", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<_PROJECT_NAME_Window2>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
    TheWindowSystem.Destroy(window);
    return 0;
})raw";

std::string const Template::msWindow3H =
R"raw(#pragma once

#include <Applications/Window3.h>
using namespace gte;

class _PROJECT_NAME_Window3 : public Window3
{
public:
    _PROJECT_NAME_Window3(Parameters& parameters);

    virtual void OnIdle() override;

private:
};)raw";

std::string const Template::msWindow3CPP =
R"raw(#include "_PROJECT_NAME_Window3.h"

_PROJECT_NAME_Window3::_PROJECT_NAME_Window3(Parameters& parameters)
    :
    Window3(parameters)
{
}

void _PROJECT_NAME_Window3::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
})raw";

std::string const Template::msWindow3MainCPP =
R"raw(#include "_PROJECT_NAME_Window3.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"_PROJECT_NAME_Window3", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<_PROJECT_NAME_Window3>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy(window);
    return 0;
})raw";
