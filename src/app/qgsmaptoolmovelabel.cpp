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

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas* canvas )
    : QgsMapToolLabel( canvas )
{
  mToolName = tr( "Move label" );
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
    if ( !rotationPoint( referencePoint, !preserveRotation(), false ) )
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
  }
  else
  {
    //transform to map crs first, because xdiff,ydiff are in map coordinates
    const QgsMapSettings& ms = mCanvas->mapSettings();
    if ( ms.hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = ms.layerToMapCoordinates( vlayer, QgsPoint( xPosOrig, yPosOrig ) );
      xPosOrig = transformedPoint.x();
      yPosOrig = transformedPoint.y();
    }
    xPosNew = xPosOrig + xdiff;
    yPosNew = yPosOrig + ydiff;
  }

  //transform back to layer crs
  if ( mCanvas )
  {
    const QgsMapSettings& s = mCanvas->mapSettings();
    if ( s.hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = s.mapToLayerCoordinates( vlayer, QgsPoint( xPosNew, yPosNew ) );
      xPosNew = transformedPoint.x();
      yPosNew = transformedPoint.y();
    }
  }

  vlayer->beginEditCommand( tr( "Moved label" ) + QString( " '%1'" ).arg( currentLabelText( 24 ) ) );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, xCol, xPosNew );
  vlayer->changeAttributeValue( mCurrentLabelPos.featureId, yCol, yPosNew );

  // set rotation to that of label, if data-defined and no rotation set yet
  // honor whether to preserve preexisting data on pin
  // must come after setting x and y positions
  int rCol;
  if ( !mCurrentLabelPos.isDiagram
       && !mCurrentLabelPos.isPinned
       && !preserveRotation()
       && layerIsRotatable( vlayer, rCol ) )
  {
    double defRot;
    bool rSuccess;
    if ( dataDefinedRotation( vlayer, mCurrentLabelPos.featureId, defRot, rSuccess ) )
    {
      double labelRot = mCurrentLabelPos.rotation * 180 / M_PI;
      vlayer->changeAttributeValue( mCurrentLabelPos.featureId, rCol, labelRot );
    }
  }
  vlayer->endEditCommand();

  mCanvas->refresh();
}



