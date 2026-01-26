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

#include "moc_qgsmeshlayertemporalproperties.cpp"

QgsMeshLayerTemporalProperties::QgsMeshLayerTemporalProperties( QObject *parent, bool enabled ):
  QgsMapLayerTemporalProperties( parent, enabled )
{}

QDomElement QgsMeshLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement temporalElement = doc.createElement( u"temporal"_s );
  temporalElement.setAttribute( u"temporal-active"_s, isActive() ? true : false );
  temporalElement.setAttribute( u"reference-time"_s, mReferenceTime.toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( u"start-time-extent"_s, mTimeExtent.begin().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( u"end-time-extent"_s, mTimeExtent.end().toTimeSpec( Qt::UTC ).toString( Qt::ISODate ) );
  temporalElement.setAttribute( u"matching-method"_s, mMatchingMethod );
  temporalElement.setAttribute( u"always-load-reference-time-from-source"_s, mAlwaysLoadReferenceTimeFromSource ? 1 : 0 );
  element.appendChild( temporalElement );

  return element;
}

bool QgsMeshLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  const QDomElement temporalElement = element.firstChildElement( u"temporal"_s );
  const bool active = temporalElement.attribute( u"temporal-active"_s ).toInt();
  setIsActive( active );

  mAlwaysLoadReferenceTimeFromSource = temporalElement.attribute( u"always-load-reference-time-from-source"_s ).toInt();

  mReferenceTime = QDateTime::fromString( temporalElement.attribute( u"reference-time"_s ), Qt::ISODate );

  if ( temporalElement.hasAttribute( u"start-time-extent"_s )
       && temporalElement.hasAttribute( u"end-time-extent"_s ) )
  {
    const QDateTime start = QDateTime::fromString( temporalElement.attribute( u"start-time-extent"_s ), Qt::ISODate );
    const QDateTime end = QDateTime::fromString( temporalElement.attribute( u"end-time-extent"_s ), Qt::ISODate );
    mTimeExtent = QgsDateTimeRange( start, end );
  }

  mMatchingMethod = static_cast<QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod>(
                      temporalElement.attribute( u"matching-method"_s ).toInt() );

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
