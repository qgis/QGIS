/***************************************************************************
  qgsquickmapsettings.cpp
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaplayer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgis.h"

#include "qgsquickmapsettings.h"

QgsQuickMapSettings::QgsQuickMapSettings( QObject *parent )
  : QObject( parent )
{
  // Connect signals for derived values
  connect( this, &QgsQuickMapSettings::destinationCrsChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapSettings::mapUnitsPerPixelChanged );
  connect( this, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
  connect( this, &QgsQuickMapSettings::rotationChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
  connect( this, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapSettings::visibleExtentChanged );
}

void QgsQuickMapSettings::setProject( QgsProject *project )
{
  if ( project == mProject )
    return;

  // If we have already something connected, disconnect it!
  if ( mProject )
  {
    mProject->disconnect( this );
  }

  mProject = project;

  // Connect all signals
  if ( mProject )
  {
    connect( mProject, &QgsProject::readProject, this, &QgsQuickMapSettings::onReadProject );
    setDestinationCrs( mProject->crs() );
    mMapSettings.setTransformContext( mProject->transformContext() );
  }
  else
  {
    mMapSettings.setTransformContext( QgsCoordinateTransformContext() );
  }

  emit projectChanged();
}

QgsProject *QgsQuickMapSettings::project() const
{
  return mProject;
}

QgsCoordinateTransformContext QgsQuickMapSettings::transformContext() const
{
  return mMapSettings.transformContext();
}

QgsRectangle QgsQuickMapSettings::extent() const
{
  return mMapSettings.extent();
}

void QgsQuickMapSettings::setExtent( const QgsRectangle &extent )
{
  if ( mMapSettings.extent() == extent )
    return;

  mMapSettings.setExtent( extent );
  emit extentChanged();
}

void QgsQuickMapSettings::setCenter( const QgsPoint &center )
{
  QgsVector delta = QgsPointXY( center ) - mMapSettings.extent().center();

  QgsRectangle e = mMapSettings.extent();
  e.setXMinimum( e.xMinimum() + delta.x() );
  e.setXMaximum( e.xMaximum() + delta.x() );
  e.setYMinimum( e.yMinimum() + delta.y() );
  e.setYMaximum( e.yMaximum() + delta.y() );

  setExtent( e );
}

double QgsQuickMapSettings::mapUnitsPerPixel() const
{
  return mMapSettings.mapUnitsPerPixel();
}

QgsRectangle QgsQuickMapSettings::visibleExtent() const
{
  return mMapSettings.visibleExtent();
}

QPointF QgsQuickMapSettings::coordinateToScreen( const QgsPoint &point ) const
{
  QgsPointXY pt( point.x(), point.y() );
  QgsPointXY pp = mMapSettings.mapToPixel().transform( pt );
  return pp.toQPointF();
}

QgsPoint QgsQuickMapSettings::screenToCoordinate( const QPointF &point ) const
{
  // use floating point precision with mapToCoordinates (i.e. do not use QPointF::toPoint)
  // this is to avoid rounding errors with an odd screen width or height
  // and the point being set to the exact center of it
  const QgsPointXY pp = mMapSettings.mapToPixel().toMapCoordinates( point.x(), point.y() );
  return QgsPoint( pp );
}

QgsMapSettings QgsQuickMapSettings::mapSettings() const
{
  return mMapSettings;
}

QSize QgsQuickMapSettings::outputSize() const
{
  return mMapSettings.outputSize();
}

void QgsQuickMapSettings::setOutputSize( const QSize &outputSize )
{
  if ( mMapSettings.outputSize() == outputSize )
    return;

  mMapSettings.setOutputSize( outputSize );
  emit outputSizeChanged();
}

double QgsQuickMapSettings::outputDpi() const
{
  return mMapSettings.outputDpi();
}

void QgsQuickMapSettings::setOutputDpi( double outputDpi )
{
  if ( qgsDoubleNear( mMapSettings.outputDpi(), outputDpi ) )
    return;

  mMapSettings.setOutputDpi( outputDpi );
  emit outputDpiChanged();
}

QgsCoordinateReferenceSystem QgsQuickMapSettings::destinationCrs() const
{
  return mMapSettings.destinationCrs();
}

void QgsQuickMapSettings::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( mMapSettings.destinationCrs() == destinationCrs )
    return;

  mMapSettings.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
}

QList<QgsMapLayer *> QgsQuickMapSettings::layers() const
{
  return mMapSettings.layers();
}

void QgsQuickMapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  mMapSettings.setLayers( layers );
  emit layersChanged();
}

void QgsQuickMapSettings::onReadProject( const QDomDocument &doc )
{
  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );

    mMapSettings.readXml( node );

    if ( !qgsDoubleNear( mMapSettings.rotation(), 0 ) )
      QgsMessageLog::logMessage( tr( "Map Canvas rotation is not supported. Resetting from %1 to 0." ).arg( mMapSettings.rotation() ) );

    mMapSettings.setRotation( 0 );

    emit extentChanged();
    emit destinationCrsChanged();
    emit outputSizeChanged();
    emit outputDpiChanged();
    emit layersChanged();
  }
}

double QgsQuickMapSettings::rotation() const
{
  return mMapSettings.rotation();
}

void QgsQuickMapSettings::setRotation( double rotation )
{
  if ( !qgsDoubleNear( rotation, 0 ) )
    QgsMessageLog::logMessage( tr( "Map Canvas rotation is not supported. Resetting from %1 to 0." ).arg( rotation ) );
}
