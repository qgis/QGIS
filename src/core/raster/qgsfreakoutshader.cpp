/* **************************************************************************
                qgsfreakoutshader.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class contains code that was originally part of the larger QgsRasterLayer
class originally created circa 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include "qgsfreakoutshader.h"

QgsFreakOutShader::QgsFreakOutShader( double theMinimumValue, double theMaximumValue ) : QgsRasterShaderFunction( theMinimumValue, theMaximumValue )
{
  setClassBreaks();
}

void QgsFreakOutShader::setClassBreaks()
{
  //set up the three class breaks for pseudocolor mapping
  mBreakSize = mMinimumMaximumRange / 3;
  mClassBreakMin1 = mMinimumValue;
  mClassBreakMax1 = mClassBreakMin1 + mBreakSize;
  mClassBreakMin2 = mClassBreakMax1;
  mClassBreakMax2 = mClassBreakMin2 + mBreakSize;
  mClassBreakMin3 = mClassBreakMax2;
}

/**
    Set the maximum value for the raster shader.

    @param theValue The new maximum value
*/
void QgsFreakOutShader::setMaximumValue( double theValue )
{
  mMaximumValue = theValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
  setClassBreaks();
}

/**
    Set the maximum value for the raster shader

    @param theValue The new minimum value
*/
void QgsFreakOutShader::setMinimumValue( double theValue )
{
  mMinimumValue = theValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
  setClassBreaks();
}

bool QgsFreakOutShader::shade( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  double myPixelValue = theValue;

  //double check that myInt >= min and <= max
  //this is relevant if we are plotting within stddevs
  if ( myPixelValue < mMinimumValue )
  {
    myPixelValue = mMinimumValue;
  }
  if ( myPixelValue > mMaximumValue )
  {
    myPixelValue = mMaximumValue;
  }

  //check if we are in the first class break
  if (( myPixelValue >= mClassBreakMin1 ) && ( myPixelValue < mClassBreakMax1 ) )
  {
    *theReturnRedValue = 0;
    *theReturnGreenValue = static_cast < int >((( 255 / mMinimumMaximumRange ) * ( myPixelValue - mClassBreakMin1 ) ) * 3 );
    *theReturnBlueValue = 255;

    *theReturnRedValue = 255 - *theReturnGreenValue;
  }
  //check if we are in the second class break
  else if (( myPixelValue >= mClassBreakMin2 ) && ( myPixelValue < mClassBreakMax2 ) )
  {
    *theReturnRedValue = static_cast < int >((( 255 / mMinimumMaximumRange ) * (( myPixelValue - mClassBreakMin2 ) / 1 ) ) * 3 );
    *theReturnGreenValue = 255;
    *theReturnBlueValue = static_cast < int >( 255 - ((( 255 / mMinimumMaximumRange ) * (( myPixelValue - mClassBreakMin2 ) / 1 ) ) * 3 ) );

    *theReturnGreenValue = *theReturnBlueValue;
  }
  //otherwise we must be in the third classbreak
  else
  {
    *theReturnGreenValue = 255;
    *theReturnGreenValue = static_cast < int >( 255 - ((( 255 / mMinimumMaximumRange ) * (( myPixelValue - mClassBreakMin3 ) / 1 ) * 3 ) ) );
    *theReturnBlueValue = 0;

    *theReturnRedValue = *theReturnGreenValue;
    *theReturnGreenValue = 255 - *theReturnGreenValue;
  }

  return true;
}

bool QgsFreakOutShader::shade( double theRedValue, double theGreenValue, double theBlueValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;

  return false;
}
