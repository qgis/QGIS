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
#include "qgsexception.h"
#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgslogger.h"
#include "qgslabelingresults.h"

QgsMapToolShowHideLabels::QgsMapToolShowHideLabels( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
  , mDragging( false )
{
  mToolName = tr( "Show/hide labels" );
  mRubberBand = nullptr;

  mPalProperties << QgsPalLayerSettings::Show;
  mDiagramProperties << QgsDiagramLayerSettings::Show;
}

QgsMapToolShowHideLabels::~QgsMapToolShowHideLabels()
{
  delete mRubberBand;
}

void QgsMapToolShowHideLabels::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )

  clearHoveredLabel();

  QgsMapLayer *layer = mCanvas->currentLayer();
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
    return;

  int showCol;
  if ( !labelCanShowHide( vlayer, showCol )
       || !diagramCanShowHide( vlayer, showCol ) )
  {
    if ( !vlayer->auxiliaryLayer() )
    {
      QgsNewAuxiliaryLayerDialog dlg( vlayer );
      dlg.exec();
      return;
    }
  }

  mSelectRect.setRect( 0, 0, 0, 0 );
  mSelectRect.setTopLeft( e->pos() );
  mSelectRect.setBottomRight( e->pos() );
  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
}

void QgsMapToolShowHideLabels::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
  {
    if ( !mDragging )
      updateHoveredLabel( e );
    return;
  }

  clearHoveredLabel();
  if ( !mDragging )
  {
    mDragging = true;
    mSelectRect.setTopLeft( e->pos() );
  }
  mSelectRect.setBottomRight( e->pos() );
  QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );
}

void QgsMapToolShowHideLabels::canvasReleaseEvent( QgsMapMouseEvent *e )
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

    mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    delete mRubberBand;
    mRubberBand = nullptr;
  }

  mDragging = false;
}

void QgsMapToolShowHideLabels::showHideLabels( QMouseEvent *e )
{
  QgsMapLayer *layer = mCanvas->currentLayer();
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
    return;

  bool doHide = e->modifiers() & Qt::ShiftModifier;
  QString editTxt = doHide ? tr( "Hid labels" ) : tr( "Showed labels" );
  vlayer->beginEditCommand( editTxt );

  bool labelChanged = false;
  if ( !mDragging )
  {
    // if single click only, clicking on an existing label hides it, while clicking on a feature shows it
    QList<QgsLabelPosition> positions;
    if ( selectedLabelFeatures( vlayer, positions ) )
    {
      // clicked on a label
      if ( showHide( positions.at( 0 ), false ) )
        labelChanged = true;
    }
    else
    {
      // clicked on a feature
      QgsFeatureIds fids;
      if ( selectedFeatures( vlayer, fids ) )
      {
        QgsLabelPosition pos;
        pos.featureId = *fids.constBegin();
        pos.layerID = vlayer->id();

        // we want to show labels...
        pos.isDiagram = false;
        if ( showHide( pos, true ) )
          labelChanged = true;

        // ... and diagrams
        pos.isDiagram = true;
        if ( showHide( pos, true ) )
          labelChanged = true;
      }
    }
  }
  else if ( doHide )
  {
    QList<QgsLabelPosition> positions;
    if ( selectedLabelFeatures( vlayer, positions ) )
    {
      for ( const QgsLabelPosition &pos : std::as_const( positions ) )
      {
        if ( showHide( pos, false ) )
          labelChanged = true;
      }
    }
  }
  else
  {
    QgsFeatureIds fids;
    if ( selectedFeatures( vlayer, fids ) )
    {
      for ( const QgsFeatureId &fid : std::as_const( fids ) )
      {
        QgsLabelPosition pos;
        pos.featureId = fid;
        pos.layerID = vlayer->id();

        // we want to show labels...
        pos.isDiagram = false;
        if ( showHide( pos, true ) )
          labelChanged = true;

        // ... and diagrams
        pos.isDiagram = true;
        if ( showHide( pos, true ) )
          labelChanged = true;
      }
    }
  }

  if ( labelChanged )
  {
    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
  else
  {
    vlayer->destroyEditCommand();
  }
}

bool QgsMapToolShowHideLabels::selectedFeatures( QgsVectorLayer *vlayer,
    QgsFeatureIds &selectedFeatIds )
{
  // culled from QgsMapToolSelectUtils::setSelectFeatures()

  QgsGeometry selectGeometry;
  if ( mDragging )
  {
    selectGeometry = mRubberBand->asGeometry();
  }
  else
  {
    selectGeometry = QgsGeometry::fromRect( QgsMapToolSelectUtils::expandSelectRectangle( mRubberBand->asGeometry().centroid().asPoint(), canvas(), vlayer ) );
  }

  // toLayerCoordinates will throw an exception for any 'invalid' points in
  // the rubber band.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  QgsGeometry selectGeomTrans( selectGeometry );

  try
  {
    QgsCoordinateTransform ct( mCanvas->mapSettings().destinationCrs(), vlayer->crs(), QgsProject::instance() );
    selectGeomTrans.transform( ct );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and leave existing selection unchanged
    QgsLogger::warning( "Caught CRS exception " + QStringLiteral( __FILE__ ) + ": " + QString::number( __LINE__ ) );
    emit messageEmitted( tr( "CRS Exception: selection extends beyond layer's coordinate system." ), Qgis::MessageLevel::Warning );
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsDebugMsg( "Selection layer: " + vlayer->name() );
  QgsDebugMsg( "Selection polygon: " + selectGeomTrans.asWkt() );

  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest()
                           .setFilterRect( selectGeomTrans.boundingBox() )
                           .setFlags( QgsFeatureRequest::NoGeometry | QgsFeatureRequest::ExactIntersect )
                           .setNoAttributes() );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    selectedFeatIds.insert( f.id() );
  }

  QApplication::restoreOverrideCursor();

  return !selectedFeatIds.empty();
}

