// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once
#include <string.h>
#include <iostream>
#include <iosfwd>
#include <sstream>

#include <Mathematics/ApprPolynomial3.h>
#include <Mathematics/Vector3.h>
using namespace gte;

class polynomial3CurveFitterWindow3
{
public:
    polynomial3CurveFitterWindow3();

private:
    void CreateScene();
    void CreateBSplinePolyline();

    enum
    {
        NUM_SAMPLES = 1000
    };

    struct Vertex
    {
        Vector3<float> position;
    };

    std::vector<std::array<float, 3>> mSamples;
    //std::array<float, 3> mSamples;
    int mDegree, mNumControls;
    //std::unique_ptr<BSplineCurveFit<float>> mSpline;
    std::unique_ptr<ApprPolynomial3<float>> mPolynomials;
    float mAvrError, mRmsError;
    std::string mMessage;
};
