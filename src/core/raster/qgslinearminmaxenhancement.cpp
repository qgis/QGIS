/* **************************************************************************
              qgslinearminmaxenhancement.cpp -  description
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

#include "qgslinearminmaxenhancement.h"

QgsLinearMinMaxEnhancement::QgsLinearMinMaxEnhancement( QGis::DataType theQgsRasterDataType, double theMinimumValue, double theMaximumValue ) : QgsContrastEnhancementFunction( theQgsRasterDataType, theMinimumValue, theMaximumValue )
{
}

int QgsLinearMinMaxEnhancement::enhance( double theValue )
{
  int myStretchedValue = static_cast<int>((( theValue - mMinimumValue ) / ( mMinimumMaximumRange ) ) * 255.0 );
  if ( myStretchedValue < 0 )
  {
    return 0;
  }
  else if ( myStretchedValue > 255 )
  {
    return 255;
  }

  return myStretchedValue;
}
