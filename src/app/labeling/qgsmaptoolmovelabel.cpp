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
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsvectorlayerlabeling.h"
#include "qgscallout.h"
#include "qgsstatusbar.h"

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
{
  mToolName = tr( "Move label or callout" );

  mPalProperties << QgsPalLayerSettings::Property::PositionX;
  mPalProperties << QgsPalLayerSettings::Property::PositionY;

  // For linestrings only:
  mPalAnchorProperties << QgsPalLayerSettings::Property::LineAnchorPercent;
  mPalAnchorProperties << QgsPalLayerSettings::Property::LineAnchorClipping;
  mPalAnchorProperties << QgsPalLayerSettings::Property::LineAnchorType;
  mPalAnchorProperties << QgsPalLayerSettings::Property::LineAnchorTextPoint;

  mDiagramProperties << QgsDiagramLayerSettings::Property::PositionX;
  mDiagramProperties << QgsDiagramLayerSettings::Property::PositionY;

  mCalloutProperties << QgsCallout::Property::OriginX;
  mCalloutProperties << QgsCallout::Property::OriginY;
  mCalloutProperties << QgsCallout::Property::DestinationX;
  mCalloutProperties << QgsCallout::Property::DestinationY;
}

QgsMapToolMoveLabel::~QgsMapToolMoveLabel()
{
  delete mCalloutMoveRubberBand;
}

void QgsMapToolMoveLabel::deleteRubberBands()
{
  QgsMapToolLabel::deleteRubberBands();
  delete mCalloutMoveRubberBand;
  mCalloutMoveRubberBand = nullptr;
}

void QgsMapToolMoveLabel::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{

  if ( mLabelRubberBand )
  {
    const QgsPointXY pointMapCoords = e->mapPoint();

    bool isCurvedOrLine = mCurrentLabel.settings.placement == Qgis::LabelPlacement::Curved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::PerimeterCurved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::Line;
    if ( isCurvedOrLine )
    {
      // Determine the closest point on the feature
      const QgsFeatureId featureId = mCurrentLabel.pos.featureId;
      if ( mCurrentLabel.layer )
      {
        const QgsFeature feature { mCurrentLabel.layer->getFeature( featureId ) };
        const QgsMapSettings &ms = mCanvas->mapSettings();

        // In map's CRS
        const QgsGeometry pointMapGeometry { QgsGeometry::fromPointXY( pointMapCoords ) };
        QgsGeometry featureMapGeometry;
        if ( feature.geometry().type() == Qgis::GeometryType::Polygon )
        {
          featureMapGeometry = QgsGeometry( feature.geometry().constGet()->boundary() );
        }
        else
        {
          featureMapGeometry = feature.geometry();
        }
        featureMapGeometry.transform( ms.layerTransform( mCurrentLabel.layer ) );

        if ( featureMapGeometry.distance( pointMapGeometry ) / mCanvas->mapUnitsPerPixel() > mLabelTearFromLineThreshold )
        {
          mAnchorDetached = true;
          isCurvedOrLine = false;
          if ( mOffsetFromLineStartRubberBand )
          {
            mOffsetFromLineStartRubberBand->hide();
          }
        }
        else
        {
          mAnchorDetached = false;
          if ( mOffsetFromLineStartRubberBand )
          {
            mOffsetFromLineStartRubberBand->setToGeometry( featureMapGeometry.nearestPoint( pointMapGeometry ) );
            mOffsetFromLineStartRubberBand->show();
          }
        }
      }
    }

    if ( isCurvedOrLine )
    {
      mLabelRubberBand->hide();
      mFixPointRubberBand->hide();
    }
    else
    {
      const double offsetX = pointMapCoords.x() - mStartPointMapCoords.x();
      const double offsetY = pointMapCoords.y() - mStartPointMapCoords.y();
      if ( mOffsetFromLineStartRubberBand )
        mOffsetFromLineStartRubberBand->hide();
      mLabelRubberBand->setTranslationOffset( offsetX, offsetY );
      mLabelRubberBand->updatePosition();
      mLabelRubberBand->update();
      mFixPointRubberBand->setTranslationOffset( offsetX, offsetY );
      mFixPointRubberBand->updatePosition();
      mFixPointRubberBand->update();
      mLabelRubberBand->show();
      mFixPointRubberBand->show();
    }
  }
  else if ( mCalloutMoveRubberBand )
  {
    const int index = mCurrentCalloutMoveOrigin ? 0 : 1;

    QgsPointXY mapPoint = e->mapPoint();
    if ( e->modifiers() & Qt::ShiftModifier )
    {
      // shift modifier = snap to common angles
      mapPoint = snapCalloutPointToCommonAngle( mapPoint, true );
    }

    mCalloutMoveRubberBand->movePoint( index, mapPoint );
    mCalloutMoveRubberBand->update();
  }
  else
  {
    updateHoveredLabel( e );
  }
}

