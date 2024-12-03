/***************************************************************************
                         qgsmaplayertemporalproperties.cpp
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

#include "qgsmaplayertemporalproperties.h"
#include "moc_qgsmaplayertemporalproperties.cpp"

QgsMapLayerTemporalProperties::QgsMapLayerTemporalProperties( QObject *parent, bool enabled )
  : QgsTemporalProperty( parent, enabled )
{
}

bool QgsMapLayerTemporalProperties::isVisibleInTemporalRange( const QgsDateTimeRange & ) const
{
  return true;
}

QgsDateTimeRange QgsMapLayerTemporalProperties::calculateTemporalExtent( QgsMapLayer * ) const
{
  return QgsDateTimeRange();
}

QList<QgsDateTimeRange> QgsMapLayerTemporalProperties::allTemporalRanges( QgsMapLayer * ) const
{
  return {};
}
