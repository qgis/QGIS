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
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgslogger.h"
#include "qgslabelingresults.h"


QgsMapToolPinLabels::QgsMapToolPinLabels( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
  , mDragging( false )
  , mShowPinned( false )

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
  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
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

    mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
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
    QgsDebugMsg( QStringLiteral( "Toggling on pinned label highlighting" ) );
    highlightPinnedLabels();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Toggling off pinned label highlighting" ) );
    removePinnedHighlights();
  }
}

// public slot to update pinned label highlights on layer edit mode change
void QgsMapToolPinLabels::updatePinnedLabels()
{
  if ( mShowPinned )
  {
    QgsDebugMsg( QStringLiteral( "Updating highlighting due to layer editing mode change" ) );
    highlightPinnedLabels();
  }
}

void QgsMapToolPinLabels::highlightLabel( const QgsLabelPosition &labelpos,
    const QString &id,
    const QColor &color )
{
  QgsRubberBand *rb = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
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

  QgsRubberBand *rb = new QgsRubberBand( mCanvas, QgsWkbTypes::PointGeometry );
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

  QgsDebugMsgLevel( QStringLiteral( "Highlighting pinned labels" ), 2 );

  // get list of all drawn labels from all layers within given extent
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
  {
    return;
  }

  QgsRectangle ext = mCanvas->extent();
  QgsDebugMsgLevel( QStringLiteral( "Getting labels from canvas extent" ), 2 );

  const QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  QList<QgsLabelPosition>::const_iterator it;
  for ( const QgsLabelPosition &pos : labelPosList )
  {
    mCurrentLabel = LabelDetails( pos, canvas() );

    if ( isPinned() )
    {
      QString labelStringID = QStringLiteral( "%0|%1|%2" ).arg( QString::number( pos.isDiagram ), pos.layerID, QString::number( pos.featureId ) );
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
        QgsDebugMsg( QStringLiteral( "Failed to cast to vector layer" ) );
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
      QString calloutStringID = QStringLiteral( "callout|%1|%2|origin" ).arg( callout.layerID, QString::number( callout.featureId ) );
      // don't highlight again
      if ( mHighlights.contains( calloutStringID ) )
        continue;

      highlightCallout( true, callout, calloutStringID, calloutColor );
    }
    if ( callout.destinationIsPinned() )
    {
      QString calloutStringID = QStringLiteral( "callout|%1|%2|destination" ).arg( callout.layerID, QString::number( callout.featureId ) );
      // don't highlight again
      if ( mHighlights.contains( calloutStringID ) )
        continue;

      highlightCallout( false, callout, calloutStringID, calloutColor );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::removePinnedHighlights()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  for ( QgsRubberBand *rb : std::as_const( mHighlights ) )
  {
    delete rb;
  }
  mHighlights.clear();
  QApplication::restoreOverrideCursor();
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
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    const QgsLabelPosition &pos = *it;

    mCurrentLabel = LabelDetails( pos, canvas() );

    if ( !mCurrentLabel.valid )
    {
      QgsDebugMsg( QStringLiteral( "Failed to get label details" ) );
      continue;
    }

    // unpin label
    if ( isPinned() && ( doUnpin  || toggleUnpinOrPin ) )
    {
      // unpin previously pinned label (set attribute table fields to NULL)
      if ( pinUnpinCurrentFeature( false ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Unpin failed for layer" ) );
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
        QgsDebugMsg( QStringLiteral( "Pin failed for layer" ) );
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
    QgsDebugMsg( QStringLiteral( "Label is diagram, skipping" ) );
    return false;
  }

  // verify attribute table has x, y or point fields mapped
  int xCol, yCol, pointCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol, pointCol ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Label X, Y or Point column not mapped, skipping" ), 2 );
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
    double labelR = labelpos.rotation * 180 / M_PI;

    // transform back to layer crs
    QgsPointXY transformedPoint = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, referencePoint );
    labelX = transformedPoint.x();
    labelY = transformedPoint.y();

    vlayer->beginEditCommand( tr( "Pinned label" ) + QStringLiteral( " '%1'" ).arg( labelText ) );
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
    vlayer->beginEditCommand( tr( "Unpinned label" ) + QStringLiteral( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant( QString() ) );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant( QString() ) ) )
      writeFailed = true;
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, QVariant( QString() ) ) )
        writeFailed = true;
    }
    vlayer->endEditCommand();
  }

  if ( writeFailed )
  {
    QgsDebugMsg( QStringLiteral( "Write to attribute table failed" ) );

#if 0
    QgsDebugMsg( QStringLiteral( "Undoing and removing failed command from layer's undo stack" ) );
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

  if ( ! mCurrentLabel.pos.isDiagram )
    rc = pinUnpinCurrentLabel( pin );
  else
    rc = pinUnpinCurrentDiagram( pin );

  return rc;
}

bool QgsMapToolPinLabels::pinUnpinCurrentDiagram( bool pin )
{

  // skip diagrams
  if ( ! mCurrentLabel.pos.isDiagram )
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

    vlayer->beginEditCommand( tr( "Pinned diagram" ) + QStringLiteral( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, labelX );
    if ( !vlayer->changeAttributeValue( fid, yCol, labelY ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->beginEditCommand( tr( "Unpinned diagram" ) + QStringLiteral( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant( QString() ) );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant( QString() ) ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }

  return !writeFailed;
}
