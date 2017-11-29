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

#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include "qgisapp.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
  deleteDistanceWidget();
  delete mSnapVertexMarker;
}

void QgsMapToolOffsetCurve::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->key() == Qt::Key_Escape && !e->isAutoRepeat() )
  {
    deleteRubberBandAndGeometry();
    deleteDistanceWidget();
  }
  else
  {
    QgsMapToolEdit::keyPressEvent( e );
  }
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
    // first click, get feature to modify
    deleteRubberBandAndGeometry();
    mGeometryModified = false;
    mCtrlWasHeldOnFeatureSelection = false;

    QgsSnappingUtils *snapping = mCanvas->snappingUtils();

    // store previous settings
    QgsSnappingConfig oldConfig = snapping->config();
    QgsSnappingConfig config = snapping->config();
    // setup new settings (temporary)
    QgsSettings settings;
    config.setEnabled( true );
    config.setMode( QgsSnappingConfig::ActiveLayer );
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
        mCtrlWasHeldOnFeatureSelection = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
        prepareGeometry( match.layer(), match, fet );
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mManipulatedGeometry, layer );
        }
        mModifiedFeature = fet.id();
        createDistanceWidget();
      }
    }

    if ( mOriginalGeometry.isNull() )
    {
      emit messageEmitted( tr( "Could not find a nearby feature in any vector layer." ) );
    }
  }
  else
  {
    // second click - apply changes
    applyOffset( e->modifiers() & Qt::ControlModifier );
  }
}

void QgsMapToolOffsetCurve::applyOffset( bool forceCopy )
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
    QgsGeometry geometry;
    int partIndex = 0;
    QgsWkbTypes::Type geomType = mOriginalGeometry.wkbType();
    if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry )
    {
      QgsMultiPolylineXY newMultiLine;
      QgsMultiPolylineXY multiLine = mOriginalGeometry.asMultiPolyline();
      QgsMultiPolylineXY::const_iterator it = multiLine.constBegin();
      for ( ; it != multiLine.constEnd(); ++it )
      {
        if ( partIndex == mModifiedPart )
        {
          newMultiLine.append( mModifiedGeometry.asPolyline() );
        }
        else
        {
          newMultiLine.append( *it );
        }
        partIndex++;
      }
      geometry = QgsGeometry::fromMultiPolylineXY( newMultiLine );
    }
    else
    {
      QgsMultiPolygonXY newMultiPoly;
      QgsMultiPolygonXY multiPoly = mOriginalGeometry.asMultiPolygon();
      QgsMultiPolygonXY::const_iterator multiPolyIt = multiPoly.constBegin();
      for ( ; multiPolyIt != multiPoly.constEnd(); ++multiPolyIt )
      {
        if ( partIndex == mModifiedPart )
        {
          if ( mModifiedGeometry.isMultipart() )
          {
            newMultiPoly += mModifiedGeometry.asMultiPolygon();
          }
          else
          {
            newMultiPoly.append( mModifiedGeometry.asPolygon() );
          }
        }
        else
        {
          newMultiPoly.append( *multiPolyIt );
        }
        partIndex++;
      }
      geometry = QgsGeometry::fromMultiPolygonXY( newMultiPoly );
    }
    geometry.convertToMultiType();
    mModifiedGeometry = geometry;
  }

  layer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( mSourceLayerId == layer->id() && !mCtrlWasHeldOnFeatureSelection && !forceCopy )
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
  mCtrlWasHeldOnFeatureSelection = false;
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
  int leftOf = 0;
  double offset = std::sqrt( mOriginalGeometry.closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  if ( offset == 0.0 )
  {
    return;
  }
  offset = leftOf < 0 ? offset : -offset;


  if ( mDistanceWidget )
  {
    // this will also set the rubber band
    mDistanceWidget->setValue( offset );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
  }
  else
  {
    //create offset geometry using geos
    setOffsetForRubberBand( offset );
  }
}

void QgsMapToolOffsetCurve::prepareGeometry( QgsVectorLayer *vl, const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  if ( !vl )
  {
    return;
  }

  mOriginalGeometry = QgsGeometry();
  mManipulatedGeometry = QgsGeometry();
  mMultiPartGeometry = false;
  mModifiedPart = 0;

  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  QgsGeometry geom = snappedFeature.geometry();
  if ( geom.isNull() )
  {
    return;
  }
  mOriginalGeometry = geom;

  QgsWkbTypes::Type geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry )
  {
    if ( !geom.isMultipart() )
    {
      mManipulatedGeometry = geom;
    }
    else
    {
      mMultiPartGeometry = true;

      int vertex = match.vertexIndex();
      QgsVertexId vertexId;
      geom.vertexIdFromVertexNr( vertex, vertexId );
      mModifiedPart = vertexId.part;

      QgsMultiPolylineXY multiLine = geom.asMultiPolyline();
      mManipulatedGeometry = QgsGeometry::fromPolylineXY( multiLine.at( mModifiedPart ) );
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PolygonGeometry )
  {
    if ( !geom.isMultipart() )
    {
      mManipulatedGeometry = geom;
    }
    else
    {
      mMultiPartGeometry = true;

      int vertex = match.vertexIndex();
      QgsVertexId vertexId;
      geom.vertexIdFromVertexNr( vertex, vertexId );
      mModifiedPart = vertexId.part;

      QgsMultiPolygonXY multiPoly = geom.asMultiPolygon();
      mManipulatedGeometry = QgsGeometry::fromPolygonXY( multiPoly.at( mModifiedPart ) );
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
  connect( mDistanceWidget, &QAbstractSpinBox::editingFinished, this, [ = ] { applyOffset(); } );
}

void QgsMapToolOffsetCurve::deleteDistanceWidget()
{
  if ( mDistanceWidget )
  {
    disconnect( mDistanceWidget, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolOffsetCurve::placeOffsetCurveToValue );
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  mOriginalGeometry.set( nullptr );
  mManipulatedGeometry.set( nullptr );
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

  QgsSettings s;
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle >( s.value( QStringLiteral( "/qgis/digitizing/offset_join_style" ), 0 ).toInt() );
  int quadSegments = s.value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt();
  double miterLimit = s.value( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), 5.0 ).toDouble();

  QgsGeometry offsetGeom;
  if ( QgsWkbTypes::geometryType( mOriginalGeometry.wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    offsetGeom = mManipulatedGeometry.offsetCurve( offset, quadSegments, joinStyle, miterLimit );
  }
  else
  {
    offsetGeom = mManipulatedGeometry.buffer( offset, quadSegments, QgsGeometry::CapRound, joinStyle, miterLimit );
  }

  if ( !offsetGeom )
  {
    deleteRubberBandAndGeometry();
    deleteDistanceWidget();
    delete mSnapVertexMarker;
    mSnapVertexMarker = nullptr;
    mCtrlWasHeldOnFeatureSelection = false;
    mGeometryModified = false;
    deleteDistanceWidget();
    emit messageEmitted( tr( "Creating offset geometry failed: %1" ).arg( offsetGeom.lastError() ), QgsMessageBar::CRITICAL );
  }
  else
  {
    mModifiedGeometry = offsetGeom;
    mRubberBand->setToGeometry( mModifiedGeometry, sourceLayer );
  }
}
