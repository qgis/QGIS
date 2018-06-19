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
{
  mQgsRasterDataType = dataType;
  mMaximumValue = maximumValue;
  mMinimumValue = minimumValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

QgsContrastEnhancementFunction::QgsContrastEnhancementFunction( const QgsContrastEnhancementFunction &f )
{
  mQgsRasterDataType = f.mQgsRasterDataType;
  mMaximumValue = f.mMaximumValue;
  mMinimumValue = f.mMinimumValue;
  mMinimumMaximumRange = f.mMinimumMaximumRange;
}

int QgsContrastEnhancementFunction::enhance( double value )
{
  if ( mQgsRasterDataType == Qgis::Byte )
  {
    return static_cast<int>( value );
  }
  else
  {
    return static_cast<int>( ( ( ( value - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) / ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) ) * 255.0 ) );
  }
}

bool QgsContrastEnhancementFunction::isValueInDisplayableRange( double value )
{
  //A default check is to see if the provided value is with the range for the data type
  // Write the test as ( v >= min && v <= max ) so that v = NaN returns false
  return value >= QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) && value <= QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType );
}

void QgsContrastEnhancementFunction::setMaximumValue( double value )
{
  if ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) < value )
  {
    mMaximumValue = QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType );
  }
  else
  {
    mMaximumValue = value;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

void QgsContrastEnhancementFunction::setMinimumValue( double value )
{

  if ( QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) > value )
  {
    mMinimumValue = QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType );
  }
  else
  {
    mMinimumValue = value;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}
