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
#include "qgsvertexmarker.h"
#include <QDoubleSpinBox>
#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include "qgisapp.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 ),
    mOriginalGeometry( 0 ), mGeometryModified( false ), mDistanceItem( 0 ), mDistanceSpinBox( 0 ), mSnapVertexMarker( 0 ), mForceCopy( false ), mMultiPartGeometry( false )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
  deleteDistanceItem();
  delete mSnapVertexMarker;
}

void QgsMapToolOffsetCurve::canvasPressEvent( QMouseEvent* e )
{
  deleteRubberBandAndGeometry();
  mGeometryModified = false;
  mForceCopy = false;

  //get selected features or snap to nearest feature if no selection
  QgsVectorLayer* layer = currentVectorLayer();
  if ( !mCanvas || !layer )
  {
    return;
  }


  QgsMapRenderer* renderer = mCanvas->mapRenderer();
  QgsSnapper snapper( renderer );
  configureSnapper( snapper );
  QList<QgsSnappingResult> snapResults;
  snapper.snapPoint( e->pos(), snapResults );
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
        mForceCopy = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
        mOriginalGeometry = createOriginGeometry( vl, snapResult, fet );
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mOriginalGeometry, layer );
        }
        mModifiedFeature = fet.id();
        createDistanceItem();
      }
    }
  }
}

void QgsMapToolOffsetCurve::canvasReleaseEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer || !mGeometryModified )
  {
    deleteRubberBandAndGeometry();
    return;
  }

  if ( mMultiPartGeometry )
  {
    mModifiedGeometry.convertToMultiType();
  }

  vlayer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( mSourceLayerId == vlayer->id() && !mForceCopy )
  {
    editOk = vlayer->changeGeometry( mModifiedFeature, &mModifiedGeometry );
  }
  else
  {
    QgsFeature f;
    f.setGeometry( mModifiedGeometry );

    //add empty values for all fields (allows to insert attribute values via the feature form in the same session)
    QgsAttributeMap attMap;
    const QgsFieldMap& fields = vlayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fields.constBegin();
    for ( ; fieldIt != fields.constEnd(); ++fieldIt )
    {
      attMap.insert( fieldIt.key(), QVariant() );
    }
    f.setAttributeMap( attMap );
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

  deleteRubberBandAndGeometry();
  deleteDistanceItem();
  delete mSnapVertexMarker; mSnapVertexMarker = 0;
  mForceCopy = false;
  mCanvas->refresh();
}

