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
#include "qgsrenderer.h"
#include "qgssnappingutils.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgisapp.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsmapmouseevent.h"
#include "qgsmarkersymbol.h"

#include <QGraphicsPixmapItem>

QgsMapToolOffsetPointSymbol::QgsMapToolOffsetPointSymbol( QgsMapCanvas *canvas )
  : QgsMapToolPointSymbol( canvas )
  , mOffsetting( false )
  , mSymbolRotation( 0.0 )
{
  mToolName = tr( "Map tool offset point symbol" );
}

QgsMapToolOffsetPointSymbol::~QgsMapToolOffsetPointSymbol()
{
  delete mOffsetItem;
}

bool QgsMapToolOffsetPointSymbol::layerIsOffsetable( QgsMapLayer *ml )
{
  if ( !ml )
  {
    return false;
  }

  //a vector layer
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( ml );
  if ( !vLayer )
  {
    return false;
  }

  //does it have point or multipoint type?
  if ( vLayer->geometryType() != QgsWkbTypes::PointGeometry )
  {
    return false;
  }

  //we consider all point layers as offsetable, as data defined offset can be set on a per
  //symbol/feature basis
  return true;
}

void QgsMapToolOffsetPointSymbol::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mOffsetting )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    // first click -- starts offsetting
    mMarkerSymbol.reset( nullptr );
    mClickedPoint = e->mapPoint();
    mSymbolRotation = 0.0;
    QgsMapToolPointSymbol::canvasPressEvent( e );
  }
  else
  {
    // second click stops it.
    // only left clicks "save" edits - right clicks discard them
    if ( e->button() == Qt::LeftButton && mActiveLayer )
    {
      const QMap<int, QVariant> attrs = calculateNewOffsetAttributes( mClickedPoint, e->mapPoint() );
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
}

void QgsMapToolOffsetPointSymbol::canvasPressOnFeature( QgsMapMouseEvent *e, const QgsFeature &feature, const QgsPointXY &snappedPoint )
{
  Q_UNUSED( e )
  mClickedFeature = feature;
  createPreviewItem( mMarkerSymbol.get() );
  mOffsetItem->setPointLocation( snappedPoint );
  updateOffsetPreviewItem( mClickedPoint, mClickedPoint );
  mOffsetting = true;
}

bool QgsMapToolOffsetPointSymbol::checkSymbolCompatibility( QgsMarkerSymbol *markerSymbol, QgsRenderContext &context )
{
  bool ok = false;

  const auto constSymbolLayers = markerSymbol->symbolLayers();
  for ( QgsSymbolLayer *layer : constSymbolLayers )
  {
    if ( !layer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyOffset ) )
      continue;

    const QgsProperty p = layer->dataDefinedProperties().property( QgsSymbolLayer::PropertyOffset );
    if ( p.propertyType() != QgsProperty::FieldBasedProperty )
      continue;

    ok = true;
    if ( !mMarkerSymbol )
    {
      double symbolRotation = markerSymbol->angle();
      if ( layer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyAngle ) )
      {
        symbolRotation = layer->dataDefinedProperties().valueAsDouble( QgsSymbolLayer::PropertyAngle, context.expressionContext(), symbolRotation );
      }

      mSymbolRotation = symbolRotation;
      mMarkerSymbol.reset( markerSymbol->clone() );
    }
  }
  return ok;
}

void QgsMapToolOffsetPointSymbol::noCompatibleSymbols()
{
  emit messageEmitted( tr( "The selected point does not have an offset attribute set." ), Qgis::MessageLevel::Critical );
}

void QgsMapToolOffsetPointSymbol::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mOffsetting )
  {
    return;
  }

  updateOffsetPreviewItem( mClickedPoint, e->mapPoint() );
}

void QgsMapToolOffsetPointSymbol::keyPressEvent( QKeyEvent *e )
{
  if ( mOffsetting && e && e->key() == Qt::Key_Escape && !e->isAutoRepeat() )
  {
    mOffsetting = false;
    delete mOffsetItem;
    mOffsetItem = nullptr;
  }
  else
  {
    QgsMapToolPointSymbol::keyPressEvent( e );
  }
}

