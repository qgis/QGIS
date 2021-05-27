// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "polynomial3CurveFitter.h"
#include <Graphics/VertexColorEffect.h>
#include <random>

void CreateXYZFromTxt(const std::string &file_path, std::vector<std::array<float, 3>> &cloud)
{
    std::ifstream file(file_path.c_str()); //c_str()：生成一个const char*指针，指向以空字符终止的数组。
    std::string line;

    float nx, ny, nz;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        ss >> nx;
        ss >> ny;
        ss >> nz;
        nx -= 272.845;
        ny -= 498.125;
        nz -= 24.815;
        std::array<float, 3> point = {nx, ny, nz};
        cloud.push_back(point);
    }
    file.close();
}

polynomial3CurveFitterWindow3::polynomial3CurveFitterWindow3()
    : mDegree(2),
      mNumControls(NUM_SAMPLES / 2),
      mPolynomials(nullptr),
      mAvrError(0.0f),
      mRmsError(0.0f)
{
    mPolynomials = std::make_unique<ApprPolynomial3<float>>(3, 3);
    CreateScene();
}



void polynomial3CurveFitterWindow3::CreateScene()
{
    // Generate samples on a helix.
    std::string name = "/media/wp/TOSHIBA EXT/code/tstdata/line156.asc";

    CreateXYZFromTxt(name, mSamples);
    CreateBSplinePolyline();
}

void polynomial3CurveFitterWindow3::CreateBSplinePolyline()
{

    //mPolynomials->FitIndexed(static_cast<int>(mSamples.size()), reinterpret_cast<std::array<float,3> const *>(&mSamples[0]),static_cast<int>(mSamples.size()),)
    bool isfit = mPolynomials->Fit(mSamples);
    std::cout << "fit?? :: " << isfit << std::endl;

    std::vector<float> parameters = mPolynomials->GetParameters();

    /*     std::cout << "parameters:-count " << parameters.size() << ": -1: " << parameters.front() << " -2:" << parameters.at(1) << " -3:" << parameters.at(2) << " -4:" << parameters.at(3) << std::endl; */
    for (size_t i = 0; i < parameters.size(); i++)
    {
        std::cout << parameters.at(i) << std::endl;
    }
    /*             nx -= 272.845;
        ny -= 498.125;
        nz -= 24.815; */
    float mm = mPolynomials->Evaluate(272.69 - 272.845, 501.0 - 498.125);
    std::cout << "Test eveluate " << mm + 24.815 << std::endl;

    unsigned int numSamples = (unsigned int)mSamples.size();
    
}