void QgsMapToolMoveLabel::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLabelRubberBand && !mCalloutMoveRubberBand && !mOffsetFromLineStartRubberBand )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    // first click starts move
    deleteRubberBands();

    QgsCalloutPosition calloutPosition;
    QgsLabelPosition labelPos;

    int xCol = 0;
    int yCol = 0;
    if ( calloutAtPosition( e, calloutPosition, mCurrentCalloutMoveOrigin ) )
    {
      if ( !canModifyCallout( calloutPosition, mCurrentCalloutMoveOrigin, xCol, yCol ) )
      {
        QgsCalloutIndexes indexes;

        if ( createAuxiliaryFields( calloutPosition, indexes ) )
          return;

        if ( !canModifyCallout( calloutPosition, mCurrentCalloutMoveOrigin, xCol, yCol ) )
          return;
      }

      // callouts are a smaller target, so they take precedence over labels
      mCurrentLabel = LabelDetails();
      mCurrentCallout = calloutPosition;

      clearHoveredLabel();

      QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( mCurrentCallout.layerID ) );
      if ( !vlayer || xCol < 0 || yCol < 0 )
      {
        return;
      }

      const bool usesAuxFields = vlayer->fields().fieldOrigin( xCol ) == Qgis::FieldOrigin::Join
                                 && vlayer->fields().fieldOrigin( yCol ) == Qgis::FieldOrigin::Join;
      if ( !usesAuxFields && !vlayer->isEditable() )
      {
        if ( vlayer->startEditing() )
        {
          QgisApp::instance()->messageBar()->pushInfo( tr( "Move Callout" ), tr( "Layer “%1” was made editable" ).arg( vlayer->name() ) );
        }
        else
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Move Callout" ), tr( "Cannot move callout — the layer “%2” could not be made editable" ).arg( vlayer->name() ) );
          return;
        }
      }

      mStartPointMapCoords = e->mapPoint();
      mClickOffsetX = 0;
      mClickOffsetY = 0;

      mCalloutMoveRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Line );
      mCalloutMoveRubberBand->addPoint( mCurrentCallout.origin() );
      mCalloutMoveRubberBand->addPoint( mCurrentCallout.destination() );
      mCalloutMoveRubberBand->setColor( QColor( 0, 255, 0, 65 ) );
      mCalloutMoveRubberBand->setWidth( 3 );
      mCalloutMoveRubberBand->show();

      // set initial cad point as the other side of the callout -- NOTE we have to add two points here!
      cadDockWidget()->addPoint( mCurrentCalloutMoveOrigin ? mCurrentCallout.destination() : mCurrentCallout.origin() );
      cadDockWidget()->addPoint( e->mapPoint() );
      cadDockWidget()->releaseLocks( false );

      return;
    }
    else
    {
      mCurrentCallout = QgsCalloutPosition();
      if ( !labelAtPosition( e, labelPos ) )
      {
        mCurrentLabel = LabelDetails();
        return;
      }

      clearHoveredLabel();

      mCurrentLabel = LabelDetails( labelPos, canvas() );
      mLabelTearFromLineThreshold = QFontMetrics( mCanvas->font() ).horizontalAdvance( 'X' ) * 5;

      QgsVectorLayer *vlayer = mCurrentLabel.layer;
      if ( !vlayer )
      {
        return;
      }

      int xCol = -1, yCol = -1, pointCol = -1, lineAnchorPercentCol = -1, lineAnchorClippingCol = -1, lineAnchorTypeCol = -1, lineAnchorTextPointCol = -1;

      // If x or y DD are set, this label was manually detached, let's start as detached
      double x, y;
      bool xSuccess, ySuccess;
      mAnchorDetached = currentLabelDataDefinedPosition( x, xSuccess, y, ySuccess, xCol, yCol, pointCol ) && xSuccess && ySuccess;

      if ( mAnchorDetached )
      {
        mCurrentLabel.settings.placement = Qgis::LabelPlacement::Horizontal;
      }

      const bool isCurvedOrLine = mCurrentLabel.settings.placement == Qgis::LabelPlacement::Curved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::PerimeterCurved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::Line;
      const bool isMovableUsingPoint = labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol, pointCol );
      const bool isMovableUsingLineAnchor = labelAnchorPercentMovable( vlayer, mCurrentLabel.settings, lineAnchorPercentCol, lineAnchorClippingCol, lineAnchorTypeCol, lineAnchorTextPointCol );

      bool useLineAnchor = false;
      // cloned branches are intentional here for improved readability
      // NOLINTBEGIN(bugprone-branch-clone)
      if ( isCurvedOrLine )
      {
        if ( isMovableUsingLineAnchor )
          useLineAnchor = true;
        else if ( isMovableUsingPoint )
          useLineAnchor = false;
        else
          useLineAnchor = true;
      }
      // NOLINTEND(bugprone-branch-clone)

      if ( useLineAnchor && !mCurrentLabel.pos.isDiagram && !isMovableUsingLineAnchor )
      {
        QgsPalIndexes indexes;
        if ( createAuxiliaryFields( indexes ) )
          return;

        if ( !labelAnchorPercentMovable( vlayer, mCurrentLabel.settings, lineAnchorPercentCol, lineAnchorClippingCol, lineAnchorTypeCol, lineAnchorTextPointCol ) )
        {
          if ( lineAnchorPercentCol < 0 )
          {
            PropertyStatus status = PropertyStatus::DoesNotExist;
            QString colName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorPercent, mCurrentLabel.settings, vlayer, status );
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label anchor percent column “%1” does not exist in the layer" ).arg( colName ) );
          }
          if ( lineAnchorClippingCol < 0 )
          {
            PropertyStatus status = PropertyStatus::DoesNotExist;
            QString colName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorClipping, mCurrentLabel.settings, vlayer, status );
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label anchor clipping column “%1” does not exist in the layer" ).arg( colName ) );
          }
          if ( lineAnchorTypeCol < 0 )
          {
            PropertyStatus status = PropertyStatus::DoesNotExist;
            QString colName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorType, mCurrentLabel.settings, vlayer, status );
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label anchor type column “%1” does not exist in the layer" ).arg( colName ) );
          }
          if ( lineAnchorTextPointCol < 0 )
          {
            PropertyStatus status = PropertyStatus::DoesNotExist;
            QString colName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorTextPoint, mCurrentLabel.settings, vlayer, status );
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label anchor text point column “%1” does not exist in the layer" ).arg( colName ) );
          }
          return;
        }

        lineAnchorPercentCol = indexes[ QgsPalLayerSettings::Property::LineAnchorPercent ];
        // TODO?
        //lineAnchorClippingCol = indexes[ QgsPalLayerSettings::LineAnchorClipping];
        //lineAnchorTypeCol = indexes[ QgsPalLayerSettings::LineAnchorType];
        //lineAnchorTextPointCol = indexes[ QgsPalLayerSettings::LineAnchorTextPoint];

      }
      else if ( !mCurrentLabel.pos.isDiagram && !isMovableUsingPoint )
      {
        if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionPoint ) )
        {
          // Point position is defined as a read only expression (not pointing to a writable geometry column)
          QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The point position expression is not pointing to a writable geometry column" ) );
          return;
        }

        QgsPalIndexes indexes;
        if ( createAuxiliaryFields( indexes ) )
          return;

        if ( !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol, pointCol ) )
        {
          PropertyStatus status = PropertyStatus::DoesNotExist;
          QString xColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, mCurrentLabel.settings, vlayer, status );
          QString yColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, mCurrentLabel.settings, vlayer, status );
          if ( xCol < 0 && yCol < 0 )
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label X/Y columns “%1” and “%2” do not exist in the layer" ).arg( xColName, yColName ) );
          else if ( xCol < 0 )
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label X column “%1” does not exist in the layer" ).arg( xColName ) );
          else if ( yCol < 0 )
            QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "The label Y column “%1” does not exist in the layer" ).arg( yColName ) );
          return;
        }

        xCol = indexes[ QgsPalLayerSettings::Property::PositionX ];
        yCol = indexes[ QgsPalLayerSettings::Property::PositionY ];
      }
      else if ( mCurrentLabel.pos.isDiagram && !diagramMoveable( vlayer, xCol, yCol ) )
      {
        QgsDiagramIndexes indexes;

        if ( createAuxiliaryFields( indexes ) )
          return;

        if ( !diagramMoveable( vlayer, xCol, yCol ) )
          return;

        xCol = indexes[ QgsDiagramLayerSettings::Property::PositionX ];
        yCol = indexes[ QgsDiagramLayerSettings::Property::PositionY ];
      }

      if ( ( isCurvedOrLine && lineAnchorPercentCol >= 0 ) || ( xCol >= 0 && yCol >= 0 ) )
      {
        const bool usesAuxFields =
          ( isCurvedOrLine && lineAnchorPercentCol >= 0 && vlayer->fields().fieldOrigin( lineAnchorPercentCol ) == Qgis::FieldOrigin::Join ) ||
          ( vlayer->fields().fieldOrigin( xCol ) == Qgis::FieldOrigin::Join && vlayer->fields().fieldOrigin( yCol ) == Qgis::FieldOrigin::Join );
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
      }

      mStartPointMapCoords = e->mapPoint();
      QgsPointXY referencePoint;
      if ( !currentLabelRotationPoint( referencePoint, !currentLabelPreserveRotation() ) )
      {
        referencePoint.setX( mCurrentLabel.pos.labelRect.xMinimum() );
        referencePoint.setY( mCurrentLabel.pos.labelRect.yMinimum() );
      }
      mClickOffsetX = mStartPointMapCoords.x() - referencePoint.x();
      mClickOffsetY = mStartPointMapCoords.y() - referencePoint.y();
      createRubberBands();
    }
  }
  else // Second click
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
        // second click drops label/callout
        const bool isCalloutMove = !mCurrentCallout.layerID.isEmpty();
        QgsPointXY releaseCoords = e->mapPoint();
        if ( isCalloutMove && e->modifiers() & Qt::ShiftModifier )
        {
          // shift modifier = snap to common angles
          releaseCoords = snapCalloutPointToCommonAngle( releaseCoords, false );
        }

        deleteRubberBands();

        QgsVectorLayer *vlayer = !isCalloutMove ? mCurrentLabel.layer : qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( mCurrentCallout.layerID ) );
        if ( !vlayer )
        {
          return;
        }
        const QgsFeatureId featureId = !isCalloutMove ? mCurrentLabel.pos.featureId : mCurrentCallout.featureId;

        const double xdiff = releaseCoords.x() - mStartPointMapCoords.x();
        const double ydiff = releaseCoords.y() - mStartPointMapCoords.y();

        int xCol = -1;
        int yCol = -1;
        int pointCol = -1;
        int lineAnchorPercentCol = -1;
        int lineAnchorClippingCol = -1;
        int lineAnchorTypeCol = -1;
        int lineAnchorTextPointCol = -1;
        double xPosOrig = 0;
        double yPosOrig = 0;
        double lineAnchorPercentOrig = 0;
        QString lineAnchorClippingOrig;
        QString lineAnchorTypeOrig;
        QString lineAnchorTextPointOrig;
        bool xSuccess = false;
        bool ySuccess = false;
        bool lineAnchorPercentSuccess = false;
        bool lineAnchorClippingSuccess = false;
        bool lineAnchorTypeSuccess = false;
        bool lineAnchorTextPointSuccess = false;

        bool isCurvedOrLine = !mAnchorDetached && ( mCurrentLabel.settings.placement == Qgis::LabelPlacement::Curved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::PerimeterCurved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::Line );

        if ( !isCalloutMove && isCurvedOrLine && !currentLabelDataDefinedLineAnchorPercent( lineAnchorPercentOrig, lineAnchorPercentSuccess, lineAnchorPercentCol,
             lineAnchorClippingOrig, lineAnchorClippingSuccess, lineAnchorClippingCol,
             lineAnchorTypeOrig, lineAnchorTypeSuccess, lineAnchorTypeCol,
             lineAnchorTextPointOrig,  lineAnchorTextPointSuccess, lineAnchorTextPointCol ) )
        {
          return;
        }
        else if ( !isCalloutMove && !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
        {
          return;
        }
        else if ( isCalloutMove && !currentCalloutDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
        {
          return;
        }

        // Handle curved offset
        if ( isCurvedOrLine )
        {
          const QgsFeature feature { mCurrentLabel.layer->getFeature( featureId ) };
          const QgsMapSettings &ms = mCanvas->mapSettings();

          // In layer's CRS:
          const QgsPointXY releaseCoordsTransformed = ms.mapToLayerCoordinates( mCurrentLabel.layer, releaseCoords );
          const QgsGeometry releaseCoordsGeometry = QgsGeometry::fromPointXY( releaseCoordsTransformed );

          double lineAnchorPercent = 0.0;
          if ( feature.geometry().type() == Qgis::GeometryType::Polygon )
          {
            QgsGeometry boundary( feature.geometry().constGet()->boundary() );
            lineAnchorPercent = boundary.lineLocatePoint( releaseCoordsGeometry ) / boundary.length();
          }
          else
          {
            lineAnchorPercent = feature.geometry().lineLocatePoint( releaseCoordsGeometry ) / feature.geometry().length();
          }

          vlayer->beginEditCommand( tr( "Moved curved label offset" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
          bool success = false;

          if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorPercent ) )
          {
            success = changeCurrentLabelDataDefinedLineAnchorPercent( lineAnchorPercent );
          }

          if ( !success )
          {
            if ( !vlayer->isEditable() )
            {
              QgisApp::instance()->messageBar()->pushWarning( tr( "Move label anchor percent" ), tr( "Layer “%1” must be editable in order to move labels from it" ).arg( vlayer->name() ) );
              vlayer->endEditCommand();
            }
          }
          else
          {
            // Set positions to NULL
            if ( currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
            {
              changeCurrentLabelDataDefinedPosition( QVariant(), QVariant() );
              vlayer->changeAttributeValue( featureId, xCol, QVariant() );
              vlayer->changeAttributeValue( featureId, yCol, QVariant() );
              // TODO: Do we need to check for success here? The layer was already checked for editable state.
            }
            vlayer->endEditCommand();
            vlayer->triggerRepaint();
          }
        }
        else
        {
          double xPosNew = 0;
          double yPosNew = 0;

          if ( !xSuccess || !ySuccess )
          {
            xPosNew = releaseCoords.x() - mClickOffsetX;
            yPosNew = releaseCoords.y() - mClickOffsetY;
          }
          else
          {
            //transform to map crs first, because xdiff,ydiff are in map coordinates
            const QgsMapSettings &ms = mCanvas->mapSettings();
            const QgsPointXY transformedPoint = ms.layerToMapCoordinates( vlayer, QgsPointXY( xPosOrig, yPosOrig ) );
            xPosOrig = transformedPoint.x();
            yPosOrig = transformedPoint.y();
            xPosNew = xPosOrig + xdiff;
            yPosNew = yPosOrig + ydiff;
          }

          //transform back to layer crs
          if ( mCanvas )
          {
            const QgsMapSettings &s = mCanvas->mapSettings();
            const QgsPointXY transformedPoint = s.mapToLayerCoordinates( vlayer, QgsPointXY( xPosNew, yPosNew ) );
            xPosNew = transformedPoint.x();
            yPosNew = transformedPoint.y();
          }

          if ( !isCalloutMove )
            vlayer->beginEditCommand( tr( "Moved label" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
          else
            vlayer->beginEditCommand( tr( "Moved callout" ) );

          bool success = false;
          if ( !isCalloutMove
               && mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionPoint ) )
          {
            success = changeCurrentLabelDataDefinedPosition( xPosNew, yPosNew );
            changeCurrentLabelDataDefinedLineAnchorPercent( QVariant() );
          }
          else
          {
            // Try to convert to the destination field type
            QVariant xNewPos( xPosNew );
            QVariant yNewPos( yPosNew );
            if ( xCol < vlayer->fields().count() )
            {
              if ( ! vlayer->fields().at( xCol ).convertCompatible( xNewPos ) )
              {
                xNewPos = xPosNew; // revert and hope for the best
              }
            }

            if ( yCol < vlayer->fields().count() )
            {
              if ( ! vlayer->fields().at( yCol ).convertCompatible( yNewPos ) )
              {
                yNewPos = yPosNew; // revert and hope for the best
              }
            }
            success = vlayer->changeAttributeValue( featureId, xCol, xNewPos );
            success = vlayer->changeAttributeValue( featureId, yCol, yNewPos ) && success;
          }

          if ( !success )
          {
            // if the edit command fails, it's likely because the label x/y is being stored in a physical field (not a auxiliary one!)
            // and the layer isn't in edit mode
            if ( !vlayer->isEditable() )
            {
              if ( !isCalloutMove )
                QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "Layer “%1” must be editable in order to move labels from it" ).arg( vlayer->name() ) );
              else
                QgisApp::instance()->messageBar()->pushWarning( tr( "Move Callout" ), tr( "Layer “%1” must be editable in order to move callouts from it" ).arg( vlayer->name() ) );
            }
            else
            {
              if ( !isCalloutMove )
                QgisApp::instance()->messageBar()->pushWarning( tr( "Move Label" ), tr( "Error encountered while storing new label position" ) );
              else
                QgisApp::instance()->messageBar()->pushWarning( tr( "Move Callout" ), tr( "Error encountered while storing new callout position" ) );
            }
            vlayer->endEditCommand();
            break;
          }

          // set rotation to that of label, if data-defined and no rotation set yet
          // honor whether to preserve preexisting data on pin
          // must come after setting x and y positions
          if ( !isCalloutMove && !mCurrentLabel.pos.isDiagram
               && !mCurrentLabel.pos.isPinned
               && !currentLabelPreserveRotation() )
          {
            double defRot;
            bool rSuccess;
            int rCol;
            if ( currentLabelDataDefinedRotation( defRot, rSuccess, rCol ) )
            {
              const double labelRot = mCurrentLabel.pos.rotation;
              vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, rCol, labelRot );
            }
          }
          vlayer->endEditCommand();

          vlayer->triggerRepaint();
          break;
        }
        break;
      }
      default:
        break;
    }
  }
}

