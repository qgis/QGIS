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
#include "qgsrenderer.h"
#include "qgssnappingutils.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgsproperty.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"


#include <QGraphicsPixmapItem>


QgsMapToolRotatePointSymbols::QgsMapToolRotatePointSymbols( QgsMapCanvas *canvas )
  : QgsMapToolPointSymbol( canvas )
  , mCurrentMouseAzimut( 0.0 )
  , mCurrentRotationFeature( 0.0 )
  , mRotating( false )
  , mCtrlPressed( false )
{}

QgsMapToolRotatePointSymbols::~QgsMapToolRotatePointSymbols()
{
  delete mRotationItem;
}

bool QgsMapToolRotatePointSymbols::layerIsRotatable( QgsMapLayer *ml )
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

  //we consider all point layers as rotatable, as data defined rotation can be set on a per
  //symbol/feature basis
  return true;
}

void QgsMapToolRotatePointSymbols::canvasPressEvent( QgsMapMouseEvent *e )
{
  mCurrentRotationAttributes.clear();
  mMarkerSymbol.reset( nullptr );
  QgsMapToolPointSymbol::canvasPressEvent( e );
}

void QgsMapToolRotatePointSymbols::canvasPressOnFeature( QgsMapMouseEvent *e, const QgsFeature &feature, const QgsPointXY &snappedPoint )
{
  //find out initial arrow direction
  QVariant attrVal = feature.attribute( mCurrentRotationAttributes.toList().at( 0 ) );

  mCurrentRotationFeature = attrVal.toDouble();
  createPixmapItem( mMarkerSymbol.get() );
  if ( mRotationItem )
  {
    mRotationItem->setPointLocation( snappedPoint );
  }
  mCurrentMouseAzimut = calculateAzimut( e->pos() );
  setPixmapItemRotation( ( int )( mCurrentMouseAzimut ) );
  mRotating = true;
}

bool QgsMapToolRotatePointSymbols::checkSymbolCompatibility( QgsMarkerSymbol *markerSymbol, QgsRenderContext & )
{
  bool ok = false;
  QgsProperty ddAngle( markerSymbol->dataDefinedAngle() );
  if ( ddAngle && ddAngle.isActive() && ddAngle.propertyType() == QgsProperty::FieldBasedProperty )
  {
    mCurrentRotationAttributes << mActiveLayer->fields().indexFromName( ddAngle.field() );
    ok = true;
    if ( !mMarkerSymbol )
    {
      mMarkerSymbol.reset( markerSymbol->clone() );
    }
  }
  return ok;
}

void QgsMapToolRotatePointSymbols::noCompatibleSymbols()
{
  emit messageEmitted( tr( "The selected point does not have a rotation attribute set." ), Qgis::Critical );
}

void QgsMapToolRotatePointSymbols::canvasMoveEvent( QgsMapMouseEvent *e )
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

void QgsMapToolRotatePointSymbols::canvasReleaseEvent( QgsMapMouseEvent *e )
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
      rotation = static_cast<int>( mCurrentRotationFeature );
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
  return 180 - std::atan2( ( double ) dx, ( double ) dy ) * 180.0 / M_PI;
}

void QgsMapToolRotatePointSymbols::createPixmapItem( QgsMarkerSymbol *markerSymbol )
{
  if ( !mCanvas )
  {
    return;
  }

  //get the image that is used for that symbol, but without point rotation
  QImage pointImage;

  if ( markerSymbol )
  {
    QgsSymbol *clone = markerSymbol->clone();
    QgsMarkerSymbol *markerClone = static_cast<QgsMarkerSymbol *>( clone );
    markerClone->setDataDefinedAngle( QgsProperty() );
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

