/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *   This is file implements Units classes                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "speedproperter.h"

RgSpeedProperter::RgSpeedProperter( int attributeId, double defaultValue, double toMetricFactor )
{
  mAttributeId = attributeId;
  mDefaultValue = defaultValue;
  mToMetricFactor = toMetricFactor;
}

QVariant RgSpeedProperter::property( double distance, const QgsFeature& f ) const
{
  const QgsAttributeMap& map = f.attributeMap();
  QgsAttributeMap::const_iterator it = map.find( mAttributeId );

  if ( it == map.end() )
    return QVariant( distance / ( mDefaultValue*mToMetricFactor ) );

  double val = distance / ( it->toDouble() * mToMetricFactor );
  if ( val <= 0.0 )
    return QVariant( distance / ( mDefaultValue / mToMetricFactor ) );

  return QVariant( val );
}

QgsAttributeList RgSpeedProperter::requiredAttributes() const
{
  QgsAttributeList l;
  l.push_back( mAttributeId );
  return l;
}
