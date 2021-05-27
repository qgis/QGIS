
#pragma once
#include <string.h>
#include <iostream>
#include <iosfwd>
#include <sstream>

#include <Mathematics/ApprPolynomial3.h>
#include <Mathematics/ApprPolynomial2.h>
#include <Mathematics/Vector3.h>
using namespace gte;

# define CurveFitter_EXPORT __declspec(dllimport)


class CurveFitter_EXPORT polynomial3CurveFitter3
{
public:
    polynomial3CurveFitter3();
    bool CreateXYPolyline();
    void ReceivePointDataXYZ(std::array<float, 3> point);

private:

    void LoadDataTest();
    bool CreateXYZPolyline();
    std::vector<std::array<float, 3>> mSamplesXYZ;
    std::vector<std::array<float, 2>> mSamplesXY;

    std::vector<std::array<float, 3>> mInterprateXYZ; // 内插结果

    int mDegree, mNumControls; 
    //std::unique_ptr<BSplineCurveFit<float>> mSpline;
    std::unique_ptr<ApprPolynomial3<float>> mPolynomialsXYZ;
    std::unique_ptr<ApprPolynomial2<float>> mPolynomialsXY;
    float mAvrError, mRmsError, minterval;
    int mTargetPts;
    std::string mMessage;
    std::array <float,3> Center;
    std::array<float, 2> mXDomain, mYDomain, mZDomain;
  

public:
  bool BeginReceiveData();
  bool EndReceiveData();
  std::array<float, 3> EveluateFromX2YZ(float X);
  float EveluateFromX2Y( float X);
  bool TransformSamples2Center();
  int GenerateXYZSeries();
  int SetInterVal(float interval);
  std::vector<std::array<float, 3>>&  GetGeneratedPoints();
};
