/***************************************************************************
                              qgsmapcanvasannotationitem.cpp
                              ------------------------------
  begin                : January 2017
  copyright            : (C) 2017 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvasannotationitem.h"
#include "qgsannotation.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include "qgsfeatureiterator.h"
#include "qgsexception.h"
#include "qgssymbollayerutils.h"
#include "qgsproject.h"
#include "qgsannotationmanager.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

#include <QPainter>


QgsMapCanvasAnnotationItem::QgsMapCanvasAnnotationItem( QgsAnnotation *annotation, QgsMapCanvas *mapCanvas )
  : QgsMapCanvasItem( mapCanvas )
  , mAnnotation( annotation )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  if ( mapCanvas && !mapCanvas->annotationsVisible() )
    setVisible( false );

  connect( mAnnotation, &QgsAnnotation::appearanceChanged, this, [this] { update(); } );
  connect( mAnnotation, &QgsAnnotation::moved, this, [this] { updatePosition(); } );
  connect( mAnnotation, &QgsAnnotation::moved, this, &QgsMapCanvasAnnotationItem::setFeatureForMapPosition );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ] { updatePosition(); } );

  connect( mAnnotation, &QgsAnnotation::appearanceChanged, this, &QgsMapCanvasAnnotationItem::updateBoundingRect );

  connect( mMapCanvas, &QgsMapCanvas::layersChanged, this, &QgsMapCanvasAnnotationItem::onCanvasLayersChanged );
  connect( mAnnotation, &QgsAnnotation::mapLayerChanged, this, &QgsMapCanvasAnnotationItem::onCanvasLayersChanged );

  //lifetime is tied to annotation!
  connect( mAnnotation, &QgsAnnotation::destroyed, this, &QgsMapCanvasAnnotationItem::annotationDeleted );

  updatePosition();
  setFeatureForMapPosition();
}

void QgsMapCanvasAnnotationItem::updatePosition()
{
  if ( !mAnnotation )
    return;

  if ( mAnnotation->hasFixedMapPosition() )
  {
    const QgsCoordinateTransform t( mAnnotation->mapPositionCrs(), mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    QgsPointXY coord = mAnnotation->mapPosition();
    try
    {
      coord = t.transform( coord );
    }
    catch ( QgsCsException & )
    {}
    setPos( toCanvasCoordinates( coord ) );
  }
  else
  {
    //relative position

    const double x = mAnnotation->relativePosition().x() * mMapCanvas->width();
    const double y = mAnnotation->relativePosition().y() * mMapCanvas->height();
    setPos( x, y );
  }
  updateBoundingRect();
}

QRectF QgsMapCanvasAnnotationItem::boundingRect() const
{
  return mBoundingRect;
}

void QgsMapCanvasAnnotationItem::updateBoundingRect()
{
  prepareGeometryChange();

  const QgsRenderContext rc = QgsRenderContext::fromQPainter( nullptr );
  const double fillSymbolBleed = mAnnotation && mAnnotation->fillSymbol() ?
                                 QgsSymbolLayerUtils::estimateMaxSymbolBleed( mAnnotation->fillSymbol(), rc ) : 0;

  const double mmToPixelScale = mMapCanvas->logicalDpiX() / 25.4;

  if ( mAnnotation && !mAnnotation->hasFixedMapPosition() )
  {
    mBoundingRect = QRectF( - fillSymbolBleed, -fillSymbolBleed,
                            mmToPixelScale * mAnnotation->frameSizeMm().width() + fillSymbolBleed * 2,
                            mmToPixelScale * mAnnotation->frameSizeMm().height() + fillSymbolBleed * 2 );
  }
  else
  {
    double halfSymbolSize = 0.0;
    if ( mAnnotation && mAnnotation->markerSymbol() )
    {
      halfSymbolSize = scaledSymbolSize() / 2.0;
    }

    const QPointF offset = mAnnotation ? QPointF( mAnnotation->frameOffsetFromReferencePointMm().x() * mmToPixelScale,
                           mAnnotation->frameOffsetFromReferencePointMm().y() * mmToPixelScale ) : QPointF( 0, 0 );

    const QSizeF frameSize = mAnnotation ? QSizeF( mAnnotation->frameSizeMm().width() * mmToPixelScale,
                             mAnnotation->frameSizeMm().height() * mmToPixelScale ) : QSizeF( 0.0, 0.0 );

    const double xMinPos = std::min( -halfSymbolSize, offset.x() - fillSymbolBleed );
    const double xMaxPos = std::max( halfSymbolSize, offset.x() + frameSize.width() + fillSymbolBleed );
    const double yMinPos = std::min( -halfSymbolSize, offset.y() - fillSymbolBleed );
    const double yMaxPos = std::max( halfSymbolSize, offset.y() + frameSize.height() + fillSymbolBleed );
    mBoundingRect = QRectF( xMinPos, yMinPos, xMaxPos - xMinPos, yMaxPos - yMinPos );
  }
}

void QgsMapCanvasAnnotationItem::onCanvasLayersChanged()
{
  if ( !mAnnotation )
    return;
  if ( !mMapCanvas->annotationsVisible() )
  {
    setVisible( false );
  }
  else if ( !mAnnotation->mapLayer() )
  {
    setVisible( true );
  }
  else
  {
    setVisible( mMapCanvas->mapSettings().layers( true ).contains( mAnnotation->mapLayer() ) );
  }
}

void QgsMapCanvasAnnotationItem::setFeatureForMapPosition()
{
  if ( !mAnnotation || !mAnnotation->hasFixedMapPosition() )
    return;

  QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( mAnnotation->mapLayer() );
  if ( !vectorLayer )
    return;

  const double halfIdentifyWidth = QgsMapTool::searchRadiusMU( mMapCanvas );
  QgsPointXY mapPosition = mAnnotation->mapPosition();

  try
  {
    const QgsCoordinateTransform ct( mAnnotation->mapPositionCrs(), mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    if ( ct.isValid() )
      mapPosition = ct.transform( mapPosition );
  }
  catch ( QgsCsException & )
  {
  }

  QgsRectangle searchRect( mapPosition.x() - halfIdentifyWidth, mapPosition.y() - halfIdentifyWidth,
                           mapPosition.x() + halfIdentifyWidth, mapPosition.y() + halfIdentifyWidth );

  searchRect = mMapCanvas->mapSettings().mapToLayerCoordinates( vectorLayer, searchRect );

  QgsFeatureIterator fit = vectorLayer->getFeatures( QgsFeatureRequest().setFilterRect( searchRect ).setFlags( QgsFeatureRequest::ExactIntersect ).setLimit( 1 ) );

  QgsFeature currentFeature;
  ( void )fit.nextFeature( currentFeature );
  mAnnotation->setAssociatedFeature( currentFeature );
}

void QgsMapCanvasAnnotationItem::annotationDeleted()
{
  mAnnotation = nullptr;
  deleteLater();
}

void QgsMapCanvasAnnotationItem::drawSelectionBoxes( QPainter *p ) const
{
  if ( !p )
  {
    return;
  }

  const double handlerSize = 10;
  p->setPen( Qt::NoPen );
  p->setBrush( QColor( 200, 200, 210, 120 ) );
  p->drawRect( QRectF( mBoundingRect.left(), mBoundingRect.top(), handlerSize, handlerSize ) );
  p->drawRect( QRectF( mBoundingRect.right() - handlerSize, mBoundingRect.top(), handlerSize, handlerSize ) );
  p->drawRect( QRectF( mBoundingRect.right() - handlerSize, mBoundingRect.bottom() - handlerSize, handlerSize, handlerSize ) );
  p->drawRect( QRectF( mBoundingRect.left(), mBoundingRect.bottom() - handlerSize, handlerSize, handlerSize ) );
}

QgsMapCanvasAnnotationItem::MouseMoveAction QgsMapCanvasAnnotationItem::moveActionForPosition( QPointF pos ) const
{
  const QPointF itemPos = mapFromScene( pos );

  const int cursorSensitivity = 7;

  if ( mAnnotation && mAnnotation->hasFixedMapPosition() &&
       std::fabs( itemPos.x() ) < cursorSensitivity && std::fabs( itemPos.y() ) < cursorSensitivity ) //move map point if position is close to the origin
  {
    return MoveMapPosition;
  }

  const double mmToPixelScale = mMapCanvas->logicalDpiX() / 25.4;

  const QPointF offset = mAnnotation && mAnnotation->hasFixedMapPosition() ? mAnnotation->frameOffsetFromReferencePointMm() * mmToPixelScale : QPointF( 0, 0 );
  const QSizeF frameSize = mAnnotation ? mAnnotation->frameSizeMm() * mmToPixelScale : QSizeF( 0, 0 );

  bool left, right, up, down;
  left = std::fabs( itemPos.x() - offset.x() ) < cursorSensitivity;
  right = std::fabs( itemPos.x() - ( offset.x() + frameSize.width() ) ) < cursorSensitivity;
  up = std::fabs( itemPos.y() - offset.y() ) < cursorSensitivity;
  down = std::fabs( itemPos.y() - ( offset.y() + frameSize.height() ) ) < cursorSensitivity;

  if ( left && up )
  {
    return ResizeFrameLeftUp;
  }
  else if ( right && up )
  {
    return ResizeFrameRightUp;
  }
  else if ( left && down )
  {
    return ResizeFrameLeftDown;
  }
  else if ( right && down )
  {
    return ResizeFrameRightDown;
  }
  if ( left )
  {
    return ResizeFrameLeft;
  }
  if ( right )
  {
    return ResizeFrameRight;
  }
  if ( up )
  {
    return ResizeFrameUp;
  }
  if ( down )
  {
    return ResizeFrameDown;
  }

  //finally test if pos is in the frame area
  if ( itemPos.x() >= offset.x() && itemPos.x() <= ( offset.x() + frameSize.width() )
       && itemPos.y() >= offset.y() && itemPos.y() <= ( offset.y() + frameSize.height() ) )
  {
    return MoveFramePosition;
  }
  return NoAction;
}

Qt::CursorShape QgsMapCanvasAnnotationItem::cursorShapeForAction( MouseMoveAction moveAction ) const
{
  switch ( moveAction )
  {
    case NoAction:
      return Qt::ArrowCursor;
    case MoveMapPosition:
    case MoveFramePosition:
      return Qt::SizeAllCursor;
    case ResizeFrameUp:
    case ResizeFrameDown:
      return Qt::SizeVerCursor;
    case ResizeFrameLeft:
    case ResizeFrameRight:
      return Qt::SizeHorCursor;
    case ResizeFrameLeftUp:
    case ResizeFrameRightDown:
      return Qt::SizeFDiagCursor;
    case ResizeFrameRightUp:
    case ResizeFrameLeftDown:
      return Qt::SizeBDiagCursor;
    default:
      return Qt::ArrowCursor;
  }
}

double QgsMapCanvasAnnotationItem::scaledSymbolSize() const
{
  if ( !mAnnotation || !mAnnotation->markerSymbol() )
  {
    return 0.0;
  }

  if ( !mMapCanvas )
  {
    return mAnnotation->markerSymbol()->size();
  }

  const double dpmm = mMapCanvas->logicalDpiX() / 25.4;
  return dpmm * mAnnotation->markerSymbol()->size();
}

void QgsMapCanvasAnnotationItem::paint( QPainter *painter )
{
  if ( !mAnnotation || !mAnnotation->isVisible() )
    return;

  QgsRenderContext rc = QgsRenderContext::fromQPainter( painter );
  rc.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

  if ( mAnnotation )
    mAnnotation->render( rc );

  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}
