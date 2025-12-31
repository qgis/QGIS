/***************************************************************************
                              qgsmaptoolchamferfillet.cpp
    ------------------------------------------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolchamferfillet.h"

#include "qgisapp.h"
#include "qgsavoidintersectionsoperation.h"
#include "qgsdoublespinbox.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometryutils.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmultipolygon.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgssnapindicator.h"
#include "qgssnappingutils.h"
#include "qgsvector.h"
#include "qgsvectorlayer.h"

#include <QDateTime>
#include <QGraphicsProxyWidget>
#include <QGridLayout>
#include <QLabel>

#include "moc_qgsmaptoolchamferfillet.cpp"

const QgsSettingsEntryEnumFlag<QgsGeometry::ChamferFilletOperationType> *QgsMapToolChamferFillet::settingsOperation = new QgsSettingsEntryEnumFlag<QgsGeometry::ChamferFilletOperationType>( u"chamferfillet-operation"_s, QgsSettingsTree::sTreeDigitizing, QgsGeometry::ChamferFilletOperationType::Chamfer );
const QgsSettingsEntryInteger *QgsMapToolChamferFillet::settingsFilletSegment = new QgsSettingsEntryInteger( u"chamferfillet-fillet-segment"_s, QgsSettingsTree::sTreeDigitizing, 8, u"For fillet operation, number of segment used to create the arc."_s, Qgis::SettingsOption(), 1, 64 );
const QgsSettingsEntryDouble *QgsMapToolChamferFillet::settingsValue1 = new QgsSettingsEntryDouble( u"chamferfillet-fillet-value1"_s, QgsSettingsTree::sTreeDigitizing, 0.0, u"For fillet/chamfer operations, radius or distance1."_s );
const QgsSettingsEntryDouble *QgsMapToolChamferFillet::settingsValue2 = new QgsSettingsEntryDouble( u"chamferfillet-fillet-value2"_s, QgsSettingsTree::sTreeDigitizing, 0.0, u"For chamfer operation, distance2."_s );
const QgsSettingsEntryBool *QgsMapToolChamferFillet::settingsLock1 = new QgsSettingsEntryBool( u"chamferfillet-fillet-lock1"_s, QgsSettingsTree::sTreeDigitizing, false, u"For fillet/chamfer operations, locks distance1."_s );
const QgsSettingsEntryBool *QgsMapToolChamferFillet::settingsLock2 = new QgsSettingsEntryBool( u"chamferfillet-fillet-lock2"_s, QgsSettingsTree::sTreeDigitizing, false, u"For fillet/chamfer operations, locks distance2."_s );

QgsMapToolChamferFillet::QgsMapToolChamferFillet( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mSnapIndicator( std::make_unique<QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Chamfer or fillet" );
}

QgsMapToolChamferFillet::~QgsMapToolChamferFillet()
{
  cancel();
}


void QgsMapToolChamferFillet::applyOperationFromWidget( Qt::KeyboardModifiers modifiers )
{
  if ( mSourceLayer && !mOriginalGeometryInSourceLayerCrs.isNull() )
  {
    computeMaxValues();
    double value1 = mUserInputWidget->value1();
    double value2 = mUserInputWidget->value2();
    if ( QgsMapToolChamferFillet::settingsOperation->value() == QgsGeometry::ChamferFilletOperationType::Fillet )
      value2 = value1;

    handleModifier( modifiers & Qt::ShiftModifier, value1, value2 );
    handleMaxAndLock( value1, value2 );
    applyOperation( value1, value2 );
  }
}

void QgsMapToolChamferFillet::applyOperation( double value1, double value2 )
{
  if ( !mSourceLayer || qgsDoubleNear( value1, 0.0 ) || qgsDoubleNear( value2, 0.0 ) )
  {
    cancel();
    return;
  }

  updateGeometryAndRubberBand( value1, value2 );

  // no modification
  if ( !mGeometryModified )
  {
    cancel();
    return;
  }

  if ( !mModifiedGeometry.isGeosValid() )
  {
    QString message;
    QString lastError;
    int i = 0;
    for ( QgsAbstractGeometry::const_part_iterator ite = mModifiedGeometry.const_parts_begin(); ite != mModifiedGeometry.const_parts_end(); ite++ )
    {
      if ( !( *ite )->isValid( lastError ) )
      {
        message += u"Invalid part %1: '%2'"_s.arg( i ).arg( lastError );
        break;
      }
      i++;
    }

    emit messageEmitted( tr( "Generated geometry is not valid: '%1'. " ).arg( mModifiedGeometry.lastError() ), Qgis::MessageLevel::Warning );
    QgsLogger::warning( tr( "Generated geometry is not valid: '%1'. " ).arg( mModifiedGeometry.lastError() ) + message );
    // no cancel, continue editing.
    return;
  }

  QgsVectorLayer *destLayer = currentVectorLayer();
  if ( !destLayer )
    return;

  destLayer->beginEditCommand( tr( "Chamfer/fillet" ) );

  bool editOk = true;
  editOk = destLayer->changeGeometry( mModifiedFeature, mModifiedGeometry );

  if ( editOk )
  {
    destLayer->endEditCommand();
  }
  else
  {
    destLayer->destroyEditCommand();
    emit messageEmitted( u"Could not apply chamfer/fillet"_s, Qgis::MessageLevel::Warning );
  }

  cancel();
  destLayer->triggerRepaint();
}

void QgsMapToolChamferFillet::cancel()
{
  deleteUserInputWidget();
  deleteRubberBandAndGeometry();
  mSourceLayer = nullptr;
}

void QgsMapToolChamferFillet::handleModifier( bool isShiftKeyPressed, double &value1, double &value2 )
{
  QgsGeometry::ChamferFilletOperationType op = QgsMapToolChamferFillet::settingsOperation->value();
  if ( op == QgsGeometry::ChamferFilletOperationType::Chamfer )
  {
    if ( isShiftKeyPressed )
    {
      value1 = ( value1 + value2 ) / 2.0;
      value2 = value1;
    }
  }
  else
  {
    value1 = ( value1 + value2 ) / 2.0;
    value2 = -1;
  }
}

void QgsMapToolChamferFillet::handleMaxAndLock( double &value1, double &value2 )
{
  bool locked = QgsMapToolChamferFillet::settingsLock1->value();
  if ( locked )
    value1 = QgsMapToolChamferFillet::settingsValue1->value();

  locked = QgsMapToolChamferFillet::settingsLock2->value();
  if ( locked )
    value2 = QgsMapToolChamferFillet::settingsValue2->value();

  value1 = std::min( value1, mMaxValue1 );
  value2 = std::min( value2, mMaxValue2 );
}

void QgsMapToolChamferFillet::computeValuesFromMousePos( const QgsPointXY &mapPoint, bool isShiftKeyPressed, double &value1, double &value2 )
{
  calculateDistances( mapPoint, value1, value2 );
  handleModifier( isShiftKeyPressed, value1, value2 );
  handleMaxAndLock( value1, value2 );

  if ( !mUserInputWidget )
    return;

  mUserInputWidget->blockSignals( true );
  QgsGeometry::ChamferFilletOperationType op = QgsMapToolChamferFillet::settingsOperation->value();
  if ( op == QgsGeometry::ChamferFilletOperationType::Chamfer )
  {
    bool locked = QgsMapToolChamferFillet::settingsLock1->value();
    if ( !locked )
      mUserInputWidget->setValue1( value1 );
    locked = QgsMapToolChamferFillet::settingsLock2->value();
    if ( !locked )
      mUserInputWidget->setValue2( value2 );
  }
  else
  {
    bool locked = QgsMapToolChamferFillet::settingsLock1->value();
    if ( !locked )
      mUserInputWidget->setValue1( value1 );
  }
  mUserInputWidget->blockSignals( false );
  mUserInputWidget->setFocus( Qt::TabFocusReason );
  mUserInputWidget->editor()->selectAll();
}

void QgsMapToolChamferFillet::calculateDistances( const QgsPointXY &mapPoint, double &value1, double &value2 )
{
  value1 = 0.0;
  value2 = 0.0;
  if ( mSourceLayer )
  {
    //get distance from current position rectangular to feature
    const QgsPointXY layerCoords = toLayerCoordinates( mSourceLayer, mapPoint );

    const QgsVector vect = layerCoords - mVertexPointInSourceLayerCrs;
    const QgsVector perpVect = vect.perpVector();

    int beforeVIdx, afterVIdx;
    mManipulatedGeometryInSourceLayerCrs.adjacentVertices( mVertexIndex, beforeVIdx, afterVIdx );
    const QgsPoint beforeVert = mManipulatedGeometryInSourceLayerCrs.vertexAt( beforeVIdx );
    const QgsPoint afterVert = mManipulatedGeometryInSourceLayerCrs.vertexAt( afterVIdx );

    QgsPoint beforeInter;
    QgsGeometryUtils::lineIntersection( QgsPoint( layerCoords.x(), layerCoords.y() ), perpVect, beforeVert, beforeVert - mVertexPointInSourceLayerCrs, beforeInter );

    QgsPoint afterInter;
    QgsGeometryUtils::lineIntersection( QgsPoint( layerCoords.x(), layerCoords.y() ), perpVect, afterVert, afterVert - mVertexPointInSourceLayerCrs, afterInter );

    value1 = beforeInter.distance( mVertexPointInSourceLayerCrs );
    value2 = afterInter.distance( mVertexPointInSourceLayerCrs );
  }
}

void QgsMapToolChamferFillet::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolChamferFillet::computeMaxValues()
{
  if ( mVertexIndex < 0 )
    return;

  // Get the segments around the vertex
  QgsPoint vertexBefore = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex - 1 );
  const QgsPoint vertex = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex );
  QgsPoint vertexAfter = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex + 1 );

  // find vertex before first or after last point.
  if ( vertexBefore.isEmpty() || vertexAfter.isEmpty() )
  {
    const QgsAbstractGeometry *absPoly;
    if ( const QgsPolygon *poly = qgsgeometry_cast<const QgsPolygon *>( mManipulatedGeometryInSourceLayerCrs.get() ) )
      absPoly = poly;
    else if ( const QgsMultiPolygon *multiPoly = qgsgeometry_cast<const QgsMultiPolygon *>( mManipulatedGeometryInSourceLayerCrs.get() ) )
      absPoly = multiPoly;
    else
      absPoly = nullptr;

    if ( absPoly != nullptr )
    {
      QgsVertexId vId;
      mManipulatedGeometryInSourceLayerCrs.vertexIdFromVertexNr( mVertexIndex, vId );
      if ( vertexBefore.isEmpty() )
      {
        vId.vertex = absPoly->vertexCount( vId.part, vId.ring ) - 2;
        vertexBefore = absPoly->vertexAt( vId );
      }
      else if ( vertexAfter.isEmpty() )
      {
        vId.vertex = 1;
        vertexAfter = absPoly->vertexAt( vId );
      }
    }
  }

  if ( !vertexBefore.isEmpty() && !vertex.isEmpty() && !vertexAfter.isEmpty() )
  {
    QgsGeometry::ChamferFilletOperationType op = QgsMapToolChamferFillet::settingsOperation->value();
    if ( op == QgsGeometry::ChamferFilletOperationType::Fillet )
    {
      const double maxRadius = QgsGeometryUtils::maxFilletRadius( vertexBefore, vertex, vertex, vertexAfter );
      if ( maxRadius > 0 )
      {
        mMaxValue1 = maxRadius;
      }
      mMaxValue2 = 64;
    }
    else
    {
      double dist1 = vertex.distance( vertexBefore );
      double dist2 = vertex.distance( vertexAfter );
      if ( dist1 > 0 )
      {
        mMaxValue1 = dist1;
      }
      if ( dist2 > 0 )
      {
        mMaxValue2 = dist2;
      }
    }

    if ( mUserInputWidget )
    {
      mUserInputWidget->blockSignals( true );
      bool locked = QgsMapToolChamferFillet::settingsLock1->value();
      if ( !locked )
        mUserInputWidget->setMaximumValue1( mMaxValue1 );
      locked = QgsMapToolChamferFillet::settingsLock2->value();
      if ( !locked )
        mUserInputWidget->setMaximumValue2( mMaxValue2 );
      mUserInputWidget->blockSignals( false );
    }
  }
}

void QgsMapToolChamferFillet::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    cancel();
    return;
  }

  if ( mOriginalGeometryInSourceLayerCrs.isNull() )
  {
    // first click, get feature to modify
    deleteRubberBandAndGeometry();
    mGeometryModified = false;

    QgsPointLocator::Match match;

    match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Types( QgsPointLocator::Vertex ) );

    if ( QgsVectorLayer *lLayer = match.layer() )
    {
      mSourceLayer = lLayer;
      QgsFeature fet;
      if ( lLayer->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
      {
        if ( !prepareGeometry( match, fet ) )
        {
          cancel();
          return;
        }
        mSourceFeature = fet;
        mRubberBand = createRubberBand();
        if ( mRubberBand )
        {
          mRubberBand->setToGeometry( mManipulatedGeometryInSourceLayerCrs, lLayer );
        }
        mModifiedFeature = fet.id();
        createUserInputWidget();

        // Set maximum values based on geometry
        computeMaxValues();

        if ( mSourceLayer != nullptr )
        {
          const bool hasZ = QgsWkbTypes::hasZ( mSourceLayer->wkbType() );
          const bool hasM = QgsWkbTypes::hasM( mSourceLayer->wkbType() );
          if ( hasZ || hasM )
          {
            emit messageEmitted( u"Layer %1 has %2%3%4 geometry. %2%3%4 values be set to 0 when using chamfer/fillet tool."_s.arg( mSourceLayer->name(), hasZ ? u"Z"_s : QString(), hasZ && hasM ? u"/"_s : QString(), hasM ? u"M"_s : QString() ), Qgis::MessageLevel::Warning );
          }
        }
      }
    }

    if ( mOriginalGeometryInSourceLayerCrs.isNull() )
    {
      cancel();
    }

    // force rubberband update
    canvasMoveEvent( e );
  }
  else
  {
    // second click - apply changes
    computeMaxValues();
    double value1, value2;
    computeValuesFromMousePos( e->mapPoint(), e->modifiers() & Qt::ShiftModifier, value1, value2 );
    applyOperation( value1, value2 );
  }
}

void QgsMapToolChamferFillet::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mOriginalGeometryInSourceLayerCrs.isNull() || !mRubberBand )
  {
    QgsPointLocator::Match match;
    match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Types( QgsPointLocator::Vertex ) );
    mSnapIndicator->setMatch( match );
    return;
  }

  // reduce the number of call to max 1 per 100ms
  if ( mLastMouseMove.isValid() && mLastMouseMove.elapsed() < 100 )
    return;
  mLastMouseMove.restart();

  mGeometryModified = true;

  const QgsPointXY mapPoint = e->mapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  computeMaxValues();
  double value1, value2;
  computeValuesFromMousePos( mapPoint, e->modifiers() & Qt::ShiftModifier, value1, value2 );

  //create chamfer geometry using geos
  updateGeometryAndRubberBand( value1, value2 );
}

bool QgsMapToolChamferFillet::prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  if ( !match.layer() )
  {
    return false;
  }

  mOriginalGeometryInSourceLayerCrs = QgsGeometry();
  mManipulatedGeometryInSourceLayerCrs = QgsGeometry();

  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  const QgsGeometry geom = snappedFeature.geometry();
  if ( geom.isNull() )
  {
    return false;
  }

  if ( !geom.isGeosValid() )
  {
    emit messageEmitted( tr( "Chamfer/fillet: input geometry is invalid!" ), Qgis::MessageLevel::Warning );
    return false;
  }

  const Qgis::WkbType geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Line && QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Polygon )
    return false;

  if ( !match.hasEdge() && !match.hasVertex() )
    return false;

  // maptool will not work with first or last point on a linestring
  if ( QgsWkbTypes::geometryType( geomType ) == Qgis::GeometryType::Line && ( match.vertexIndex() == 0 || match.vertexIndex() == geom.constGet()->vertexCount( 0 ) - 1 ) )
    return false;

  mOriginalGeometryInSourceLayerCrs = geom;
  mManipulatedGeometryInSourceLayerCrs = geom;

  mVertexIndex = match.vertexIndex();
  mVertexPointInSourceLayerCrs = geom.vertexAt( mVertexIndex );

  return true;
}

void QgsMapToolChamferFillet::createUserInputWidget()
{
  deleteUserInputWidget();

  mUserInputWidget = new QgsChamferFilletUserWidget();
  QgisApp::instance()->addUserInputWidget( mUserInputWidget );
  mUserInputWidget->setFocus( Qt::TabFocusReason );

  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingFinished, this, &QgsMapToolChamferFillet::applyOperationFromWidget );
  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingCanceled, this, &QgsMapToolChamferFillet::cancel );
  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceConfigChanged, this, &QgsMapToolChamferFillet::configChanged );
}

void QgsMapToolChamferFillet::deleteUserInputWidget()
{
  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingFinished, this, &QgsMapToolChamferFillet::applyOperationFromWidget );
    disconnect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingCanceled, this, &QgsMapToolChamferFillet::cancel );
    disconnect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceConfigChanged, this, &QgsMapToolChamferFillet::configChanged );
    mUserInputWidget->releaseKeyboard();
    mUserInputWidget->deleteLater();
  }
  mUserInputWidget = nullptr;
}

void QgsMapToolChamferFillet::deleteRubberBandAndGeometry()
{
  mOriginalGeometryInSourceLayerCrs.set( nullptr );
  mManipulatedGeometryInSourceLayerCrs.set( nullptr );
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolChamferFillet::configChanged()
{
  if ( mUserInputWidget )
  {
    computeMaxValues();
    double value1 = mUserInputWidget->value1();
    double value2 = mUserInputWidget->value2();
    if ( QgsMapToolChamferFillet::settingsOperation->value() == QgsGeometry::ChamferFilletOperationType::Fillet )
      value2 = value1;

    handleModifier( false, value1, value2 );
    handleMaxAndLock( value1, value2 );
    updateGeometryAndRubberBand( value1, value2 );
  }
}

void QgsMapToolChamferFillet::updateGeometryAndRubberBand( double value1, double value2 )
{
  if ( !mRubberBand || mOriginalGeometryInSourceLayerCrs.isNull() || !mSourceLayer )
  {
    return;
  }

  QgsGeometry newGeom;
  const QgsGeometry::ChamferFilletOperationType op = QgsMapToolChamferFillet::settingsOperation->value();
  const int segments = QgsMapToolChamferFillet::settingsFilletSegment->value();

  if ( op == QgsGeometry::ChamferFilletOperationType::Chamfer )
  {
    QgsDebugMsgLevel( u"will chamfer %1 / %2"_s.arg( value1 ).arg( value2 ), 3 );
    newGeom = mManipulatedGeometryInSourceLayerCrs.chamfer( mVertexIndex, value1, value2 );
  }
  else
  {
    QgsDebugMsgLevel( u"will fillet %1 / %2"_s.arg( value1 ).arg( segments ), 3 );
    newGeom = mManipulatedGeometryInSourceLayerCrs.fillet( mVertexIndex, value1, segments );
  }

  if ( newGeom.isNull() )
  {
    cancel();
    mGeometryModified = false;
    emit messageDiscarded();
    emit messageEmitted( tr( "Creating chamfer/fillet geometry failed: %1" ).arg( mManipulatedGeometryInSourceLayerCrs.lastError() ), Qgis::MessageLevel::Warning );
  }
  else
  {
    mGeometryModified = true;
    mModifiedGeometry = newGeom;
    mRubberBand->setToGeometry( mModifiedGeometry, mSourceLayer );
  }
}


// ******************
// Offset User Widget

QgsChamferFilletUserWidget::QgsChamferFilletUserWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  // fill comboboxes
  mOperationComboBox->addItem( tr( "Chamfer" ), QVariant::fromValue( QgsGeometry::ChamferFilletOperationType::Chamfer ) );
  mOperationComboBox->addItem( tr( "Fillet" ), QVariant::fromValue( QgsGeometry::ChamferFilletOperationType::Fillet ) );

  QgsGeometry::ChamferFilletOperationType op = QgsMapToolChamferFillet::settingsOperation->value();
  mOperationComboBox->setCurrentIndex( mOperationComboBox->findData( QVariant::fromValue( op ) ) );

  auto updateLabels = [this]( const QgsGeometry::ChamferFilletOperationType &op ) {
    mValue1SpinBox->blockSignals( true );
    mValue2SpinBox->blockSignals( true );

    if ( op == QgsGeometry::ChamferFilletOperationType::Chamfer )
    {
      mVal1Label->setText( tr( "Distance 1" ) );
      mValue1SpinBox->setDecimals( 3 );
      mValue1SpinBox->setMinimum( 0.01 );
      if ( QgsMapToolChamferFillet::settingsValue1->value() < mValue1SpinBox->minimum() )
        QgsMapToolChamferFillet::settingsValue1->setValue( mValue1SpinBox->minimum() );
      mValue1SpinBox->setValue( QgsMapToolChamferFillet::settingsValue1->value() );

      mVal2Label->setText( tr( "Distance 2" ) );
      mValue2SpinBox->setDecimals( 3 );
      mValue2SpinBox->setMinimum( 0.01 );
      if ( QgsMapToolChamferFillet::settingsValue2->value() < mValue2SpinBox->minimum() )
        QgsMapToolChamferFillet::settingsValue2->setValue( mValue2SpinBox->minimum() );
      mValue2SpinBox->setValue( QgsMapToolChamferFillet::settingsValue2->value() );

      mVal2Locker->setEnabled( true );
    }
    else
    {
      mVal1Label->setText( tr( "Radius" ) );
      mValue1SpinBox->setDecimals( 3 );
      mValue1SpinBox->setMinimum( 0.01 );
      if ( QgsMapToolChamferFillet::settingsValue1->value() < mValue1SpinBox->minimum() )
        QgsMapToolChamferFillet::settingsValue1->setValue( mValue1SpinBox->minimum() );
      mValue1SpinBox->setValue( QgsMapToolChamferFillet::settingsValue1->value() );

      mVal2Label->setText( tr( "Fillet segments" ) );
      mValue2SpinBox->setDecimals( 0 );
      mValue2SpinBox->setMinimum( 1.0 );
      if ( QgsMapToolChamferFillet::settingsFilletSegment->value() < mValue2SpinBox->minimum() )
        QgsMapToolChamferFillet::settingsFilletSegment->setValue( mValue2SpinBox->minimum() );
      mValue2SpinBox->setValue( QgsMapToolChamferFillet::settingsFilletSegment->value() );

      mVal2Locker->setEnabled( false );
    }

    bool checked = QgsMapToolChamferFillet::settingsLock1->value();
    mVal1Locker->setChecked( checked );

    checked = QgsMapToolChamferFillet::settingsLock2->value();
    mVal2Locker->setChecked( checked );

    mValue1SpinBox->blockSignals( false );
    mValue2SpinBox->blockSignals( false );
  };

  updateLabels( op );

  connect( mOperationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [this, updateLabels] {
    QgsGeometry::ChamferFilletOperationType op = operation();
    QgsMapToolChamferFillet::settingsOperation->setValue( op );
    updateLabels( op );
    emit distanceConfigChanged();
  } );

  connect( mValue1SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    QgsMapToolChamferFillet::settingsValue1->setValue( value );
    emit distanceConfigChanged();
  } );

  connect( mValue2SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    if ( operation() == QgsGeometry::ChamferFilletOperationType::Fillet )
      QgsMapToolChamferFillet::settingsFilletSegment->setValue( static_cast<int>( value ) );
    else
      QgsMapToolChamferFillet::settingsValue2->setValue( value );
    emit distanceConfigChanged();
  } );

  connect( mVal1Locker, &QPushButton::clicked, this, [this]( bool checked ) {
    QgsMapToolChamferFillet::settingsLock1->setValue( checked );
    emit distanceConfigChanged();
  } );

  connect( mVal2Locker, &QPushButton::clicked, this, [this]( bool checked ) {
    QgsMapToolChamferFillet::settingsLock2->setValue( checked );
    emit distanceConfigChanged();
  } );

  mValue1SpinBox->installEventFilter( this );
  mValue2SpinBox->installEventFilter( this );

  // config focus
  setFocusProxy( mValue1SpinBox );
}

void QgsChamferFilletUserWidget::setValue1( double value )
{
  mValue1SpinBox->setValue( value );
}

void QgsChamferFilletUserWidget::setValue2( double value )
{
  mValue2SpinBox->setValue( value );
}

void QgsChamferFilletUserWidget::setMaximumValue1( double maximum )
{
  mValue1SpinBox->setMaximum( maximum );
}

void QgsChamferFilletUserWidget::setMaximumValue2( double maximum )
{
  mValue2SpinBox->setMaximum( maximum );
}

double QgsChamferFilletUserWidget::value1() const
{
  return mValue1SpinBox->value();
}

double QgsChamferFilletUserWidget::value2() const
{
  return mValue2SpinBox->value();
}

QgsGeometry::ChamferFilletOperationType QgsChamferFilletUserWidget::operation() const
{
  return mOperationComboBox->currentData().value<QgsGeometry::ChamferFilletOperationType>();
}

bool QgsChamferFilletUserWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( ( obj == mValue1SpinBox || obj == mValue2SpinBox ) && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit distanceEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit distanceEditingFinished( event->modifiers() );
      return true;
    }
  }

  return false;
}
