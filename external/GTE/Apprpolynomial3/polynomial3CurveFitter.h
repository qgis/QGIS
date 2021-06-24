
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
    polynomial3CurveFitter3(int sanweijie,int erweijie, bool usespecial = false, double coff_error = 0.85);
    ~polynomial3CurveFitter3();
    bool CreateXYPolyline();
    void ReceivePointDataXYZ(std::array<double, 3>& point);

private:

    void LoadDataTest();
    bool CreateXYZPolyline();
    std::vector<std::array<double, 3>> mSamplesXYZ;
    std::vector<std::array<double, 3>> mSamplesXYError;
    std::vector<std::array<double, 2>> mSamplesXY;

    std::vector<std::array<double, 3>> mInterprateXYZ; // 内插结果

    int mDegree, mNumControls; 
    std::shared_ptr<gte::ApprQuery<double, std::array<double, 3>>> mPolynomialsXYZ;
    std::shared_ptr<gte::ApprPolynomial2<double>> mPolynomialsXY;

    std::shared_ptr<gte::ApprPolynomial3<double>> mPolynomialsXYError;


    double mAvrError, mRmsError, minterval;
    int mTargetPts;
    std::string mMessage;
    std::array <double,3> Center;
    std::array<double, 2> mXDomain, mYDomain, mZDomain;
    double min_z;
    bool mUseSpecial;
    bool EveluateErrorFromXY();

public:
  bool BeginReceiveData();
  bool EndReceiveData();
  std::array<double, 3> EveluateFromX2YZ(double X);
  std::array<double, 3> GetOffset();
  double EveluateFromX2Y(double X);
  bool TransformSamples2Center();
  int GenerateXYZSeries();
  int SetInterVal(double interval);
  std::vector<std::array<double, 3>>&  GetGeneratedPoints();
  double m_coff_error ;
};
