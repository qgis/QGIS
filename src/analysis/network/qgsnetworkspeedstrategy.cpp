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
{
  mAttributeId = attributeId;
  mDefaultValue = defaultValue;
  mToMetricFactor = toMetricFactor;
}

QVariant QgsNetworkSpeedStrategy::cost( double distance, const QgsFeature &f ) const
{
  const QgsAttributes attrs = f.attributes();

  if ( mAttributeId < 0 || mAttributeId >= attrs.count() )
    return QVariant( distance / ( mDefaultValue * mToMetricFactor ) );

  const double val = distance / ( attrs.at( mAttributeId ).toDouble() * mToMetricFactor );
  if ( val <= 0.0 )
    return QVariant( distance / ( mDefaultValue / mToMetricFactor ) );

  return QVariant( val );
}

QSet<int> QgsNetworkSpeedStrategy::requiredAttributes() const
{
  QSet< int > l;
  l.insert( mAttributeId );
  return l;
}
