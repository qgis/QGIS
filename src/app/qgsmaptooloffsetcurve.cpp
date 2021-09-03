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
#include "qgssettingsregistrycore.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mSnapIndicator( std::make_unique< QgsSnapIndicator >( canvas ) )
{
  mToolName = tr( "Map tool offset curve" );
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

    QgsPointLocator::Match match;

    if ( e->modifiers() & Qt::ControlModifier )
    {
      match = mCanvas->snappingUtils()->snapToMap( e->pos(), nullptr );
    }
    else
    {
      match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(),
              QgsPointLocator::Types( QgsPointLocator::Edge | QgsPointLocator::Area ) );
    }

    if ( auto *lLayer = match.layer() )
    {
      mSourceLayer = lLayer;
      QgsFeature fet;
      if ( lLayer->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
      {
        mSourceFeature = fet;
        mCtrlHeldOnFirstClick = ( e->modifiers() & Qt::ControlModifier ); //no geometry modification if ctrl is pressed
        prepareGeometry( match, fet );
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mManipulatedGeometry, lLayer );
        }
        mModifiedFeature = fet.id();
        createUserInputWidget();

        const bool hasZ = QgsWkbTypes::hasZ( mSourceLayer->wkbType() );
        const bool hasM = QgsWkbTypes::hasZ( mSourceLayer->wkbType() );
        if ( hasZ || hasM )
        {
          emit messageEmitted( QStringLiteral( "layer %1 has %2%3%4 geometry. %2%3%4 values be set to 0 when using offset tool." )
                               .arg( mSourceLayer->name(),
                                     hasZ ? QStringLiteral( "Z" ) : QString(),
                                     hasZ && hasM ? QStringLiteral( "/" ) : QString(),
                                     hasM ? QStringLiteral( "M" ) : QString() )
                               , Qgis::MessageLevel::Warning );
        }
      }
    }

    if ( mOriginalGeometry.isNull() )
    {
      emit messageEmitted( tr( "Could not find a nearby feature in any vector layer." ) );
      cancel();
    }
  }
  else
  {
    // second click - apply changes
    const double offset = calculateOffset( e->snapPoint() );
    applyOffset( offset, e->modifiers() );
  }
}

void QgsMapToolOffsetCurve::applyOffsetFromWidget( double offset, Qt::KeyboardModifiers modifiers )
{
  if ( mSourceLayer && !mOriginalGeometry.isNull() && !qgsDoubleNear( offset, 0 ) )
  {
    mGeometryModified = true;
    applyOffset( offset, modifiers );
  }
}

void QgsMapToolOffsetCurve::applyOffset( double offset, Qt::KeyboardModifiers modifiers )
{
  if ( !mSourceLayer || offset == 0.0 )
  {
    cancel();
    return;
  }

  updateGeometryAndRubberBand( offset );

  // no modification
  if ( !mGeometryModified )
  {
    cancel();
    return;
  }

  if ( mModifiedPart >= 0 )
  {
    QgsGeometry geometry;
    int partIndex = 0;
    const QgsWkbTypes::Type geomType = mOriginalGeometry.wkbType();
    if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry )
    {
      QgsMultiPolylineXY newMultiLine;
      const QgsMultiPolylineXY multiLine = mOriginalGeometry.asMultiPolyline();
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
    emit messageEmitted( tr( "Generated geometry is not valid." ), Qgis::MessageLevel::Critical );
    // no cancel, continue editing.
    return;
  }

  QgsVectorLayer *destLayer = qobject_cast< QgsVectorLayer * >( canvas()->currentLayer() );
  if ( !destLayer )
    return;

  destLayer->beginEditCommand( tr( "Offset curve" ) );

  bool editOk = true;
  if ( !mCtrlHeldOnFirstClick && !( modifiers & Qt::ControlModifier ) )
  {
    editOk = destLayer->changeGeometry( mModifiedFeature, mModifiedGeometry );
  }
  else
  {
    const QgsCoordinateTransform ct( mSourceLayer->crs(), destLayer->crs(), QgsProject::instance() );
    try
    {
      QgsGeometry g = mModifiedGeometry;
      g.transform( ct );

      QgsFeature f = mSourceFeature;
      f.setGeometry( g );

      // auto convert source feature attributes to destination attributes, make geometry compatible
      // note that this may result in multiple features, e.g. if inserting multipart feature into single-part layer
      const QgsFeatureList features = QgsVectorLayerUtils::makeFeatureCompatible( f, destLayer );
      for ( const QgsFeature &feature : features )
      {
        QgsAttributeMap attrs;
        for ( int idx = 0; idx < destLayer->fields().count(); ++idx )
        {
          if ( !feature.attribute( idx ).isNull() )
            attrs[idx] = feature.attribute( idx );
        }

        QgsExpressionContext context = destLayer->createExpressionContext();
        // use createFeature to ensure default values and provider side constraints are respected
        f = QgsVectorLayerUtils::createFeature( destLayer, feature.geometry(), attrs, &context );

        editOk = editOk && destLayer->addFeature( f );
      }
    }
    catch ( QgsCsException & )
    {
      editOk = false;
    }
  }

  if ( editOk )
  {
    destLayer->endEditCommand();
  }
  else
  {
    destLayer->destroyEditCommand();
    emit messageEmitted( QStringLiteral( "Could not apply offset" ), Qgis::MessageLevel::Critical );
  }

  deleteRubberBandAndGeometry();
  deleteUserInputWidget();
  destLayer->triggerRepaint();
  mSourceLayer = nullptr;
}

