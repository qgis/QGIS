/***************************************************************************
                         qgsrasterlayertemporalproperties.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayertemporalproperties.h"

QgsRasterLayerTemporalProperties::QgsRasterLayerTemporalProperties( bool enabled )
  :  QgsMapLayerTemporalProperties( enabled )
{
}

QgsRasterLayerTemporalProperties::TemporalMode QgsRasterLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsRasterLayerTemporalProperties::setMode( QgsRasterLayerTemporalProperties::TemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

void  QgsRasterLayerTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &range )
{
  if ( range == mRange )
    return;

  mRange = range;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::fixedTemporalRange() const
{
  return mRange;
}

void  QgsRasterLayerTemporalProperties::setWmstRelatedSettings( const QString &dimension )
{
  // For temporal instant

  // For temporal List

  // For temporal intervals
}

bool QgsRasterLayerTemporalProperties::readXml( QDomElement ... )
{
  return false;
}

QDomElement QgsRasterLayerTemporalProperties::writeXml( ... )
{
  return QDomElement();
}
