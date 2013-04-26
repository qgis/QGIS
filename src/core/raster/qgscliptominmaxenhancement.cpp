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

QgsClipToMinMaxEnhancement::QgsClipToMinMaxEnhancement( QGis::DataType theQgsRasterDataType, double theMinimumValue, double theMaximumValue ) : QgsContrastEnhancementFunction( theQgsRasterDataType, theMinimumValue, theMaximumValue )
{
}

int QgsClipToMinMaxEnhancement::enhance( double theValue )
{
  if ( theValue < mMinimumValue || theValue > mMaximumValue )
  {
    return -1;
  }

  if ( mQgsRasterDataType == QGis::Byte )
  {
    return static_cast<int>( theValue );
  }
  else
  {
    return static_cast<int>(((( theValue - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) / ( QgsContrastEnhancement::maximumValuePossible( mQgsRasterDataType ) - QgsContrastEnhancement::minimumValuePossible( mQgsRasterDataType ) ) )*255.0 ) );
  }
}

bool QgsClipToMinMaxEnhancement::isValueInDisplayableRange( double theValue )
{
  if ( theValue < mMinimumValue || theValue > mMaximumValue )
  {
    return false;
  }

  return true;
}
