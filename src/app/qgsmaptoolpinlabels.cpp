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
#include "qgslegend.h"
#include "qgsvectorlayer.h"

#include "qgsmaptoolselectutils.h"
#include "qgshighlight.h"
#include "qgsrubberband.h"
#include <qgslogger.h>
#include <QMouseEvent>

QgsMapToolPinLabels::QgsMapToolPinLabels( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas )
{
  mRender = 0;
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
  mRubberBand = new QgsRubberBand( mCanvas, true );
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

    mRubberBand->reset( true );
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
    mCanvas->refresh();
  }
}

void QgsMapToolPinLabels::highlightLabel( QgsVectorLayer* vlayer,
    const QgsLabelPosition& labelpos,
    const QString& id,
    const QColor& color )
{
  QgsRectangle rect = labelpos.labelRect;

  if ( vlayer->crs().isValid() && mRender->destinationCrs().isValid() )
  {
    // if label's layer is on-fly transformed, reverse-transform label rect
    // QgsHighlight will convert it, yet again, to the correct map coords
    if ( vlayer->crs() != mRender->destinationCrs() )
    {
      rect = mRender->mapToLayerCoordinates( vlayer, rect );
      QgsDebugMsg( QString( "Reverse transform needed for highlight rectangle" ) );
    }
  }

  QgsGeometry* highlightgeom = QgsGeometry::fromRect( rect );

  QgsHighlight *h = new QgsHighlight( mCanvas, highlightgeom, vlayer );
  if ( h )
  {
    h->setWidth( 0 );
    h->setColor( color );
    h->show();
    mHighlights.insert( id, h );
  }
}

// public slot to render highlight rectangles around pinned labels
void QgsMapToolPinLabels::highlightPinnedLabels()
{
  removePinnedHighlights();

  if ( !mShowPinned )
  {
    return;
  }

  if ( mCanvas )
  {
    mRender = mCanvas->mapRenderer();
    if ( !mRender )
    {
      QgsDebugMsg( QString( "Failed to acquire map renderer" ) );
      return;
    }
  }

  QgsDebugMsg( QString( "Highlighting pinned labels" ) );

  // get list of all drawn labels from all layers within given extent
  QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mRender->labelingEngine() );
  if ( !labelEngine )
  {
    QgsDebugMsg( QString( "No labeling engine" ) );
    return;
  }

  QgsRectangle ext = mCanvas->extent();
  QgsDebugMsg( QString( "Getting labels from canvas extent" ) );

  QList<QgsLabelPosition> labelPosList = labelEngine->labelsWithinRect( ext );

  QApplication::setOverrideCursor( Qt::BusyCursor );
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

      QColor lblcolor = QColor( 54, 129, 255, 255 );
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
        lblcolor = QColor( 54, 129, 0, 255 );
      }

      highlightLabel( vlayer, ( *it ), labelStringID, lblcolor );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::removePinnedHighlights()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  foreach ( QgsHighlight *h, mHighlights )
  {
    delete h;
  }
  mHighlights.clear();
  QApplication::restoreOverrideCursor();
}

void QgsMapToolPinLabels::pinUnpinLabels( const QgsRectangle& ext, QMouseEvent * e )
{

  bool doUnpin = e->modifiers() & Qt::ShiftModifier ? true : false;
  bool toggleUnpinOrPin = e->modifiers() & Qt::ControlModifier ? true : false;
  bool doHide = ( doUnpin && toggleUnpinOrPin );

  // get list of all drawn labels from all layers within, or touching, chosen extent
  bool labelChanged = false;

  if ( mCanvas )
  {
    mRender = mCanvas->mapRenderer();
    if ( !mRender )
    {
      QgsDebugMsg( QString( "Failed to acquire map renderer" ) );
      return;
    }
  }

  QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mRender->labelingEngine() );
  if ( !labelEngine )
  {
    QgsDebugMsg( QString( "No labeling engine" ) );
    return;
  }

  QList<QgsLabelPosition> labelPosList = labelEngine->labelsWithinRect( ext );

  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    mCurrentLabelPos = *it;

