// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "polynomial3CurveFitter.h"
#include <Graphics/VertexColorEffect.h>
#include <random>
using namespace gte;
void CreateXYZFromTxt(const std::string &file_path, std::vector<std::array<double, 3>> &cloud)
{
    std::ifstream file(file_path.c_str()); //c_str()：生成一个const char*指针，指向以空字符终止的数组。
    std::string line;

    double nx, ny, nz;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        ss >> nx;
        ss >> ny;
        ss >> nz;
        nx -= 272.845;
        ny -= 498.125;
        nz -= 24.815;
        std::array<double, 3> point = {nx, ny, nz};
        cloud.push_back(point);
    }
    file.close();
}

polynomial3CurveFitter3::polynomial3CurveFitter3(int sanweijie, int erweijie , bool usespecial, double coff_error)
    : mDegree(2),
    mPolynomialsXYZ(nullptr),
    mPolynomialsXY(nullptr),
    mAvrError(0.0f),
    mRmsError(0.0f),
    mUseSpecial(usespecial),
    m_coff_error(coff_error)
{
     std::vector<int> degrees;
     for (size_t i = 0; i < sanweijie+1; i++)
     {
       degrees.push_back(i);
     }

    if (usespecial)
    {
      mPolynomialsXYZ = (std::make_shared<ApprPolynomialSpecial3<double>>(degrees, degrees));
    }
    else
    {
     mPolynomialsXYZ =  std::make_shared<ApprPolynomial3<double>>(sanweijie, sanweijie);
    }

    mPolynomialsXY = std::make_unique<ApprPolynomial2<double>>(erweijie);

    mPolynomialsXYError = std::make_unique<ApprPolynomial3<double>>(2,2);


    mXDomain[0] = std::numeric_limits<double>::max();
    mXDomain[1] = -mXDomain[0];
    mYDomain[0] = std::numeric_limits<double>::max();
    mYDomain[1] = -mYDomain[0];
    mZDomain[0] = std::numeric_limits<double>::max();
    mZDomain[1] = -mZDomain[0];
    //LoadDataTest();
}

polynomial3CurveFitter3:: ~polynomial3CurveFitter3()
{
}

void polynomial3CurveFitter3::LoadDataTest()
{
    // Generate samples on a helix.
    std::string name = "/media/wp/TOSHIBA EXT/code/tstdata/line156.asc";

    CreateXYZFromTxt(name, mSamplesXYZ);
    CreateXYZPolyline();
}

bool polynomial3CurveFitter3::CreateXYZPolyline()
{
    bool isfit = mPolynomialsXYZ->Fit(mSamplesXYZ);
    if (isfit)
    {
    #ifdef DEBUG
      //  std::vector<double> parameters = mPolynomialsXYZ->GetParameters();
          for (size_t i = 0; i < parameters.size(); i++)
          {
            std::cout << parameters.at(i) << std::endl;
          }
    #endif // DEBUG
          return true;
    }
    else
    {
      return false;
    }
}


bool polynomial3CurveFitter3::CreateXYPolyline()
{
  bool isfit = mPolynomialsXY->Fit(mSamplesXY);
  if (isfit)
  {
    std::vector<double> parameters = mPolynomialsXY->GetParameters();
#ifdef DEBUG
    for (size_t i = 0; i < parameters.size(); i++)
    {
      std::cout << parameters.at(i) << std::endl;
    }
#endif // DEBUG
    return true;
  }
  else
  {
    return false;
  }
}


void polynomial3CurveFitter3::ReceivePointDataXYZ(std::array<double, 3> &point )
{
  mSamplesXYZ.push_back(point);
  double x = point[0];
  double y = point[1];
  double z = point[2];
  if (z<min_z)
  {
    min_z = z;
    Center = {x,y,min_z };
  }
  mXDomain[0] = std::min(x, mXDomain[0]);
  mXDomain[1] = std::max(x, mXDomain[1]);
  mYDomain[0] = std::min(y, mYDomain[0]);
  mYDomain[1] = std::max(y, mYDomain[1]);
  mZDomain[0] = std::min(z, mZDomain[0]);
  mZDomain[1] = std::max(z, mZDomain[1]);
}

