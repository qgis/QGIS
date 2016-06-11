/***************************************************************************
    qgsmaptooloffsetpointsymbol.h
    -----------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooloffsetpointsymbol.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgspointmarkeritem.h"
#include "qgsrendererv2.h"
#include "qgssnappingutils.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2.h"
#include "qgsdatadefined.h"
#include "qgisapp.h"

#include <QGraphicsPixmapItem>
#include <QMouseEvent>

QgsMapToolOffsetPointSymbol::QgsMapToolOffsetPointSymbol( QgsMapCanvas* canvas )
    : QgsMapToolPointSymbol( canvas )
    , mOffsetting( false )
    , mOffsetItem( nullptr )
    , mSymbolRotation( 0.0 )
{}

QgsMapToolOffsetPointSymbol::~QgsMapToolOffsetPointSymbol()
{
  delete mOffsetItem;
}

bool QgsMapToolOffsetPointSymbol::layerIsOffsetable( QgsMapLayer* ml )
{
  if ( !ml )
  {
    return false;
  }

  //a vector layer
  QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer *>( ml );
  if ( !vLayer )
  {
    return false;
  }

  //does it have point or multipoint type?
  if ( vLayer->geometryType() != QGis::Point )
  {
    return false;
  }

  //we consider all point layers as offsetable, as data defined offset can be set on a per
  //symbol/feature basis
  return true;
}

void QgsMapToolOffsetPointSymbol::canvasPressEvent( QgsMapMouseEvent* e )
{
  mMarkerSymbol.reset( nullptr );
  mClickedPoint = e->mapPoint();
  mSymbolRotation = 0.0;
  QgsMapToolPointSymbol::canvasPressEvent( e );
}

void QgsMapToolOffsetPointSymbol::canvasPressOnFeature( QgsMapMouseEvent *e, const QgsFeature &feature, const QgsPoint &snappedPoint )
{
  Q_UNUSED( e );
  mClickedFeature = feature;
  createPreviewItem( mMarkerSymbol.data() );
  mOffsetItem->setPointLocation( snappedPoint );
  updateOffsetPreviewItem( mClickedPoint, mClickedPoint );
  mOffsetting = true;
}

bool QgsMapToolOffsetPointSymbol::checkSymbolCompatibility( QgsMarkerSymbolV2* markerSymbol, QgsRenderContext &context )
{
  bool ok = false;

  Q_FOREACH ( QgsSymbolLayerV2* layer, markerSymbol->symbolLayers() )
  {
    if ( !layer->hasDataDefinedProperty( "offset" ) )
      continue;

    if ( layer->getDataDefinedProperty( "offset" )->useExpression() )
      continue;

    ok = true;
    if ( mMarkerSymbol.isNull() )
    {
      double symbolRotation = markerSymbol->angle();
      if ( layer->hasDataDefinedProperty( "angle" ) )
      {
        QString rotationExp = layer->getDataDefinedProperty( "angle" )->expressionOrField();
        QgsExpression exp( rotationExp );
        QVariant val = exp.evaluate( &context.expressionContext() );
        bool convertOk = false;
        double rotation = val.toDouble( &convertOk );
        if ( convertOk )
          symbolRotation = rotation;
      }

      mSymbolRotation = symbolRotation;
      mMarkerSymbol.reset( markerSymbol->clone() );
    }
  }
  return ok;
}

void QgsMapToolOffsetPointSymbol::noCompatibleSymbols()
{
  emit messageEmitted( tr( "The selected point does not have an offset attribute set." ), QgsMessageBar::CRITICAL );
}

void QgsMapToolOffsetPointSymbol::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mOffsetting )
  {
    return;
  }

  updateOffsetPreviewItem( mClickedPoint, e->mapPoint() );
}

void QgsMapToolOffsetPointSymbol::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );

  if ( mOffsetting && mActiveLayer )
  {
    QMap<int, QVariant> attrs = calculateNewOffsetAttributes( mClickedPoint, e->mapPoint() );
    mActiveLayer->beginEditCommand( tr( "Offset symbol" ) );
    bool offsetSuccess = true;

    //write offset to attributes
    QMap<int, QVariant>::const_iterator it = attrs.constBegin();
    for ( ; it != attrs.constEnd(); ++it )
    {
      if ( !mActiveLayer->changeAttributeValue( mFeatureNumber, it.key(), it.value() ) )
      {
        offsetSuccess = false;
      }
    }

    if ( offsetSuccess )
    {
      mActiveLayer->endEditCommand();
    }
    else
    {
      mActiveLayer->destroyEditCommand();
    }
  }
  mOffsetting = false;
  delete mOffsetItem;
  mOffsetItem = nullptr;
  if ( mActiveLayer )
    mActiveLayer->triggerRepaint();
}

void QgsMapToolOffsetPointSymbol::createPreviewItem( QgsMarkerSymbolV2* markerSymbol )
{
  delete mOffsetItem;
  mOffsetItem = nullptr;

  if ( !mCanvas )
  {
    return;
  }

  mOffsetItem = new QgsPointMarkerItem( mCanvas );
  mOffsetItem->setTransparency( 0.3 );
  mOffsetItem->setSymbol( markerSymbol->clone() );
}

QMap<int, QVariant> QgsMapToolOffsetPointSymbol::calculateNewOffsetAttributes( const QgsPoint& startPoint, const QgsPoint& endPoint ) const
{
  QMap<int, QVariant> newAttrValues;
  Q_FOREACH ( QgsSymbolLayerV2* layer, mMarkerSymbol->symbolLayers() )
  {
    if ( !layer->hasDataDefinedProperty( "offset" ) )
      continue;

    if ( layer->getDataDefinedProperty( "offset" )->useExpression() )
      continue;

    QgsMarkerSymbolLayerV2* ml = dynamic_cast< QgsMarkerSymbolLayerV2* >( layer );
    if ( !ml )
      continue;

    QPointF offset = calculateOffset( startPoint, endPoint, ml->offsetUnit() );
    int fieldIdx = mActiveLayer->fields().indexFromName( layer->getDataDefinedProperty( "offset" )->field() );
    if ( fieldIdx >= 0 )
      newAttrValues[ fieldIdx ] = QgsSymbolLayerV2Utils::encodePoint( offset );
  }
  return newAttrValues;
}

void QgsMapToolOffsetPointSymbol::updateOffsetPreviewItem( const QgsPoint& startPoint, const QgsPoint& endPoint )
{
  if ( !mOffsetItem )
    return;

  QgsFeature f = mClickedFeature;
  QMap<int, QVariant> attrs = calculateNewOffsetAttributes( startPoint, endPoint );
  QMap<int, QVariant>::const_iterator it = attrs.constBegin();
  for ( ; it != attrs.constEnd(); ++it )
  {
    f.setAttribute( it.key(), it.value() );
  }

  mOffsetItem->setFeature( f );
  mOffsetItem->updateSize();
}

QPointF QgsMapToolOffsetPointSymbol::calculateOffset( const QgsPoint& startPoint, const QgsPoint& endPoint, QgsSymbolV2::OutputUnit unit ) const
{
  double dx = endPoint.x() - startPoint.x();
  double dy = -( endPoint.y() - startPoint.y() );

  double factor = 1.0;

  switch ( unit )
  {
    case QgsSymbolV2::MM:
      factor = 25.4 / mCanvas->mapSettings().outputDpi() / mCanvas->mapSettings().mapUnitsPerPixel() ;
      break;

    case QgsSymbolV2::Pixel:
      factor = 1.0 / mCanvas->mapSettings().mapUnitsPerPixel();
      break;

    case QgsSymbolV2::MapUnit:
      factor = 1.0;
      break;

    case QgsSymbolV2::Mixed:
    case QgsSymbolV2::Percentage:
      //no sensible value
      factor = 1.0;
      break;
  }

  return rotatedOffset( QPointF( dx * factor, dy * factor ), mSymbolRotation );
}

QPointF QgsMapToolOffsetPointSymbol::rotatedOffset( QPointF offset, double angle ) const
{
  angle = DEG2RAD( 360 - angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