bool QgsMapToolShowHideLabels::selectedLabelFeatures( QgsVectorLayer *vlayer,
    QList<QgsLabelPosition> &listPos )
{
  listPos.clear();

  // get list of all drawn labels from current layer that intersect rubberband
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
  {
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsRectangle ext = mRubberBand->asGeometry().boundingBox();
  QList<QgsLabelPosition> labelPosList = labelingResults->labelsWithinRect( ext );

  QList<QgsLabelPosition>::const_iterator it;
  for ( it = labelPosList.constBegin() ; it != labelPosList.constEnd(); ++it )
  {
    const QgsLabelPosition &pos = *it;

    if ( pos.layerID != vlayer->id() )
    {
      // only work with labels from the current active and editable layer
      continue;
    }

    listPos.append( pos );
  }

  QApplication::restoreOverrideCursor();

  return !listPos.empty();
}

bool QgsMapToolShowHideLabels::showHide( const QgsLabelPosition &pos, bool show )
{
  LabelDetails details = LabelDetails( pos, canvas() );

  if ( !details.valid )
    return false;

  QgsVectorLayer *vlayer = details.layer;
  if ( !vlayer )
    return false;

  int showCol = -1;
  if ( pos.isDiagram )
  {
    if ( !diagramCanShowHide( vlayer, showCol ) )
    {
      QgsDiagramIndexes indexes;
      createAuxiliaryFields( details, indexes );

      showCol = indexes[ QgsDiagramLayerSettings::Show ];
    }
  }
  else
  {
    if ( !labelCanShowHide( vlayer, showCol ) )
    {
      QgsPalIndexes indexes;
      createAuxiliaryFields( details, indexes );

      showCol = indexes[ QgsPalLayerSettings::Show ];
    }
  }

  if ( showCol >= 0 )
  {
    int showVal = show ? 1 : 0;
    vlayer->changeAttributeValue( pos.featureId, showCol, showVal );
    return true;
  }

  return false;
}