void QgsMapToolMoveLabel::cadCanvasReleaseEvent( QgsMapMouseEvent * )
{
  if ( !mLabelRubberBand && !mCalloutMoveRubberBand )
  {
    // this tool doesn't collect points -- we want the angle constraints to be reset whenever we drop a label
    cadDockWidget()->clearPoints();
  }
}

void QgsMapToolMoveLabel::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mCalloutMoveRubberBand )
    return; // don't allow cad dock widget points to be cleared after starting to move a callout endpoint

  QgsMapToolLabel::canvasReleaseEvent( e );
}

void QgsMapToolMoveLabel::keyPressEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand || mCalloutMoveRubberBand )
  {
    switch ( e->key() )
    {
      case Qt::Key_Delete:
      {
        e->ignore();  // Override default shortcut management
        return;
      }
    }
  }

  QgsMapToolLabel::keyPressEvent( e );
}

void QgsMapToolMoveLabel::keyReleaseEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand || mCalloutMoveRubberBand )
  {
    switch ( e->key() )
    {
      case Qt::Key_Delete:
      {
        e->ignore();  // Override default shortcut management

        // delete the stored label/callout position
        mAnchorDetached = false;
        const bool isCalloutMove = !mCurrentCallout.layerID.isEmpty();
        const bool isCurvedOrLine = mCurrentLabel.settings.placement == Qgis::LabelPlacement::Curved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::PerimeterCurved || mCurrentLabel.settings.placement == Qgis::LabelPlacement::Line;
        QgsVectorLayer *vlayer = !isCalloutMove ? mCurrentLabel.layer : qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( mCurrentCallout.layerID ) );
        const QgsFeatureId featureId = !isCalloutMove ? mCurrentLabel.pos.featureId : mCurrentCallout.featureId;
        if ( vlayer )
        {
          int xCol = -1;
          int yCol = -1;
          int pointCol = -1;
          int lineAnchorPercentCol = -1;
          int lineAnchorClippingCol = -1;
          int lineAnchorTypeCol = -1;
          int lineAnchorTextPointCol = -1;
          QString lineAnchorClippingOrig;
          QString lineAnchorTypeOrig;
          QString lineAnchorTextPointOrig;
          bool lineAnchorPercentSuccess = false;
          bool lineAnchorClippingSuccess = false;
          bool lineAnchorTypeSuccess = false;
          bool lineAnchorTextPointSuccess = false;
          double xPosOrig = 0;
          double yPosOrig = 0;
          double lineAnchorPercentOrig = 0;
          bool xSuccess = false;
          bool ySuccess = false;

          if ( !isCalloutMove && isCurvedOrLine && ! currentLabelDataDefinedLineAnchorPercent( lineAnchorPercentOrig, lineAnchorPercentSuccess, lineAnchorPercentCol,
               lineAnchorClippingOrig, lineAnchorClippingSuccess, lineAnchorClippingCol,
               lineAnchorTypeOrig, lineAnchorTypeSuccess, lineAnchorTypeCol,
               lineAnchorTextPointOrig,  lineAnchorTextPointSuccess, lineAnchorTextPointCol ) )
          {
            break;
          }
          else if ( !isCalloutMove && !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
          {
            break;
          }
          else if ( isCalloutMove && !currentCalloutDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
          {
            break;
          }

          if ( !isCalloutMove && isCurvedOrLine )
          {
            vlayer->beginEditCommand( tr( "Delete Label Anchor Percent '%1'" ).arg( currentLabelText( 24 ) ) );
            bool success = vlayer->changeAttributeValue( featureId, lineAnchorPercentCol, QVariant() );

            if ( success )
            {
              success = success && vlayer->changeAttributeValue( featureId, lineAnchorPercentCol, QVariant() );
            }

            if ( success )
            {
              success = success && vlayer->changeAttributeValue( featureId, lineAnchorClippingCol, QVariant() );
            }

            if ( success )
            {
              success = success && vlayer->changeAttributeValue( featureId, lineAnchorTypeCol, QVariant() );
            }

            if ( success )
            {
              success = success && vlayer->changeAttributeValue( featureId, lineAnchorTextPointCol, QVariant() );
            }

            if ( !success )
            {
              // if the edit command fails, it's likely because the label anchor percent is being stored in a physical field (not a auxiliary one!)
              // and the layer isn't in edit mode
              if ( !vlayer->isEditable() )
              {
                QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Anchor Percent" ), tr( "Layer “%1” must be editable in order to remove stored label anchor percent" ).arg( vlayer->name() ) );
              }
              else
              {
                QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Anchor Percent" ), tr( "Error encountered while removing stored label anchor percent" ) );
              }
            }
          }
          else
          {
            vlayer->beginEditCommand( !isCalloutMove ? tr( "Delete Label Position" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) : tr( "Delete Callout Position" ) );
            bool success = vlayer->changeAttributeValue( featureId, xCol, QVariant() );
            success = vlayer->changeAttributeValue( featureId, yCol, QVariant() ) && success;
            if ( !success )
            {
              // if the edit command fails, it's likely because the label x/y is being stored in a physical field (not a auxiliary one!)
              // and the layer isn't in edit mode
              if ( !vlayer->isEditable() )
              {
                if ( !isCalloutMove )
                  QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Position" ), tr( "Layer “%1” must be editable in order to remove stored label positions" ).arg( vlayer->name() ) );
                else
                  QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Callout Position" ), tr( "Layer “%1” must be editable in order to remove stored callout positions" ).arg( vlayer->name() ) );
              }
              else
              {
                if ( !isCalloutMove )
                  QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Position" ), tr( "Error encountered while removing stored label position" ) );
                else
                  QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Callout Position" ), tr( "Error encountered while removing stored callout position" ) );
              }
            }
          }
          vlayer->endEditCommand();
          deleteRubberBands();
          vlayer->triggerRepaint();
        }
        break;
      }

      case Qt::Key_Escape:
      {
        // escape is cancel
        deleteRubberBands();
        break;
      }
    }
  }
}

