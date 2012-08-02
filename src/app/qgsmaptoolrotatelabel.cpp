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
#include "qgsapplication.h"

QgsMapToolRotateLabel::QgsMapToolRotateLabel( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas ), mRotationItem( 0 ), mRotationPreviewBox( 0 )
{
}

QgsMapToolRotateLabel::~QgsMapToolRotateLabel()
{
  delete mRotationItem;
  delete mRotationPreviewBox;
}

void QgsMapToolRotateLabel::canvasPressEvent( QMouseEvent *e )
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
      mStartRotation = mCurrentRotation;
      createRubberBands();

      mRotationPreviewBox = createRotationPreviewBox();

      mRotationItem = new QgsPointRotationItem( mCanvas );
      mRotationItem->setOrientation( QgsPointRotationItem::Counterclockwise );
      mRotationItem->setSymbol( QgsApplication::getThemePixmap( "mActionRotatePointSymbols.png" ).toImage() );
      mRotationItem->setPointLocation( mRotationPoint );
      mRotationItem->setSymbolRotation( mCurrentRotation );
    }
  }
}

void QgsMapToolRotateLabel::canvasMoveEvent( QMouseEvent *e )
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
      setRotationPreviewBox( displayValue - mStartRotation );
      mRotationItem->update();
    }
  }
}

void QgsMapToolRotateLabel::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );

  if ( !mLabelRubberBand ) //no rubber band created (most likely because the current label cannot be rotated )
  {
    return;
  }

  deleteRubberBands();
  delete mRotationItem;
  mRotationItem = 0;
  delete mRotationPreviewBox;
  mRotationPreviewBox = 0;

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
  if ( rotation == mStartRotation ) //mouse button pressed / released, but no rotation
  {
    return;
  }

  vlayer->beginEditCommand( tr( "Label rotated" ) );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, rotationCol, rotation, false );
  vlayer->endEditCommand();
  mCanvas->refresh();
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

QgsRubberBand* QgsMapToolRotateLabel::createRotationPreviewBox()
{
  delete mRotationPreviewBox;
  QVector< QgsPoint > boxPoints = mCurrentLabelPos.cornerPoints;
  if ( boxPoints.size() < 1 )
  {
    return 0;
  }

  mRotationPreviewBox = new QgsRubberBand( mCanvas, false );
  mRotationPreviewBox->setColor( Qt::blue );
  mRotationPreviewBox->setWidth( 3 );
  setRotationPreviewBox( mCurrentRotation - mStartRotation );
  return mRotationPreviewBox;
}

void QgsMapToolRotateLabel::setRotationPreviewBox( double rotation )
{
  if ( !mRotationPreviewBox )
  {
    return;
  }

  mRotationPreviewBox->reset();
  QVector< QgsPoint > boxPoints = mCurrentLabelPos.cornerPoints;
  if ( boxPoints.size() < 1 )
  {
    return;
  }

  for ( int i = 0; i < boxPoints.size(); ++i )
  {
    mRotationPreviewBox->addPoint( rotatePointCounterClockwise( boxPoints.at( i ), mRotationPoint, rotation ) );
  }
  mRotationPreviewBox->addPoint( rotatePointCounterClockwise( boxPoints.at( 0 ), mRotationPoint, rotation ) );
  mRotationPreviewBox->show();
}

QgsPoint QgsMapToolRotateLabel::rotatePointCounterClockwise( const QgsPoint& input, const QgsPoint& centerPoint, double degrees )
{
  double rad = degrees / 180 * M_PI;
  double v1x = input.x() - centerPoint.x();
  double v1y = input.y() - centerPoint.y();

  double v2x = cos( rad ) * v1x - sin( rad ) * v1y;
  double v2y = sin( rad ) * v1x + cos( rad ) * v1y;

  return QgsPoint( centerPoint.x() + v2x, centerPoint.y() + v2y );
}
