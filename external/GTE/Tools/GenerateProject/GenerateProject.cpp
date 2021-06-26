// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <cstdlib>
#include <iostream>
#include "ProjectTemplate.v14.h"
#include "ProjectTemplate.v15.h"
#include "ProjectTemplate.v16.h"

int main(int numArguments, char* arguments[])
{
    if (4 != numArguments)
    {
        std::cout << "usage: GenerateProject [c,w2,w3] [nesting] projname" << std::endl;
        std::cout << "Use c for Console application." << std::endl;
        std::cout << "Use w2 for Window2 application." << std::endl;
        std::cout << "Use w3 for Window3 application." << std::endl;
        std::cout << "Nesting is the number of levels from the GTE folder." << std::endl;
        std::cout << "Example: GenerateProject w3 3 GTE/Samples/Graphics/VertexColoring" << std::endl;
        std::cout << "generates the vertex-coloring sample projects." << std::endl;
        return -1;
    }

    std::string appType = arguments[1];
    if (appType != "c" && appType != "w2" && appType != "w3")
    {
        std::cout << "Application type must be c, w2 or w3." << std::endl;
        return -2;
    }

    int nesting = atoi(arguments[2]);
    if (nesting <= 0)
    {
        std::cout << "Nesting must be positive" << std::endl;
        return -3;
    }

    // Generate the relative path to GeometricTools/GTE.
    std::string gt4RelativePath;
    for (int i = 0; i < nesting; ++i)
    {
        gt4RelativePath += "..\\";
    }

    // Generate the files for the project.
    std::string projectName = arguments[3];
    bool success = false;

    TemplateV14 generatev14(gt4RelativePath);
    success = generatev14.Execute(projectName, appType);
    if (!success)
    {
        std::cout << "Could not create the V14 project files." << std::endl;
        return -4;
    }

    TemplateV15 generatev15(gt4RelativePath);
    success = generatev15.Execute(projectName, appType);
    if (!success)
    {
        std::cout << "Could not create the V15 project files." << std::endl;
        return -5;
    }

    TemplateV16 generatev16(gt4RelativePath);
    success = generatev16.Execute(projectName, appType);
    if (!success)
    {
        std::cout << "Could not create the V16 project files." << std::endl;
        return -6;
    }

    return 0;
}
