/***************************************************************************
                         qgsmaplayertemporalproperties.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayertemporalproperties.h"

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
