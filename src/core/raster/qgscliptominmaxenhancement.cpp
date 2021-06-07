/* **************************************************************************
              qgscliptominmaxenhancement.cpp -  description
                       -------------------
begin                : Fri Nov 16 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
    return static_cast<int>( ( ( ( value - mMinimumValuePossible ) / ( mMaximumValuePossible - mMinimumValuePossible ) ) * 255.0 ) );
  }
}

bool QgsClipToMinMaxEnhancement::isValueInDisplayableRange( double value )
{
  return !( value < mMinimumValue || value > mMaximumValue );
}
