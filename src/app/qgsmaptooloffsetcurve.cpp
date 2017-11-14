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

#include "qgsdoublespinbox.h"
#include "qgsfeatureiterator.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"
#include "qgssnappingconfig.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsgeos.h"

#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include "qgisapp.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mModifiedFeature( -1 )
  , mGeometryModified( false )
  , mForceCopy( false )
  , mMultiPartGeometry( false )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
  deleteDistanceWidget();
  delete mSnapVertexMarker;
}


void QgsMapToolOffsetCurve::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( !layer )
  {
    deleteRubberBandAndGeometry();
    notifyNotVectorLayer();
    return;
  }

  if ( e->button() == Qt::RightButton )
  {
    deleteRubberBandAndGeometry();
    deleteDistanceWidget();
    return;
  }

  if ( mOriginalGeometry.isNull() )
  {
    deleteRubberBandAndGeometry();
    mGeometryModified = false;
    mForceCopy = false;

    if ( e->button() == Qt::RightButton )
    {
      return;
    }

    QgsSnappingUtils *snapping = mCanvas->snappingUtils();

    // store previous settings
    QgsSnappingConfig oldConfig = snapping->config();
    QgsSnappingConfig config = snapping->config();
    // setup new settings (temporary)
    QgsSettings settings;
    config.setEnabled( true );
    config.setMode( QgsSnappingConfig::AllLayers );
    config.setType( QgsSnappingConfig::Segment );
    config.setTolerance( settings.value( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit" ), 10 ).toDouble() );
    config.setUnits( static_cast<QgsTolerance::UnitType>( settings.value( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit_unit" ), QgsTolerance::Pixels ).toInt() ) );
    snapping->setConfig( config );

    QgsPointLocator::Match match = snapping->snapToMap( e->pos() );

    // restore old settings
    snapping->setConfig( oldConfig );

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
        createDistanceWidget();
      }
    }

    if ( mOriginalGeometry.isNull() )
    {
      emit messageEmitted( tr( "Could not find a nearby feature in any vector layer." ) );
    }
    return;
  }

  applyOffset();
}

void QgsMapToolOffsetCurve::applyOffset()
{
  QgsVectorLayer *layer = currentVectorLayer();
  if ( !layer )
  {
    deleteRubberBandAndGeometry();
    notifyNotVectorLayer();
    return;
  }

  // no modification
  if ( !mGeometryModified )
  {
    deleteRubberBandAndGeometry();
    layer->destroyEditCommand();
    deleteDistanceWidget();
    return;
  }

  if ( mMultiPartGeometry )
  {
    mModifiedGeometry.convertToMultiType();
  }

  layer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( mSourceLayerId == layer->id() && !mForceCopy )
  {
    editOk = layer->changeGeometry( mModifiedFeature, mModifiedGeometry );
  }
  else
  {
    QgsFeature f;
    f.setGeometry( mModifiedGeometry );

    //add empty values for all fields (allows inserting attribute values via the feature form in the same session)
    QgsAttributes attrs( layer->fields().count() );
    const QgsFields &fields = layer->fields();
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      attrs[idx] = QVariant();
    }
    f.setAttributes( attrs );
    editOk = layer->addFeature( f );
  }

  if ( editOk )
  {
    layer->endEditCommand();
  }
  else
  {
    layer->destroyEditCommand();
  }

  deleteRubberBandAndGeometry();
  deleteDistanceWidget();
  delete mSnapVertexMarker;
  mSnapVertexMarker = nullptr;
  mForceCopy = false;
  layer->triggerRepaint();
}

void QgsMapToolOffsetCurve::placeOffsetCurveToValue()
{
  setOffsetForRubberBand( mDistanceWidget->value() );
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QgsMapMouseEvent *e )
{
  delete mSnapVertexMarker;
  mSnapVertexMarker = nullptr;

  if ( mOriginalGeometry.isNull() || !mRubberBand )
  {
    return;
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }


  mGeometryModified = true;

  //get offset from current position rectangular to feature
  QgsPointXY layerCoords = toLayerCoordinates( layer, e->pos() );

  //snap cursor to background layers
  QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToMap( e->pos() );
  if ( m.isValid() )
  {
    if ( ( m.layer() && m.layer()->id() != mSourceLayerId ) || m.featureId() != mModifiedFeature )
    {
      layerCoords = toLayerCoordinates( layer, m.point() );
      mSnapVertexMarker = new QgsVertexMarker( mCanvas );
      mSnapVertexMarker->setIconType( QgsVertexMarker::ICON_CROSS );
      mSnapVertexMarker->setColor( Qt::green );
      mSnapVertexMarker->setPenWidth( 1 );
      mSnapVertexMarker->setCenter( m.point() );
    }
  }

  QgsPointXY minDistPoint;
  int beforeVertex;
  double leftOf;
  double offset = std::sqrt( mOriginalGeometry.closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  if ( offset == 0.0 )
  {
    return;
  }



  if ( mDistanceWidget )
  {
    // this will also set the rubber band
    mDistanceWidget->setValue( leftOf < 0 ? offset : -offset );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
  }
  else
  {
    //create offset geometry using geos
    setOffsetForRubberBand( leftOf < 0 ? offset : -offset );
  }
}

QgsGeometry QgsMapToolOffsetCurve::createOriginGeometry( QgsVectorLayer *vl, const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  if ( !vl )
  {
    return QgsGeometry();
  }

  mMultiPartGeometry = false;
  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  int partVertexNr = match.vertexIndex();

  if ( vl == currentVectorLayer() && !mForceCopy )
  {
    //don't consider selected geometries, only the snap result
    return convertToSingleLine( snappedFeature.geometry(), partVertexNr, mMultiPartGeometry );
  }
  else //snapped to a background layer
  {
    //if source layer is polygon / multipolygon, create a linestring from the snapped ring
    if ( vl->geometryType() == QgsWkbTypes::PolygonGeometry )
    {
      //make linestring from polygon ring and return this geometry
      return linestringFromPolygon( snappedFeature.geometry(), partVertexNr );
    }

    //for background layers, try to merge selected entries together if snapped feature is contained in selection
    const QgsFeatureIds &selection = vl->selectedFeatureIds();
    if ( selection.empty() || !selection.contains( match.featureId() ) )
    {
      return convertToSingleLine( snappedFeature.geometry(), partVertexNr, mMultiPartGeometry );
    }
    else
    {
      //merge together if several features
      QgsFeatureList selectedFeatures = vl->selectedFeatures();
      QgsFeatureList::iterator selIt = selectedFeatures.begin();
      QgsGeometry geom = selIt->geometry();
      ++selIt;
      for ( ; selIt != selectedFeatures.end(); ++selIt )
      {
        geom = geom.combine( selIt->geometry() );
      }

      //if multitype, return only the snapped to geometry
      if ( geom.isMultipart() )
      {
        return convertToSingleLine( snappedFeature.geometry(),
                                    match.vertexIndex(), mMultiPartGeometry );
      }

      return geom;
    }
  }
}

void QgsMapToolOffsetCurve::createDistanceWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteDistanceWidget();

  mDistanceWidget = new QgsDoubleSpinBox();
  mDistanceWidget->setMinimum( -99999999 );
  mDistanceWidget->setMaximum( 99999999 );
  mDistanceWidget->setDecimals( 6 );
  mDistanceWidget->setPrefix( tr( "Offset: " ) );
  mDistanceWidget->setClearValue( 0.0 );
  QgisApp::instance()->addUserInputWidget( mDistanceWidget );

  mDistanceWidget->setFocus( Qt::TabFocusReason );

  connect( mDistanceWidget, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolOffsetCurve::placeOffsetCurveToValue );
  connect( mDistanceWidget, &QAbstractSpinBox::editingFinished, this, &QgsMapToolOffsetCurve::applyOffset );
}

void QgsMapToolOffsetCurve::deleteDistanceWidget()
{
  if ( mDistanceWidget )
  {
    disconnect( mDistanceWidget, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolOffsetCurve::placeOffsetCurveToValue );
    disconnect( mDistanceWidget, &QAbstractSpinBox::editingFinished, this, &QgsMapToolOffsetCurve::applyOffset );
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  mOriginalGeometry.set( nullptr );
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolOffsetCurve::setOffsetForRubberBand( double offset )
{
  if ( !mRubberBand || mOriginalGeometry.isNull() )
  {
    return;
  }

  QgsVectorLayer *sourceLayer = dynamic_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( mSourceLayerId ) );
  if ( !sourceLayer )
  {
    return;
  }

  QgsGeometry geomCopy( mOriginalGeometry );
  geos::unique_ptr geosGeom( geomCopy.exportToGeos() );
  if ( geosGeom )
  {
    QgsSettings s;
    int joinStyle = s.value( QStringLiteral( "/qgis/digitizing/offset_join_style" ), 0 ).toInt();
    int quadSegments = s.value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt();
    double miterLimit = s.value( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), 5.0 ).toDouble();

    geos::unique_ptr offsetGeom( GEOSOffsetCurve_r( QgsGeometry::getGEOSHandler(), geosGeom.get(), offset, quadSegments, joinStyle, miterLimit ) );
    if ( !offsetGeom )
    {
      deleteRubberBandAndGeometry();
      deleteDistanceWidget();
      delete mSnapVertexMarker;
      mSnapVertexMarker = nullptr;
      mForceCopy = false;
      mGeometryModified = false;
      deleteDistanceWidget();
      emit messageEmitted( tr( "Creating offset geometry failed" ), QgsMessageBar::CRITICAL );
      return;
    }

    if ( offsetGeom )
    {
      mModifiedGeometry.fromGeos( offsetGeom.release() );
      mRubberBand->setToGeometry( mModifiedGeometry, sourceLayer );
    }
  }
}

QgsGeometry QgsMapToolOffsetCurve::linestringFromPolygon( const QgsGeometry &featureGeom, int vertex )
{
  if ( featureGeom.isNull() )
  {
    return QgsGeometry();
  }

  QgsWkbTypes::Type geomType = featureGeom.wkbType();
  int currentVertex = 0;
  QgsMultiPolygonXY multiPoly;

  if ( geomType == QgsWkbTypes::Polygon || geomType == QgsWkbTypes::Polygon25D )
  {
    QgsPolygonXY polygon = featureGeom.asPolygon();
    multiPoly.append( polygon );
  }
  else if ( geomType == QgsWkbTypes::MultiPolygon || geomType == QgsWkbTypes::MultiPolygon25D )
  {
    //iterate all polygons / rings
    multiPoly = featureGeom.asMultiPolygon();
  }
  else
  {
    return QgsGeometry();
  }

  QgsMultiPolygonXY::const_iterator multiPolyIt = multiPoly.constBegin();
  for ( ; multiPolyIt != multiPoly.constEnd(); ++multiPolyIt )
  {
    QgsPolygonXY::const_iterator polyIt = multiPolyIt->constBegin();
    for ( ; polyIt != multiPolyIt->constEnd(); ++polyIt )
    {
      currentVertex += polyIt->size();
      if ( vertex < currentVertex )
      {
        //found, return ring
        return QgsGeometry::fromPolylineXY( *polyIt );
      }
    }
  }

  return QgsGeometry();
}


QgsGeometry QgsMapToolOffsetCurve::convertToSingleLine( const QgsGeometry &geom, int vertex, bool &isMulti )
{
  if ( geom.isNull() )
  {
    return QgsGeometry();
  }

  isMulti = false;
  QgsWkbTypes::Type geomType = geom.wkbType();
  if ( geomType == QgsWkbTypes::LineString || geomType == QgsWkbTypes::LineString25D )
  {
    return geom;
  }
  else if ( geomType == QgsWkbTypes::MultiLineString || geomType == QgsWkbTypes::MultiLineString25D )
  {
    //search vertex
    isMulti = true;
    int currentVertex = 0;
    QgsMultiPolylineXY multiLine = geom.asMultiPolyline();
    QgsMultiPolylineXY::const_iterator it = multiLine.constBegin();
    for ( ; it != multiLine.constEnd(); ++it )
    {
      currentVertex += it->size();
      if ( vertex < currentVertex )
      {
        return QgsGeometry::fromPolylineXY( *it );
      }
    }
  }
  return QgsGeometry();
}
