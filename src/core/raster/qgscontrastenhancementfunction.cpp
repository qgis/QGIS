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

QgsContrastEnhancementFunction::QgsContrastEnhancementFunction( QGis::DataType theDataType, double theMinimumValue, double theMaximumValue )
{
  mQgsRasterDataType = theDataType;
  mMaximumValue = theMaximumValue;
  mMinimumValue = theMinimumValue;
  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

QgsContrastEnhancementFunction::QgsContrastEnhancementFunction( const QgsContrastEnhancementFunction& f )
{
  mQgsRasterDataType = f.mQgsRasterDataType;
  mMaximumValue = f.mMaximumValue;
  mMinimumValue = f.mMinimumValue;
  mMinimumMaximumRange = f.mMinimumMaximumRange;
}

int QgsContrastEnhancementFunction::enhance( double theValue )
{
  if ( mQgsRasterDataType == QGis::Byte )
  {
    return static_cast<int>( theValue );
  }
  else
  {
    return static_cast<int>(((( theValue - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) / ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) )*255.0 ) );
  }
}

bool QgsContrastEnhancementFunction::isValueInDisplayableRange( double theValue )
{
  //A default check is to see if the provided value is with the range for the data type
  if ( theValue < QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) || theValue > QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) )
  {
    return false;
  }

  return true;
}

void QgsContrastEnhancementFunction::setMaximumValue( double theValue )
{
  if ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) < theValue )
  {
    mMaximumValue = QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType );
  }
  else
  {
    mMaximumValue = theValue;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}

void QgsContrastEnhancementFunction::setMinimumValue( double theValue )
{

  if ( QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) > theValue )
  {
    mMinimumValue = QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType );
  }
  else
  {
    mMinimumValue = theValue;
  }

  mMinimumMaximumRange = mMaximumValue - mMinimumValue;
}
