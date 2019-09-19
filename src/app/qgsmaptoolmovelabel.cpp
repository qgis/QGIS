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
#include "qgsmapmouseevent.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas *canvas )
  : QgsMapToolLabel( canvas )
{
  mToolName = tr( "Move label" );

  mPalProperties << QgsPalLayerSettings::PositionX;
  mPalProperties << QgsPalLayerSettings::PositionY;

  mDiagramProperties << QgsDiagramLayerSettings::PositionX;
  mDiagramProperties << QgsDiagramLayerSettings::PositionY;
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

void QgsMapToolMoveLabel::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLabelRubberBand )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    // first click starts move
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

    if ( !mCurrentLabel.pos.isDiagram && !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol ) )
    {
      QgsPalIndexes indexes;

      if ( createAuxiliaryFields( indexes ) )
        return;

      if ( !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol ) )
        return;

      xCol = indexes[ QgsPalLayerSettings::PositionX ];
      yCol = indexes[ QgsPalLayerSettings::PositionY ];
    }
    else if ( mCurrentLabel.pos.isDiagram && !diagramMoveable( vlayer, xCol, yCol ) )
    {
      QgsDiagramIndexes indexes;

      if ( createAuxiliaryFields( indexes ) )
        return;

      if ( !diagramMoveable( vlayer, xCol, yCol ) )
        return;

      xCol = indexes[ QgsDiagramLayerSettings::PositionX ];
      yCol = indexes[ QgsDiagramLayerSettings::PositionY ];
    }

    if ( xCol >= 0 && yCol >= 0 )
    {
      const bool usesAuxFields = vlayer->fields().fieldOrigin( xCol ) == QgsFields::OriginJoin
                                 && vlayer->fields().fieldOrigin( yCol ) == QgsFields::OriginJoin;
      if ( !usesAuxFields && !vlayer->isEditable() )
      {
        if ( vlayer->startEditing() )
        {
          QgisApp::instance()->messageBar()->pushInfo( tr( "Move Label" ), tr( "Layer “%1” was made editable" ).arg( vlayer->name() ) );
        }
        else
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "Cannot move “%1” — the layer “%2” could not be made editable" ).arg( mCurrentLabel.pos.labelText, vlayer->name() ) );
          return;
        }
      }

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
  else
  {
    switch ( e->button() )
    {
      case Qt::RightButton:
      {
        // right click is cancel
        deleteRubberBands();
        return;
      }

      case Qt::LeftButton:
      {
        // second click drops label
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
        bool success = vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, xCol, xPosNew );
        success = vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, yCol, yPosNew ) && success;
        if ( !success )
        {
          // if the edit command fails, it's likely because the label x/y is being stored in a physical field (not a auxiliary one!)
          // and the layer isn't in edit mode
          if ( !vlayer->isEditable() )
          {
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "Layer “%1” must be editable in order to move labels from it" ).arg( vlayer->name() ) );
          }
          else
          {
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "Error encountered while storing new label position" ) );
          }
          vlayer->endEditCommand();
          break;
        }

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
        break;
      }
      default:
        break;
    }
  }
}

void QgsMapToolMoveLabel::keyReleaseEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand && e->key() == Qt::Key_Escape )
  {
    // escape is cancel
    deleteRubberBands();
  }
}



