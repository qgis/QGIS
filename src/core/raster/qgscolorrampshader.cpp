/* **************************************************************************
                qgscolorrampshader.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class is based off of code that was originally written by Marco Hugentobler and 
originally part of the larger QgsRasterLayer class
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"

#include "qgscolorrampshader.h"

QgsColorRampShader::QgsColorRampShader(double theMinimumValue, double theMaximumValue) : QgsRasterShaderFunction(theMinimumValue, theMaximumValue)
{
#ifdef QGISDEBUG
      QgsDebugMsg("QgsColorRampShader::QgsColorRampShader called");
#endif
}

bool QgsColorRampShader::generateShadedValue(double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue)
{
  if(QgsColorRampShader::DISCRETE == mColorRampType)
  {
    return getDiscreteColor(theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue);
  }

  return getInterpolatedColor(theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue);
}

bool QgsColorRampShader::generateShadedValue(double theRedValue, double theGreenValue, double theBlueValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue)
{
  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;
  
  return false;
}

bool QgsColorRampShader::getDiscreteColor(double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue)
{
  if(mColorRampItemList.count() <= 0)
  {
    return false;
  }
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it;
  QList<QgsColorRampShader::ColorRampItem>::const_iterator last_it = mColorRampItemList.begin();
  double myCurrentRampValue;
  for(it = mColorRampItemList.begin(); it != mColorRampItemList.end(); ++it)
  {
    myCurrentRampValue = it->value;
    if(theValue <= myCurrentRampValue)
    {
      if(last_it != mColorRampItemList.end())
      {
        *theReturnRedValue = last_it->color.red();
        *theReturnGreenValue = last_it->color.green();
        *theReturnBlueValue = last_it->color.blue();
        return true;
      }
    }
    last_it = it;
  }
  
  return false; // value not found
}

bool QgsColorRampShader::getInterpolatedColor(double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue)
{
  if(mColorRampItemList.count() <= 0)
  {
    return false;
  }
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it;
  QList<QgsColorRampShader::ColorRampItem>::const_iterator last_it = mColorRampItemList.end();
  double myCurrentRampValue;
  double myCurrentRampRange; //difference between two consecutive entry values
  double myDiffTheValueLastRampValue; //difference between value and last entry value
  double myDiffCurrentRampValueTheValue; //difference between this entry value and value
  
  for(it = mColorRampItemList.begin(); it != mColorRampItemList.end(); ++it)
  {
    myCurrentRampValue = it->value;
    if(theValue <= myCurrentRampValue)
    {
      if(last_it != mColorRampItemList.end())
      {
        myCurrentRampRange = myCurrentRampValue - last_it->value;
        myDiffTheValueLastRampValue = theValue - last_it->value;
        myDiffCurrentRampValueTheValue = myCurrentRampValue - theValue;

        *theReturnRedValue = (int)((it->color.red() * myDiffTheValueLastRampValue + last_it->color.red() * myDiffCurrentRampValueTheValue)/myCurrentRampRange);
        *theReturnGreenValue = (int)((it->color.green() * myDiffTheValueLastRampValue + last_it->color.green() * myDiffCurrentRampValueTheValue)/myCurrentRampRange);
        *theReturnBlueValue = (int)((it->color.blue() * myDiffTheValueLastRampValue + last_it->color.blue() * myDiffCurrentRampValueTheValue)/myCurrentRampRange);
        return true;
      }
    }
    last_it = it;
  }
  
  return false;
}
