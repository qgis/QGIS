/***************************************************************************
 *   qgscoordinateboundspreviewmapwidget.h                                 *
 *   Copyright (C) 2019 by Nyall Dawson                                    *
 *   nyall dot dawson at gmail dot com                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgscoordinateboundspreviewmapwidget.h"

#include "qgsapplication.h"
#include "qgsmaptoolpan.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QString>

#include "moc_qgscoordinateboundspreviewmapwidget.cpp"

using namespace Qt::StringLiterals;

QgsCoordinateBoundsPreviewMapWidget::QgsCoordinateBoundsPreviewMapWidget( QWidget *parent )
  : QgsMapCanvas( parent )
{
  mPreviewBand = new QgsRubberBand( this, Qgis::GeometryType::Polygon );
  mPreviewBand->setWidth( 4 );

  mCanvasPreviewBand = new QgsRubberBand( this, Qgis::GeometryType::Polygon );
  mCanvasPreviewBand->setWidth( 4 );
  const QColor rectColor = QColor( 185, 84, 210, 60 );
  mCanvasPreviewBand->setColor( rectColor );

  mCanvasCenterMarker = new QgsVertexMarker( this );
  mCanvasCenterMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mCanvasCenterMarker->setColor( QColor( 185, 84, 210 ) );
  mCanvasCenterMarker->setPenWidth( 3 );

  const QgsCoordinateReferenceSystem srs( u"EPSG:4326"_s );
  setDestinationCrs( srs );

  const QString layerPath = QgsApplication::pkgDataPath() + u"/resources/data/world_map.gpkg|layername=countries"_s;
  QgsVectorLayer::LayerOptions options;
  options.forceReadOnly = true;
  mLayers << new QgsVectorLayer( layerPath, tr( "World Map" ), u"ogr"_s, options );
  setLayers( mLayers );
  mPanTool = new QgsMapToolPan( this );
  setMapTool( mPanTool );
  setPreviewJobsEnabled( true );
}

QgsCoordinateBoundsPreviewMapWidget::~QgsCoordinateBoundsPreviewMapWidget()
{
  setMapTool( nullptr );


  qDeleteAll( mLayers );
}

void QgsCoordinateBoundsPreviewMapWidget::setPreviewRect( const QgsRectangle &rect )
{
  if ( !qgsDoubleNear( rect.area(), 0.0 ) )
  {
    const QgsGeometry geom = setBandGeometryToRect( mPreviewBand, rect );
    mPreviewBand->setColor( QColor( 255, 0, 0, 65 ) );
    QgsRectangle extent = geom.boundingBox();
    extent.scale( 1.1 );
    setExtent( extent );
    refresh();
    mPreviewBand->show();
  }
  else
  {
    mPreviewBand->hide();
    zoomToFullExtent();
  }
}

QgsGeometry QgsCoordinateBoundsPreviewMapWidget::setBandGeometryToRect( QgsRubberBand *band, const QgsRectangle &rect ) const
{
  QgsGeometry geom;

  const std::pair< QgsRectangle, QgsRectangle > parts = rect.splitAtAntimeridian();
  if ( !parts.second.isNull() )
  {
    geom = QgsGeometry::fromRect( parts.first );
    geom.addPart( QgsGeometry::fromRect( parts.second ) );
  }
  else
  {
    geom = QgsGeometry::fromRect( parts.first );
  }
  band->setToGeometry( geom, nullptr );
  return geom;
}

QgsRectangle QgsCoordinateBoundsPreviewMapWidget::canvasRect() const
{
  return mCanvasRect;
}

void QgsCoordinateBoundsPreviewMapWidget::setCanvasRect( const QgsRectangle &rect )
{
  mCanvasRect = rect;
  const QgsGeometry geom = setBandGeometryToRect( mCanvasPreviewBand, rect );
  mCanvasPreviewBand->show();

  const std::pair< QgsRectangle, QgsRectangle > parts = rect.splitAtAntimeridian();
  if ( !parts.second.isNull() )
  {
    const double y = rect.center().y();
    const double x1 = parts.first.center().x();
    const double x2 = parts.second.center().x() + 360;
    double x = ( x1 + x2 ) * 0.5;
    if ( x > 180 )
      x -= 360;

    mCanvasCenterMarker->setCenter( QgsPointXY( x, y ) );
  }
  else
  {
    mCanvasCenterMarker->setCenter( rect.center() );
  }

  mCanvasCenterMarker->show();
}
