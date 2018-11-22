/* **************************************************************************
              qgscliptominmaxenhancement.cpp -  description
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

#include "qgscliptominmaxenhancement.h"
#include "qgscontrastenhancement.h"

QgsClipToMinMaxEnhancement::QgsClipToMinMaxEnhancement( Qgis::DataType qgsRasterDataType, double minimumValue, double maximumValue ) : QgsContrastEnhancementFunction( qgsRasterDataType, minimumValue, maximumValue )
{
}

int QgsClipToMinMaxEnhancement::enhance( double value )
{
  if ( value < mMinimumValue || value > mMaximumValue )
  {
    return -1;
  }

  if ( mQgsRasterDataType == Qgis::Byte )
  {
    return static_cast<int>( value );
  }
  else
  {
    return static_cast<int>( ( ( ( value - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) / ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) ) * 255.0 ) );
  }
}

bool QgsClipToMinMaxEnhancement::isValueInDisplayableRange( double value )
{
  return !( value < mMinimumValue || value > mMaximumValue );
}
