/***************************************************************************
  qgsvectorlayergpslogger.cpp
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayergpslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsgpsconnection.h"
#include "qgslinestring.h"

QgsVectorLayerGpsLogger::QgsVectorLayerGpsLogger( QgsGpsConnection *connection, QObject *parent )
  : QgsGpsLogger( connection, parent )
{
  connect( this, &QgsGpsLogger::stateChanged, this, &QgsVectorLayerGpsLogger::gpsStateChanged );
}

QgsVectorLayerGpsLogger::~QgsVectorLayerGpsLogger()
{
  endCurrentTrack();
}

void QgsVectorLayerGpsLogger::setPointsLayer( QgsVectorLayer *layer )
{
  mPointsLayer = layer;

  if ( mPointsLayer )
  {
    mWgs84toPointLayerTransform = QgsCoordinateTransform( mWgs84CRS, mPointsLayer->crs(), transformContext() );
  }
}

void QgsVectorLayerGpsLogger::setTracksLayer( QgsVectorLayer *layer )
{
  mTracksLayer = layer;

  if ( mTracksLayer )
  {
    mWgs84toTrackLayerTransform = QgsCoordinateTransform( mWgs84CRS, mTracksLayer->crs(), transformContext() );
  }
}

QgsVectorLayer *QgsVectorLayerGpsLogger::pointsLayer()
{
  return mPointsLayer;
}

QgsVectorLayer *QgsVectorLayerGpsLogger::tracksLayer()
{
  return mTracksLayer;
}

void QgsVectorLayerGpsLogger::setDestinationField( Qgis::GpsInformationComponent component, const QString &field )
{
  if ( field.isEmpty() )
    mDestinationFields.remove( component );
  else
    mDestinationFields[ component ] = field;
}

QString QgsVectorLayerGpsLogger::destinationField( Qgis::GpsInformationComponent component ) const
{
  return mDestinationFields.value( component );
}

void QgsVectorLayerGpsLogger::setTransformContext( const QgsCoordinateTransformContext &context )
{
  QgsGpsLogger::setTransformContext( context );

  if ( mPointsLayer )
  {
    mWgs84toPointLayerTransform = QgsCoordinateTransform( mWgs84CRS, mPointsLayer->crs(), transformContext() );
  }
  if ( mTracksLayer )
  {
    mWgs84toTrackLayerTransform = QgsCoordinateTransform( mWgs84CRS, mTracksLayer->crs(), transformContext() );
  }
}

void QgsVectorLayerGpsLogger::endCurrentTrack()
{
  const QVector< QgsPoint > track = currentTrack();
  if ( track.isEmpty() )
    return;

  if ( mTracksLayer )
  {
    // record track
    QgsGeometry geometry = QgsGeometry( new QgsLineString( track ) );

    try
    {
      geometry.transform( mWgs84toTrackLayerTransform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming GPS track" ) );
    }

    if ( geometry.constGet()->is3D() && !QgsWkbTypes::hasZ( mTracksLayer->wkbType() ) )
    {
      geometry.get()->dropZValue();
    }
    if ( geometry.constGet()->isMeasure() && !QgsWkbTypes::hasM( mTracksLayer->wkbType() ) )
    {
      geometry.get()->dropMValue();
    }

    QgsAttributeMap attributes;

    for ( auto it = mDestinationFields.constBegin(); it != mDestinationFields.constEnd(); ++it )
    {
      if ( it.value().isEmpty() )
        continue;

      switch ( it.key() )
      {
        case Qgis::GpsInformationComponent::Location:
        case Qgis::GpsInformationComponent::Altitude:
        case Qgis::GpsInformationComponent::GeoidalSeparation:
        case Qgis::GpsInformationComponent::EllipsoidAltitude:
        case Qgis::GpsInformationComponent::GroundSpeed:
        case Qgis::GpsInformationComponent::Bearing:
        case Qgis::GpsInformationComponent::Pdop:
        case Qgis::GpsInformationComponent::Hdop:
        case Qgis::GpsInformationComponent::Vdop:
        case Qgis::GpsInformationComponent::HorizontalAccuracy:
        case Qgis::GpsInformationComponent::VerticalAccuracy:
        case Qgis::GpsInformationComponent::HvAccuracy:
        case Qgis::GpsInformationComponent::SatellitesUsed:
        case Qgis::GpsInformationComponent::Timestamp:
        case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
        case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
          continue; // points layer fields

        case Qgis::GpsInformationComponent::TrackStartTime:
        case Qgis::GpsInformationComponent::TrackEndTime:
        {
          // time field
          const int fieldIdx = mTracksLayer->fields().lookupField( it.value() );
          if ( fieldIdx >= 0 )
          {
            const QVariant value = componentValue( it.key() );
            if ( value.toDateTime().isValid() )
            {
              attributes.insert( fieldIdx, timestamp( mTracksLayer, fieldIdx, value.toDateTime() ) );
            }
          }
          break;
        }

        case Qgis::GpsInformationComponent::TotalTrackLength:
        case Qgis::GpsInformationComponent::TrackDistanceFromStart:
        {
          // non-time fields
          const int fieldIdx = mTracksLayer->fields().lookupField( it.value() );
          if ( fieldIdx >= 0 )
          {
            const QVariant value = componentValue( it.key() );
            if ( !QgsVariantUtils::isNull( value ) )
            {
              attributes.insert( fieldIdx, value );
            }
          }
          break;
        }
      }
    }

    QgsExpressionContext context = mTracksLayer->createExpressionContext();

    QgsFeature feature = QgsVectorLayerUtils::createFeature( mTracksLayer, geometry, attributes, &context );

    if ( mUseEditBuffer )
      mTracksLayer->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
    else
      mTracksLayer->dataProvider()->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
  }
  resetTrack();
}

void QgsVectorLayerGpsLogger::gpsStateChanged( const QgsGpsInformation &info )
{
  if ( mPointsLayer && info.isValid() )
  {
    // record point
    const QgsPointXY newPosition = lastPosition();
    QgsGeometry geometry( new QgsPoint( newPosition.x(), newPosition.y(), lastElevation(), lastMValue() ) );

    if ( geometry.constGet()->is3D() && !QgsWkbTypes::hasZ( mPointsLayer->wkbType() ) )
    {
      geometry.get()->dropZValue();
    }
    if ( geometry.constGet()->isMeasure() && !QgsWkbTypes::hasM( mPointsLayer->wkbType() ) )
    {
      geometry.get()->dropMValue();
    }

    try
    {
      geometry.transform( mWgs84toPointLayerTransform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming GPS point" ) );
    }

    QgsAttributeMap attributes;

    for ( auto it = mDestinationFields.constBegin(); it != mDestinationFields.constEnd(); ++it )
    {
      if ( it.value().isEmpty() )
        continue;

      switch ( it.key() )
      {
        case Qgis::GpsInformationComponent::Location:
        case Qgis::GpsInformationComponent::Altitude:
        case Qgis::GpsInformationComponent::GeoidalSeparation:
        case Qgis::GpsInformationComponent::EllipsoidAltitude:
        case Qgis::GpsInformationComponent::GroundSpeed:
        case Qgis::GpsInformationComponent::Bearing:
        case Qgis::GpsInformationComponent::Pdop:
        case Qgis::GpsInformationComponent::Hdop:
        case Qgis::GpsInformationComponent::Vdop:
        case Qgis::GpsInformationComponent::HorizontalAccuracy:
        case Qgis::GpsInformationComponent::VerticalAccuracy:
        case Qgis::GpsInformationComponent::HvAccuracy:
        case Qgis::GpsInformationComponent::SatellitesUsed:
        case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
        case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
        {
          // non-time fields
          const int fieldIdx = mPointsLayer->fields().lookupField( it.value() );
          if ( fieldIdx >= 0 )
          {
            const QVariant value = componentValue( it.key() );
            if ( !QgsVariantUtils::isNull( value ) )
            {
              attributes.insert( fieldIdx, value );
            }
          }
          break;
        }

        case Qgis::GpsInformationComponent::Timestamp:
        {
          // time field
          const int fieldIdx = mPointsLayer->fields().lookupField( it.value() );
          if ( fieldIdx >= 0 )
          {
            const QVariant value = componentValue( it.key() );
            if ( value.toDateTime().isValid() )
            {
              attributes.insert( fieldIdx, timestamp( mPointsLayer, fieldIdx, value.toDateTime() ) );
            }
          }
          break;
        }

        case Qgis::GpsInformationComponent::TrackStartTime:
        case Qgis::GpsInformationComponent::TrackEndTime:
        case Qgis::GpsInformationComponent::TotalTrackLength:
        case Qgis::GpsInformationComponent::TrackDistanceFromStart:
          continue; // track related field
      }
    }

    QgsExpressionContext context = mPointsLayer->createExpressionContext();

    QgsFeature feature = QgsVectorLayerUtils::createFeature( mPointsLayer, geometry, attributes, &context );

    if ( mUseEditBuffer )
      mPointsLayer->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
    else
      mPointsLayer->dataProvider()->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
  }
}

QVariant QgsVectorLayerGpsLogger::timestamp( QgsVectorLayer *vlayer, int idx, const QDateTime &time )
{
  QVariant value;
  if ( idx != -1 && time.isValid() )
  {
    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = time.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = time;
        break;
      default:
        break;
    }
  }
  return value;
}

