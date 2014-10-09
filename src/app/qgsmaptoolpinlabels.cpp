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
{
  mToolName = tr( "Pin labels" );
  mRubberBand = 0;
  mShowPinned = false;

  connect( QgisApp::instance()->actionToggleEditing(), SIGNAL( triggered() ), this, SLOT( updatePinnedLabels() ) );
  connect( canvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( highlightPinnedLabels() ) );
}

QgsMapToolPinLabels::~QgsMapToolPinLabels()
{
  delete mRubberBand;
  removePinnedHighlights();
}

void QgsMapToolPinLabels::canvasPressEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
}

void QgsMapToolPinLabels::canvasMoveEvent( QMouseEvent * e )
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

void QgsMapToolPinLabels::canvasReleaseEvent( QMouseEvent * e )
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
    mRubberBand = 0;
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
    mCurrentLabelPos = *it;

    if ( mCurrentLabelPos.isPinned )
    {
      QString labelStringID = QString( "%0|%1" ).arg( mCurrentLabelPos.layerID, QString::number( mCurrentLabelPos.featureId ) );

      // don't highlight again
      if ( mHighlights.contains( labelStringID ) )
      {
        continue;
      }

      QColor lblcolor = QColor( 54, 129, 255, 63 );
      QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID );
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

      highlightLabel(( *it ), labelStringID, lblcolor );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::removePinnedHighlights()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  foreach ( QgsRubberBand *rb, mHighlights )
  {
    delete rb;
  }
  mHighlights.clear();
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::pinUnpinLabels( const QgsRectangle& ext, QMouseEvent * e )
{

  bool doUnpin = e->modifiers() & Qt::ShiftModifier ? true : false;
  bool toggleUnpinOrPin = e->modifiers() & Qt::ControlModifier ? true : false;

  // get list of all drawn labels from all layers within, or touching, chosen extent
  bool labelChanged = false;

  const QgsLabelingResults* labelingResults = mCanvas->labelingResults();
  if ( !labelingResults )
  {
    QgsDebugMsg( QString( "No labeling engine" ) );
    return;
  }

  QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    mCurrentLabelPos = *it;

#ifdef QGISDEBUG
    QString labellyr = currentLayer()->name();
    QString labeltxt = currentLabelText();
#endif
    QgsDebugMsg( QString( "Layer: %0" ).arg( labellyr ) );
    QgsDebugMsg( QString( "Label: %0" ).arg( labeltxt ) );

    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID );
    if ( !layer )
    {
      QgsDebugMsg( QString( "Failed to get label layer" ) );
      continue;
    }
    QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
    if ( !vlayer )
    {
      QgsDebugMsg( QString( "Failed to cast label layer to vector layer" ) );
      continue;
    }
    if ( !vlayer->isEditable() )
    {
      QgsDebugMsg( QString( "Vector layer not editable, skipping label" ) );
      continue;
    }

    QString labelStringID = QString( "%0|%1" ).arg( mCurrentLabelPos.layerID, QString::number( mCurrentLabelPos.featureId ) );

    // unpin label
    if ( mCurrentLabelPos.isPinned && ( doUnpin  || toggleUnpinOrPin ) )
    {
      // unpin previously pinned label (set attribute table fields to NULL)
      if ( pinUnpinLabel( vlayer, mCurrentLabelPos, false ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QString( "Unpin failed for layer, label: %0, %1" ).arg( labellyr, labeltxt ) );
      }
    }

    // pin label
    if ( !mCurrentLabelPos.isPinned && ( !doUnpin || toggleUnpinOrPin ) )
    {
      // pin label's location, and optionally rotation, to attribute table
      if ( pinUnpinLabel( vlayer, mCurrentLabelPos, true ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QString( "Pin failed for layer, label: %0, %1" ).arg( labellyr, labeltxt ) );
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

bool QgsMapToolPinLabels::pinUnpinLabel( QgsVectorLayer* vlayer,
    const QgsLabelPosition& labelpos,
    bool pin )
{
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

  if ( !dataDefinedPosition( vlayer, mCurrentLabelPos.featureId, xPosOrig, xSuccess, yPosOrig, ySuccess, xCol, yCol ) )
  {
    QgsDebugMsg( QString( "Label X or Y column not mapped, skipping" ) );
    return false;
  }

  // rotation field is optional, but will be used if available, unless data exists
  int rCol;
  bool rSuccess = false;
  double defRot;

  bool hasRCol = ( layerIsRotatable( vlayer, rCol )
                   && dataDefinedRotation( vlayer, mCurrentLabelPos.featureId, defRot, rSuccess, true ) );

  // get whether to preserve predefined rotation data during label pin/unpin operations
  bool preserveRot = preserveRotation();

  // edit attribute table
  int fid = labelpos.featureId;

  bool writeFailed = false;
  QString labelText = currentLabelText( 24 );

  if ( pin )
  {

//     QgsPoint labelpoint = labelpos.cornerPoints.at( 0 );

    QgsPoint referencePoint;
    if ( !rotationPoint( referencePoint, !preserveRot, false ) )
    {
      referencePoint.setX( mCurrentLabelPos.labelRect.xMinimum() );
      referencePoint.setY( mCurrentLabelPos.labelRect.yMinimum() );
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
