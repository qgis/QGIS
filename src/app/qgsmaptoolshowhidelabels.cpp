/***************************************************************************
                          qgsmaptoolshowhidelabels.cpp
                          -----------------------
    begin                : 2012-08-12
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

#include "qgsmaptoolshowhidelabels.h"

#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include <qgslogger.h>

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolShowHideLabels::QgsMapToolShowHideLabels( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas )
{
  mRender = 0;
  mRubberBand = 0;
}

QgsMapToolShowHideLabels::~QgsMapToolShowHideLabels()
{
  delete mRubberBand;
}

void QgsMapToolShowHideLabels::canvasPressEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, true );
}

void QgsMapToolShowHideLabels::canvasMoveEvent( QMouseEvent * e )
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

void QgsMapToolShowHideLabels::canvasReleaseEvent( QMouseEvent * e )
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

    showHideLabels( e );

    mRubberBand->reset( true );
    delete mRubberBand;
    mRubberBand = 0;
  }

  mDragging = false;
}

void QgsMapToolShowHideLabels::showHideLabels( QMouseEvent * e )
{

  if ( !mCanvas || mCanvas->isDrawing() )
  {
    QgsDebugMsg( "Canvas not ready" );
    return;
  }

  mRender = mCanvas->mapRenderer();
  if ( !mRender )
  {
    QgsDebugMsg( "Failed to acquire map renderer" );
    return;
  }

  QgsMapLayer* layer = mCanvas->currentLayer();

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vlayer )
  {
    QgsDebugMsg( "Failed to cast label layer to vector layer" );
    return;
  }
  if ( !vlayer->isEditable() )
  {
    QgsDebugMsg( "Vector layer not editable, skipping label" );
    return;
  }

  bool doHide = e->modifiers() & Qt::ShiftModifier ? true : false;

  QgsFeatureIds selectedFeatIds;

  if ( !doHide )
  {
    QgsDebugMsg( "Showing labels operation" );

    if ( !selectedFeatures( vlayer, selectedFeatIds ) )
    {
      return;
    }
  }
  else
  {
    QgsDebugMsg( "Hiding labels operation" );

    if ( !selectedLabelFeatures( vlayer, selectedFeatIds ) )
    {
      return;
    }
  }

  QgsDebugMsg( "Number of selected labels or features: " + QString::number( selectedFeatIds.size() ) );


  bool labelChanged = false;

  foreach ( const QgsFeatureId &fid, selectedFeatIds )
  {
    if ( showHideLabel( vlayer, fid, doHide ) )
    {
      // TODO: highlight features (maybe with QTimer?)
      labelChanged = true;
    }
  }

  if ( labelChanged )
  {
    mCanvas->refresh();
  }
}

bool QgsMapToolShowHideLabels::selectedFeatures( QgsVectorLayer* vlayer,
    QgsFeatureIds& selectedFeatIds )
{
  // culled from QgsMapToolSelectUtils::setSelectFeatures()

  QgsGeometry* selectGeometry = mRubberBand->asGeometry();

  // toLayerCoordinates will throw an exception for any 'invalid' points in
  // the rubber band.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  QgsGeometry selectGeomTrans( *selectGeometry );

  if ( mRender->hasCrsTransformEnabled() )
  {
    try
    {
      QgsCoordinateTransform ct( mRender->destinationCrs(), vlayer->crs() );
      selectGeomTrans.transform( ct );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      // catch exception for 'invalid' point and leave existing selection unchanged
      QgsLogger::warning( "Caught CRS exception " + QString( __FILE__ ) + ": " + QString::number( __LINE__ ) );
      QMessageBox::warning( mCanvas, QObject::tr( "CRS Exception" ),
                            QObject::tr( "Selection extends beyond layer's coordinate system." ) );
      return false;
    }
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsDebugMsg( "Selection layer: " + vlayer->name() );
  QgsDebugMsg( "Selection polygon: " + selectGeomTrans.exportToWkt() );

  vlayer->select( QgsAttributeList(), selectGeomTrans.boundingBox(), false, true );

  QgsFeature f;
  while ( vlayer->nextFeature( f ) )
  {
    QgsGeometry* g = f.geometry();

    if ( !selectGeomTrans.intersects( g ) )
      continue;

    selectedFeatIds.insert( f.id() );
  }

  QApplication::restoreOverrideCursor();

  return true;
}

bool QgsMapToolShowHideLabels::selectedLabelFeatures( QgsVectorLayer* vlayer,
    QgsFeatureIds& selectedFeatIds )
{
  // get list of all drawn labels from current layer that intersect rubberband

  QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mRender->labelingEngine() );
  if ( !labelEngine )
  {
    QgsDebugMsg( "No labeling engine" );
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsRectangle ext = mRubberBand->asGeometry()->boundingBox();

  QList<QgsLabelPosition> labelPosList = labelEngine->labelsWithinRect( ext );

  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    mCurrentLabelPos = *it;

    if ( mCurrentLabelPos.layerID != vlayer->id() )
    {
      // only work with labels from the current active and editable layer
      continue;
    }

    selectedFeatIds.insert( mCurrentLabelPos.featureId );
  }

  QApplication::restoreOverrideCursor();

  return true;
}

bool QgsMapToolShowHideLabels::showHideLabel( QgsVectorLayer* vlayer,
    int fid,
    bool hide )
{

  // verify attribute table has proper field setup
  bool showSuccess;
  int showCol;
  int show;

  if ( !dataDefinedShowHide( vlayer, fid, show, showSuccess, showCol ) )
  {
    return false;
  }

  // edit attribute table
  QString editTxt = hide ? tr( "Label hidden" ) : tr( "Label shown" );
  vlayer->beginEditCommand( editTxt );
  if ( !vlayer->changeAttributeValue( fid, showCol, ( hide ? 0 : 1 ), false ) )
  {
    QgsDebugMsg( "Failed write to attribute table" );
    return false;
  }
  vlayer->endEditCommand();
  return true;
}
