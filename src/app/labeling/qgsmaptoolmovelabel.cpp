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

QgsMapToolMoveLabel::QgsMapToolMoveLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
{
  mToolName = tr( "Move label or callout" );

  mPalProperties << QgsPalLayerSettings::PositionX;
  mPalProperties << QgsPalLayerSettings::PositionY;

  mDiagramProperties << QgsDiagramLayerSettings::PositionX;
  mDiagramProperties << QgsDiagramLayerSettings::PositionY;
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
    QgsPointXY pointMapCoords = e->mapPoint();
    double offsetX = pointMapCoords.x() - mStartPointMapCoords.x();
    double offsetY = pointMapCoords.y() - mStartPointMapCoords.y();
    mLabelRubberBand->setTranslationOffset( offsetX, offsetY );
    mLabelRubberBand->updatePosition();
    mLabelRubberBand->update();
    mFixPointRubberBand->setTranslationOffset( offsetX, offsetY );
    mFixPointRubberBand->updatePosition();
    mFixPointRubberBand->update();
  }
  else if ( mCalloutMoveRubberBand )
  {
    const int index = mCurrentCalloutMoveOrigin ? 0 : 1;
    mCalloutMoveRubberBand->movePoint( index, e->mapPoint() );
    mCalloutMoveRubberBand->update();
  }
  else
  {
    updateHoveredLabel( e );
  }
}

void QgsMapToolMoveLabel::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLabelRubberBand && !mCalloutMoveRubberBand )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    // first click starts move
    deleteRubberBands();

    QgsCalloutPosition calloutPosition;
    QgsLabelPosition labelPos;

    int xCol = 0;
    int yCol = 0;
    if ( calloutAtPosition( e, calloutPosition, mCurrentCalloutMoveOrigin ) && canModifyCallout( calloutPosition, mCurrentCalloutMoveOrigin, xCol, yCol ) )
    {
      // callouts are a smaller target, so they take precedence over labels
      mCurrentLabel = LabelDetails();
      mCurrentCallout = calloutPosition;

      clearHoveredLabel();

      QgsVectorLayer *vlayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( mCurrentCallout.layerID );
      if ( !vlayer || xCol < 0 || yCol < 0 )
      {
        return;
      }

      const bool usesAuxFields = vlayer->fields().fieldOrigin( xCol ) == QgsFields::OriginJoin
                                 && vlayer->fields().fieldOrigin( yCol ) == QgsFields::OriginJoin;
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

      mCalloutMoveRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );
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

        mStartPointMapCoords = e->mapPoint();
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
        // second click drops label/callout
        deleteRubberBands();

        const bool isCalloutMove = !mCurrentCallout.layerID.isEmpty();
        QgsVectorLayer *vlayer = !isCalloutMove ? mCurrentLabel.layer : QgsProject::instance()->mapLayer<QgsVectorLayer *>( mCurrentCallout.layerID );
        if ( !vlayer )
        {
          return;
        }
        const QgsFeatureId featureId = !isCalloutMove ? mCurrentLabel.pos.featureId : mCurrentCallout.featureId;

        QgsPointXY releaseCoords = e->mapPoint();
        double xdiff = releaseCoords.x() - mStartPointMapCoords.x();
        double ydiff = releaseCoords.y() - mStartPointMapCoords.y();

        int xCol = -1;
        int  yCol = -1;
        double xPosOrig = 0;
        double yPosOrig = 0;
        bool xSuccess = false;
        bool ySuccess = false;

        if ( !isCalloutMove && !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
        {
          return;
        }
        else if ( isCalloutMove && !currentCalloutDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
        {
          return;
        }

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

        if ( !isCalloutMove )
          vlayer->beginEditCommand( tr( "Moved label" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
        else
          vlayer->beginEditCommand( tr( "Moved callout" ) );
        bool success = vlayer->changeAttributeValue( featureId, xCol, xPosNew );
        success = vlayer->changeAttributeValue( featureId, yCol, yPosNew ) && success;
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
        const bool isCalloutMove = !mCurrentCallout.layerID.isEmpty();
        QgsVectorLayer *vlayer = !isCalloutMove ? mCurrentLabel.layer : QgsProject::instance()->mapLayer<QgsVectorLayer *>( mCurrentCallout.layerID );
        const QgsFeatureId featureId = !isCalloutMove ? mCurrentLabel.pos.featureId : mCurrentCallout.featureId;
        if ( vlayer )
        {
          int xCol = -1;
          int yCol = -1;
          double xPosOrig = 0;
          double yPosOrig = 0;
          bool xSuccess = false;
          bool ySuccess = false;

          if ( !isCalloutMove && !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
          {
            break;
          }
          else if ( isCalloutMove && !currentCalloutDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
          {
            break;
          }

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
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( pos.layerID );
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

    QgsProperty prop = callout->dataDefinedProperties().property( p );
    if ( prop.propertyType() != QgsProperty::FieldBasedProperty )
      return QString();

    return prop.field();
  };

  const QStringList subProviders = layer->labeling()->subProviders();
  for ( const QString &provider : subProviders )
  {
    ( void )provider;

    const QString xColName = isOrigin ? calloutPropertyColumnName( QgsCallout::OriginX ) : calloutPropertyColumnName( QgsCallout::DestinationX );
    const QString yColName = isOrigin ? calloutPropertyColumnName( QgsCallout::OriginY ) : calloutPropertyColumnName( QgsCallout::DestinationY );

    xCol = layer->fields().lookupField( xColName );
    yCol = layer->fields().lookupField( yColName );
    return ( xCol != -1 && yCol != -1 );
  }

  return false;
}

bool QgsMapToolMoveLabel::currentCalloutDataDefinedPosition( double &x, bool &xSuccess, double &y, bool &ySuccess, int &xCol, int &yCol )
{
  QgsVectorLayer *vlayer =  QgsProject::instance()->mapLayer<QgsVectorLayer *>( mCurrentCallout.layerID );
  QgsFeatureId featureId = mCurrentCallout.featureId;

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
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  QgsAttributes attributes = f.attributes();
  if ( !attributes.at( xCol ).isNull() )
    x = attributes.at( xCol ).toDouble( &xSuccess );
  if ( !attributes.at( yCol ).isNull() )
    y = attributes.at( yCol ).toDouble( &ySuccess );

  return true;
}



