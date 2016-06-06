/***************************************************************************
    qgsmaptoolrotatepointsymbols.cpp
    --------------------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgspointrotationitem.h"
#include "qgsrendererv2.h"
#include "qgssnappingutils.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgsdatadefined.h"
#include "qgisapp.h"

#include <QGraphicsPixmapItem>
#include <QMouseEvent>

QgsMapToolRotatePointSymbols::QgsMapToolRotatePointSymbols( QgsMapCanvas* canvas )
    : QgsMapToolPointSymbol( canvas )
    , mCurrentMouseAzimut( 0.0 )
    , mCurrentRotationFeature( 0.0 )
    , mRotating( false )
    , mRotationItem( nullptr )
    , mCtrlPressed( false )
{}

QgsMapToolRotatePointSymbols::~QgsMapToolRotatePointSymbols()
{
  delete mRotationItem;
}

bool QgsMapToolRotatePointSymbols::layerIsRotatable( QgsMapLayer* ml )
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

  //we consider all point layers as rotatable, as data defined rotation can be set on a per
  //symbol/feature basis
  return true;
}

void QgsMapToolRotatePointSymbols::canvasPressEvent( QgsMapMouseEvent* e )
{
  mCurrentRotationAttributes.clear();
  mMarkerSymbol.reset( nullptr );
  QgsMapToolPointSymbol::canvasPressEvent( e );
}

void QgsMapToolRotatePointSymbols::canvasPressOnFeature( QgsMapMouseEvent *e, const QgsFeature &feature, const QgsPoint &snappedPoint )
{
  //find out initial arrow direction
  QVariant attrVal = feature.attribute( mCurrentRotationAttributes.toList().at( 0 ) );
  if ( !attrVal.isValid() )
  {
    return;
  }

  mCurrentRotationFeature = attrVal.toDouble();
  createPixmapItem( mMarkerSymbol.data() );
  if ( mRotationItem )
  {
    mRotationItem->setPointLocation( snappedPoint );
  }
  mCurrentMouseAzimut = calculateAzimut( e->pos() );
  setPixmapItemRotation(( int )( mCurrentMouseAzimut ) );
  mRotating = true;
}

bool QgsMapToolRotatePointSymbols::checkSymbolCompatibility( QgsMarkerSymbolV2* markerSymbol, QgsRenderContext& )
{
  bool ok = false;
  if ( markerSymbol->dataDefinedAngle().isActive() && !markerSymbol->dataDefinedAngle().useExpression() )
  {
    mCurrentRotationAttributes << mActiveLayer->fields().indexFromName( markerSymbol->dataDefinedAngle().field() );
    ok = true;
    if ( mMarkerSymbol.isNull() )
    {
      mMarkerSymbol.reset( markerSymbol->clone() );
    }
  }
  return ok;
}

void QgsMapToolRotatePointSymbols::noCompatibleSymbols()
{
  emit messageEmitted( tr( "The selected point does not have a rotation attribute set." ), QgsMessageBar::CRITICAL );
}

void QgsMapToolRotatePointSymbols::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mRotating )
  {
    return;
  }

  double azimut = calculateAzimut( e->pos() );
  double azimutDiff = azimut - mCurrentMouseAzimut;

  //assign new feature rotation, making sure to respect the 0 - 360 degree range
  mCurrentRotationFeature += azimutDiff;
  if ( mCurrentRotationFeature < 0 )
  {
    mCurrentRotationFeature = 360 - mCurrentRotationFeature;
  }
  else if ( mCurrentRotationFeature >= 360 )
  {
    mCurrentRotationFeature -= 360;
  }
  mCurrentMouseAzimut = azimut;
  if ( mCurrentMouseAzimut < 0 )
  {
    mCurrentMouseAzimut = 360 - mCurrentMouseAzimut;
  }
  else if ( mCurrentMouseAzimut >= 360 )
  {
    mCurrentMouseAzimut -= 360;
  }

  //if shift-modifier is pressed, round to 15 degrees
  int displayValue;
  if ( e->modifiers() & Qt::ControlModifier )
  {
    displayValue = roundTo15Degrees( mCurrentRotationFeature );
    mCtrlPressed = true;
  }
  else
  {
    displayValue = ( int )( mCurrentRotationFeature );
    mCtrlPressed = false;
  }
  setPixmapItemRotation( displayValue );
}

void QgsMapToolRotatePointSymbols::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );

  if ( mRotating && mActiveLayer )
  {
    mActiveLayer->beginEditCommand( tr( "Rotate symbol" ) );
    bool rotateSuccess = true;

    //write mCurrentRotationFeature to all rotation attributes of feature (mFeatureNumber)
    int rotation;
    if ( mCtrlPressed ) //round to 15 degrees
    {
      rotation = roundTo15Degrees( mCurrentRotationFeature );
    }
    else
    {
      rotation = ( int )mCurrentRotationFeature;
    }

    QSet<int>::const_iterator it = mCurrentRotationAttributes.constBegin();
    for ( ; it != mCurrentRotationAttributes.constEnd(); ++it )
    {
      if ( !mActiveLayer->changeAttributeValue( mFeatureNumber, *it, rotation ) )
      {
        rotateSuccess = false;
      }
    }

    if ( rotateSuccess )
    {
      mActiveLayer->endEditCommand();
    }
    else
    {
      mActiveLayer->destroyEditCommand();
    }
  }
  mRotating = false;
  delete mRotationItem;
  mRotationItem = nullptr;
  if ( mActiveLayer )
    mActiveLayer->triggerRepaint();
}

double QgsMapToolRotatePointSymbols::calculateAzimut( QPoint mousePos )
{
  int dx = mousePos.x() - mSnappedPoint.x();
  int dy = mousePos.y() - mSnappedPoint.y();
  return 180 - atan2(( double ) dx, ( double ) dy ) * 180.0 / M_PI;
}

void QgsMapToolRotatePointSymbols::createPixmapItem( QgsMarkerSymbolV2* markerSymbol )
{
  if ( !mCanvas )
  {
    return;
  }

  //get the image that is used for that symbol, but without point rotation
  QImage pointImage;

  if ( markerSymbol )
  {
    QgsSymbolV2* clone = markerSymbol->clone();
    QgsMarkerSymbolV2* markerClone = static_cast<QgsMarkerSymbolV2*>( clone );
    markerClone->setDataDefinedAngle( QgsDataDefined() );
    pointImage = markerClone->bigSymbolPreviewImage();
    delete clone;
  }

  mRotationItem = new QgsPointRotationItem( mCanvas );
  mRotationItem->setSymbol( pointImage );
}

void QgsMapToolRotatePointSymbols::setPixmapItemRotation( double rotation )
{
  if ( mRotationItem )
  {
    mRotationItem->setSymbolRotation( rotation );
    mRotationItem->update();
  }
}

int QgsMapToolRotatePointSymbols::roundTo15Degrees( double n )
{
  int m = ( int )( n / 15.0 + 0.5 );
  return ( m * 15 );
}

