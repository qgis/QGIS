/* **************************************************************************
              qgscontrastenhancementfunction.cpp -  description
                       -------------------
begin                : Fri Nov 16 2007
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

#include "qgscontrastenhancementfunction.h"
#include "qgscontrastenhancement.h"

QgsContrastEnhancementFunction::QgsContrastEnhancementFunction( Qgis::DataType dataType, double minimumValue, double maximumValue )
  : mMaximumValue( maximumValue )
  , mMinimumValue( minimumValue )
  , mMinimumMaximumRange( mMaximumValue - mMinimumValue )
  , mQgsRasterDataType( dataType )
  , mMaximumValuePossible( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) )
  , mMinimumValuePossible( QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) )
{
}

QgsContrastEnhancementFunction::QgsContrastEnhancementFunction( const QgsContrastEnhancementFunction &f )
  : mMaximumValue( f.mMaximumValue )
  , mMinimumValue( f.mMinimumValue )
  , mMinimumMaximumRange( f.mMinimumMaximumRange )
  , mQgsRasterDataType( f.mQgsRasterDataType )
  , mMaximumValuePossible( f.mMaximumValuePossible )
  , mMinimumValuePossible( f.mMinimumValuePossible )
{
}

int QgsContrastEnhancementFunction::enhance( double value )
{
  if ( mQgsRasterDataType == Qgis::Byte )
  {
    return static_cast<int>( value );
  }
  else
  {
    return static_cast<int>( ( ( ( value - mMinimumValuePossible ) / ( mMaximumValuePossible - mMinimumValuePossible ) ) * 255.0 ) );
  }
}

bool QgsContrastEnhancementFunction::isValueInDisplayableRange( double value )
{
  //A default check is to see if the provided value is with the range for the data type
  // Write the test as ( v >= min && v <= max ) so that v = NaN returns false
  return value >= mMinimumValuePossible && value <= mMaximumValuePossible;
}

void QgsContrastEnhancementFunction::setMaximumValue( double value )
{
  if ( mMaximumValuePossible < value )
  {
    mMaximumValue = mMaximumValuePossible;
  }
  else
  {
    mMaximumValue = value;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

void QgsContrastEnhancementFunction::setMinimumValue( double value )
{

  if ( mMinimumValuePossible > value )
  {
    mMinimumValue = mMinimumValuePossible;
  }
  else
  {
    mMinimumValue = value;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}
