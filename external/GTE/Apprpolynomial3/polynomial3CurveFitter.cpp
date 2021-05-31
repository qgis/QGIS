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

polynomial3CurveFitter3::polynomial3CurveFitter3(int sanweijie, int erweijie , bool usespecial)
    : mDegree(2),
    mPolynomialsXYZ(nullptr),
    mPolynomialsXY(nullptr),
    mAvrError(0.0f),
    mRmsError(0.0f),
    mUseSpecial(usespecial)
{
     std::vector<int> degrees;
     for (size_t i = 0; i < sanweijie+1; i++)
     {
       degrees.push_back(i);
     }

    if (usespecial)
    {
      mPolynomialsXYZ = (std::make_shared<ApprPolynomialSpecial3<float>>(degrees, degrees));
    }
    else
    {
     mPolynomialsXYZ =  std::make_shared<ApprPolynomial3<float>>(sanweijie, sanweijie);
    }

    mPolynomialsXY = std::make_unique<ApprPolynomial2<float>>(erweijie);

    mXDomain[0] = std::numeric_limits<float>::max();
    mXDomain[1] = -mXDomain[0];
    mYDomain[0] = std::numeric_limits<float>::max();
    mYDomain[1] = -mYDomain[0];
    mZDomain[0] = std::numeric_limits<float>::max();
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
      //  std::vector<float> parameters = mPolynomialsXYZ->GetParameters();
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
    std::vector<float> parameters = mPolynomialsXY->GetParameters();
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


void polynomial3CurveFitter3::ReceivePointDataXYZ(std::array<float, 3> &point )
{
  mSamplesXYZ.push_back(point);
  float x = point[0];
  float y = point[1];
  float z = point[2];
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
    mXDomain[0] = std::numeric_limits<float>::max();
    mXDomain[1] = -mXDomain[0];
    mYDomain[0] = std::numeric_limits<float>::max();
    mYDomain[1] = -mYDomain[0];
    mZDomain[0] = std::numeric_limits<float>::max();
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
std::array<float, 3> polynomial3CurveFitter3::EveluateFromX2YZ(float X)
{
  float Y = EveluateFromX2Y(X);  // Y 也是去中心的
  float Z = -999;
  /*
    if (mUseSpecial)
  {
   Z= std::make_shared<ApprPolynomialSpecial3<float>>(mPolynomialsXYZ)->Evaluate(X, Y);
  }
  else
  {
   Z = std::make_shared<ApprPolynomial3<float>>(mPolynomialsXYZ)->Evaluate(X, Y);
  }
  */
  Z =mPolynomialsXYZ->Evaluate(X, Y);
  std::array<float, 3> point = { X+ Center[0], Y+ Center[1], Z+ Center[2] };
  return point;
}


float polynomial3CurveFitter3::EveluateFromX2Y(float X)
{
  float Y = mPolynomialsXY->Evaluate(X);
  return Y;
}

// 同时生成了2d：mSamplesXY
bool polynomial3CurveFitter3::TransformSamples2Center()
{
  for (std::array<float, 3>& var : mSamplesXYZ)
  {
    var[0] -= Center[0];
    var[1] -= Center[1];
    var[2] -= Center[2];
    std::array<float, 2> xy = { var[0], var[1] };
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
  for (size_t i = 0; i < mTargetPts; i++)
  {
    float percent = float((i+0.01) / mTargetPts);
    if (percent > 0.15 && percent<0.85)
    {
      float x = mXDomain[0] - Center[0] + minterval * i;
      std::array<float, 3> pt = EveluateFromX2YZ(x);
      mInterprateXYZ.push_back(pt);
    }
  }
  mTargetPts = mInterprateXYZ.size();
  return mInterprateXYZ.size() ;
}

//输入： 插值间隔
//输出： 插值点数
int polynomial3CurveFitter3::SetInterVal(float interval)
{
  minterval = interval;
  float delata = mXDomain[1] - mXDomain[0];
  int numpts =int(delata / minterval);
 // mTargetPts = numpts;
  return numpts;
}


std::vector<std::array<float, 3>>&  polynomial3CurveFitter3::GetGeneratedPoints()
{
  // TODO: 在此处添加实现代码.
  if (mInterprateXYZ.size()>0)
  {
    return mInterprateXYZ;
  }
}
