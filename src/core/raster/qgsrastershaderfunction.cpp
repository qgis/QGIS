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
  QgsDebugMsgLevel( QStringLiteral( "entered." ), 4 );
}

void QgsRasterShaderFunction::setMaximumValue( double value )
{
  QgsDebugMsgLevel( "value = " + QString::number( value ), 4 );

  mMaximumValue = value;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

void QgsRasterShaderFunction::setMinimumValue( double value )
{
  QgsDebugMsgLevel( "value = " + QString::number( value ), 4 );

  mMinimumValue = value;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

bool QgsRasterShaderFunction::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue ) const
{
  Q_UNUSED( value );

  *returnRedValue = 0;
  *returnGreenValue = 0;
  *returnBlueValue = 0;
  *returnAlphaValue = 0;

  return false;
}

bool QgsRasterShaderFunction::shade( double redValue, double greenValue, double blueValue, double alphaValue, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue ) const
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
