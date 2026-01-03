/***************************************************************************
                          qgsmaptoolpinlabels.cpp
                          -----------------------
    begin                : 2012-07-12
    copyright            : (C) 2012 by Larry Shaffer
    email                : larrys at dakotacarto dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolpinlabels.h"

#include "qgisapp.h"
#include "qgslabelingresults.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"

#include "moc_qgsmaptoolpinlabels.cpp"

QgsMapToolPinLabels::QgsMapToolPinLabels( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
{
  mToolName = tr( "Pin labels" );

  connect( QgisApp::instance()->actionToggleEditing(), &QAction::triggered, this, &QgsMapToolPinLabels::updatePinnedLabels );
  connect( canvas, &QgsMapCanvas::renderComplete, this, &QgsMapToolPinLabels::highlightPinnedLabels );
}

QgsMapToolPinLabels::~QgsMapToolPinLabels()
{
  delete mRubberBand;
  removePinnedHighlights();
}

void QgsMapToolPinLabels::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon );
}

void QgsMapToolPinLabels::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    mSelectRect.setTopLeft( e->pos() );
  }
  mSelectRect.setBottomRight( e->pos() );
  QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );
}

void QgsMapToolPinLabels::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  //if the user simply clicked without dragging a rect
  //we will fabricate a small 1x1 pix rect and then continue
  //as if they had dragged a rect
  if ( !mDragging )
  {
    mSelectRect.setLeft( e->pos().x() - 1 );
    mSelectRect.setRight( e->pos().x() + 1 );
    mSelectRect.setTop( e->pos().y() - 1 );
    mSelectRect.setBottom( e->pos().y() + 1 );
  }
  else
  {
    // Set valid values for rectangle's width and height
    if ( mSelectRect.width() == 1 )
    {
      mSelectRect.setLeft( mSelectRect.left() + 1 );
    }
    if ( mSelectRect.height() == 1 )
    {
      mSelectRect.setBottom( mSelectRect.bottom() + 1 );
    }
  }

  if ( mRubberBand )
  {
    QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );

    QgsGeometry selectGeom = mRubberBand->asGeometry();
    QgsRectangle ext = selectGeom.boundingBox();

    pinUnpinLabels( ext, e );

    mRubberBand->reset( Qgis::GeometryType::Polygon );
    delete mRubberBand;
    mRubberBand = nullptr;
  }

  mDragging = false;
}

void QgsMapToolPinLabels::showPinnedLabels( bool show )
{
  mShowPinned = show;
  if ( mShowPinned )
  {
    QgsDebugMsgLevel( u"Toggling on pinned label highlighting"_s, 2 );
    highlightPinnedLabels();
  }
  else
  {
    QgsDebugMsgLevel( u"Toggling off pinned label highlighting"_s, 2 );
    removePinnedHighlights();
  }
}

// public slot to update pinned label highlights on layer edit mode change
void QgsMapToolPinLabels::updatePinnedLabels()
{
  if ( mShowPinned )
  {
    QgsDebugMsgLevel( u"Updating highlighting due to layer editing mode change"_s, 2 );
    highlightPinnedLabels();
  }
}

void QgsMapToolPinLabels::highlightLabel( const QgsLabelPosition &labelpos, const QString &id, const QColor &color )
{
  QgsRubberBand *rb = new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon );
  rb->addPoint( labelpos.cornerPoints.at( 0 ) );
  rb->addPoint( labelpos.cornerPoints.at( 1 ) );
  rb->addPoint( labelpos.cornerPoints.at( 2 ) );
  rb->addPoint( labelpos.cornerPoints.at( 3 ) );
  rb->addPoint( labelpos.cornerPoints.at( 0 ) );
  rb->setColor( color );
  rb->setWidth( 0 );
  rb->show();

  mHighlights.insert( id, rb );
}

void QgsMapToolPinLabels::highlightCallout( bool isOrigin, const QgsCalloutPosition &calloutPosition, const QString &id, const QColor &color )
{
  double scaleFactor = mCanvas->fontMetrics().xHeight();

  QgsRubberBand *rb = new QgsRubberBand( mCanvas, Qgis::GeometryType::Point );
  rb->setWidth( 2 );
  rb->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  rb->setColor( color );
  rb->setIcon( QgsRubberBand::ICON_X );
  rb->setIconSize( scaleFactor );
  rb->addPoint( isOrigin ? calloutPosition.origin() : calloutPosition.destination() );
  mHighlights.insert( id, rb );
}

// public slot to render highlight rectangles around pinned labels
void QgsMapToolPinLabels::highlightPinnedLabels()
{
  removePinnedHighlights();

  if ( !mShowPinned )
  {
    return;
  }

  QgsDebugMsgLevel( u"Highlighting pinned labels"_s, 2 );

  // get list of all drawn labels from all layers within given extent
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
  {
    return;
  }

  QgsRectangle ext = mCanvas->extent();
  QgsDebugMsgLevel( u"Getting labels from canvas extent"_s, 2 );

  const QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  for ( const QgsLabelPosition &pos : labelPosList )
  {
    mCurrentLabel = LabelDetails( pos, canvas() );

    if ( isPinned() )
    {
      QString labelStringID = u"%0|%1|%2"_s.arg( QString::number( pos.isDiagram ), pos.layerID, QString::number( pos.featureId ) );
      if ( pos.groupedLabelId )
      {
        // for curved labels we do want to show a highlight for every part
        labelStringID += '|' + QString::number( mHighlights.size() );
      }
      else if ( mHighlights.contains( labelStringID ) )
      {
        // don't highlight again
        continue;
      }

      QColor lblcolor = QColor( 54, 129, 255, 63 );
      QgsMapLayer *layer = QgsMapTool::layer( pos.layerID );
      if ( !layer )
      {
        continue;
      }
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
      {
        QgsDebugError( u"Failed to cast to vector layer"_s );
        continue;
      }
      if ( vlayer->isEditable() )
      {
        lblcolor = QColor( 54, 129, 0, 63 );
      }

      highlightLabel( pos, labelStringID, lblcolor );
    }
  }

  // highlight pinned callouts
  const QList<QgsCalloutPosition> calloutPosList = labelingResults->calloutsWithinRectangle( ext );
  const QColor calloutColor = QColor( 54, 129, 255, 160 );
  for ( const QgsCalloutPosition &callout : calloutPosList )
  {
    if ( callout.originIsPinned() )
    {
      QString calloutStringID = u"callout|%1|%2|origin"_s.arg( callout.layerID, QString::number( callout.featureId ) );
      // don't highlight again
      if ( mHighlights.contains( calloutStringID ) )
        continue;

      highlightCallout( true, callout, calloutStringID, calloutColor );
    }
    if ( callout.destinationIsPinned() )
    {
      QString calloutStringID = u"callout|%1|%2|destination"_s.arg( callout.layerID, QString::number( callout.featureId ) );
      // don't highlight again
      if ( mHighlights.contains( calloutStringID ) )
        continue;

      highlightCallout( false, callout, calloutStringID, calloutColor );
    }
  }
}

void QgsMapToolPinLabels::removePinnedHighlights()
{
  for ( QgsRubberBand *rb : std::as_const( mHighlights ) )
  {
    delete rb;
  }
  mHighlights.clear();
}

void QgsMapToolPinLabels::pinUnpinLabels( const QgsRectangle &ext, QMouseEvent *e )
{
  bool doUnpin = e->modifiers() & Qt::ShiftModifier;
  bool toggleUnpinOrPin = e->modifiers() & Qt::ControlModifier;

  // get list of all drawn labels from all layers within, or touching, chosen extent
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
  {
    return;
  }

  QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  bool labelChanged = false;
  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin(); it != labelPosList.constEnd(); ++it )
  {
    const QgsLabelPosition &pos = *it;

    mCurrentLabel = LabelDetails( pos, canvas() );

    if ( !mCurrentLabel.valid )
    {
      QgsDebugError( u"Failed to get label details"_s );
      continue;
    }

    // unpin label
    if ( isPinned() && ( doUnpin || toggleUnpinOrPin ) )
    {
      // unpin previously pinned label (set attribute table fields to NULL)
      if ( pinUnpinCurrentFeature( false ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugError( u"Unpin failed for layer"_s );
      }
    }
    // pin label
    else if ( !isPinned() && ( !doUnpin || toggleUnpinOrPin ) )
    {
      // pin label's location, and optionally rotation, to attribute table
      if ( pinUnpinCurrentFeature( true ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugError( u"Pin failed for layer"_s );
      }
    }
  }

  if ( labelChanged )
  {
    mCurrentLabel.layer->triggerRepaint();

    if ( !mShowPinned )
    {
      // toggle it on (pin-unpin tool doesn't work well without it)
      QgisApp::instance()->actionShowPinnedLabels()->setChecked( true );
    }
  }
}

bool QgsMapToolPinLabels::pinUnpinCurrentLabel( bool pin )
{
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  const QgsLabelPosition &labelpos = mCurrentLabel.pos;

  // skip diagrams
  if ( labelpos.isDiagram )
  {
    QgsDebugMsgLevel( u"Label is diagram, skipping"_s, 2 );
    return false;
  }

  // verify attribute table has x, y or point fields mapped
  int xCol, yCol, pointCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
  {
    QgsDebugMsgLevel( u"Label X, Y or Point column not mapped, skipping"_s, 2 );
    return false;
  }

  // rotation field is optional, but will be used if available, unless data exists
  int rCol;
  bool rSuccess = false;
  double defRot;

  bool hasRCol = currentLabelDataDefinedRotation( defRot, rSuccess, rCol, true );

  // get whether to preserve predefined rotation data during label pin/unpin operations
  bool preserveRot = currentLabelPreserveRotation();

  // edit attribute table
  int fid = labelpos.featureId;

  bool writeFailed = false;
  QString labelText = currentLabelText( 24 );

  if ( pin )
  {
    //     QgsPointXY labelpoint = labelpos.cornerPoints.at( 0 );

    QgsPointXY referencePoint;
    if ( !currentLabelRotationPoint( referencePoint, !preserveRot ) )
    {
      referencePoint.setX( labelpos.labelRect.xMinimum() );
      referencePoint.setY( labelpos.labelRect.yMinimum() );
    }

    double labelX = referencePoint.x();
    double labelY = referencePoint.y();
    double labelR = labelpos.rotation;

    // transform back to layer crs
    QgsPointXY transformedPoint = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, referencePoint );
    labelX = transformedPoint.x();
    labelY = transformedPoint.y();

    vlayer->beginEditCommand( tr( "Pinned label" ) + u" '%1'"_s.arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, labelX );
    if ( !vlayer->changeAttributeValue( fid, yCol, labelY ) )
      writeFailed = true;
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, labelR ) )
        writeFailed = true;
    }
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->beginEditCommand( tr( "Unpinned label" ) + u" '%1'"_s.arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant() );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant() ) )
      writeFailed = true;
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, QVariant() ) )
        writeFailed = true;
    }
    vlayer->endEditCommand();
  }

  if ( writeFailed )
  {
    QgsDebugError( u"Write to attribute table failed"_s );

#if 0
    QgsDebugError( u"Undoing and removing failed command from layer's undo stack"_s );
    int lastCmdIndx = vlayer->undoStack()->count();
    const QgsUndoCommand *lastCmd = qobject_cast<const QgsUndoCommand *>( vlayer->undoStack()->command( lastCmdIndx ) );
    if ( lastCmd )
    {
      vlayer->undoEditCommand( lastCmd );
      delete vlayer->undoStack()->command( lastCmdIndx );
    }
#endif

    return false;
  }

  return true;
}

bool QgsMapToolPinLabels::pinUnpinCurrentFeature( bool pin )
{
  bool rc = false;

  if ( !mCurrentLabel.pos.isDiagram )
    rc = pinUnpinCurrentLabel( pin );
  else
    rc = pinUnpinCurrentDiagram( pin );

  return rc;
}

bool QgsMapToolPinLabels::pinUnpinCurrentDiagram( bool pin )
{
  // skip diagrams
  if ( !mCurrentLabel.pos.isDiagram )
    return false;

  // verify attribute table has x, y fields mapped
  int xCol, yCol, pointCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
    return false;

  // edit attribute table
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  int fid = mCurrentLabel.pos.featureId;

  bool writeFailed = false;
  QString labelText = currentLabelText( 24 );

  if ( pin )
  {
    QgsPointXY referencePoint = mCurrentLabel.pos.labelRect.center();
    double labelX = referencePoint.x();
    double labelY = referencePoint.y();

    // transform back to layer crs
    QgsPointXY transformedPoint = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, referencePoint );
    labelX = transformedPoint.x();
    labelY = transformedPoint.y();

    vlayer->beginEditCommand( tr( "Pinned diagram" ) + u" '%1'"_s.arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, labelX );
    if ( !vlayer->changeAttributeValue( fid, yCol, labelY ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->beginEditCommand( tr( "Unpinned diagram" ) + u" '%1'"_s.arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant() );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant() ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }

  return !writeFailed;
}
