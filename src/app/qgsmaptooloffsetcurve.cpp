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
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"
#include <QDoubleSpinBox>
#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include "qgisapp.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mRubberBand( 0 )
    , mOriginalGeometry( 0 )
    , mModifiedFeature( -1 )
    , mGeometryModified( false )
    , mDistanceItem( 0 )
    , mDistanceSpinBox( 0 )
    , mSnapVertexMarker( 0 )
    , mForceCopy( false )
    , mMultiPartGeometry( false )
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

  if ( !mCanvas )
  {
    return;
  }

  //get selected features or snap to nearest feature if no selection
  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    notifyNotVectorLayer();
    return;
  }

  QgsSnappingUtils* snapping = mCanvas->snappingUtils();

  // store previous settings
  int oldType;
  double oldSearchRadius;
  QgsTolerance::UnitType oldSearchRadiusUnit;
  QgsSnappingUtils::SnapToMapMode oldMode = snapping->snapToMapMode();
  snapping->defaultSettings( oldType, oldSearchRadius, oldSearchRadiusUnit );

  // setup new settings (temporary)
  QSettings settings;
  snapping->setSnapToMapMode( QgsSnappingUtils::SnapAllLayers );
  snapping->setDefaultSettings( QgsPointLocator::Edge,
                                settings.value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble(),
                                ( QgsTolerance::UnitType ) settings.value( "/qgis/digitizing/search_radius_vertex_edit_unit", QgsTolerance::Pixels ).toInt() );

  QgsPointLocator::Match match = snapping->snapToMap( e->pos() );

  // restore old settings
  snapping->setSnapToMapMode( oldMode );
  snapping->setDefaultSettings( oldType, oldSearchRadius, oldSearchRadiusUnit );

  if ( match.hasEdge() && match.layer() )
  {
    mSourceLayerId = match.layer()->id();
    QgsFeature fet;
    if ( match.layer()->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
    {
      mForceCopy = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
      mOriginalGeometry = createOriginGeometry( match.layer(), match, fet );
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

void QgsMapToolOffsetCurve::canvasReleaseEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRubberBandAndGeometry();
    return;
  }

  if ( !mGeometryModified )
  {
    deleteRubberBandAndGeometry();
    vlayer->destroyEditCommand();
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

    //add empty values for all fields (allows inserting attribute values via the feature form in the same session)
    QgsAttributes attrs( vlayer->pendingFields().count() );
    const QgsFields& fields = vlayer->pendingFields();
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      attrs[idx] = QVariant();
    }
    f.setAttributes( attrs );
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
    const QgsPoint *firstPoint = mRubberBand->getPoint( 0 );
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
  QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToMap( e->pos() );
  if ( m.isValid() )
  {
    if (( m.layer() && m.layer()->id() != mSourceLayerId ) || m.featureId() != mModifiedFeature )
    {
      layerCoords = toLayerCoordinates( layer, m.point() );
      mSnapVertexMarker = new QgsVertexMarker( mCanvas );
      mSnapVertexMarker->setIconType( QgsVertexMarker::ICON_CROSS );
      mSnapVertexMarker->setColor( Qt::green );
      mSnapVertexMarker->setPenWidth( 1 );
      mSnapVertexMarker->setCenter( m.point() );
    }
  }

  QgsPoint minDistPoint;
  int beforeVertex;
  double leftOf;
  double offset = sqrt( mOriginalGeometry->closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  if ( offset == 0.0 )
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

QgsGeometry* QgsMapToolOffsetCurve::createOriginGeometry( QgsVectorLayer* vl, const QgsPointLocator::Match& match, QgsFeature& snappedFeature )
{
  if ( !vl )
  {
    return 0;
  }

  mMultiPartGeometry = false;
  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  int partVertexNr = match.vertexIndex();

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
    if ( selection.size() < 1 || !selection.contains( match.featureId() ) )
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
        QgsGeometry* combined = geom->combine( selIt->geometry() );
        delete geom;
        geom = combined;
      }

      //if multitype, return only the snapped to geometry
      if ( geom->isMultipart() )
      {
        delete geom;
        return convertToSingleLine( snappedFeature.geometryAndOwnership(), match.vertexIndex(), mMultiPartGeometry );
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
#ifndef Q_OS_UNIX
  mDistanceItem = new QGraphicsProxyWidget();
  mDistanceItem->setWidget( mDistanceSpinBox );
  mCanvas->scene()->addItem( mDistanceItem );
  mDistanceItem->hide();
#else
  mDistanceItem = 0;
  QgisApp::instance()->statusBar()->addWidget( mDistanceSpinBox );
#endif
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
#ifdef Q_OS_UNIX
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
  // need at least geos 3.3 for OffsetCurve tool
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
  ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
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
  const GEOSGeometry* geosGeom = geomCopy.asGeos();
  if ( geosGeom )
  {
    QSettings s;
    int joinStyle = s.value( "/qgis/digitizing/offset_join_style", 0 ).toInt();
    int quadSegments = s.value( "/qgis/digitizing/offset_quad_seg", 8 ).toInt();
    double mitreLimit = s.value( "/qgis/digitizing/offset_miter_limit", 5.0 ).toDouble();

    GEOSGeometry* offsetGeom = GEOSOffsetCurve_r( QgsGeometry::getGEOSHandler(), geosGeom, leftSide ? offset : -offset, quadSegments, joinStyle, mitreLimit );
    if ( !offsetGeom )
    {
      deleteRubberBandAndGeometry();
      deleteDistanceItem();
      delete mSnapVertexMarker; mSnapVertexMarker = 0;
      mForceCopy = false;
      mGeometryModified = false;
      deleteDistanceItem();
      emit messageEmitted( tr( "Creating offset geometry failed" ), QgsMessageBar::CRITICAL );
      return;
    }

    if ( offsetGeom )
    {
      mModifiedGeometry.fromGeos( offsetGeom );
      mRubberBand->setToGeometry( &mModifiedGeometry, sourceLayer );
    }
  }
#else //GEOS_VERSION>=3.3
  Q_UNUSED( offset );
  Q_UNUSED( leftSide );
#endif //GEOS_VERSION>=3.3
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
  Q_UNUSED( geom );
  return 0;
}
