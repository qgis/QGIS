/***************************************************************************
                         qgsmeshlayertemporalproperties.cpp
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
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

QgsMeshLayerTemporalProperties::QgsMeshLayerTemporalProperties( QObject *parent, bool enabled ):
  QgsMapLayerTemporalProperties( parent, enabled )
{}

QDomElement QgsMeshLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement temporalElement = doc.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "temporal-active" ), isActive() ? true : false );
  temporalElement.setAttribute( QStringLiteral( "reference-time" ), mReferenceTime.toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( QStringLiteral( "start-time-extent" ), mTimeExtent.begin().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( QStringLiteral( "end-time-extent" ), mTimeExtent.end().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( QStringLiteral( "matching-method" ), mMatchingMethod );
  temporalElement.setAttribute( QStringLiteral( "always-load-reference-time-from-source" ), mAlwaysLoadReferenceTimeFromSource ? 1 : 0 );
  element.appendChild( temporalElement );

  return element;
}

bool QgsMeshLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  const QDomElement temporalElement = element.firstChildElement( QStringLiteral( "temporal" ) );
  const bool active = temporalElement.attribute( QStringLiteral( "temporal-active" ) ).toInt();
  setIsActive( active );

  mAlwaysLoadReferenceTimeFromSource = temporalElement.attribute( QStringLiteral( "always-load-reference-time-from-source" ) ).toInt();

  mReferenceTime = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "reference-time" ) ), Qt::ISODate );

  if ( temporalElement.hasAttribute( QStringLiteral( "start-time-extent" ) )
       && temporalElement.hasAttribute( QStringLiteral( "end-time-extent" ) ) )
  {
    const QDateTime start = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "start-time-extent" ) ), Qt::ISODate );
    const QDateTime end = QDateTime::fromString( temporalElement.attribute( QStringLiteral( "end-time-extent" ) ), Qt::ISODate );
    mTimeExtent = QgsDateTimeRange( start, end );
  }

  mMatchingMethod = static_cast<QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod>(
                      temporalElement.attribute( QStringLiteral( "matching-method" ) ).toInt() );

  mIsValid = true;
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

  mIsValid = true;
}

QgsDateTimeRange QgsMeshLayerTemporalProperties::calculateTemporalExtent( QgsMapLayer * ) const
{
  return mTimeExtent;
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
  mReferenceTime = referenceTime;
  if ( capabilities )
  {
    const QgsMeshDataProviderTemporalCapabilities *tempCap = static_cast<const QgsMeshDataProviderTemporalCapabilities *>( capabilities );
    mTimeExtent = tempCap->timeExtent( referenceTime );
  }
  else
    mTimeExtent = QgsDateTimeRange( referenceTime, referenceTime );
}

QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod QgsMeshLayerTemporalProperties::matchingMethod() const
{
  return mMatchingMethod;
}

void QgsMeshLayerTemporalProperties::setMatchingMethod( const QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod &matchingMethod )
{
  mMatchingMethod = matchingMethod;
}

bool QgsMeshLayerTemporalProperties::isValid() const
{
  return mIsValid;
}

void QgsMeshLayerTemporalProperties::setIsValid( bool isValid )
{
  mIsValid = isValid;
}

bool QgsMeshLayerTemporalProperties::alwaysLoadReferenceTimeFromSource() const
{
  return mAlwaysLoadReferenceTimeFromSource;
}

void QgsMeshLayerTemporalProperties::setAlwaysLoadReferenceTimeFromSource( bool autoReloadFromProvider )
{
  mAlwaysLoadReferenceTimeFromSource = autoReloadFromProvider;
}
