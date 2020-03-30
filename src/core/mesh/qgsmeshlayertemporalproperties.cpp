/***************************************************************************
                         qgsmeshlayertemporalproperties.cpp
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshlayertemporalproperties.h"
#include "qgsmeshdataprovidertemporalcapabilities.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"

QgsMeshLayerTemporalProperties::QgsMeshLayerTemporalProperties( QObject *parent, bool enabled ): QgsMapLayerTemporalProperties( parent, enabled )
{}

QDomElement QgsMeshLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement temporalElement = doc.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "temporal-active" ), isActive() ? true : false );
  temporalElement.setAttribute( QStringLiteral( "reference-time" ), mReferenceTime.toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( QStringLiteral( "start-time-extent" ), mTimeExtent.begin().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( QStringLiteral( "end-time-extent" ), mTimeExtent.end().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );

  element.appendChild( temporalElement );

  return element;
}

bool QgsMeshLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement temporalElement = element.firstChildElement( QStringLiteral( "temporal" ) );
  bool active = temporalElement.attribute( QStringLiteral( "temporal-active" ) ).toInt();
  setIsActive( active );

  mReferenceTime = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "reference-time" ) ), Qt::ISODate );

  if ( temporalElement.hasAttribute( QStringLiteral( "start-time-extent" ) )
       && temporalElement.hasAttribute( QStringLiteral( "end-time-extent" ) ) )
  {
    QDateTime start = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "start-time-extent" ) ), Qt::ISODate );
    QDateTime end = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "end-time-extent" ) ), Qt::ISODate );
    mTimeExtent = QgsDateTimeRange( start, end );
  }

  return true;
}

void QgsMeshLayerTemporalProperties::setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities )
{
  const QgsMeshDataProviderTemporalCapabilities *temporalCapabilities =
    static_cast<const QgsMeshDataProviderTemporalCapabilities *>( capabilities );

  setIsActive( temporalCapabilities->hasTemporalCapabilities() );
  mReferenceTime = temporalCapabilities->referenceTime();

  if ( mReferenceTime.isValid() )
    mTimeExtent = temporalCapabilities->timeExtent();
  else // If the provider capabilities doesn't provide a reference time, try to bring one frome projet settings
  {
    // Not sure it is a good idea to call instance of project here ...
    mReferenceTime = QgsProject::instance()->timeSettings()->temporalRange().begin();
    if ( !mReferenceTime.isValid() ) // If project reference time is invalid, use current date
      mReferenceTime = QDateTime( QDate::currentDate(), QTime( 0, 0, 0, Qt::UTC ) );
    mTimeExtent = temporalCapabilities->timeExtent( mReferenceTime );
  }
}

QgsDateTimeRange QgsMeshLayerTemporalProperties::timeExtent() const
{
  return mTimeExtent;
}

QDateTime QgsMeshLayerTemporalProperties::referenceTime() const
{
  return mReferenceTime;
}

void QgsMeshLayerTemporalProperties::setReferenceTime( const QDateTime &referenceTime, const QgsDataProviderTemporalCapabilities *capabilities )
{
  const QgsMeshDataProviderTemporalCapabilities *tempCap = static_cast<const QgsMeshDataProviderTemporalCapabilities *>( capabilities );
  mReferenceTime = referenceTime;
  mTimeExtent = tempCap->timeExtent( referenceTime );
}
