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

#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include <QGridLayout>
#include <QLabel>

#include "qgsdoublespinbox.h"
#include "qgsfeatureiterator.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgssnapindicator.h"
#include "qgssnappingconfig.h"
#include "qgssettings.h"
#include "qgisapp.h"

#include "qgisapp.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator >( canvas ) )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  cancel();
}

void QgsMapToolOffsetCurve::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->key() == Qt::Key_Escape && !e->isAutoRepeat() )
  {
    cancel();
  }
  else
  {
    QgsMapToolEdit::keyPressEvent( e );
  }
}


void QgsMapToolOffsetCurve::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mCtrlHeldOnFirstClick = false;

  if ( e->button() == Qt::RightButton )
  {
    cancel();
    return;
  }

  if ( mOriginalGeometry.isNull() )
  {
    // first click, get feature to modify
    deleteRubberBandAndGeometry();
    mGeometryModified = false;

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
      mLayer = match.layer();
      QgsFeature fet;
      if ( match.layer()->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
      {
        mCtrlHeldOnFirstClick = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
        prepareGeometry( match.layer(), match, fet );
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mManipulatedGeometry, match.layer() );
        }
        mModifiedFeature = fet.id();
        createUserInputWidget();
      }
    }

    if ( mOriginalGeometry.isNull() )
    {
      emit messageEmitted( tr( "Could not find a nearby feature in any vector layer." ) );
      cancel();
      notifyNotVectorLayer();
    }
  }
  else
  {
    // second click - apply changes
    double offset = calculateOffset( e->snapPoint() );
    applyOffset( offset, e->modifiers() );
  }
}

void QgsMapToolOffsetCurve::applyOffset( const double &offset, const Qt::KeyboardModifiers &modifiers )
{
  if ( !mLayer || offset == 0.0 )
  {
    cancel();
    notifyNotVectorLayer();
    return;
  }

  updateGeometryAndRubberBand( offset );

  // no modification
  if ( !mGeometryModified )
  {
    mLayer->destroyEditCommand();
    cancel();
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

  mLayer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( !mCtrlHeldOnFirstClick && !modifiers.testFlag( Qt::ControlModifier ) )
  {
    editOk = mLayer->changeGeometry( mModifiedFeature, mModifiedGeometry );
  }
  else
  {
    QgsFeature f;
    f.setGeometry( mModifiedGeometry );

    //add empty values for all fields (allows inserting attribute values via the feature form in the same session)
    QgsAttributes attrs( mLayer->fields().count() );
    const QgsFields &fields = mLayer->fields();
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      attrs[idx] = QVariant();
    }
    f.setAttributes( attrs );
    editOk = mLayer->addFeature( f );
  }

  if ( editOk )
  {
    mLayer->endEditCommand();
  }
  else
  {
    mLayer->destroyEditCommand();
    emit messageEmitted( "Could not apply offset", Qgis::Critical );
  }

  deleteRubberBandAndGeometry();
  deleteUserInputWidget();
  mLayer->triggerRepaint();
  mLayer = nullptr;
}

void QgsMapToolOffsetCurve::cancel()
{
  deleteUserInputWidget();
  deleteRubberBandAndGeometry();
  mLayer = nullptr;
}

double QgsMapToolOffsetCurve::calculateOffset( QgsPointXY mapPoint )
{
  double offset = 0.0;
  if ( mLayer )
  {
    //get offset from current position rectangular to feature
    QgsPointXY layerCoords = toLayerCoordinates( mLayer, mapPoint );

    QgsPointXY minDistPoint;
    int beforeVertex;
    int leftOf = 0;

    offset = std::sqrt( mOriginalGeometry.closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
    offset = leftOf < 0 ? offset : -offset;
  }
  return offset;
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mOriginalGeometry.isNull() || !mRubberBand )
  {
    return;
  }

  mGeometryModified = true;

  QgsPointXY mapPoint = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  double offset = calculateOffset( mapPoint );
  if ( offset == 0.0 )
  {
    return;
  }

  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
    mUserInputWidget->setOffset( offset );
    connect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
    mUserInputWidget->setFocus( Qt::TabFocusReason );
    mUserInputWidget->editor()->selectAll();
  }

  //create offset geometry using geos
  updateGeometryAndRubberBand( offset );
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

void QgsMapToolOffsetCurve::createUserInputWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteUserInputWidget();
  mUserInputWidget = new QgsOffsetUserWidget();
  QgisApp::instance()->addUserInputWidget( mUserInputWidget );
  mUserInputWidget->setFocus( Qt::TabFocusReason );

  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingFinished, this, &QgsMapToolOffsetCurve::applyOffset );
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingCanceled, this, &QgsMapToolOffsetCurve::cancel );
}

void QgsMapToolOffsetCurve::deleteUserInputWidget()
{
  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingFinished, this, &QgsMapToolOffsetCurve::applyOffset );
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingCanceled, this, &QgsMapToolOffsetCurve::cancel );
    mUserInputWidget->releaseKeyboard();
    mUserInputWidget->deleteLater();
  }
  mUserInputWidget = nullptr;
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  mOriginalGeometry.set( nullptr );
  mManipulatedGeometry.set( nullptr );
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolOffsetCurve::updateGeometryAndRubberBand( double offset )
{
  if ( !mRubberBand || mOriginalGeometry.isNull() )
  {
    return;
  }

  if ( !mLayer )
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
    deleteUserInputWidget();
    mLayer = nullptr;
    mGeometryModified = false;
    emit messageEmitted( tr( "Creating offset geometry failed: %1" ).arg( offsetGeom.lastError() ), Qgis::Critical );
  }
  else
  {
    mModifiedGeometry = offsetGeom;
    mRubberBand->setToGeometry( mModifiedGeometry, mLayer );
  }
}


// ******************
// Offset User Widget

QgsOffsetUserWidget::QgsOffsetUserWidget( QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QGridLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  //mLayout->setAlignment( Qt::AlignLeft );
  setLayout( mLayout );

  QLabel *lbl = new QLabel( tr( "Offset" ), this );
  lbl->setAlignment( Qt::AlignRight | Qt::AlignCenter );
  mLayout->addWidget( lbl, 0, 0 );

  mOffsetSpinBox = new QgsDoubleSpinBox();
  mOffsetSpinBox->setMinimum( -99999999 );
  mOffsetSpinBox->setMaximum( 99999999 );
  mOffsetSpinBox->setDecimals( 6 );
  mOffsetSpinBox->setClearValue( 0.0 );
  mOffsetSpinBox->setShowClearButton( false );
  mLayout->addWidget( mOffsetSpinBox, 0, 1 );

  // connect signals
  mOffsetSpinBox->installEventFilter( this );
  connect( mOffsetSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsOffsetUserWidget::offsetSpinBoxValueChanged );

  // config focus
  setFocusProxy( mOffsetSpinBox );
}

void QgsOffsetUserWidget::setOffset( double offset )
{
  mOffsetSpinBox->setValue( offset );
}

double QgsOffsetUserWidget::offset()
{
  return mOffsetSpinBox->value();
}

bool QgsOffsetUserWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mOffsetSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit offsetEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit offsetEditingFinished( offset(), event->modifiers() );
      return true;
    }
  }

  return false;
}

void QgsOffsetUserWidget::offsetSpinBoxValueChanged( double offset )
{
  emit offsetChanged( offset );
}
