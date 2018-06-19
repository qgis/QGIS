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
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas *canvas )
  : QgsMapToolLabel( canvas )
  , mClickOffsetX( 0 )
  , mClickOffsetY( 0 )
{
  mToolName = tr( "Move label" );

  mPalProperties << QgsPalLayerSettings::PositionX;
  mPalProperties << QgsPalLayerSettings::PositionY;

  mDiagramProperties << QgsDiagramLayerSettings::PositionX;
  mDiagramProperties << QgsDiagramLayerSettings::PositionY;
}

void QgsMapToolMoveLabel::canvasPressEvent( QgsMapMouseEvent *e )
{
  deleteRubberBands();

  QgsLabelPosition labelPos;
  if ( !labelAtPosition( e, labelPos ) )
  {
    mCurrentLabel = LabelDetails();
    return;
  }

  mCurrentLabel = LabelDetails( labelPos );

  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return;
  }

  int xCol = -1, yCol = -1;

  if ( !mCurrentLabel.pos.isDiagram &&  !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol ) )
  {
    QgsPalIndexes indexes;

    if ( createAuxiliaryFields( indexes ) )
      return;

    xCol = indexes[ QgsPalLayerSettings::PositionX ];
    yCol = indexes[ QgsPalLayerSettings::PositionY ];
  }
  else if ( mCurrentLabel.pos.isDiagram && !diagramMoveable( vlayer, xCol, yCol ) )
  {
    QgsDiagramIndexes indexes;

    if ( createAuxiliaryFields( indexes ) )
      return;

    xCol = indexes[ QgsDiagramLayerSettings::PositionX ];
    yCol = indexes[ QgsDiagramLayerSettings::PositionY ];
  }

  if ( xCol >= 0 && yCol >= 0 )
  {
    mStartPointMapCoords = toMapCoordinates( e->pos() );
    QgsPointXY referencePoint;
    if ( !currentLabelRotationPoint( referencePoint, !currentLabelPreserveRotation(), false ) )
    {
      referencePoint.setX( mCurrentLabel.pos.labelRect.xMinimum() );
      referencePoint.setY( mCurrentLabel.pos.labelRect.yMinimum() );
    }
    mClickOffsetX = mStartPointMapCoords.x() - referencePoint.x();
    mClickOffsetY = mStartPointMapCoords.y() - referencePoint.y();
    createRubberBands();
  }
}

void QgsMapToolMoveLabel::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mLabelRubberBand )
  {
    QgsPointXY pointCanvasCoords = toMapCoordinates( e->pos() );
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

void QgsMapToolMoveLabel::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mLabelRubberBand )
  {
    return;
  }

  deleteRubberBands();

  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return;
  }

  QgsPointXY releaseCoords = toMapCoordinates( e->pos() );
  double xdiff = releaseCoords.x() - mStartPointMapCoords.x();
  double ydiff = releaseCoords.y() - mStartPointMapCoords.y();

  int xCol, yCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
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
    const QgsMapSettings &ms = mCanvas->mapSettings();
    QgsPointXY transformedPoint = ms.layerToMapCoordinates( vlayer, QgsPointXY( xPosOrig, yPosOrig ) );
    xPosOrig = transformedPoint.x();
    yPosOrig = transformedPoint.y();
    xPosNew = xPosOrig + xdiff;
    yPosNew = yPosOrig + ydiff;
  }

  //transform back to layer crs
  if ( mCanvas )
  {
    const QgsMapSettings &s = mCanvas->mapSettings();
    QgsPointXY transformedPoint = s.mapToLayerCoordinates( vlayer, QgsPointXY( xPosNew, yPosNew ) );
    xPosNew = transformedPoint.x();
    yPosNew = transformedPoint.y();
  }

  vlayer->beginEditCommand( tr( "Moved label" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
  vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, xCol, xPosNew );
  vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, yCol, yPosNew );

  // set rotation to that of label, if data-defined and no rotation set yet
  // honor whether to preserve preexisting data on pin
  // must come after setting x and y positions
  if ( !mCurrentLabel.pos.isDiagram
       && !mCurrentLabel.pos.isPinned
       && !currentLabelPreserveRotation() )
  {
    double defRot;
    bool rSuccess;
    int rCol;
    if ( currentLabelDataDefinedRotation( defRot, rSuccess, rCol ) )
    {
      double labelRot = mCurrentLabel.pos.rotation * 180 / M_PI;
      vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, rCol, labelRot );
    }
  }
  vlayer->endEditCommand();

  vlayer->triggerRepaint();
}



