// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

// Insert a function call to ReportGLError after each OpenGL function call.
#define CALL_REPORT_GL_ERROR

// Insert a function call to ReportGLNullFunction when a queried function
// pointer is null.
#define CALL_REPORT_GL_NULL_FUNCTION

// This tool assumes a well-formed glcorearb.h.  Assertions are triggered
// on unexpected conditions, and the tool does not attempt to recover.

struct Prototype
{
    // The bool parameter of inputTypes is 'true' if the type is a
    // pointer type, 'false' otherwise.
    std::string returnType;
    std::string functionName;
    std::vector<std::pair<std::string,bool>> inputTypes;
    std::vector<std::string> inputNames;
};

std::string gCommentLine = "";

void WriteOpenGLVersion(std::ofstream& output)
{
    std::ifstream input("Version.txt");
    assert(input);

    while (!input.eof())
    {
        std::string line;
        getline(input, line);
        output << line << std::endl;
    }

    input.close();
}

void WriteOpenGLInitialize(std::ofstream& output)
{
    std::ifstream input("Initialize.txt");
    assert(input);

    while (!input.eof())
    {
        std::string line;
        getline(input, line);
        output << line << std::endl;
    }

    input.close();
}

std::string GetFunctionPointerType(std::string const& line)
{
    std::string::size_type begin = line.find("PFNGL");
    if (begin == std::string::npos)
    {
        return "";
    }
    std::string::size_type end = line.find_first_of(')', begin);
    assert(end != std::string::npos);
    return line.substr(begin, end - begin);
}

Prototype ParseFunctionPrototype(std::string const& line)
{
    Prototype result;
    std::string::size_type begin = line.find("GLAPI");
    if (begin == std::string::npos)
    {
        return result;
    }

    // Get the return type.
    std::string::size_type ret0 = line.find_first_of(' ');
    assert(ret0 != std::string::npos);
    ret0 = line.find_first_not_of(' ', ret0);
    assert(ret0 != std::string::npos);
    std::string::size_type ret1 = line.find("APIENTRY", ret0);
    assert(ret1 != std::string::npos);
    result.returnType = line.substr(ret0, ret1 - ret0);
    if (result.returnType.back() != '*')
    {
        std::string::size_type blank = result.returnType.find_last_of(' ');
        assert(blank != std::string::npos);
        result.returnType = result.returnType.substr(0, blank);
    }

    // Get the function name.
    std::string::size_type fnm0 = line.find_first_of(' ', ret1);
    assert(fnm0 != std::string::npos);
    fnm0 = line.find_first_not_of(' ', fnm0);
    assert(fnm0 != std::string::npos);
    std::string::size_type fnm1 = line.find_first_of(' ', fnm0);
    assert(fnm1 != std::string::npos);
    result.functionName = line.substr(fnm0, fnm1 - fnm0);

    // Get all the arguments.
    std::string::size_type arg0 = line.find('(');
    assert(arg0 != std::string::npos);
    std::string::size_type arg1 = line.find(')');
    assert(arg1 != std::string::npos);
    assert(arg1 - arg0 > 1);
    ++arg0;

    // Extract all the type-name pairs.
    std::string::size_type last = arg1, nam0;
    std::string argument;
    int const imax = 1024;
    int i;
    for (i = 0; i < imax; ++i)
    {
        // Extract a single type-name pair from the arguments.
        arg1 = line.find(',', arg0);
        if (arg1 != std::string::npos)
        {
            argument = line.substr(arg0, arg1 - arg0);
            arg0 = line.find_first_of(' ', arg1);
            assert(arg0 != std::string::npos);
            arg0 = line.find_first_not_of(' ', arg0);
            assert(arg0 != std::string::npos);
        }
        else
        {
            argument = line.substr(arg0, last - arg0);
            arg0 = std::string::npos;
        }

        // Extract the name and type.
        nam0 = argument.find_last_of(' ');
        if (nam0 != std::string::npos)
        {
            // We need to determine whether or not the input is a pointer
            // type.  If it is, we need to include the '*' with the type
            // and not the name.
            std::string::size_type loc = argument.find_last_of('*');
            if (loc != std::string::npos)
            {
                // This is a pointer type.
                nam0 = loc;
                nam0 = argument.find_first_not_of(" *", nam0);
                std::string type = argument.substr(0, nam0);
                result.inputTypes.push_back(std::make_pair(type, true));
                result.inputNames.push_back(argument.substr(nam0));
            }
            else
            {
                // This is a non-pointer type.
                std::string type = argument.substr(0, nam0);
                result.inputTypes.push_back(std::make_pair(type, false));
                nam0 = argument.find_first_not_of(' ', nam0);
                result.inputNames.push_back(argument.substr(nam0));
            }
        }
        else
        {
            // There is no need to write "void" in the CPP code when there
            // are no input parameters.
            assert(argument == "void");
        }

        if (arg0 == std::string::npos)
        {
            break;
        }
    }
    assert(i < imax);
    return result;
}

