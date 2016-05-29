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
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include <qgslogger.h>
#include <QMouseEvent>

QgsMapToolPinLabels::QgsMapToolPinLabels( QgsMapCanvas* canvas )
    : QgsMapToolLabel( canvas )
    , mDragging( false )
    , mShowPinned( false )
    , mRubberBand( nullptr )
{
  mToolName = tr( "Pin labels" );

  connect( QgisApp::instance()->actionToggleEditing(), SIGNAL( triggered() ), this, SLOT( updatePinnedLabels() ) );
  connect( canvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( highlightPinnedLabels() ) );
}

QgsMapToolPinLabels::~QgsMapToolPinLabels()
{
  delete mRubberBand;
  removePinnedHighlights();
}

void QgsMapToolPinLabels::canvasPressEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
}

void QgsMapToolPinLabels::canvasMoveEvent( QgsMapMouseEvent* e )
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

void QgsMapToolPinLabels::canvasReleaseEvent( QgsMapMouseEvent* e )
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

    QgsGeometry* selectGeom = mRubberBand->asGeometry();
    QgsRectangle ext = selectGeom->boundingBox();

    pinUnpinLabels( ext, e );

    delete selectGeom;

    mRubberBand->reset( QGis::Polygon );
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
    QgsDebugMsg( QString( "Toggling on pinned label highlighting" ) );
    highlightPinnedLabels();
  }
  else
  {
    QgsDebugMsg( QString( "Toggling off pinned label highlighting" ) );
    removePinnedHighlights();
  }
}

// public slot to update pinned label highlights on layer edit mode change
void QgsMapToolPinLabels::updatePinnedLabels()
{
  if ( mShowPinned )
  {
    QgsDebugMsg( QString( "Updating highlighting due to layer editing mode change" ) );
    highlightPinnedLabels();
  }
}

