/***************************************************************************
                          qgsmaptoolrotatelabel.cpp
                          -------------------------
    begin                : 2010-11-09
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolrotatelabel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgspallabeling.h"
#include "qgspointrotationitem.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

#include "qgisapp.h"

QgsMapToolRotateLabel::QgsMapToolRotateLabel( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas ), mRotationItem( 0 )
{
}

QgsMapToolRotateLabel::~QgsMapToolRotateLabel()
{
  delete mRotationItem;
}

void QgsMapToolRotateLabel::canvasPressEvent( QMouseEvent * e )
{
  deleteRubberBands();

  if ( !labelAtPosition( e, mCurrentLabelPos ) )
  {
    return;
  }

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID ) );
  if ( !vlayer )
  {
    return;
  }

  if ( !rotationPoint( mRotationPoint ) )
  {
    return;
  }

  int rotationCol;
  if ( layerIsRotatable( vlayer, rotationCol ) )
  {
    mCurrentMouseAzimuth = azimuthToCCW( mRotationPoint.azimuth( toMapCoordinates( e->pos() ) ) );


    bool hasRotationValue;
    if ( dataDefinedRotation( vlayer, mCurrentLabelPos.featureId, mCurrentRotation, hasRotationValue ) )
    {
      if ( !hasRotationValue )
      {
        mCurrentRotation = 0;
      }
      createRubberBands();

      mRotationItem = new QgsPointRotationItem( mCanvas );
      mRotationItem->setOrientation( QgsPointRotationItem::Counterclockwise );
      mRotationItem->setSymbol( QgisApp::instance()->getThemePixmap( "mActionRotatePointSymbols.png" ).toImage() );
      mRotationItem->setPointLocation( mRotationPoint );
      mRotationItem->setSymbolRotation( mCurrentRotation );
    }
  }
}

void QgsMapToolRotateLabel::canvasMoveEvent( QMouseEvent * e )
{
  if ( mLabelRubberBand )
  {
    QgsPoint currentPoint = toMapCoordinates( e->pos() );
    double azimuth = azimuthToCCW( mRotationPoint.azimuth( currentPoint ) );
    double azimuthDiff = azimuth - mCurrentMouseAzimuth;
    azimuthDiff = azimuthDiff > 180 ? azimuthDiff - 360 : azimuthDiff;

    mCurrentRotation += azimuthDiff;
    mCurrentRotation = mCurrentRotation - static_cast<float>( static_cast<int>( mCurrentRotation / 360 ) ) * 360; //mCurrentRotation % 360;
    mCurrentRotation = mCurrentRotation < 0 ? 360 - mCurrentRotation : mCurrentRotation;

    mCurrentMouseAzimuth = azimuth - static_cast<float>( static_cast<int>( azimuth / 360 ) ) * 360;

    //if shift-modifier is pressed, round to 15 degrees
    int displayValue;
    if ( e->modifiers() & Qt::ControlModifier )
    {
      displayValue = roundTo15Degrees( mCurrentRotation );
      mCtrlPressed = true;
    }
    else
    {
      displayValue = ( int )( mCurrentRotation );
      mCtrlPressed = false;
    }

    if ( mRotationItem )
    {
      mRotationItem->setSymbolRotation( displayValue );
      mRotationItem->update();
    }
  }
}

void QgsMapToolRotateLabel::canvasReleaseEvent( QMouseEvent * e )
{
  deleteRubberBands();
  delete mRotationItem;
  mRotationItem = 0;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID );
  if ( !layer )
  {
    return;
  }

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vlayer )
  {
    return;
  }

  int rotationCol;
  if ( !layerIsRotatable( vlayer, rotationCol ) )
  {
    return;
  }

  double rotation = mCtrlPressed ? roundTo15Degrees( mCurrentRotation ) : mCurrentRotation;

  vlayer->beginEditCommand( tr( "Label rotated" ) );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, rotationCol, rotation, false );
  vlayer->endEditCommand();
  mCanvas->refresh();
}

bool QgsMapToolRotateLabel::layerIsRotatable( const QgsMapLayer* layer, int& rotationCol ) const
{
  const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>( layer );
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  QVariant rotation = layer->customProperty( "labeling/dataDefinedProperty14" );
  if ( !rotation.isValid() )
  {
    return false;
  }

  bool rotationOk;
  rotationCol = rotation.toInt( &rotationOk );
  if ( !rotationOk )
  {
    return false;
  }
  return true;
}

bool QgsMapToolRotateLabel::dataDefinedRotation( QgsVectorLayer* vlayer, int featureId, double& rotation, bool& rotationSuccess )
{
  rotationSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  int rotationCol;
  if ( !layerIsRotatable( vlayer, rotationCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->featureAtId( featureId, f, false, true ) )
  {
    return false;
  }

  QgsAttributeMap attributes = f.attributeMap();
  rotation = attributes[rotationCol].toDouble( &rotationSuccess );
  return true;
}

int QgsMapToolRotateLabel::roundTo15Degrees( double n )
{
  int m = ( int )( n / 15.0 + 0.5 );
  return ( m * 15 );
}

double QgsMapToolRotateLabel::azimuthToCCW( double a )
{
  return ( a > 0 ? 360 - a : -a );
}