void BuildStringArray(std::string const& version,
    std::vector<std::string> const& pfntypes,
    std::vector<Prototype> const& fnprotos,
    std::vector<std::string>& preamble,
    std::vector<std::string>& functions,
    std::vector<std::string>& /* postamble */)
{
    preamble.push_back("// " + version);
    preamble.push_back(gCommentLine);

    // Insert the static function pointer declarations.
    for (size_t j = 0; j < pfntypes.size(); ++j)
    {
        std::string line = "static " + pfntypes[j] + " s" +
            fnprotos[j].functionName + " = nullptr;";
        preamble.push_back(line);
    }
    preamble.push_back(gCommentLine);

    // Insert the function prototypes.
    for (size_t j = 0; j < fnprotos.size(); ++j)
    {
        // Get the OpenGL function name and static function pointer name.
        std::string glFunction = fnprotos[j].functionName;
        std::string sglFunction = "s" + glFunction;
        std::string returnType = fnprotos[j].returnType;
        bool hasReturn = (returnType != "void");
        std::vector<std::pair<std::string, bool>> const& inputTypes =
            fnprotos[j].inputTypes;
        std::vector<std::string> const& inputNames = fnprotos[j].inputNames;
        size_t const numInputs = inputTypes.size();

        std::string line = fnprotos[j].returnType + " APIENTRY "
            + fnprotos[j].functionName + "(";

        if (numInputs > 0)
        {
            std::string suffix = inputTypes[0].first;
            if (!inputTypes[0].second)
            {
                // No extra space for pointer types.
                suffix += " ";
            }
            suffix += inputNames[0];
            line += suffix;

            for (size_t k = 1; k < numInputs; ++k)
            {
                std::string remainder = inputTypes[k].first;
                if (!inputTypes[k].second)
                {
                    // No extra space for pointer types.
                    remainder += " ";
                }
                remainder += inputNames[k];
                suffix = ", " + remainder;
                line += suffix;
            }
        }
        line += ")";
        functions.push_back(line);

        functions.push_back("{");

        if (hasReturn)
        {
            functions.push_back("    " + returnType + " result;");
        }
        functions.push_back("    if (" + sglFunction + ")");
        functions.push_back("    {");

        line = "        ";
        if (hasReturn)
        {
            line += "result = ";
        }
        line += sglFunction + "(";
        if (numInputs > 0)
        {
            line += inputNames[0];
            for (size_t k = 1; k < numInputs; ++k)
            {
                line += ", " + fnprotos[j].inputNames[k];
            }
        }
        line += ");";
        functions.push_back(line);
#if defined(CALL_REPORT_GL_ERROR)
        if (glFunction != "glGetError")
        {
            line = "        ReportGLError(\"" + glFunction + "\");";
            functions.push_back(line);
        }
#endif
        functions.push_back("    }");
        functions.push_back("    else");
        functions.push_back("    {");
#if defined(CALL_REPORT_GL_NULL_FUNCTION)
        functions.push_back("        ReportGLNullFunction(\"" +
            glFunction + "\");");
#endif
        if (hasReturn)
        {
            functions.push_back("        result = 0;");
        }
        functions.push_back("    }");
        if (hasReturn)
        {
            functions.push_back("    return result;");
        }
        functions.push_back("}");
        functions.push_back(gCommentLine);
    }

    // Insert the initializer for function pointer queries.
    std::string indent;
    if (version != "GL_VERSION_1_0")
    {
        indent = "        ";
    }
    else
    {
        indent = "    ";
    }
    functions.push_back("static void Initialize_OPEN" + version + "()");
    functions.push_back("{");
    if (version != "GL_VERSION_1_0")
    {
        functions.push_back("    if (GetOpenGLVersion() >= OPEN" + version + ")");
        functions.push_back("    {");
    }
    for (size_t j = 0; j < fnprotos.size(); ++j)
    {
        std::string line = indent + "GetOpenGLFunction(\"" +
            fnprotos[j].functionName + "\", s" + fnprotos[j].functionName +
            ");";
        functions.push_back(line);
    }
    if (version != "GL_VERSION_1_0")
    {
        functions.push_back("    }");
    }
    functions.push_back("}");
    functions.push_back(gCommentLine);
}

