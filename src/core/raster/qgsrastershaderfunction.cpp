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

QgsRasterShaderFunction::QgsRasterShaderFunction( double minimumValue, double maximumValue )
  : mMaximumValue( maximumValue )
  , mMinimumValue( minimumValue )
  , mMinimumMaximumRange( mMaximumValue - mMinimumValue )
{
  QgsDebugMsgLevel( "entered.", 4 );
}

/**
    Set the maximum value for the raster shader.

    \param value The new maximum value
*/
void QgsRasterShaderFunction::setMaximumValue( double value )
{
  QgsDebugMsgLevel( "value = " + QString::number( value ), 4 );

  mMaximumValue = value;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

/**
    Set the maximum value for the raster shader

    \param value The new minimum value
*/
void QgsRasterShaderFunction::setMinimumValue( double value )
{
  QgsDebugMsgLevel( "value = " + QString::number( value ), 4 );

  mMinimumValue = value;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

/**
  Generates and new RGBA value based on one input value

  \param value The original value to base a new RGBA value on
  \param returnRedValue  The red component of the new RGBA value
  \param returnGreenValue  The green component of the new RGBA value
  \param returnBlueValue  The blue component of the new RGBA value
  \param returnAlphaValue  The blue component of the new RGBA value
  \return True if the return values are valid otherwise false
*/
bool QgsRasterShaderFunction::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue )
{
  Q_UNUSED( value );

  *returnRedValue = 0;
  *returnGreenValue = 0;
  *returnBlueValue = 0;
  *returnAlphaValue = 0;

  return false;
}

/**
  Generates and new RGBA value based on an original RGBA value


  \param redValue The red component of the original value to base a new RGBA value on
  \param greenValue The green component of the original value to base a new RGBA value on
  \param blueValue The blue component of the original value to base a new RGBA value on
  \param alphaValue The alpha component of the original value to base a new RGBA value on
  \param returnRedValue  The red component of the new RGBA value
  \param returnGreenValue  The green component of the new RGBA value
  \param returnBlueValue  The blue component of the new RGBA value
  \param returnAlphaValue  The alpha component of the new RGBA value
  \return True if the return values are valid otherwise false
*/
bool QgsRasterShaderFunction::shade( double redValue, double greenValue, double blueValue, double alphaValue, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue )
{
  Q_UNUSED( redValue );
  Q_UNUSED( greenValue );
  Q_UNUSED( blueValue );
  Q_UNUSED( alphaValue );

  *returnRedValue = 0;
  *returnGreenValue = 0;
  *returnBlueValue = 0;
  *returnAlphaValue = 0;

  return false;
}
