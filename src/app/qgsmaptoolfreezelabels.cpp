/***************************************************************************
                          qgsmaptoolfreezelabels.cpp
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

#include "qgsmaptoolfreezelabels.h"

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

QgsMapToolFreezeLabels::QgsMapToolFreezeLabels( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas )
{
  mRender = 0;
  mRubberBand = 0;
  mShowFrozen = false;

  connect( QgisApp::instance()->actionToggleEditing(), SIGNAL( triggered() ), this, SLOT( updateFrozenLabels() ) );
  connect( canvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( highlightFrozenLabels() ) );
}

QgsMapToolFreezeLabels::~QgsMapToolFreezeLabels()
{
  delete mRubberBand;
  removeFrozenHighlights();
}

void QgsMapToolFreezeLabels::canvasPressEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, true );
}

void QgsMapToolFreezeLabels::canvasMoveEvent( QMouseEvent * e )
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

void QgsMapToolFreezeLabels::canvasReleaseEvent( QMouseEvent * e )
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

    freezeThawLabels( ext, e );

    delete selectGeom;

    mRubberBand->reset( true );
    delete mRubberBand;
    mRubberBand = 0;
  }

  mDragging = false;
}

void QgsMapToolFreezeLabels::showFrozenLabels( bool show )
{
  mShowFrozen = show;
  if ( mShowFrozen )
  {
    QgsDebugMsg( QString( "Toggling on frozen label highlighting" ));
    highlightFrozenLabels();
  }
  else
  {
    QgsDebugMsg( QString( "Toggling off frozen label highlighting" ));
    removeFrozenHighlights();
  }
}

// public slot to update frozen label highlights on layer edit mode change
void QgsMapToolFreezeLabels::updateFrozenLabels()
{
  if ( mShowFrozen )
  {
    QgsDebugMsg( QString( "Updating highlighting due to layer editing mode change" ));
    mCanvas->refresh();
  }
}

void QgsMapToolFreezeLabels::highlightLabel( QgsVectorLayer* vlayer,
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
      QgsDebugMsg( QString( "Reverse transform needed for highlight rectangle" ));
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

// public slot to render highlight rectangles around frozen labels
void QgsMapToolFreezeLabels::highlightFrozenLabels()
{
  removeFrozenHighlights();

  if ( !mShowFrozen )
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

  QgsDebugMsg( QString( "Highlighting frozen labels" ) );

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

    if( mCurrentLabelPos.isFrozen )
    {
      QString labelStringID = QString("%0|%1").arg(mCurrentLabelPos.layerID, QString::number( mCurrentLabelPos.featureId ) );

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

      highlightLabel( vlayer, ( *it ), labelStringID, lblcolor);
    }
  }
  QApplication::restoreOverrideCursor();
}

void QgsMapToolFreezeLabels::removeFrozenHighlights()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  foreach( QgsHighlight *h, mHighlights )
  {
    delete h;
  }
  mHighlights.clear();
  QApplication::restoreOverrideCursor();
}

void QgsMapToolFreezeLabels::freezeThawLabels( const QgsRectangle& ext, QMouseEvent * e  )
{

  bool doThaw = e->modifiers() & Qt::ShiftModifier ? true : false;
  bool toggleThawOrFreeze = e->modifiers() & Qt::AltModifier ? true : false;

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

    QString labelStringID = QString("%0|%1").arg(mCurrentLabelPos.layerID, QString::number( mCurrentLabelPos.featureId ) );

    // thaw label
    if ( mCurrentLabelPos.isFrozen && ( doThaw  || toggleThawOrFreeze  ) )
    {

      // thaw previously frozen label (set attribute table fields to NULL)
      if ( freezeThawLabel( vlayer, mCurrentLabelPos, false ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QString( "Thaw failed for layer, label: %0, %1" ).arg( labellyr, labeltxt ) );
      }
    }

    // freeze label
    if ( !mCurrentLabelPos.isFrozen && ( !doThaw || toggleThawOrFreeze ) )
    {

      // freeze label's location, and optionally rotation, to attribute table
      if ( freezeThawLabel( vlayer, mCurrentLabelPos, true ) )
      {
        labelChanged = true;
      }
      else
      {
        QgsDebugMsg( QString( "Freeze failed for layer, label: %0, %1" ).arg( labellyr, labeltxt ) );
      }
    }
  }

  if ( labelChanged )
  {
    mCanvas->refresh();

    if ( !mShowFrozen )
    {
      // toggle it on (freeze-thaw tool doesn't work well without it)
      QgisApp::instance()->actionShowFrozenLabels()->setChecked( true );
    }
  }
}

bool QgsMapToolFreezeLabels::freezeThawLabel( QgsVectorLayer* vlayer,
                                              const QgsLabelPosition& labelpos,
                                              bool freeze )
{
  // skip diagrams
  if ( labelpos.isDiagram )
  {
    QgsDebugMsg( QString( "Label is diagram, skipping" ) );
    return false;
  }
  // verify attribute table has proper fields setup
  bool xColOk, yColOk, rColOk;
  int xCol, yCol, rCol;

  QVariant xColumn = vlayer->customProperty( "labeling/dataDefinedProperty9" );
  if ( !xColumn.isValid() )
  {
    QgsDebugMsg( QString( "X column not set" ) );
    return false;
  }
  xCol = xColumn.toInt( &xColOk );
  if ( !xColOk )
  {
    QgsDebugMsg( QString( "X column not convertible to integer" ) );
    return false;
  }

  QVariant yColumn = vlayer->customProperty( "labeling/dataDefinedProperty10" );
  if ( !yColumn.isValid() )
  {
    QgsDebugMsg( QString( "Y column not set" ) );
    return false;
  }
  yCol = yColumn.toInt( &yColOk );
  if ( !yColOk )
  {
    QgsDebugMsg( QString( "Y column not convertible to integer" ) );
    return false;
  }

  // rotation field is optional, but will be used if available
  bool hasRCol = true;
  QVariant rColumn = vlayer->customProperty( "labeling/dataDefinedProperty14" );
  if ( !rColumn.isValid() )
  {
    QgsDebugMsg( QString( "Rotation column not set" ) );
    hasRCol = false;
  }
  rCol = rColumn.toInt( &rColOk );
  if ( !rColOk )
  {
    QgsDebugMsg( QString( "Rotation column not convertible to integer" ) );
    hasRCol = false;
  }

  // edit attribute table
  int fid = labelpos.featureId;

  QString failedWrite = QString( "Failed write to attribute table" );

  if ( freeze )
  {

    QgsPoint labelpoint = labelpos.cornerPoints.at( 0 );

    double labelX = labelpoint.x();
    double labelY = labelpoint.y();
    double labelR = labelpos.rotation * 180 / M_PI;

    // transform back to layer crs, if on-fly on
    if ( mRender->hasCrsTransformEnabled() )
    {
      QgsPoint transformedPoint = mRender->mapToLayerCoordinates( vlayer, labelpoint );
      labelX = transformedPoint.x();
      labelY = transformedPoint.y();
    }

    vlayer->beginEditCommand( tr( "Label frozen" ) );
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
    if ( hasRCol )
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
    vlayer->beginEditCommand( tr( "Label thawed" ) );
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
    if ( hasRCol )
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
