/* **************************************************************************
                qgsrastershaderfunction.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

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

#include "qgsrastershaderfunction.h"

QgsRasterShaderFunction::QgsRasterShaderFunction( double theMinimumValue, double theMaximumValue )
{
  QgsDebugMsg( "entered." );

  mMinimumValue = theMinimumValue;
  mMaximumValue = theMaximumValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

/**
    Set the maximum value for the raster shader.

    @param theValue The new maximum value
*/
void QgsRasterShaderFunction::setMaximumValue( double theValue )
{
  QgsDebugMsg( "value = " + QString::number( theValue ) );

  mMaximumValue = theValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

/**
    Set the maximum value for the raster shader

    @param theValue The new minimum value
*/
void QgsRasterShaderFunction::setMinimumValue( double theValue )
{
  QgsDebugMsg( "value = " + QString::number( theValue ) );

  mMinimumValue = theValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

/**
  Generates and new RGB value based on one input value

  @param theValue The original value to base a new RGB value on
  @param theReturnRedValue  The red component of the new RGB value
  @param theReturnGreenValue  The green component of the new RGB value
  @param theReturnBlueValue  The blue component of the new RGB value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShaderFunction::shade( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  Q_UNUSED( theValue );

  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;

  return false;
}

/**
  Generates and new RGB value based on an original RGB value


  @param theRedValue The red component of the original value to base a new RGB value on
  @param theGreenValue The green component of the original value to base a new RGB value on
  @param theBlueValue The blue component of the original value to base a new RGB value on
  @param theReturnRedValue  The red component of the new RGB value
  @param theReturnGreenValue  The green component of the new RGB value
  @param theReturnBlueValue  The blue component of the new RGB value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShaderFunction::shade( double theRedValue, double theGreenValue, double theBlueValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  Q_UNUSED( theRedValue );
  Q_UNUSED( theGreenValue );
  Q_UNUSED( theBlueValue );

  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;

  return false;
}