#ifdef QGISDEBUG
    QString labeltxt = currentLabelText();
    QString labellyr = currentLayer()->name();
#endif
    QgsDebugMsg( QString( "Label: %0" ).arg( labeltxt ) );
    QgsDebugMsg( QString( "Layer: %0" ).arg( labellyr ) );

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
    if ( mCurrentLabelPos.isPinned && !doHide && ( doUnpin  || toggleUnpinOrPin ) )
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
    if ( !mCurrentLabelPos.isPinned && !doHide && ( !doUnpin || toggleUnpinOrPin ) )
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

    // hide label
    if ( doHide )
    {
      // write 0 font size to attribute table
      if ( hideLabel( vlayer, mCurrentLabelPos ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QString( "Hide failed for layer, label: %0, %1" ).arg( labellyr, labeltxt ) );
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

  QString failedWrite = QString( "Failed write to attribute table" );

  if ( pin )
  {

//     QgsPoint labelpoint = labelpos.cornerPoints.at( 0 );

    QgsPoint referencePoint;
    if ( !rotationPoint( referencePoint, true ) )
    {
      referencePoint.setX( mCurrentLabelPos.labelRect.xMinimum() );
      referencePoint.setY( mCurrentLabelPos.labelRect.yMinimum() );
    }

    double labelX = referencePoint.x();
    double labelY = referencePoint.y();
    double labelR = labelpos.rotation * 180 / M_PI;

    // transform back to layer crs, if on-fly on
    if ( mRender->hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = mRender->mapToLayerCoordinates( vlayer, referencePoint );
      labelX = transformedPoint.x();
      labelY = transformedPoint.y();
    }

    vlayer->beginEditCommand( tr( "Label pinned" ) );
    if ( !vlayer->changeAttributeValue( fid, xCol, labelX, false ) )
    {
      QgsDebugMsg( failedWrite );
      return false;
    }
    if ( !vlayer->changeAttributeValue( fid, yCol, labelY, false ) )
    {
      QgsDebugMsg( failedWrite );
      return false;
    }
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, labelR, false ) )
      {
        QgsDebugMsg( failedWrite );
        return false;
      }
    }
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->beginEditCommand( tr( "Label unpinned" ) );
    if ( !vlayer->changeAttributeValue( fid, xCol, QVariant(), false ) )
    {
      QgsDebugMsg( failedWrite );
      return false;
    }
    if ( !vlayer->changeAttributeValue( fid, yCol, QVariant(), false ) )
    {
      QgsDebugMsg( failedWrite );
      return false;
    }
    if ( hasRCol && !preserveRot )
    {
      if ( !vlayer->changeAttributeValue( fid, rCol, QVariant(), false ) )
      {
        QgsDebugMsg( failedWrite );
        return false;
      }
    }
    vlayer->endEditCommand();
  }
  return true;
}

bool QgsMapToolPinLabels::hideLabel( QgsVectorLayer* vlayer,
                                        const QgsLabelPosition& labelpos )
{
  // skip diagrams
  if ( labelpos.isDiagram )
  {
    QgsDebugMsg( QString( "Label is diagram, skipping" ) );
    return false;
  }
  // verify attribute table has proper fields setup
  bool sizeColOk;
  int sizeCol;

  QVariant sizeColumn = vlayer->customProperty( "labeling/dataDefinedProperty0" );
  if ( !sizeColumn.isValid() )
  {
    QgsDebugMsg( QString( "Size column not set" ) );
    return false;
  }
  sizeCol = sizeColumn.toInt( &sizeColOk );
  if ( !sizeColOk )
  {
    QgsDebugMsg( QString( "Size column not convertible to integer" ) );
    return false;
  }

  // edit attribute table
  int fid = labelpos.featureId;

  vlayer->beginEditCommand( tr( "Label hidden" ) );
  if ( !vlayer->changeAttributeValue( fid, sizeCol, 0, false ) )
  {
    QgsDebugMsg( QString( "Failed write to attribute table" ) );
    return false;
  }
  vlayer->endEditCommand();
  return true;
}