void ProcessOpenGLBlock(std::ifstream& input, std::ofstream& output,
    std::string const& version)
{
    std::string line, token;
    Prototype fnp;
    std::vector<std::string> pfntypes;
    std::vector<Prototype> fnprotos;

    // Start processing after the line "#ifndef version".
    int const imax = 1024;
    int i;
    for (i = 0; i < imax; ++i)
    {
        getline(input, line);
        assert(input.good());
        if (line.find(version) != std::string::npos)
        {
            break;
        }
    }
    assert(i < imax);

    // Discard the line "#define version 1".
    getline(input, line);
    assert(input.good());

    // Get the function pointer types.  Stop when "#ifdef GL_GLEXT_PROTOTYPES"
    // is encountered.
    for (i = 0; i < imax; ++i)
    {
        getline(input, line);
        assert(input.good());
        if (line.find("GL_GLEXT_PROTOTYPES") == std::string::npos)
        {
            token = GetFunctionPointerType(line);
            if (token != "")
            {
                pfntypes.push_back(token);
            }
        }
        else
        {
            break;
        }
    }
    assert(i < imax);

    // Get the function prototypes.  Stop when "#endif /* version */" is
    // encountered
    for (i = 0; i < imax; ++i)
    {
        getline(input, line);
        assert(input.good());
        if (line.find(version) == std::string::npos)
        {
            fnp = ParseFunctionPrototype(line);
            if (fnp.returnType != "")
            {
                fnprotos.push_back(fnp);
            }
        }
        else
        {
            break;
        }
    }
    assert(i < imax);
    assert(pfntypes.size() == fnprotos.size());

    // Generate the source code.
    std::vector<std::string> preamble, functions, postamble;
    BuildStringArray(version, pfntypes, fnprotos, preamble, functions,
        postamble);
    for (size_t j = 0; j < preamble.size(); ++j)
    {
        output << preamble[j] << std::endl;
    }
    for (size_t j = 0; j < functions.size(); ++j)
    {
        output << functions[j] << std::endl;
    }
    for (size_t j = 0; j < postamble.size(); ++j)
    {
        output << postamble[j] << std::endl;
    }
}

int main()
{
    std::ifstream input("glcorearb.h");
    assert(input);

    std::ofstream output("GL45.cpp");
    assert(output);

    WriteOpenGLVersion(output);

    output << "#if !defined(GTE_USE_MSWINDOWS)" << std::endl;
    output << gCommentLine << std::endl;
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_0");
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_1");
    output << "#endif" << std::endl;
    output << gCommentLine << std::endl;
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_2");
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_3");
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_4");
    ProcessOpenGLBlock(input, output, "GL_VERSION_1_5");
    ProcessOpenGLBlock(input, output, "GL_VERSION_2_0");
    ProcessOpenGLBlock(input, output, "GL_VERSION_2_1");
    ProcessOpenGLBlock(input, output, "GL_VERSION_3_0");
    ProcessOpenGLBlock(input, output, "GL_VERSION_3_1");
    ProcessOpenGLBlock(input, output, "GL_VERSION_3_2");
    ProcessOpenGLBlock(input, output, "GL_VERSION_3_3");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_0");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_1");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_2");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_3");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_4");
    ProcessOpenGLBlock(input, output, "GL_VERSION_4_5");

    WriteOpenGLInitialize(output);

    output.close();
    input.close();
    return 0;
}
