/***************************************************************************
                         qgstemporalnavigationobject.cpp
                         ---------------
    begin                : March 2020
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

#include "qgstemporalnavigationobject.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"

QgsTemporalNavigationObject::QgsTemporalNavigationObject()
{
}

QDateTime QgsTemporalNavigationObject::addToDateTime( QDateTime datetime, QString time, int value )
{
  if ( time == QString( "Minutes" ) )
  {
    return datetime.addSecs( value * 60 );
  }
  if ( time == QString( "Hours" ) )
  {
    return datetime.addSecs( value * 3600 );
  }
  if ( time == QString( "Days" ) )
  {
    return datetime.addDays( value );
  }
  if ( time == QString( "Months" ) )
  {
    return datetime.addMonths( value );
  }
  if ( time == QString( "Years" ) )
  {
    return datetime.addYears( value );
  }

  return datetime;
}

void QgsTemporalNavigationObject::updateLayersTemporalRange( QDateTime datetime, QString time, int value )
{
  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();
  QgsMapLayer *currentLayer = nullptr;

  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    currentLayer = it.value();
    if ( !mPlayActive )
      break;
    if ( currentLayer->type() == QgsMapLayerType::RasterLayer &&
         currentLayer->dataProvider() &&
         currentLayer->dataProvider()->temporalCapabilities() )
    {
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( currentLayer );
      if ( rasterLayer && rasterLayer->dataProvider() &&
           rasterLayer->dataProvider()->temporalCapabilities() &&
           rasterLayer->dataProvider()->temporalCapabilities()->isActive() )
      {
        QgsDateTimeRange range = rangeFromMode( rasterLayer, datetime, time, value );
        if ( range.begin().isValid() && range.end().isValid() )
        {
          rasterLayer->dataProvider()->temporalCapabilities()->setTemporalRange( range );
          rasterLayer->triggerRepaint();
        }
      }
    }
  }
}

QgsDateTimeRange QgsTemporalNavigationObject::rangeFromMode( QgsMapLayer *layer, QDateTime dateTime, QString time, int value  )
{
  QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
  QList<QDateTime> availableTimes;

  if ( rasterLayer &&
       rasterLayer->dataProvider() &&
       rasterLayer->dataProvider()->temporalCapabilities()
     )
    availableTimes = rasterLayer->dataProvider()->temporalCapabilities()->dateTimes();

  if ( mode() == Mode::Snapshot )
  {
    if ( availableTimes.contains( dateTime ) )
      return QgsDateTimeRange( dateTime,  dateTime );
    else
      return QgsDateTimeRange();
  }

  if ( mode() == Mode::Composite )
  {
    QDateTime endDateTime = addToDateTime( dateTime, time, value );
    return QgsDateTimeRange( dateTime,  endDateTime );
  }

  if ( mode() == Mode::NearestPreviousProduct )
  {
    if ( availableTimes.contains( dateTime ) )
      return QgsDateTimeRange( dateTime,  dateTime );
    else
    {
      QDateTime nearest = lessNearestDateTime( availableTimes, dateTime );
      return  QgsDateTimeRange( nearest,  nearest );
    }
  }

  return QgsDateTimeRange();
}

QDateTime QgsTemporalNavigationObject::lessNearestDateTime( QList<QDateTime> dateTimes, QDateTime dateTime )
{
  int difference;
  int lowest = 0;
  bool found = false;
  QDateTime result;

  if ( dateTimes.empty() )
    return dateTime;

  for ( QDateTime currentDateTime : dateTimes )
  {
    difference = dateTime.msecsTo( currentDateTime );
    if ( difference < 0 && difference < lowest )
    {
      result = currentDateTime;
      found = true;
    }
  }

  if ( !found )
    result = dateTimes.at( 0 );

  return result;
}

void QgsTemporalNavigationObject::setNavigationStatus( NavigationStatus status )
{
  mStatus = status;
}

QgsTemporalNavigationObject::NavigationStatus QgsTemporalNavigationObject::navigationStatus() const
{
  return mStatus;
}

void QgsTemporalNavigationObject::setMode( Mode mode )
{
  mMode = mode;
}

QgsTemporalNavigationObject::Mode QgsTemporalNavigationObject::mode() const
{
  return mMode;
}

QList<QDateTime> QgsTemporalNavigationObject::dateTimes() const
{
  return mDateTimes;
}

void QgsTemporalNavigationObject::setDateTimes( QList<QDateTime> dateTimes )
{
  mDateTimes = dateTimes;
}

void QgsTemporalNavigationObject::setIsPlaying( bool playing )
{
    mPlayActive = playing;
}

bool QgsTemporalNavigationObject::isPlaying() const
{
    return mPlayActive;
}