// 重置samples
bool polynomial3CurveFitter3::BeginReceiveData()
{
  mSamplesXYZ.clear();
  mSamplesXY.clear();
  mNumControls = (unsigned int)mSamplesXYZ.size();
  min_z = FLT_MAX;
  Center = { FLT_MAX ,FLT_MAX ,FLT_MAX };
  if (mNumControls == 0)
  {
    mXDomain[0] = std::numeric_limits<double>::max();
    mXDomain[1] = -mXDomain[0];
    mYDomain[0] = std::numeric_limits<double>::max();
    mYDomain[1] = -mYDomain[0];
    mZDomain[0] = std::numeric_limits<double>::max();
    mZDomain[1] = -mZDomain[0];
    return true;
  }
  else
  {
    return false;
  }
}


bool polynomial3CurveFitter3::EndReceiveData()
{
   mNumControls = (unsigned int)mSamplesXYZ.size();
   if (mNumControls>0)
   {
     if (TransformSamples2Center()) // mSamplesXY 生成
     {
       CreateXYPolyline();
       CreateXYZPolyline();
       return true;
     }
     else
     {
       return false;
     }
   }
   else
   {
     return false;
   }
}

// 调用之前应该  确保已经对X进行去中心处理 
std::array<double, 3> polynomial3CurveFitter3::EveluateFromX2YZ(double X)
{
  double Y = EveluateFromX2Y(X);  // Y 也是去中心的
  double Z = -999;
  Z =mPolynomialsXYZ->Evaluate(X, Y);
  double z_error = mPolynomialsXYError->Evaluate(X, Y);
  std::array<double, 3> point = { X, Y, Z- z_error* m_coff_error };
  return point;
}

std::array<double, 3> polynomial3CurveFitter3::GetOffset()
{
  return  Center;
}

bool polynomial3CurveFitter3::EveluateErrorFromXY()
{
  mSamplesXYError.clear();
  for (std::array<double, 3> point :   mSamplesXYZ)
  {
   double errorZ = mPolynomialsXYZ->Error(point);

   std::array<double, 3> pointError = { point[0],point[1],errorZ };

   mSamplesXYError.push_back(pointError);
  }
  if (mSamplesXYError.size() == mSamplesXYZ.size())
  {
    
    bool isfit = mPolynomialsXYError->Fit(mSamplesXYError);
    if (isfit)
    {
      return true;
    }
  }
  return false;
}


double polynomial3CurveFitter3::EveluateFromX2Y(double X)
{
  double Y = mPolynomialsXY->Evaluate(X);
  return Y;
}

// 同时生成了2d：mSamplesXY
bool polynomial3CurveFitter3::TransformSamples2Center()
{
  for (std::array<double, 3>& var : mSamplesXYZ)
  {
    var[0] -= Center[0];
    var[1] -= Center[1];
    var[2] -= Center[2];
    std::array<double, 2> xy = { var[0], var[1] };
    mSamplesXY.push_back(xy);
  }
  if (mSamplesXY.size() == mSamplesXYZ.size())
  {
    return true;
  }
  else
  {
    return false;
  }

}


int polynomial3CurveFitter3::GenerateXYZSeries()
{
  mInterprateXYZ.clear();
  EveluateErrorFromXY();

  for (size_t i = 0; i < mTargetPts; i++)
  {
    double percent = double((i+0.0001) / mTargetPts);
    if (percent >= 0.0001 && percent<=0.99999)
    {
      double x = mXDomain[0] - Center[0] + minterval * i;
      std::array<double, 3> pt = EveluateFromX2YZ(x);
      mInterprateXYZ.push_back(pt);
    }
  }
  mTargetPts = mInterprateXYZ.size();
  return mInterprateXYZ.size() ;
}

//输入： 插值间隔
//输出： 插值点数
int polynomial3CurveFitter3::SetInterVal(double interval)
{
  minterval = interval;
  double delata = mXDomain[1] - mXDomain[0];
  int numpts =int(delata / minterval);
  mTargetPts = numpts;
  return numpts;
}


std::vector<std::array<double, 3>>&  polynomial3CurveFitter3::GetGeneratedPoints()
{
  // TODO: 在此处添加实现代码.
  if (mInterprateXYZ.size()>0)
  {
    return mInterprateXYZ;
  }
}
