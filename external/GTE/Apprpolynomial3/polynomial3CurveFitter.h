
#pragma once
#include <string.h>
#include <iostream>
#include <iosfwd>
#include <sstream>

#include <Mathematics/ApprPolynomial3.h>
#include <Mathematics/ApprPolynomial2.h>
#include <Mathematics/ApprPolynomialSpecial3.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/ApprQuery.h>

# define CurveFitter_EXPORT __declspec(dllimport)
//using namespace gte;

class CurveFitter_EXPORT polynomial3CurveFitter3
{
public:
    polynomial3CurveFitter3(int sanweijie,int erweijie, bool usespecial = false, float coff_error = 0.95);
    ~polynomial3CurveFitter3();
    bool CreateXYPolyline();
    void ReceivePointDataXYZ(std::array<float, 3>& point);

private:

    void LoadDataTest();
    bool CreateXYZPolyline();
    std::vector<std::array<float, 3>> mSamplesXYZ;
    std::vector<std::array<float, 3>> mSamplesXYError;
    std::vector<std::array<float, 2>> mSamplesXY;

    std::vector<std::array<float, 3>> mInterprateXYZ; // 内插结果

    int mDegree, mNumControls; 
    std::shared_ptr<gte::ApprQuery<float, std::array<float, 3>>> mPolynomialsXYZ;
    std::shared_ptr<gte::ApprPolynomial2<float>> mPolynomialsXY;

    std::shared_ptr<gte::ApprPolynomial3<float>> mPolynomialsXYError;


    float mAvrError, mRmsError, minterval;
    int mTargetPts;
    std::string mMessage;
    std::array <float,3> Center;
    std::array<float, 2> mXDomain, mYDomain, mZDomain;
    float min_z;
    bool mUseSpecial;
    bool EveluateErrorFromXY();

public:
  bool BeginReceiveData();
  bool EndReceiveData();
  std::array<float, 3> EveluateFromX2YZ(float X);
  float EveluateFromX2Y( float X);
  bool TransformSamples2Center();
  int GenerateXYZSeries();
  int SetInterVal(float interval);
  std::vector<std::array<float, 3>>&  GetGeneratedPoints();
  float m_coff_error ;
};
