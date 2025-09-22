/***************************************************************************
  qgsmapoverlaytexturegenerator_p.cpp
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapoverlaytexturegenerator_p.h"
#include "moc_qgsmapoverlaytexturegenerator_p.cpp"

#include "qgsmaprenderersequentialjob.h"
#include "qgsmapsettings.h"
#include "qgsmapthemecollection.h"
#include "qgsproject.h"
#include "qgs3dmapsettings.h"
#include "qgseventtracing.h"

///@cond PRIVATE

QgsMapOverlayTextureGenerator::QgsMapOverlayTextureGenerator( const Qgs3DMapSettings &mapSettings, int arrowSize )
  : m3DMapSettings( mapSettings )
  , mSize( QSize( arrowSize, arrowSize ) )
  , mLastJobId( 0 )
{
}

QgsMapOverlayTextureGenerator::~QgsMapOverlayTextureGenerator()
{
  cancelActiveJob();
}

int QgsMapOverlayTextureGenerator::render( const QgsRectangle &extent, double rotationDegrees )
{
  cancelActiveJob();
  QgsMapSettings mapSettings( baseMapSettings() );
  mapSettings.setExtent( extent );

  mRotation = rotationDegrees;
  mLastJobId++;
  QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "Texture" ), QString::number( mLastJobId ) );

  mActiveJob = new QgsMapRendererSequentialJob( mapSettings );
  connect( mActiveJob, &QgsMapRendererJob::finished, this, &QgsMapOverlayTextureGenerator::onRenderingFinished );

  mActiveJob->start();

  return mLastJobId;
}

void QgsMapOverlayTextureGenerator::cancelActiveJob()
{
  if ( !mActiveJob )
  {
    return;
  }

  mActiveJob->cancelWithoutBlocking();
  disconnect( mActiveJob, &QgsMapRendererJob::finished, this, &QgsMapOverlayTextureGenerator::onRenderingFinished );
  mActiveJob->deleteLater();
  mActiveJob = nullptr;
}

void QgsMapOverlayTextureGenerator::onRenderingFinished()
{
  QImage renderedImage = mActiveJob->renderedImage();
  const QPoint center = renderedImage.rect().center();

  QPainter painter( &renderedImage );
  painter.setRenderHint( QPainter::Antialiasing );
  painter.setPen( QPen( Qt::black, 2 ) );

  // draw a black border
  painter.setBackgroundMode( Qt::OpaqueMode );
  painter.drawRect( renderedImage.rect().adjusted( 1, 1, -1, -1 ) );

  // draw a circle around the center (ie. where the camera is looking at)
  const int arrowSize = 16;
  painter.setBrush( QColor( 0, 0, 0, 50 ) );
  painter.setPen( QPen( QColor( 0, 0, 0, 120 ), 1 ) );
  painter.drawEllipse( center, arrowSize, arrowSize );

  // draw an oriented arrow at the center
  painter.setBrush( QColor( 0, 0, 0, 160 ) );
  painter.setPen( Qt::NoPen );

  QPolygon arrow;
  arrow << QPoint( 0, -arrowSize )     // arrowhead
        << QPoint( -arrowSize / 3, 0 ) // left corner
        << QPoint( arrowSize / 3, 0 ); // right corner

  painter.translate( center );
  // Qt applies rotation in a clockwise direction with the Y-axis points downward
  painter.rotate( -mRotation );
  painter.drawPolygon( arrow );
  painter.end();

  mActiveJob->deleteLater();
  mActiveJob = nullptr;

  QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Texture" ), QString::number( mLastJobId ) );

  // pass QImage further
  emit textureReady( renderedImage );
}

QgsMapSettings QgsMapOverlayTextureGenerator::baseMapSettings() const
{
  QgsMapSettings mapSettings;

  mapSettings.setOutputSize( mSize );
  mapSettings.setDestinationCrs( m3DMapSettings.sceneMode() == Qgis::SceneMode::Globe ? m3DMapSettings.crs().toGeographicCrs() : m3DMapSettings.crs() );
  mapSettings.setBackgroundColor( m3DMapSettings.backgroundColor() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, m3DMapSettings.showLabels() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::Render3DMap );
  mapSettings.setTransformContext( m3DMapSettings.transformContext() );
  mapSettings.setPathResolver( m3DMapSettings.pathResolver() );
  mapSettings.setRendererUsage( m3DMapSettings.rendererUsage() );

  QList<QgsMapLayer *> layers;
  QgsMapThemeCollection *mapThemes = m3DMapSettings.mapThemeCollection();
  QString mapThemeName = m3DMapSettings.terrainMapTheme();
  if ( mapThemeName.isEmpty() || !mapThemes || !mapThemes->hasMapTheme( mapThemeName ) )
  {
    layers = m3DMapSettings.layers();
  }
  else
  {
    layers = mapThemes->mapThemeVisibleLayers( mapThemeName );
    mapSettings.setLayerStyleOverrides( mapThemes->mapThemeStyleOverrides( mapThemeName ) );
  }
  mapSettings.setLayers( layers );

  return mapSettings;
}

/// @endcond
