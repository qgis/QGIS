/***************************************************************************
                              qgsmaptooloffsetcurve.cpp
    ------------------------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooloffsetcurve.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 ), mOriginalGeometry( 0 ), mGeometryModified( false )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
}

void QgsMapToolOffsetCurve::canvasPressEvent( QMouseEvent * e )
{
  deleteRubberBandAndGeometry();
  mGeometryModified = false;

  //get selected features or snap to nearest feature if no selection
  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  QList<QgsSnappingResult> snapResults;
  QgsMapCanvasSnapper snapper( mCanvas );
  snapper.snapToBackgroundLayers( e->pos(), snapResults );
  if ( snapResults.size() > 0 )
  {
    QgsFeature fet;
    const QgsSnappingResult& snapResult = snapResults.at( 0 );
    if ( snapResult.layer )
    {
      mSourceLayerId = snapResult.layer->id();

      QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mSourceLayerId ) );
      if ( vl && vl->featureAtId( snapResult.snappedAtGeometry, fet ) )
      {
        mOriginalGeometry = createOriginGeometry( vl, snapResult, fet );
        mRubberBand = createRubberBand();
        mRubberBand->setToGeometry( mOriginalGeometry, layer );
        mModifiedFeature = fet.id();
      }
    }
  }
}

void QgsMapToolOffsetCurve::canvasReleaseEvent( QMouseEvent * e )
{
  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer || !mGeometryModified )
  {
    deleteRubberBandAndGeometry();
    return;
  }

  vlayer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( mSourceLayerId == vlayer->id() )
  {
    editOk = vlayer->changeGeometry( mModifiedFeature, &mModifiedGeometry );
  }
  else
  {
    QgsFeature f;
    f.setGeometry( mModifiedGeometry );
    editOk = vlayer->addFeature( f );
  }

  if ( editOk )
  {
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->destroyEditCommand();
  }

  delete mRubberBand;
  mRubberBand = 0;
  mCanvas->refresh();
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mOriginalGeometry || !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  mGeometryModified = true;

  //get offset from current position rectangular to feature
  QgsPoint layerCoords = toLayerCoordinates( layer, e->pos() );
  QgsPoint minDistPoint;
  int beforeVertex;
  double leftOf;
  double offset = sqrt( mOriginalGeometry->closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  //qWarning( QString::number( offset ).toLocal8Bit().data() );

  //create offset geometry using geos
  QgsGeometry geomCopy( *mOriginalGeometry );
  GEOSGeometry* geosGeom = geomCopy.asGeos();
  if ( geosGeom )
  {
    GEOSGeometry* offsetGeom = GEOSSingleSidedBuffer( geosGeom, offset, 8, 1, 1, ( leftOf < 0 ) ? 1 : 0 );
    if ( offsetGeom )
    {
      mModifiedGeometry.fromGeos( offsetGeom );
      mRubberBand->setToGeometry( &mModifiedGeometry, layer );
    }
  }
}

QgsGeometry* QgsMapToolOffsetCurve::createOriginGeometry( QgsVectorLayer* vl, const QgsSnappingResult& sr, QgsFeature& snappedFeature )
{
  if ( !vl )
  {
    return 0;
  }

  if ( vl == currentVectorLayer() )
  {
    //don't consider selected geometries, only the snap result
    return snappedFeature.geometryAndOwnership();
  }
  else
  {
    //for background layers, try to merge selected entries together if snapped feature is contained in selection
    const QgsFeatureIds& selection = vl->selectedFeaturesIds();
    if ( selection.size() < 1 || !selection.contains( sr.snappedAtGeometry ) )
    {
      return snappedFeature.geometryAndOwnership();
    }
    else
    {
      //merge together if several features
      QgsFeatureList selectedFeatures = vl->selectedFeatures();
      QgsFeatureList::iterator selIt = selectedFeatures.begin();
      QgsGeometry* geom = selIt->geometryAndOwnership();
      ++selIt;
      for ( ; selIt != selectedFeatures.end(); ++selIt )
      {
        geom = geom->combine( selIt->geometry() );
      }

      //if multitype, return only the snaped to geometry
      if ( geom->isMultipart() )
      {
        delete geom;
        return snappedFeature.geometryAndOwnership();
      }

      return geom;
    }
  }
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  delete mRubberBand;
  mRubberBand = 0;
  delete mOriginalGeometry;
  mOriginalGeometry = 0;
}
