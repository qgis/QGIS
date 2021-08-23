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
#include "qgsrubberband.h"
#include "qgsvertexmarker.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsmaptoolpan.h"

QgsCoordinateBoundsPreviewMapWidget::QgsCoordinateBoundsPreviewMapWidget( QWidget *parent )
  : QgsMapCanvas( parent )
{
  mPreviewBand = new QgsRubberBand( this, QgsWkbTypes::PolygonGeometry );
  mPreviewBand->setWidth( 4 );

  mCanvasPreviewBand = new QgsRubberBand( this, QgsWkbTypes::PolygonGeometry );
  mCanvasPreviewBand->setWidth( 4 );
  const QColor rectColor = QColor( 185, 84, 210, 60 );
  mCanvasPreviewBand->setColor( rectColor );

  mCanvasCenterMarker = new QgsVertexMarker( this );
  mCanvasCenterMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mCanvasCenterMarker->setColor( QColor( 185, 84, 210 ) );
  mCanvasCenterMarker->setPenWidth( 3 );

  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:4326" ) );
  setDestinationCrs( srs );

  const QString layerPath = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/world_map.gpkg|layername=countries" );
  mLayers << new QgsVectorLayer( layerPath );
  setLayers( mLayers );
  setMapTool( new QgsMapToolPan( this ) );
  setPreviewJobsEnabled( true );
}

QgsCoordinateBoundsPreviewMapWidget::~QgsCoordinateBoundsPreviewMapWidget()
{
  qDeleteAll( mLayers );
  delete mPreviewBand;
  delete mCanvasPreviewBand;
  delete mCanvasCenterMarker;
}

void QgsCoordinateBoundsPreviewMapWidget::setPreviewRect( const QgsRectangle &rect )
{
  if ( !qgsDoubleNear( rect.area(), 0.0 ) )
  {
    QgsGeometry geom;
    if ( rect.xMinimum() > rect.xMaximum() )
    {
      const QgsRectangle rect1 = QgsRectangle( -180, rect.yMinimum(), rect.xMaximum(), rect.yMaximum() );
      const QgsRectangle rect2 = QgsRectangle( rect.xMinimum(), rect.yMinimum(), 180, rect.yMaximum() );
      geom = QgsGeometry::fromRect( rect1 );
      geom.addPart( QgsGeometry::fromRect( rect2 ) );
    }
    else
    {
      geom = QgsGeometry::fromRect( rect );
    }
    mPreviewBand->setToGeometry( geom, nullptr );
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

QgsRectangle QgsCoordinateBoundsPreviewMapWidget::canvasRect() const
{
  return mCanvasRect;
}

void QgsCoordinateBoundsPreviewMapWidget::setCanvasRect( const QgsRectangle &rect )
{
  mCanvasRect = rect;
  mCanvasPreviewBand->setToGeometry( QgsGeometry::fromRect( mCanvasRect ), nullptr );
  mCanvasPreviewBand->show();
  mCanvasCenterMarker->setCenter( rect.center() );
  mCanvasCenterMarker->show();
}
