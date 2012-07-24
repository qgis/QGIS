/***************************************************************************
                          qgsmaptoolmovelabel.cpp
                          -----------------------
    begin                : 2010-11-03
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

#include "qgsmaptoolmovelabel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas )
{
}

QgsMapToolMoveLabel::~QgsMapToolMoveLabel()
{
}

void QgsMapToolMoveLabel::canvasPressEvent( QMouseEvent * e )
{
  deleteRubberBands();

  if ( !labelAtPosition( e, mCurrentLabelPos ) )
  {
    return;
  }

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID );
  if ( !layer || !layer->isEditable() )
  {
    return;
  }

  int xCol, yCol;
  if ( labelMoveable( layer, xCol, yCol ) || diagramMoveable( layer, xCol, yCol ) )
  {
    mStartPointMapCoords = toMapCoordinates( e->pos() );
    QgsPoint referencePoint;
    if ( !rotationPoint( referencePoint ) )
    {
      referencePoint.setX( mCurrentLabelPos.labelRect.xMinimum() );
      referencePoint.setY( mCurrentLabelPos.labelRect.yMinimum() );
    }
    mClickOffsetX = mStartPointMapCoords.x() - referencePoint.x();
    mClickOffsetY = mStartPointMapCoords.y() - referencePoint.y();
    createRubberBands();
  }
}

void QgsMapToolMoveLabel::canvasMoveEvent( QMouseEvent * e )
{
  if ( mLabelRubberBand )
  {
    QgsPoint pointCanvasCoords = toMapCoordinates( e->pos() );
    double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
    double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
    mLabelRubberBand->setTranslationOffset( offsetX, offsetY );
    mLabelRubberBand->updatePosition();
    mLabelRubberBand->update();
    mFixPointRubberBand->setTranslationOffset( offsetX, offsetY );
    mFixPointRubberBand->updatePosition();
    mFixPointRubberBand->update();
  }
}

void QgsMapToolMoveLabel::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mLabelRubberBand )
  {
    return;
  }

  deleteRubberBands();

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

  if ( !vlayer->isEditable() )
  {
    return;
  }

  QgsPoint releaseCoords = toMapCoordinates( e->pos() );
  double xdiff = releaseCoords.x() - mStartPointMapCoords.x();
  double ydiff = releaseCoords.y() - mStartPointMapCoords.y();

  int xCol, yCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !dataDefinedPosition( vlayer, mCurrentLabelPos.featureId, xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
  {
    return;
  }

  double xPosNew, yPosNew;

  if ( !xSuccess || !ySuccess )
  {
    xPosNew = releaseCoords.x() - mClickOffsetX;
    yPosNew = releaseCoords.y() - mClickOffsetY;

    // handle upsidedown offset on first freeze
    // TODO: test if this is enough relative to hali and vali options
    if ( mCurrentLabelPos.upsideDown )
    {
      QgsPoint rsu = mCurrentLabelPos.cornerPoints.at( 0 );
      QgsPoint usd = mCurrentLabelPos.cornerPoints.at( 2 );
      xPosNew = xPosNew - ( usd.x() - rsu.x() );
      yPosNew = yPosNew - ( usd.y() - rsu.y() );
    }
  }
  else
  {
    //transform to map crs first, because xdiff,ydiff are in map coordinates
    QgsMapRenderer* r = mCanvas->mapRenderer();
    if ( r && r->hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = r->layerToMapCoordinates( vlayer, QgsPoint( xPosOrig, yPosOrig ) );
      xPosOrig = transformedPoint.x();
      yPosOrig = transformedPoint.y();
    }
    xPosNew = xPosOrig + xdiff;
    yPosNew = yPosOrig + ydiff;
  }

  //transform back to layer crs
  if ( mCanvas )
  {
    QgsMapRenderer* r = mCanvas->mapRenderer();
    if ( r && r->hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = r->mapToLayerCoordinates( vlayer, QgsPoint( xPosNew, yPosNew ) );
      xPosNew = transformedPoint.x();
      yPosNew = transformedPoint.y();
    }
  }

  // set rotation to that of label, if data-defined and no rotation set yet
  // handle case of initially set rotation column fields of 0 instead of NULL
  int rCol;
  double defRot;
  double labelRot = 0;
  bool rSuccess;
  bool setRot = false;

  if ( !mCurrentLabelPos.isDiagram && !mCurrentLabelPos.isFrozen
       && dataDefinedRotation( vlayer, mCurrentLabelPos.featureId, defRot, rSuccess, rCol ) )
  {
    labelRot = mCurrentLabelPos.rotation * 180 / M_PI;
    if ( !rSuccess || ( rSuccess && defRot != labelRot ) )
    {
      setRot = true;
    }
  }

  vlayer->beginEditCommand( tr( "Label moved" ) );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, xCol, xPosNew, false );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, yCol, yPosNew, false );
  if ( setRot )
  {
    vlayer->changeAttributeValue( mCurrentLabelPos.featureId, rCol, labelRot, false );
  }
  vlayer->endEditCommand();

  mCanvas->refresh();
}



