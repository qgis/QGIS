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
#include "qgsmapmouseevent.h"
#include "qgslogger.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator >( canvas ) )
{}

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

    QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(),
                                   QgsPointLocator::Types( QgsPointLocator::Edge | QgsPointLocator::Area ) );

    if ( ( match.hasEdge() || match.hasArea() ) && match.layer() )
    {
      mLayer = match.layer();
      QgsFeature fet;
      if ( match.layer()->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
      {
        mCtrlHeldOnFirstClick = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
        prepareGeometry( match, fet );
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mManipulatedGeometry, match.layer() );
        }
        mModifiedFeature = fet.id();
        createUserInputWidget();

        bool hasZ = QgsWkbTypes::hasZ( mLayer->wkbType() );
        bool hasM = QgsWkbTypes::hasZ( mLayer->wkbType() );
        if ( hasZ || hasM )
        {
          emit messageEmitted( QStringLiteral( "layer %1 has %2%3%4 geometry. %2%3%4 values be set to 0 when using offset tool." )
                               .arg( mLayer->name() )
                               .arg( hasZ ? "Z" : "" )
                               .arg( hasZ && hasM ? "/" : "" )
                               .arg( hasM ? "M" : "" )
                               , Qgis::Warning );
        }
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

void QgsMapToolOffsetCurve::applyOffset( double offset, Qt::KeyboardModifiers modifiers )
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

  if ( mModifiedPart >= 0 )
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
      const QgsMultiPolygonXY multiPoly = mOriginalGeometry.asMultiPolygon();
      QgsMultiPolygonXY::const_iterator multiPolyIt = multiPoly.constBegin();
      for ( ; multiPolyIt != multiPoly.constEnd(); ++multiPolyIt )
      {
        if ( partIndex == mModifiedPart )
        {
          if ( mModifiedGeometry.isMultipart() )
          {
            // not a ring
            if ( mModifiedRing <= 0 )
            {
              // part became mulitpolygon, that means discard original rings from the part
              newMultiPoly += mModifiedGeometry.asMultiPolygon();
            }
            else
            {
              // ring became multipolygon, oh boy!
              QgsPolygonXY newPoly;
              int ringIndex = 0;
              QgsPolygonXY::const_iterator polyIt = multiPolyIt->constBegin();
              for ( ; polyIt != multiPolyIt->constEnd(); ++polyIt )
              {
                if ( ringIndex == mModifiedRing )
                {
                  const QgsMultiPolygonXY ringParts = mModifiedGeometry.asMultiPolygon();
                  QgsPolygonXY newRings;
                  QgsMultiPolygonXY::const_iterator ringIt = ringParts.constBegin();
                  for ( ; ringIt != ringParts.constEnd(); ++ringIt )
                  {
                    // the different parts of the new rings cannot have rings themselves
                    newRings.append( ringIt->at( 0 ) );
                  }
                  newPoly += newRings;
                }
                else
                {
                  newPoly.append( *polyIt );
                }
                ringIndex++;
              }
              newMultiPoly.append( newPoly );
            }
          }
          else
          {
            // original part had no ring
            if ( mModifiedRing == -1 )
            {
              newMultiPoly.append( mModifiedGeometry.asPolygon() );
            }
            else
            {
              QgsPolygonXY newPoly;
              int ringIndex = 0;
              QgsPolygonXY::const_iterator polyIt = multiPolyIt->constBegin();
              for ( ; polyIt != multiPolyIt->constEnd(); ++polyIt )
              {
                if ( ringIndex == mModifiedRing )
                {
                  newPoly.append( mModifiedGeometry.asPolygon().at( 0 ) );
                }
                else
                {
                  newPoly.append( *polyIt );
                }
                ringIndex++;
              }
              newMultiPoly.append( newPoly );
            }
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
  else if ( mModifiedRing >= 0 )
  {
    // original geometry had some rings
    if ( mModifiedGeometry.isMultipart() )
    {
      // not a ring
      if ( mModifiedRing == 0 )
      {
        // polygon became mulitpolygon, that means discard original rings from the part
        // keep the modified geometry as is
      }
      else
      {
        QgsPolygonXY newPoly;
        const QgsPolygonXY poly = mOriginalGeometry.asPolygon();

        // ring became multipolygon, oh boy!
        int ringIndex = 0;
        QgsPolygonXY::const_iterator polyIt = poly.constBegin();
        for ( ; polyIt != poly.constEnd(); ++polyIt )
        {
          if ( ringIndex == mModifiedRing )
          {
            QgsMultiPolygonXY ringParts = mModifiedGeometry.asMultiPolygon();
            QgsPolygonXY newRings;
            QgsMultiPolygonXY::const_iterator ringIt = ringParts.constBegin();
            for ( ; ringIt != ringParts.constEnd(); ++ringIt )
            {
              // the different parts of the new rings cannot have rings themselves
              newRings.append( ringIt->at( 0 ) );
            }
            newPoly += newRings;
          }
          else
          {
            newPoly.append( *polyIt );
          }
          ringIndex++;
        }
        mModifiedGeometry = QgsGeometry::fromPolygonXY( newPoly );
      }
    }
    else
    {
      // simple case where modified geom is a polygon (not multi)
      QgsPolygonXY newPoly;
      const QgsPolygonXY poly = mOriginalGeometry.asPolygon();

      int ringIndex = 0;
      QgsPolygonXY::const_iterator polyIt = poly.constBegin();
      for ( ; polyIt != poly.constEnd(); ++polyIt )
      {
        if ( ringIndex == mModifiedRing )
        {
          newPoly.append( mModifiedGeometry.asPolygon().at( 0 ) );
        }
        else
        {
          newPoly.append( *polyIt );
        }
        ringIndex++;
      }
      mModifiedGeometry = QgsGeometry::fromPolygonXY( newPoly );
    }
  }

  if ( !mModifiedGeometry.isGeosValid() )
  {
    emit messageEmitted( tr( "Generated geometry is not valid." ), Qgis::Critical );
    // no cancel, continue editing.
    return;
  }

  mLayer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk;
  if ( !mCtrlHeldOnFirstClick && !( modifiers & Qt::ControlModifier ) )
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
    emit messageEmitted( QStringLiteral( "Could not apply offset" ), Qgis::Critical );
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

    offset = std::sqrt( mManipulatedGeometry.closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
    if ( QgsWkbTypes::geometryType( mManipulatedGeometry.wkbType() ) == QgsWkbTypes::LineGeometry )
    {
      offset = leftOf < 0 ? offset : -offset;
    }
    else
    {
      offset = mManipulatedGeometry.contains( &layerCoords ) ? -offset : offset;
    }
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

void QgsMapToolOffsetCurve::prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  QgsVectorLayer *vl = match.layer();
  if ( !vl )
  {
    return;
  }

  mOriginalGeometry = QgsGeometry();
  mManipulatedGeometry = QgsGeometry();
  mModifiedPart = -1;
  mModifiedRing = -1;

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
    if ( !match.hasEdge() )
    {
      return;
    }
    if ( !geom.isMultipart() )
    {
      mManipulatedGeometry = geom;
    }
    else
    {
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
    if ( !match.hasEdge() && match.hasArea() )
    {
      if ( !geom.isMultipart() )
      {
        mManipulatedGeometry = geom;
      }
      else
      {
        // get the correct part
        QgsMultiPolygonXY mpolygon = geom.asMultiPolygon();
        for ( int part = 0; part < mpolygon.count(); part++ ) // go through the polygons
        {
          const QgsPolygonXY &polygon = mpolygon[part];
          QgsGeometry partGeo = QgsGeometry::fromPolygonXY( polygon );
          const QgsPointXY layerCoords = match.point();
          if ( partGeo.contains( &layerCoords ) )
          {
            mModifiedPart = part;
            mManipulatedGeometry = partGeo;
          }
        }
      }
    }
    else if ( match.hasEdge() )
    {
      int vertex = match.vertexIndex();
      QgsVertexId vertexId;
      geom.vertexIdFromVertexNr( vertex, vertexId );
      QgsDebugMsg( QStringLiteral( "%1" ).arg( vertexId.ring ) );

      if ( !geom.isMultipart() )
      {
        QgsPolygonXY poly = geom.asPolygon();
        // if has rings
        if ( poly.count() > 0 )
        {
          mModifiedRing = vertexId.ring;
          mManipulatedGeometry = QgsGeometry::fromPolygonXY( QgsPolygonXY() << poly.at( mModifiedRing ) );
        }
        else
        {
          mManipulatedGeometry = QgsGeometry::fromPolygonXY( poly );
        }

      }
      else
      {
        mModifiedPart = vertexId.part;
        // get part, get ring
        QgsMultiPolygonXY multiPoly = geom.asMultiPolygon();
        // if has rings
        if ( multiPoly.at( mModifiedPart ).count() > 0 )
        {
          mModifiedRing = vertexId.ring;
          mManipulatedGeometry = QgsGeometry::fromPolygonXY( QgsPolygonXY() << multiPoly.at( mModifiedPart ).at( mModifiedRing ) );
        }
        else
        {
          mManipulatedGeometry = QgsGeometry::fromPolygonXY( multiPoly.at( mModifiedPart ) );
        }
      }
    }
  }
}

void QgsMapToolOffsetCurve::createUserInputWidget()
{
  deleteUserInputWidget();

  mUserInputWidget = new QgsOffsetUserWidget();
  mUserInputWidget->setPolygonMode( QgsWkbTypes::geometryType( mOriginalGeometry.wkbType() ) != QgsWkbTypes::LineGeometry );
  QgisApp::instance()->addUserInputWidget( mUserInputWidget );
  mUserInputWidget->setFocus( Qt::TabFocusReason );

  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingFinished, this, &QgsMapToolOffsetCurve::applyOffset );
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingCanceled, this, &QgsMapToolOffsetCurve::cancel );

  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetConfigChanged, this, [ = ] {updateGeometryAndRubberBand( mUserInputWidget->offset() );} );
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

  QgsGeometry offsetGeom;
  QgsSettings s;
  QgsGeometry::JoinStyle joinStyle = s.enumValue( QStringLiteral( "/qgis/digitizing/offset_join_style" ),  QgsGeometry::JoinStyleRound );
  int quadSegments = s.value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt();
  double miterLimit = s.value( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), 5.0 ).toDouble();
  QgsGeometry::EndCapStyle capStyle = s.enumValue( QStringLiteral( "/qgis/digitizing/offset_cap_style" ),  QgsGeometry::CapRound );


  if ( QgsWkbTypes::geometryType( mOriginalGeometry.wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    offsetGeom = mManipulatedGeometry.offsetCurve( offset, quadSegments, joinStyle, miterLimit );
  }
  else
  {
    offsetGeom = mManipulatedGeometry.buffer( offset, quadSegments, capStyle, joinStyle, miterLimit );
  }

  if ( !offsetGeom )
  {
    deleteRubberBandAndGeometry();
    deleteUserInputWidget();
    mLayer = nullptr;
    mGeometryModified = false;
    emit messageDiscarded();
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
  setupUi( this );

  mOffsetSpinBox->setDecimals( 6 );

  // fill comboboxes
  mJoinStyleComboBox->addItem( tr( "Round" ), QgsGeometry::JoinStyleRound );
  mJoinStyleComboBox->addItem( tr( "Miter" ), QgsGeometry::JoinStyleMiter );
  mJoinStyleComboBox->addItem( tr( "Bevel" ), QgsGeometry::JoinStyleBevel );
  mCapStyleComboBox->addItem( tr( "Round" ), QgsGeometry::CapRound );
  mCapStyleComboBox->addItem( tr( "Flat" ), QgsGeometry::CapFlat );
  mCapStyleComboBox->addItem( tr( "Square" ), QgsGeometry::CapSquare );

  QgsSettings s;
  QgsGeometry::JoinStyle joinStyle = s.enumValue( QStringLiteral( "/qgis/digitizing/offset_join_style" ),  QgsGeometry::JoinStyleRound );
  int quadSegments = s.value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt();
  double miterLimit = s.value( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), 5.0 ).toDouble();
  QgsGeometry::EndCapStyle capStyle = s.enumValue( QStringLiteral( "/qgis/digitizing/offset_cap_style" ),  QgsGeometry::CapRound );

  mJoinStyleComboBox->setCurrentIndex( mJoinStyleComboBox->findData( joinStyle ) );
  mQuadrantSpinBox->setValue( quadSegments );
  mMiterLimitSpinBox->setValue( miterLimit );
  mCapStyleComboBox->setCurrentIndex( mCapStyleComboBox->findData( capStyle ) );

  // connect signals
  mOffsetSpinBox->installEventFilter( this );
  connect( mOffsetSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsOffsetUserWidget::offsetChanged );

  connect( mJoinStyleComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, [ = ] { QgsSettings().setEnumValue( QStringLiteral( "/qgis/digitizing/offset_join_style" ), ( QgsGeometry::JoinStyle )mJoinStyleComboBox->currentData().toInt() ); emit offsetConfigChanged(); } );
  connect( mQuadrantSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, [ = ]( const int quadSegments ) { QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), quadSegments ); emit offsetConfigChanged(); } );
  connect( mMiterLimitSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, [ = ]( double miterLimit ) { QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), miterLimit ); emit offsetConfigChanged(); } );
  connect( mCapStyleComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, [ = ] { QgsSettings().setEnumValue( QStringLiteral( "/qgis/digitizing/offset_cap_style" ), ( QgsGeometry::EndCapStyle ) mCapStyleComboBox->currentData().toInt() ); emit offsetConfigChanged(); } );

  bool showAdvanced = s.value( QStringLiteral( "/qgis/digitizing/offset_show_advanced" ), false ).toBool();
  mShowAdvancedButton->setChecked( showAdvanced );
  mAdvancedConfigWidget->setVisible( showAdvanced );
  connect( mShowAdvancedButton, &QToolButton::clicked, mAdvancedConfigWidget, &QWidget::setVisible );
  connect( mShowAdvancedButton, &QToolButton::clicked, this, [ = ]( const bool clicked ) {QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/offset_show_advanced" ), clicked );} );

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

void QgsOffsetUserWidget::setPolygonMode( bool polygon )
{
  mCapStyleLabel->setEnabled( polygon );
  mCapStyleComboBox->setEnabled( polygon );
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