bool QgsMapToolMoveLabel::canModifyCallout( const QgsCalloutPosition &pos, bool isOrigin, int &xCol, int &yCol )
{
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( pos.layerID ) );
  QgsPalLayerSettings settings;
  if ( layer && layer->labelsEnabled() )
  {
    settings = layer->labeling()->settings( pos.providerID );
  }

  const QgsCallout *callout = settings.callout();

  if ( !layer || !layer->labelsEnabled() || !callout )
  {
    return false;
  }

  auto calloutPropertyColumnName = [callout]( QgsCallout::Property p )
  {
    if ( !callout->dataDefinedProperties().isActive( p ) )
      return QString();

    const QgsProperty prop = callout->dataDefinedProperties().property( p );
    if ( prop.propertyType() != Qgis::PropertyType::Field )
      return QString();

    return prop.field();
  };

  const QStringList subProviders = layer->labeling()->subProviders();
  for ( const QString &provider : subProviders )
  {
    ( void )provider;

    const QString xColName = isOrigin ? calloutPropertyColumnName( QgsCallout::Property::OriginX ) : calloutPropertyColumnName( QgsCallout::Property::DestinationX );
    const QString yColName = isOrigin ? calloutPropertyColumnName( QgsCallout::Property::OriginY ) : calloutPropertyColumnName( QgsCallout::Property::DestinationY );

    xCol = layer->fields().lookupField( xColName );
    yCol = layer->fields().lookupField( yColName );
    return ( xCol != -1 && yCol != -1 );
  }

  return false;
}

