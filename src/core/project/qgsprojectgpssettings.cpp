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

#include "moc_qgsprojectgpssettings.cpp"

QgsProjectGpsSettings::QgsProjectGpsSettings( QObject *parent )
  : QObject( parent )
{

}

void QgsProjectGpsSettings::resolveReferences( const QgsProject *project )
{
  mDestinationLayer.resolveWeakly( project );

  emit destinationLayerChanged( mDestinationLayer.get() );

  if ( mDestinationLayer )
  {
    emit destinationTimeStampFieldChanged( mDestinationTimestampFields.value( mDestinationLayer->id() ) );
  }
}

QgsProjectGpsSettings::~QgsProjectGpsSettings() = default;

void QgsProjectGpsSettings::reset()
{
  mAutoAddTrackVertices = false;
  mAutoCommitFeatures = false;
  mDestinationFollowsActiveLayer = true;

  mDestinationLayer.setLayer( nullptr );
  mDestinationTimestampFields.clear();

  emit automaticallyAddTrackVerticesChanged( mAutoAddTrackVertices );
  emit automaticallyCommitFeaturesChanged( mAutoCommitFeatures );
  emit destinationFollowsActiveLayerChanged( mDestinationFollowsActiveLayer );
  emit destinationLayerChanged( nullptr );
  emit destinationTimeStampFieldChanged( QString() );
}

bool QgsProjectGpsSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mAutoAddTrackVertices = element.attribute( u"autoAddTrackVertices"_s, "0" ).toInt();
  mAutoCommitFeatures = element.attribute( u"autoCommitFeatures"_s, "0" ).toInt();
  mDestinationFollowsActiveLayer = element.attribute( u"destinationFollowsActiveLayer"_s, "1" ).toInt();

  const QString layerId = element.attribute( u"destinationLayer"_s );
  const QString layerName = element.attribute( u"destinationLayerName"_s );
  const QString layerSource = element.attribute( u"destinationLayerSource"_s );
  const QString layerProvider = element.attribute( u"destinationLayerProvider"_s );

  mDestinationLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );

  mDestinationTimestampFields.clear();
  {
    const QDomElement timeStampElement = element.firstChildElement( u"timeStampFields"_s );
    QDomElement layerElement = timeStampElement.firstChildElement();
    while ( !layerElement.isNull() )
    {
      const QString layerId = layerElement.attribute( u"destinationLayer"_s );
      const QString field = layerElement.attribute( u"field"_s );
      mDestinationTimestampFields[ layerId ] = field;
      layerElement = layerElement.nextSiblingElement();
    }
  }

  emit automaticallyAddTrackVerticesChanged( mAutoAddTrackVertices );
  emit automaticallyCommitFeaturesChanged( mAutoCommitFeatures );
  emit destinationFollowsActiveLayerChanged( mDestinationFollowsActiveLayer );
  emit destinationLayerChanged( nullptr ); // won't be set until resolve is called
  emit destinationTimeStampFieldChanged( QString() );
  return true;
}

QDomElement QgsProjectGpsSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( u"ProjectGpsSettings"_s );

  element.setAttribute( u"autoAddTrackVertices"_s,  mAutoAddTrackVertices ? 1 : 0 );
  element.setAttribute( u"autoCommitFeatures"_s,  mAutoCommitFeatures ? 1 : 0 );
  element.setAttribute( u"destinationFollowsActiveLayer"_s,  mDestinationFollowsActiveLayer ? 1 : 0 );

  if ( mDestinationLayer )
  {
    element.setAttribute( u"destinationLayer"_s, mDestinationLayer.layerId );
    element.setAttribute( u"destinationLayerName"_s, mDestinationLayer.name );
    element.setAttribute( u"destinationLayerSource"_s, mDestinationLayer.source );
    element.setAttribute( u"destinationLayerProvider"_s, mDestinationLayer.provider );
  }
  else
  {
    element.setAttribute( u"destinationLayer"_s, QString() );
  }

  {
    QDomElement timeStampElement = doc.createElement( u"timeStampFields"_s );
    for ( auto it = mDestinationTimestampFields.constBegin(); it != mDestinationTimestampFields.constEnd(); ++it )
    {
      const QString layerId = it.key();
      if ( QgsProject *project = qobject_cast< QgsProject * >( parent() ) )
      {
        // do some housekeeping and don't save removed layers in the project
        if ( !project->mapLayer( layerId ) )
          continue;
      }

      QDomElement layerElement = doc.createElement( u"field"_s );
      layerElement.setAttribute( u"destinationLayer"_s, layerId );
      layerElement.setAttribute( u"field"_s, it.value() );
      timeStampElement.appendChild( layerElement );
    }
    element.appendChild( timeStampElement );
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

QMap<QString, QString> QgsProjectGpsSettings::destinationTimeStampFields() const
{
  return mDestinationTimestampFields;
}

QString QgsProjectGpsSettings::destinationTimeStampField() const
{
  if ( QgsVectorLayer *vl = destinationLayer() )
  {
    return mDestinationTimestampFields.value( vl->id() );
  }
  return QString();
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

  if ( layer )
  {
    emit destinationTimeStampFieldChanged( mDestinationTimestampFields.value( layer->id() ) );
  }
  else
  {
    emit destinationTimeStampFieldChanged( QString() );
  }
}

void QgsProjectGpsSettings::setDestinationTimeStampField( QgsVectorLayer *layer, const QString &field )
{
  if ( !layer )
    return;

  if ( mDestinationTimestampFields.value( layer->id() ) != field )
  {
    mDestinationTimestampFields.insert( layer->id(), field );
    if ( layer == destinationLayer() )
    {
      emit destinationTimeStampFieldChanged( field );
    }
  }
}
