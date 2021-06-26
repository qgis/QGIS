// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.15

#include "ProjectTemplate.v16.h"

TemplateV16::TemplateV16(std::string const& gt4RelativePath)
    :
    Template(gt4RelativePath)
{
}

std::string const TemplateV16::msGTMathematicsGUID("3F553568-76EF-48F0-88DD-270482C058B9");
std::string const TemplateV16::msGTGraphicsGUID("ED37722A-40DE-4B07-9A4A-65E978D43643");
std::string const TemplateV16::msGTGraphicsDX11GUID("B24EA8FC-D596-42A1-AA46-72DEF1BE24BC");
std::string const TemplateV16::msGTApplicationsDX11GUID("85367BB5-0DE5-477A-B880-0005612F8D1C");
std::string const TemplateV16::msGTGraphicsGL45GUID("93806879-D052-48B1-AFAF-BF190FC67B7F");
std::string const TemplateV16::msGTApplicationsGL45GUID("EA72EE75-14B3-448F-828A-231FE02D0B38");

std::string const TemplateV16::msSolutionLines =
R"raw(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 16
VisualStudioVersion = 16.0.28803.156
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_PROJECT_NAME__GRAPHICS_API_.v16", "_PROJECT_NAME__GRAPHICS_API_.v16.vcxproj", "{_PROJECT_GUID_}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Required", "Required", "{_REQUIRED_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTMathematics.v16", "_GT4_RELATIVE_PATH_GTMathematics.v16.vcxproj", "{_GTMATHEMATICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics.v16", "_GT4_RELATIVE_PATH_GTGraphics.v16.vcxproj", "{_GTGRAPHICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics_GRAPHICS_API_.v16", "_GT4_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v16.vcxproj", "{_GTGRAPHICSAPI_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTApplications_GRAPHICS_API_.v16", "_GT4_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v16.vcxproj", "{_GTAPPLICATIONSAPI_GUID_}"
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
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {_SOLUTION_GUID_}
	EndGlobalSection
EndGlobal)raw";

std::string const TemplateV16::msProjectLines =
R"raw(<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>15.0</VCProjectVersion>
    <ProjectGuid>{_PROJECT_GUID_}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <RootNamespace>_PROJECT_NAME_.v16</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
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
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets">
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
      <PrecompiledHeader>
      </PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
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
      <PrecompiledHeader>
      </PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
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
      <PrecompiledHeader>
      </PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GT4_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="_PROJECT_NAME__APPTYPE_.cpp" />
    <ClCompile Include="_PROJECT_NAME_Main.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="_PROJECT_NAME__APPTYPE_.h" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v16.vcxproj">
      <Project>{_GTAPPLICATIONSAPI_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTGraphics.v16.vcxproj">
      <Project>{_GTGRAPHICS_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GT4_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v16.vcxproj">
      <Project>{_GTGRAPHICSAPI_GUID_}</Project>
    </ProjectReference>
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>)raw";

std::string const TemplateV16::msFilterLines =
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