void QgsMapToolOffsetCurve::cancel()
{
  deleteUserInputWidget();
  deleteRubberBandAndGeometry();
  mSourceLayer = nullptr;
}

double QgsMapToolOffsetCurve::calculateOffset( const QgsPointXY &mapPoint )
{
  double offset = 0.0;
  if ( mSourceLayer )
  {
    //get offset from current position rectangular to feature
    const QgsPointXY layerCoords = toLayerCoordinates( mSourceLayer, mapPoint );

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
    QgsPointLocator::Match match;
    if ( e->modifiers() & Qt::ControlModifier )
    {
      match = mCanvas->snappingUtils()->snapToMap( e->pos(), nullptr );
    }
    else
    {
      match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(),
              QgsPointLocator::Types( QgsPointLocator::Edge | QgsPointLocator::Area ) );
    }
    mSnapIndicator->setMatch( match );
    return;
  }

  mGeometryModified = true;

  const QgsPointXY mapPoint = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  const double offset = calculateOffset( mapPoint );

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
  const QgsGeometry geom = snappedFeature.geometry();
  if ( geom.isNull() )
  {
    return;
  }
  mOriginalGeometry = geom;

  const QgsWkbTypes::Type geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry )
  {
    if ( !geom.isMultipart() )
    {
      mManipulatedGeometry = geom;
    }
    else
    {
      const int vertex = match.vertexIndex();
      QgsVertexId vertexId;
      geom.vertexIdFromVertexNr( vertex, vertexId );
      mModifiedPart = vertexId.part;

      const QgsMultiPolylineXY multiLine = geom.asMultiPolyline();
      mManipulatedGeometry = QgsGeometry::fromPolylineXY( multiLine.at( mModifiedPart ) );
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PolygonGeometry )
  {
    if ( !match.hasEdge() && !match.hasVertex() && match.hasArea() )
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
          const QgsGeometry partGeo = QgsGeometry::fromPolygonXY( polygon );
          const QgsPointXY layerCoords = match.point();
          if ( partGeo.contains( &layerCoords ) )
          {
            mModifiedPart = part;
            mManipulatedGeometry = partGeo;
          }
        }
      }
    }
    else if ( match.hasEdge() || match.hasVertex() )
    {
      const int vertex = match.vertexIndex();
      QgsVertexId vertexId;
      geom.vertexIdFromVertexNr( vertex, vertexId );
      QgsDebugMsgLevel( QString::number( vertexId.ring ), 2 );

      if ( !geom.isMultipart() )
      {
        const QgsPolygonXY poly = geom.asPolygon();
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
        const QgsMultiPolygonXY multiPoly = geom.asMultiPolygon();
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
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingFinished, this, &QgsMapToolOffsetCurve::applyOffsetFromWidget );
  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingCanceled, this, &QgsMapToolOffsetCurve::cancel );

  connect( mUserInputWidget, &QgsOffsetUserWidget::offsetConfigChanged, this, [ = ] {updateGeometryAndRubberBand( mUserInputWidget->offset() );} );
}

