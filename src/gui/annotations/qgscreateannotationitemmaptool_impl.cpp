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

bool QgsMapToolCaptureAnnotationItem::supportsTechnique( CaptureTechnique ) const
{
  return true;
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

void QgsCreateLineItemMapTool::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
    if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), Qgis::MessageLevel::Warning );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    deleteTempRubberBand();

    //find out bounding box of mCaptureList
    if ( size() < 1 )
    {
      stopCapturing();
      return;
    }

    // do it!
    std::unique_ptr< QgsAbstractGeometry > geometry( captureCurve()->simplifiedTypeRef()->clone() );
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
    stopCapturing();
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

void QgsCreatePolygonItemMapTool::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
    if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), Qgis::MessageLevel::Warning );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    deleteTempRubberBand();

    //find out bounding box of mCaptureList
    if ( size() < 1 )
    {
      stopCapturing();
      return;
    }

    closePolygon();

    std::unique_ptr< QgsAbstractGeometry > geometry( captureCurve()->simplifiedTypeRef()->clone() );
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
    stopCapturing();
  }
}

///@endcond PRIVATE

