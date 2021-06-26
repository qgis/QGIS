// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.15

#include "ProjectTemplate.v14.h"

TemplateV14::TemplateV14(std::string const& gt4RelativePath)
    :
    Template(gt4RelativePath)
{
}

std::string const TemplateV14::msGTMathematicsGUID("10A02379-886E-46F8-93F1-1E14235D42F9");
std::string const TemplateV14::msGTGraphicsGUID("7BE071FB-E33C-4903-9278-6D12BBA46E0D");
std::string const TemplateV14::msGTGraphicsDX11GUID("56D7218D-8CCB-49A4-A2B9-D38D538EA42E");
std::string const TemplateV14::msGTApplicationsDX11GUID("741CFCD7-01A2-4325-AF0D-6678F7686FD8");
std::string const TemplateV14::msGTGraphicsGL45GUID("77148ADE-5F3A-47D4-9F7F-B8CB621592AD");
std::string const TemplateV14::msGTApplicationsGL45GUID("8AE08D71-B1FF-471C-9EEB-2ADF97593747");

std::string const TemplateV14::msSolutionLines =
R"raw(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 14
VisualStudioVersion = 14.0.25420.1
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_PROJECT_NAME__GRAPHICS_API_.v14", "_PROJECT_NAME__GRAPHICS_API_.v14.vcxproj", "{_PROJECT_GUID_}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Required", "Required", "{_REQUIRED_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTMathematics.v14", "_GT4_RELATIVE_PATH_GTMathematics.v14.vcxproj", "{_GTMATHEMATICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics.v14", "_GT4_RELATIVE_PATH_GTGraphics.v14.vcxproj", "{_GTGRAPHICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics_GRAPHICS_API_.v14", "_GT4_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v14.vcxproj", "{_GTGRAPHICSAPI_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTApplications_GRAPHICS_API_.v14", "_GT4_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v14.vcxproj", "{_GTAPPLICATIONSAPI_GUID_}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Debug|x86 = Debug|x86
		Release|x64 = Release|x64
		Release|x86 = Release|x86
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{_PROJECT_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_PROJECT_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_PROJECT_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_PROJECT_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_PROJECT_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_PROJECT_GUID_}.Release|x64.Build.0 = Release|x64
		{_PROJECT_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_PROJECT_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTMATHEMATICS_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTMATHEMATICS_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTMATHEMATICS_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTMATHEMATICS_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTMATHEMATICS_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTMATHEMATICS_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTMATHEMATICS_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTMATHEMATICS_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTGRAPHICS_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTGRAPHICS_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTGRAPHICS_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTGRAPHICS_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTGRAPHICS_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTGRAPHICS_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTGRAPHICS_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTGRAPHICS_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTGRAPHICSAPI_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTGRAPHICSAPI_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTGRAPHICSAPI_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTGRAPHICSAPI_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTGRAPHICSAPI_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTGRAPHICSAPI_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTGRAPHICSAPI_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTGRAPHICSAPI_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTAPPLICATIONSAPI_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTAPPLICATIONSAPI_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Release|x86.Build.0 = Release|Win32
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(NestedProjects) = preSolution
		{_GTMATHEMATICS_GUID_} = {_REQUIRED_GUID_}
		{_GTGRAPHICS_GUID_} = {_REQUIRED_GUID_}
		{_GTGRAPHICSAPI_GUID_} = {_REQUIRED_GUID_}
		{_GTAPPLICATIONSAPI_GUID_} = {_REQUIRED_GUID_}
	EndGlobalSection
EndGlobal)raw";

std::string const TemplateV14::msProjectLines =
R"raw(<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="14.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{_PROJECT_GUID_}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <RootNamespace>_PROJECT_NAME_.v14</RootNamespace>
    <WindowsTargetPlatformVersion>10.0.17763.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v140</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v140</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v140</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v140</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="Shared">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <LinkIncremental>true</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LinkIncremental>true</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <SDLCheck>true</SDLCheck>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <SDLCheck>true</SDLCheck>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;WIN32;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <SDLCheck>true</SDLCheck>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <SDLCheck>true</SDLCheck>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClInclude Include="_PROJECT_NAME__APPTYPE_.h" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="_PROJECT_NAME__APPTYPE_.cpp" />
    <ClCompile Include="_PROJECT_NAME_Main.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v14.vcxproj">
      <Project>{_GTAPPLICATIONSAPI_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTGraphics.v14.vcxproj">
      <Project>{_GTGRAPHICS_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v14.vcxproj">
      <Project>{_GTGRAPHICSAPI_GUID_}</Project>
    </ProjectReference>
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>)raw";

std::string const TemplateV14::msFilterLines =
R"raw(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>
      <Extensions>h;hh;hpp;hxx;hm;inl;inc;xsd</Extensions>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="_PROJECT_NAME__APPTYPE_.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
    <ClCompile Include="_PROJECT_NAME_Main.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="_PROJECT_NAME__APPTYPE_.h">
      <Filter>Header Files</Filter>
    </ClInclude>
  </ItemGroup>
</Project>)raw";
