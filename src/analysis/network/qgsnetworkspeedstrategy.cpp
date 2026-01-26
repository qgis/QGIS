/***************************************************************************
  qgsspeedstrategy.h
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

#include "qgsnetworkspeedstrategy.h"

QgsNetworkSpeedStrategy::QgsNetworkSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor )
  : mAttributeId( attributeId )
  , mDefaultValue( defaultValue )
  , mToMetricFactor( toMetricFactor )
{
}

QVariant QgsNetworkSpeedStrategy::cost( double distance, const QgsFeature &f ) const
{
  double speed = mDefaultValue;
  const QgsAttributes attrs = f.attributes();
  if ( mAttributeId >= 0 && mAttributeId < attrs.count() )
  {
    const QVariant value = attrs.at( mAttributeId );
    if ( !QgsVariantUtils::isNull( value ) )
    {
      speed = attrs.at( mAttributeId ).toDouble();
      if ( speed < 0 )
        speed = mDefaultValue;
    }
  }

  return distance / ( speed * mToMetricFactor );
}

QSet<int> QgsNetworkSpeedStrategy::requiredAttributes() const
{
  QSet<int> l;
  if ( mAttributeId >= 0 )
    l.insert( mAttributeId );
  return l;
}