bool QgsMapToolMoveLabel::currentCalloutDataDefinedPosition( double &x, bool &xSuccess, double &y, bool &ySuccess, int &xCol, int &yCol )
{
  QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( mCurrentCallout.layerID ) );
  const QgsFeatureId featureId = mCurrentCallout.featureId;

  xSuccess = false;
  ySuccess = false;

  if ( !vlayer )
  {
    return false;
  }

  if ( !canModifyCallout( mCurrentCallout, mCurrentCalloutMoveOrigin, xCol, yCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  const QgsAttributes attributes = f.attributes();
  if ( !QgsVariantUtils::isNull( attributes.at( xCol ) ) )
    x = attributes.at( xCol ).toDouble( &xSuccess );
  if ( !QgsVariantUtils::isNull( attributes.at( yCol ) ) )
    y = attributes.at( yCol ).toDouble( &ySuccess );

  return true;
}

QgsPointXY QgsMapToolMoveLabel::snapCalloutPointToCommonAngle( const QgsPointXY &mapPoint, bool showStatusMessage ) const
{
  const int index = mCurrentCalloutMoveOrigin ? 0 : 1;

  const QgsPointXY start = *mCalloutMoveRubberBand->getPoint( 0, index == 0 ? 1 : 0 );
  const double cursorDistance = start.distance( mapPoint );

  // snap to common angles (15 degree increments)
  double closestDist = std::numeric_limits< double >::max();
  double closestX = 0;
  double closestY = 0;
  int bestAngle = 0;

  const double angleOffset = -canvas()->rotation();

  for ( int angle = 0; angle < 360; angle += 15 )
  {
    const QgsPointXY end = start.project( cursorDistance * 2, angle + angleOffset );
    double minDistX = 0;
    double minDistY = 0;
    const double angleDist = QgsGeometryUtilsBase::sqrDistToLine( mapPoint.x(), mapPoint.y(), start.x(), start.y(), end.x(), end.y(), minDistX, minDistY, 4 * std::numeric_limits<double>::epsilon() );
    if ( angleDist < closestDist )
    {
      closestDist = angleDist;
      closestX = minDistX;
      closestY = minDistY;
      bestAngle = angle;
    }
  }

  if ( showStatusMessage )
    QgisApp::instance()->statusBarIface()->showMessage( tr( "Callout angle: %1°" ).arg( bestAngle ), 2000 );

  return QgsPointXY( closestX, closestY );
}