void QgsMapToolOffsetPointSymbol::createPreviewItem( QgsMarkerSymbol *markerSymbol )
{
  delete mOffsetItem;
  mOffsetItem = nullptr;

  if ( !mCanvas )
  {
    return;
  }

  mOffsetItem = new QgsMapCanvasMarkerSymbolItem( mCanvas );
  mOffsetItem->setOpacity( 0.7 );
  mOffsetItem->setSymbol( std::unique_ptr< QgsSymbol >( markerSymbol->clone() ) );
}

QMap<int, QVariant> QgsMapToolOffsetPointSymbol::calculateNewOffsetAttributes( const QgsPointXY &startPoint, const QgsPointXY &endPoint ) const
{
  QMap<int, QVariant> newAttrValues;
  const auto constSymbolLayers = mMarkerSymbol->symbolLayers();
  for ( QgsSymbolLayer *layer : constSymbolLayers )
  {
    if ( !layer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyOffset ) )
      continue;

    const QgsProperty ddOffset = layer->dataDefinedProperties().property( QgsSymbolLayer::PropertyOffset );
    if ( ddOffset.propertyType() != QgsProperty::FieldBasedProperty )
      continue;

    QgsMarkerSymbolLayer *ml = dynamic_cast< QgsMarkerSymbolLayer * >( layer );
    if ( !ml )
      continue;

    const QPointF offset = calculateOffset( startPoint, endPoint, ml->offsetUnit() );
    const int fieldIdx = mActiveLayer->fields().indexFromName( ddOffset.field() );
    if ( fieldIdx >= 0 )
      newAttrValues[ fieldIdx ] = QgsSymbolLayerUtils::encodePoint( offset );
  }
  return newAttrValues;
}

void QgsMapToolOffsetPointSymbol::updateOffsetPreviewItem( const QgsPointXY &startPoint, const QgsPointXY &endPoint )
{
  if ( !mOffsetItem )
    return;

  QgsFeature f = mClickedFeature;
  const QMap<int, QVariant> attrs = calculateNewOffsetAttributes( startPoint, endPoint );
  QMap<int, QVariant>::const_iterator it = attrs.constBegin();
  for ( ; it != attrs.constEnd(); ++it )
  {
    f.setAttribute( it.key(), it.value() );
  }

  mOffsetItem->setFeature( f );
  mOffsetItem->updateSize();
}

QPointF QgsMapToolOffsetPointSymbol::calculateOffset( const QgsPointXY &startPoint, const QgsPointXY &endPoint, QgsUnitTypes::RenderUnit unit ) const
{
  const double dx = endPoint.x() - startPoint.x();
  const double dy = -( endPoint.y() - startPoint.y() );

  double factor = 1.0;

  switch ( unit )
  {
    case QgsUnitTypes::RenderMillimeters:
      factor = 25.4 / mCanvas->mapSettings().outputDpi() / mCanvas->mapSettings().mapUnitsPerPixel();
      break;

    case QgsUnitTypes::RenderPoints:
      factor = 2.83464567 * 25.4 / mCanvas->mapSettings().outputDpi() / mCanvas->mapSettings().mapUnitsPerPixel();
      break;

    case QgsUnitTypes::RenderInches:
      factor = 1.0 / mCanvas->mapSettings().outputDpi() / mCanvas->mapSettings().mapUnitsPerPixel();
      break;

    case QgsUnitTypes::RenderPixels:
      factor = 1.0 / mCanvas->mapSettings().mapUnitsPerPixel();
      break;

    case QgsUnitTypes::RenderMapUnits:
      factor = 1.0;
      break;

    case QgsUnitTypes::RenderMetersInMapUnits:
    {
      QgsDistanceArea distanceArea;
      distanceArea.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
      distanceArea.setEllipsoid( mCanvas->mapSettings().ellipsoid() );
      // factor=1.0 / 1 meter in MapUnits
      factor = 1.0 / distanceArea.measureLineProjected( startPoint );
    }
    break;
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      factor = 1.0;
      break;
  }

  return rotatedOffset( QPointF( dx * factor, dy * factor ), mSymbolRotation );
}

QPointF QgsMapToolOffsetPointSymbol::rotatedOffset( QPointF offset, double angle ) const
{
  angle = DEG2RAD( 360 - angle );
  double c = std::cos( angle ), s = std::sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