void QgsMapToolOffsetCurve::deleteUserInputWidget()
{
  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetChanged, this, &QgsMapToolOffsetCurve::updateGeometryAndRubberBand );
    disconnect( mUserInputWidget, &QgsOffsetUserWidget::offsetEditingFinished, this, &QgsMapToolOffsetCurve::applyOffsetFromWidget );
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

  if ( !mSourceLayer )
  {
    return;
  }

  QgsGeometry offsetGeom;
  const Qgis::JoinStyle joinStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle.value();
  const int quadSegments = QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg.value();
  const double miterLimit = QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit.value();
  const Qgis::EndCapStyle capStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle.value();


  if ( QgsWkbTypes::geometryType( mOriginalGeometry.wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    offsetGeom = mManipulatedGeometry.offsetCurve( offset, quadSegments, joinStyle, miterLimit );
  }
  else
  {
    offsetGeom = mManipulatedGeometry.buffer( offset, quadSegments, capStyle, joinStyle, miterLimit );
  }

  if ( offsetGeom.isNull() )
  {
    deleteRubberBandAndGeometry();
    deleteUserInputWidget();
    mSourceLayer = nullptr;
    mGeometryModified = false;
    emit messageDiscarded();
    emit messageEmitted( tr( "Creating offset geometry failed: %1" ).arg( offsetGeom.lastError() ), Qgis::MessageLevel::Critical );
  }
  else
  {
    mModifiedGeometry = offsetGeom;
    mRubberBand->setToGeometry( mModifiedGeometry, mSourceLayer );
  }
}


// ******************
// Offset User Widget

QgsOffsetUserWidget::QgsOffsetUserWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mOffsetSpinBox->setDecimals( 6 );
  mOffsetSpinBox->setClearValue( 0.0 );

  // fill comboboxes
  mJoinStyleComboBox->addItem( tr( "Round" ), static_cast< int >( Qgis::JoinStyle::Round ) );
  mJoinStyleComboBox->addItem( tr( "Miter" ), static_cast< int >( Qgis::JoinStyle::Miter ) );
  mJoinStyleComboBox->addItem( tr( "Bevel" ), static_cast< int >( Qgis::JoinStyle::Bevel ) );
  mCapStyleComboBox->addItem( tr( "Round" ), static_cast< int >( Qgis::EndCapStyle::Round ) );
  mCapStyleComboBox->addItem( tr( "Flat" ), static_cast< int >( Qgis::EndCapStyle::Flat ) );
  mCapStyleComboBox->addItem( tr( "Square" ), static_cast< int >( Qgis::EndCapStyle::Square ) );

  const Qgis::JoinStyle joinStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle.value();
  const int quadSegments = QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg.value();
  const double miterLimit = QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit.value();
  const Qgis::EndCapStyle capStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle.value();

  mJoinStyleComboBox->setCurrentIndex( mJoinStyleComboBox->findData( static_cast< int >( joinStyle ) ) );
  mQuadrantSpinBox->setValue( quadSegments );
  mQuadrantSpinBox->setClearValue( 8 );
  mMiterLimitSpinBox->setValue( miterLimit );
  mMiterLimitSpinBox->setClearValue( 5.0 );
  mCapStyleComboBox->setCurrentIndex( mCapStyleComboBox->findData( static_cast< int >( capStyle ) ) );

  // connect signals
  mOffsetSpinBox->installEventFilter( this );
  connect( mOffsetSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsOffsetUserWidget::offsetChanged );

  connect( mJoinStyleComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, [ = ] { QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle.setValue( static_cast< Qgis::JoinStyle >( mJoinStyleComboBox->currentData().toInt() ) ); emit offsetConfigChanged(); } );
  connect( mQuadrantSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, [ = ]( const int quadSegments ) { QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg.setValue( quadSegments ); emit offsetConfigChanged(); } );
  connect( mMiterLimitSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, [ = ]( double miterLimit ) { QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit.setValue( miterLimit ); emit offsetConfigChanged(); } );
  connect( mCapStyleComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, [ = ] { QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle.setValue( static_cast< Qgis::EndCapStyle >( mCapStyleComboBox->currentData().toInt() ) ); emit offsetConfigChanged(); } );

  const bool showAdvanced = QgsSettingsRegistryCore::settingsDigitizingOffsetShowAdvanced.value();
  mShowAdvancedButton->setChecked( showAdvanced );
  mAdvancedConfigWidget->setVisible( showAdvanced );
  connect( mShowAdvancedButton, &QToolButton::clicked, mAdvancedConfigWidget, &QWidget::setVisible );
  connect( mShowAdvancedButton, &QToolButton::clicked, this, [ = ]( const bool clicked ) {QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance.setValue( clicked );} );

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
