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
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

QgsMapToolRotatePointSymbols::QgsMapToolRotatePointSymbols( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ),
    mActiveLayer( 0 ), mFeatureNumber( 0 ), mCurrentMouseAzimut( 0.0 ), mCurrentRotationFeature( 0.0 ),
    mRotating( false ), mRotationItem( 0 ), mCtrlPressed( false )
{

}

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

void QgsMapToolRotatePointSymbols::canvasPressEvent( QMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  mActiveLayer = currentVectorLayer();
  if ( !mActiveLayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( !mActiveLayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  if ( mActiveLayer->geometryType() != QGis::Point )
  {
    return;
  }

  //find the closest feature to the pressed position
  QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Vertex );
  if ( !m.isValid() )
  {
    emit messageEmitted( tr( "No point feature was detected at the clicked position. Please click closer to the feature or enhance the search tolerance under Settings->Options->Digitizing->Serch radius for vertex edits" ), QgsMessageBar::CRITICAL );
    return; //error during snapping
  }

  mFeatureNumber = m.featureId();
  mCurrentRotationAttributes.clear();
  mSnappedPoint = toCanvasCoordinates( m.point() );

  QgsFeature pointFeature;
  if ( !mActiveLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setFilterFid( mFeatureNumber ) ).nextFeature( pointFeature ) )
  {
    return;
  }

  //check whether selected feature has a rotatable symbol
  QgsFeatureRendererV2* renderer = mActiveLayer->rendererV2();
  if ( !renderer )
    return;
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( mActiveLayer );
  context.expressionContext().setFeature( pointFeature );
  renderer->startRender( context, mActiveLayer->fields() );

  //find all rotation fields used by renderer for feature
  QgsMarkerSymbolV2* markerSymbol = 0;
  if ( renderer->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature )
  {
    //could be multiple symbols for this feature, so check them all
    foreach ( QgsSymbolV2* s, renderer->originalSymbolsForFeature( pointFeature, context ) )
    {
      if ( s && s->type() == QgsSymbolV2::Marker )
      {
        markerSymbol = static_cast< QgsMarkerSymbolV2* >( s );
        QString rotationField = ( markerSymbol->dataDefinedAngle().isActive() && !markerSymbol->dataDefinedAngle().useExpression() ) ?
                                markerSymbol->dataDefinedAngle().field() : QString();
        if ( !rotationField.isEmpty() )
        {
          int fieldIndex = mActiveLayer->fields().indexFromName( rotationField );
          if ( !mCurrentRotationAttributes.contains( fieldIndex ) )
            mCurrentRotationAttributes << fieldIndex;
        }
      }
    }
  }
  else
  {
    QgsSymbolV2* s = renderer->originalSymbolForFeature( pointFeature, context );
    if ( s && s->type() == QgsSymbolV2::Marker )
    {
      markerSymbol = static_cast< QgsMarkerSymbolV2* >( s );
      QString rotationField = ( markerSymbol->dataDefinedAngle().isActive() && !markerSymbol->dataDefinedAngle().useExpression() ) ?
                              markerSymbol->dataDefinedAngle().field() : QString();
      if ( !rotationField.isEmpty() )
        mCurrentRotationAttributes << mActiveLayer->fields().indexFromName( rotationField );
    }
  }

  if ( mCurrentRotationAttributes.isEmpty() )
  {
    emit messageEmitted( tr( "The selected point does not have a rotation attribute." ), QgsMessageBar::CRITICAL );
    return;
  }

  //find out initial arrow direction
  QVariant attrVal = pointFeature.attribute( mCurrentRotationAttributes.at( 0 ) );
  if ( !attrVal.isValid() )
  {
    return;
  }

  mCurrentRotationFeature = attrVal.toDouble();
  createPixmapItem( markerSymbol );
  if ( mRotationItem )
  {
    mRotationItem->setPointLocation( m.point() );
  }
  mCurrentMouseAzimut = calculateAzimut( e->pos() );
  setPixmapItemRotation(( int )( mCurrentMouseAzimut ) );
  mRotating = true;
}

void QgsMapToolRotatePointSymbols::canvasMoveEvent( QMouseEvent *e )
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

void QgsMapToolRotatePointSymbols::canvasReleaseEvent( QMouseEvent *e )
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

    QList<int>::const_iterator it = mCurrentRotationAttributes.constBegin();
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
  mRotationItem = 0;
  mCanvas->refresh();
}

double QgsMapToolRotatePointSymbols::calculateAzimut( const QPoint& mousePos )
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