void QgsMapToolOffsetCurve::placeOffsetCurveToValue()
{
  if ( mOriginalGeometry && mRubberBand && mRubberBand->numberOfVertices() > 0 )
  {
    //is rubber band left or right of original geometry
    double leftOf = 0;
    const QgsPoint* firstPoint = mRubberBand->getPoint( 0 );
    if ( firstPoint )
    {
      QgsPoint minDistPoint;
      int beforeVertex;
      mOriginalGeometry->closestSegmentWithContext( *firstPoint, minDistPoint, beforeVertex, &leftOf );
    }
    setOffsetForRubberBand( mDistanceSpinBox->value(), leftOf < 0 );
  }
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QMouseEvent * e )
{
  delete mSnapVertexMarker;
  mSnapVertexMarker = 0;

  if ( !mOriginalGeometry || !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  if ( mDistanceItem )
  {
    mDistanceItem->show();
    mDistanceItem->setPos( e->posF() + QPointF( 10, 10 ) );
  }

  mGeometryModified = true;

  //get offset from current position rectangular to feature
  QgsPoint layerCoords = toLayerCoordinates( layer, e->pos() );

  //snap cursor to background layers
  QList<QgsSnappingResult> results;
  QList<QgsPoint> snapExcludePoints;
  if ( mSnapper.snapToBackgroundLayers( e->pos(), results ) == 0 )
  {
    if ( results.size() > 0 )
    {
      QgsSnappingResult snap = results.at( 0 );
      if ( snap.layer && snap.layer->id() != mSourceLayerId && snap.snappedAtGeometry != mModifiedFeature )
      {
        layerCoords = results.at( 0 ).snappedVertex;
        mSnapVertexMarker = new QgsVertexMarker( mCanvas );
        mSnapVertexMarker->setIconType( QgsVertexMarker::ICON_CROSS );
        mSnapVertexMarker->setColor( Qt::green );
        mSnapVertexMarker->setPenWidth( 1 );
        mSnapVertexMarker->setCenter( layerCoords );
      }
    }
  }

  QgsPoint minDistPoint;
  int beforeVertex;
  double leftOf;
  double offset = sqrt( mOriginalGeometry->closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  if ( !offset > 0 )
  {
    return;
  }

  //create offset geometry using geos
  setOffsetForRubberBand( offset, leftOf < 0 );

  if ( mDistanceSpinBox )
  {
    mDistanceSpinBox->setValue( offset );
  }
}

QgsGeometry* QgsMapToolOffsetCurve::createOriginGeometry( QgsVectorLayer* vl, const QgsSnappingResult& sr, QgsFeature& snappedFeature )
{
  if ( !vl )
  {
    return 0;
  }
  mMultiPartGeometry = false;
  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  int partVertexNr = ( sr.snappedVertexNr == -1 ? sr.beforeVertexNr : sr.snappedVertexNr );

  if ( vl == currentVectorLayer() && !mForceCopy )
  {
    //don't consider selected geometries, only the snap result
    return convertToSingleLine( snappedFeature.geometryAndOwnership(), partVertexNr, mMultiPartGeometry );
  }
  else //snapped to a background layer
  {
    //if source layer is polygon / multipolygon, create a linestring from the snapped ring
    if ( vl->geometryType() == QGis::Polygon )
    {
      //make linestring from polygon ring and return this geometry
      return linestringFromPolygon( snappedFeature.geometry(), partVertexNr );
    }


    //for background layers, try to merge selected entries together if snapped feature is contained in selection
    const QgsFeatureIds& selection = vl->selectedFeaturesIds();
    if ( selection.size() < 1 || !selection.contains( sr.snappedAtGeometry ) )
    {
      return convertToSingleLine( snappedFeature.geometryAndOwnership(), partVertexNr, mMultiPartGeometry );
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
        return convertToSingleLine( snappedFeature.geometryAndOwnership(), sr.snappedVertexNr, mMultiPartGeometry );
      }

      return geom;
    }
  }
}

void QgsMapToolOffsetCurve::createDistanceItem()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteDistanceItem();

  mDistanceSpinBox = new QDoubleSpinBox();
  mDistanceSpinBox->setMaximum( 99999999 );
  mDistanceSpinBox->setDecimals( 2 );
  mDistanceSpinBox->setPrefix( tr( "Offset: " ) );
#ifndef Q_WS_X11
  mDistanceItem = new QGraphicsProxyWidget();
  mDistanceItem->setWidget( mDistanceSpinBox );
  mCanvas->scene()->addItem( mDistanceItem );
  mDistanceItem->hide();
#else
  mDistanceItem = 0;
  QgisApp::instance()->statusBar()->addWidget( mDistanceSpinBox );
#endif
  //mDistanceSpinBox->grabKeyboard();
  mDistanceSpinBox->setFocus( Qt::TabFocusReason );

  QObject::connect( mDistanceSpinBox, SIGNAL( editingFinished() ), this, SLOT( placeOffsetCurveToValue() ) );
}

void QgsMapToolOffsetCurve::deleteDistanceItem()
{
  if ( mDistanceSpinBox )
  {
    mDistanceSpinBox->releaseKeyboard();
  }
  delete mDistanceItem;
  mDistanceItem = 0;
#ifdef Q_WS_X11
  QgisApp::instance()->statusBar()->removeWidget( mDistanceSpinBox );
  delete mDistanceSpinBox;
#endif
  mDistanceSpinBox = 0;
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  delete mRubberBand;
  mRubberBand = 0;
  delete mOriginalGeometry;
  mOriginalGeometry = 0;
}