void QgsMapToolPinLabels::highlightLabel( const QgsLabelPosition& labelpos,
    const QString& id,
    const QColor& color )
{
  QgsRectangle rect = labelpos.labelRect;
  QgsRubberBand *rb = new QgsRubberBand( mCanvas, QGis::Polygon );
  rb->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  rb->addPoint( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  rb->addPoint( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  rb->addPoint( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  rb->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  rb->setColor( color );
  rb->setWidth( 0 );
  rb->show();

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

  QgsDebugMsg( QString( "Highlighting pinned labels" ) );

  // get list of all drawn labels from all layers within given extent
  const QgsLabelingResults* labelingResults = mCanvas->labelingResults();
  if ( !labelingResults )
  {
    QgsDebugMsg( QString( "No labeling engine" ) );
    return;
  }

  QgsRectangle ext = mCanvas->extent();
  QgsDebugMsg( QString( "Getting labels from canvas extent" ) );

  QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    const QgsLabelPosition& pos = *it;

    mCurrentLabel = LabelDetails( pos );

    if ( isPinned() )
    {
      QString labelStringID = QString( "%0|%1|%2" ).arg( QString::number( pos.isDiagram ), pos.layerID, QString::number( pos.featureId ) );

      // don't highlight again
      if ( mHighlights.contains( labelStringID ) )
      {
        continue;
      }

      QColor lblcolor = QColor( 54, 129, 255, 63 );
      QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( pos.layerID );
      if ( !layer )
      {
        continue;
      }
      QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
      if ( !vlayer )
      {
        QgsDebugMsg( QString( "Failed to cast to vector layer" ) );
        continue;
      }
      if ( vlayer->isEditable() )
      {
        lblcolor = QColor( 54, 129, 0, 63 );
      }

      highlightLabel( pos, labelStringID, lblcolor );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::removePinnedHighlights()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  Q_FOREACH ( QgsRubberBand *rb, mHighlights )
  {
    delete rb;
  }
  mHighlights.clear();
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::pinUnpinLabels( const QgsRectangle& ext, QMouseEvent * e )
{
  bool doUnpin = e->modifiers() & Qt::ShiftModifier;
  bool toggleUnpinOrPin = e->modifiers() & Qt::ControlModifier;

  // get list of all drawn labels from all layers within, or touching, chosen extent
  const QgsLabelingResults* labelingResults = mCanvas->labelingResults();
  if ( !labelingResults )
  {
    QgsDebugMsg( QString( "No labeling engine" ) );
    return;
  }

  QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  bool labelChanged = false;
  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    const QgsLabelPosition& pos = *it;

    mCurrentLabel = LabelDetails( pos );

    if ( !mCurrentLabel.valid )
    {
      QgsDebugMsg( QString( "Failed to get label details" ) );
      continue;
    }

    QgsVectorLayer* vlayer = mCurrentLabel.layer;
    if ( !vlayer->isEditable() )
    {
      QgsDebugMsg( QString( "Vector layer not editable, skipping label" ) );
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
        QgsDebugMsg( QString( "Unpin failed for layer" ) );
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
        QgsDebugMsg( QString( "Pin failed for layer" ) );
      }
    }
  }

  if ( labelChanged )
  {
    mCanvas->refresh();

    if ( !mShowPinned )
    {
      // toggle it on (pin-unpin tool doesn't work well without it)
      QgisApp::instance()->actionShowPinnedLabels()->setChecked( true );
    }
  }
}

bool QgsMapToolPinLabels::pinUnpinCurrentLabel( bool pin )
{
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  const QgsLabelPosition& labelpos = mCurrentLabel.pos;

  // skip diagrams
  if ( labelpos.isDiagram )
  {
    QgsDebugMsg( QString( "Label is diagram, skipping" ) );
    return false;
  }

  // verify attribute table has x, y fields mapped
  int xCol, yCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
  {
    QgsDebugMsg( QString( "Label X or Y column not mapped, skipping" ) );
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

//     QgsPoint labelpoint = labelpos.cornerPoints.at( 0 );

    QgsPoint referencePoint;
    if ( !currentLabelRotationPoint( referencePoint, !preserveRot, false ) )
    {
      referencePoint.setX( labelpos.labelRect.xMinimum() );
      referencePoint.setY( labelpos.labelRect.yMinimum() );
    }

    double labelX = referencePoint.x();
    double labelY = referencePoint.y();
    double labelR = labelpos.rotation * 180 / M_PI;

    // transform back to layer crs, if on-fly on
    if ( mCanvas->mapSettings().hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, referencePoint );
      labelX = transformedPoint.x();
      labelY = transformedPoint.y();
    }

    vlayer->beginEditCommand( tr( "Pinned label" ) + QString( " '%1'" ).arg( labelText ) );
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
    vlayer->beginEditCommand( tr( "Unpinned label" ) + QString( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant( QString::null ) );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant( QString::null ) ) )
      writeFailed = true;
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, QVariant( QString::null ) ) )
        writeFailed = true;
    }
    vlayer->endEditCommand();
  }

  if ( writeFailed )
  {
    QgsDebugMsg( QString( "Write to attribute table failed" ) );

#if 0
    QgsDebugMsg( QString( "Undoing and removing failed command from layer's undo stack" ) );
    int lastCmdIndx = vlayer->undoStack()->count();
    const QgsUndoCommand* lastCmd = qobject_cast<const QgsUndoCommand *>( vlayer->undoStack()->command( lastCmdIndx ) );
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
  int xCol, yCol;
  double xPosOrig, yPosOrig;
  bool xSuccess, ySuccess;

  if ( !currentLabelDataDefinedPosition( xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
    return false;

  // edit attribute table
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  int fid = mCurrentLabel.pos.featureId;

  bool writeFailed = false;
  QString labelText = currentLabelText( 24 );

  if ( pin )
  {
    QgsPoint referencePoint = mCurrentLabel.pos.labelRect.center();
    double labelX = referencePoint.x();
    double labelY = referencePoint.y();

    // transform back to layer crs, if on-fly on
    if ( mCanvas->mapSettings().hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, referencePoint );
      labelX = transformedPoint.x();
      labelY = transformedPoint.y();
    }

    vlayer->beginEditCommand( tr( "Pinned diagram" ) + QString( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, labelX );
    if ( !vlayer->changeAttributeValue( fid, yCol, labelY ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->beginEditCommand( tr( "Unpinned diagram" ) + QString( " '%1'" ).arg( labelText ) );
    writeFailed = !vlayer->changeAttributeValue( fid, xCol, QVariant( QString::null ) );
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant( QString::null ) ) )
      writeFailed = true;
    vlayer->endEditCommand();
  }

  if ( writeFailed )
    return false;

  return true;
}
