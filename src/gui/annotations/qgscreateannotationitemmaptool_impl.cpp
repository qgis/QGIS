/***************************************************************************
                             qgscreateannotationitemmaptool_impl.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscreateannotationitemmaptool_impl.h"
#include "qgsmapmouseevent.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationlayer.h"
#include "qgsstyle.h"
#include "qgsmapcanvas.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsrecentstylehandler.h"

///@cond PRIVATE

//
// QgsMapToolCaptureAnnotationItem
//

QgsMapToolCaptureAnnotationItem::QgsMapToolCaptureAnnotationItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolCapture( canvas, cadDockWidget, mode )
{
  mToolName = tr( "Annotation tool" );
}

QgsCreateAnnotationItemMapToolHandler *QgsMapToolCaptureAnnotationItem::handler()
{
  return mHandler;
}

QgsMapTool *QgsMapToolCaptureAnnotationItem::mapTool()
{
  return this;
}

QgsMapLayer *QgsMapToolCaptureAnnotationItem::layer() const
{
  return mHandler->targetLayer();
}


QgsMapToolCapture::Capabilities QgsMapToolCaptureAnnotationItem::capabilities() const
{
  // no geometry validation!
  return SupportsCurves;
}

bool QgsMapToolCaptureAnnotationItem::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::Shape:
      return true;
  }
  BUILTIN_UNREACHABLE
}


//
// QgsCreatePointTextItemMapTool
//

QgsCreatePointTextItemMapTool::QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget ) )
{

}

QgsCreatePointTextItemMapTool::~QgsCreatePointTextItemMapTool() = default;

void QgsCreatePointTextItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );

  std::unique_ptr< QgsAnnotationPointTextItem > createdItem = std::make_unique< QgsAnnotationPointTextItem >( tr( "Text" ), layerPoint );
  createdItem->setAlignment( Qt::AlignLeft );
  createdItem->setFormat( QgsStyle::defaultStyle()->defaultTextFormat( QgsStyle::TextFormatContext::Labeling ) );
  // newly created point text items default to using symbology reference scale at the current map scale
  createdItem->setUseSymbologyReferenceScale( true );
  createdItem->setSymbologyReferenceScale( canvas()->scale() );
  mHandler->pushCreatedItem( createdItem.release() );
}

QgsCreateAnnotationItemMapToolHandler *QgsCreatePointTextItemMapTool::handler()
{
  return mHandler;
}

QgsMapTool *QgsCreatePointTextItemMapTool::mapTool()
{
  return this;
}



//
// QgsCreateMarkerMapTool
//

QgsCreateMarkerItemMapTool::QgsCreateMarkerItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CapturePoint )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreateMarkerItemMapTool::cadCanvasReleaseEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );
  std::unique_ptr< QgsAnnotationMarkerItem > createdItem = std::make_unique< QgsAnnotationMarkerItem >( QgsPoint( layerPoint ) );

  std::unique_ptr< QgsMarkerSymbol > markerSymbol = QgsApplication::recentStyleHandler()->recentSymbol< QgsMarkerSymbol >( QStringLiteral( "marker_annotation_item" ) );
  if ( !markerSymbol )
    markerSymbol.reset( qgis::down_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
  createdItem->setSymbol( markerSymbol.release() );

  // set reference scale to match canvas scale, but don't enable it by default for marker items
  createdItem->setSymbologyReferenceScale( canvas()->scale() );

  mHandler->pushCreatedItem( createdItem.release() );

  stopCapturing();

  cadDockWidget()->clearPoints();
}

//
// QgsCreateLineMapTool
//

QgsCreateLineItemMapTool::QgsCreateLineItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CaptureLine )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreateLineItemMapTool::lineCaptured( const QgsCurve *line )
{
  // do it!
  std::unique_ptr< QgsAbstractGeometry > geometry( line->simplifiedTypeRef()->clone() );
  if ( qgsgeometry_cast< QgsCurve * >( geometry.get() ) )
  {
    std::unique_ptr< QgsAnnotationLineItem > createdItem = std::make_unique< QgsAnnotationLineItem >( qgsgeometry_cast< QgsCurve * >( geometry.release() ) );

    std::unique_ptr< QgsLineSymbol > lineSymbol = QgsApplication::recentStyleHandler()->recentSymbol< QgsLineSymbol >( QStringLiteral( "line_annotation_item" ) );
    if ( !lineSymbol )
      lineSymbol.reset( qgis::down_cast< QgsLineSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::LineGeometry ) ) );
    createdItem->setSymbol( lineSymbol.release() );

    // set reference scale to match canvas scale, but don't enable it by default for marker items
    createdItem->setSymbologyReferenceScale( canvas()->scale() );

    mHandler->pushCreatedItem( createdItem.release() );
  }
}

//
// QgsCreatePolygonItemMapTool
//

QgsCreatePolygonItemMapTool::QgsCreatePolygonItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CapturePolygon )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreatePolygonItemMapTool::polygonCaptured( const QgsCurvePolygon *polygon )
{
  std::unique_ptr< QgsAbstractGeometry > geometry( polygon->exteriorRing()->simplifiedTypeRef()->clone() );
  if ( qgsgeometry_cast< QgsCurve * >( geometry.get() ) )
  {
    std::unique_ptr< QgsCurvePolygon > newPolygon = std::make_unique< QgsCurvePolygon >();
    newPolygon->setExteriorRing( qgsgeometry_cast< QgsCurve * >( geometry.release() ) );
    std::unique_ptr< QgsAnnotationPolygonItem > createdItem = std::make_unique< QgsAnnotationPolygonItem >( newPolygon.release() );

    std::unique_ptr< QgsFillSymbol > fillSymbol = QgsApplication::recentStyleHandler()->recentSymbol< QgsFillSymbol >( QStringLiteral( "polygon_annotation_item" ) );
    if ( !fillSymbol )
      fillSymbol.reset( qgis::down_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) ) );
    createdItem->setSymbol( fillSymbol.release() );

    // set reference scale to match canvas scale, but don't enable it by default for marker items
    createdItem->setSymbologyReferenceScale( canvas()->scale() );

    mHandler->pushCreatedItem( createdItem.release() );
  }
}

///@endcond PRIVATE