void QgsMapToolOffsetCurve::setOffsetForRubberBand( double offset, bool leftSide )
{
  if ( !mRubberBand || !mOriginalGeometry )
  {
    return;
  }

  QgsVectorLayer* sourceLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mSourceLayerId ) );
  if ( !sourceLayer )
  {
    return;
  }


  QgsGeometry geomCopy( *mOriginalGeometry );
  GEOSGeometry* geosGeom = geomCopy.asGeos();
  if ( geosGeom )
  {
    QSettings s;
    int joinStyle = s.value( "/qgis/digitizing/offset_join_style", 0 ).toInt();
    int quadSegments = s.value( "/qgis/digitizing/offset_quad_seg", 8 ).toInt();
    double mitreLimit = s.value( "/qgis/digitizine/offset_miter_limit", 5.0 ).toDouble();
    GEOSGeometry* offsetGeom = GEOSSingleSidedBuffer( geosGeom, offset, quadSegments, joinStyle, mitreLimit, leftSide ? 1 : 0 );
    if ( offsetGeom )
    {
      mModifiedGeometry.fromGeos( offsetGeom );
      mRubberBand->setToGeometry( &mModifiedGeometry, sourceLayer );
    }
  }
}

QgsGeometry* QgsMapToolOffsetCurve::linestringFromPolygon( QgsGeometry* featureGeom, int vertex )
{
  if ( !featureGeom )
  {
    return 0;
  }

  QGis::WkbType geomType = featureGeom->wkbType();
  int currentVertex = 0;
  QgsMultiPolygon multiPoly;

  if ( geomType == QGis::WKBPolygon || geomType == QGis::WKBPolygon25D )
  {
    QgsPolygon polygon = featureGeom->asPolygon();
    multiPoly.append( polygon );
  }
  else if ( geomType == QGis::WKBMultiPolygon || geomType == QGis::WKBMultiPolygon25D )
  {
    //iterate all polygons / rings
    QgsMultiPolygon multiPoly = featureGeom->asMultiPolygon();
  }
  else
  {
    return 0;
  }

  QgsMultiPolygon::const_iterator multiPolyIt = multiPoly.constBegin();
  for ( ; multiPolyIt != multiPoly.constEnd(); ++multiPolyIt )
  {
    QgsPolygon::const_iterator polyIt = multiPolyIt->constBegin();
    for ( ; polyIt != multiPolyIt->constEnd(); ++polyIt )
    {
      currentVertex += polyIt->size();
      if ( vertex < currentVertex )
      {
        //found, return ring
        return QgsGeometry::fromPolyline( *polyIt );
      }
    }
  }

  return 0;
}

void QgsMapToolOffsetCurve::configureSnapper( QgsSnapper& s )
{
  //use default vertex snap tolerance to all visible layers, but always to vertex and segment
  QList<QgsSnapper::SnapLayer> snapLayers;
  if ( mCanvas )
  {
    QList<QgsMapLayer*> layerList = mCanvas->layers();
    QList<QgsMapLayer*>::const_iterator layerIt = layerList.constBegin();
    for ( ; layerIt != layerList.constEnd(); ++layerIt )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( *layerIt );
      if ( vl )
      {
        QgsSnapper::SnapLayer sl;
        sl.mLayer = vl;
        QSettings settings;
        sl.mTolerance = settings.value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble();
        sl.mUnitType = ( QgsTolerance::UnitType ) settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", 0 ).toInt();
        sl.mSnapTo = QgsSnapper::SnapToVertexAndSegment;
        snapLayers.push_back( sl );
      }
    }
  }
  s.setSnapLayers( snapLayers );
  s.setSnapMode( QgsSnapper::SnapWithOneResult );
}

QgsGeometry* QgsMapToolOffsetCurve::convertToSingleLine( QgsGeometry* geom, int vertex, bool& isMulti )
{
  if ( !geom )
  {
    return 0;
  }

  isMulti = false;
  QGis::WkbType geomType = geom->wkbType();
  if ( geomType == QGis::WKBLineString || geomType == QGis::WKBLineString25D )
  {
    return geom;
  }
  else if ( geomType == QGis::WKBMultiLineString || geomType == QGis::WKBMultiLineString25D )
  {
    //search vertex
    isMulti = true;
    int currentVertex = 0;
    QgsMultiPolyline multiLine = geom->asMultiPolyline();
    QgsMultiPolyline::const_iterator it = multiLine.constBegin();
    for ( ; it != multiLine.constEnd(); ++it )
    {
      currentVertex += it->size();
      if ( vertex < currentVertex )
      {
        QgsGeometry* g = QgsGeometry::fromPolyline( *it );
        delete geom;
        return g;
      }
    }
  }
  delete geom;
  return 0;
}

QgsGeometry* QgsMapToolOffsetCurve::convertToMultiLine( QgsGeometry* geom )
{
  return 0;
}
