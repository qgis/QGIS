/***************************************************************************
    qgsprojectgpssettings.cpp
    ---------------------------
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
#include "qgsprojectgpssettings.h"

#include <QDomElement>

QgsProjectGpsSettings::QgsProjectGpsSettings( QObject *parent )
  : QObject( parent )
{

}

void QgsProjectGpsSettings::resolveReferences( const QgsProject *project )
{
  mDestinationLayer.resolveWeakly( project );

  emit destinationLayerChanged( mDestinationLayer.get() );
}

QgsProjectGpsSettings::~QgsProjectGpsSettings() = default;

void QgsProjectGpsSettings::reset()
{
  mAutoAddTrackVertices = false;
  mAutoCommitFeatures = false;
  mDestinationFollowsActiveLayer = true;

  mDestinationLayer.setLayer( nullptr );

  emit automaticallyAddTrackVerticesChanged( mAutoAddTrackVertices );
  emit automaticallyCommitFeaturesChanged( mAutoCommitFeatures );
  emit destinationFollowsActiveLayerChanged( mDestinationFollowsActiveLayer );
  emit destinationLayerChanged( nullptr );
}

bool QgsProjectGpsSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mAutoAddTrackVertices = element.attribute( QStringLiteral( "autoAddTrackVertices" ), "0" ).toInt();
  mAutoCommitFeatures = element.attribute( QStringLiteral( "autoCommitFeatures" ), "0" ).toInt();
  mDestinationFollowsActiveLayer = element.attribute( QStringLiteral( "destinationFollowsActiveLayer" ), "1" ).toInt();

  const QString layerId = element.attribute( QStringLiteral( "destinationLayer" ) );
  const QString layerName = element.attribute( QStringLiteral( "destinationLayerName" ) );
  const QString layerSource = element.attribute( QStringLiteral( "destinationLayerSource" ) );
  const QString layerProvider = element.attribute( QStringLiteral( "destinationLayerProvider" ) );

  mDestinationLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );

  emit automaticallyAddTrackVerticesChanged( mAutoAddTrackVertices );
  emit automaticallyCommitFeaturesChanged( mAutoCommitFeatures );
  emit destinationFollowsActiveLayerChanged( mDestinationFollowsActiveLayer );
  emit destinationLayerChanged( nullptr ); // wont' be set until resolve is called
  return true;
}

QDomElement QgsProjectGpsSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectGpsSettings" ) );

  element.setAttribute( QStringLiteral( "autoAddTrackVertices" ),  mAutoAddTrackVertices ? 1 : 0 );
  element.setAttribute( QStringLiteral( "autoCommitFeatures" ),  mAutoCommitFeatures ? 1 : 0 );
  element.setAttribute( QStringLiteral( "destinationFollowsActiveLayer" ),  mDestinationFollowsActiveLayer ? 1 : 0 );

  if ( mDestinationLayer )
  {
    element.setAttribute( QStringLiteral( "destinationLayer" ), mDestinationLayer.layerId );
    element.setAttribute( QStringLiteral( "destinationLayerName" ), mDestinationLayer.name );
    element.setAttribute( QStringLiteral( "destinationLayerSource" ), mDestinationLayer.source );
    element.setAttribute( QStringLiteral( "destinationLayerProvider" ), mDestinationLayer.provider );
  }
  else
  {
    element.setAttribute( QStringLiteral( "destinationLayer" ), QString() );
  }

  return element;
}

bool QgsProjectGpsSettings::automaticallyAddTrackVertices() const
{
  return mAutoAddTrackVertices;
}

bool QgsProjectGpsSettings::automaticallyCommitFeatures() const
{
  return mAutoCommitFeatures;
}

bool QgsProjectGpsSettings::destinationFollowsActiveLayer() const
{
  return mDestinationFollowsActiveLayer;
}

QgsVectorLayer *QgsProjectGpsSettings::destinationLayer() const
{
  return mDestinationLayer.get();
}

void QgsProjectGpsSettings::setAutomaticallyAddTrackVertices( bool enabled )
{
  if ( enabled == mAutoAddTrackVertices )
    return;

  mAutoAddTrackVertices = enabled;
  emit automaticallyAddTrackVerticesChanged( enabled );
}

void QgsProjectGpsSettings::setAutomaticallyCommitFeatures( bool enabled )
{
  if ( enabled == mAutoCommitFeatures )
    return;

  mAutoCommitFeatures = enabled;
  emit automaticallyCommitFeaturesChanged( enabled );
}

void QgsProjectGpsSettings::setDestinationFollowsActiveLayer( bool follow )
{
  if ( follow == mDestinationFollowsActiveLayer )
    return;

  mDestinationFollowsActiveLayer = follow;
  emit destinationFollowsActiveLayerChanged( follow );
}

void QgsProjectGpsSettings::setDestinationLayer( QgsVectorLayer *layer )
{
  if ( layer == mDestinationLayer.get() )
  {
    return;
  }

  mDestinationLayer.setLayer( layer );
  emit destinationLayerChanged( layer );
}
