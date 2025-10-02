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

#include <QGraphicsProxyWidget>
#include <QGridLayout>
#include <QLabel>
#include <QDateTime>

#include "qgsavoidintersectionsoperation.h"
#include "qgsdoublespinbox.h"
#include "qgsfeatureiterator.h"
#include "qgsmaptoolchamferfillet.h"
#include "moc_qgsmaptoolchamferfillet.cpp"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgssnapindicator.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingsentryimpl.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"
#include "qgslogger.h"
#include "qgsgeometryutils.h"
#include "qgsvector.h"

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
    double value1 = mUserInputWidget->value1();
    double value2 = mUserInputWidget->value2();
    if ( !qgsDoubleNear( value1, 0 ) && !qgsDoubleNear( value2, 0 ) )
    {
      mGeometryModified = true;
      applyOperation( value1, value2, modifiers );
    }
  }
}

void QgsMapToolChamferFillet::applyOperation( double value1, double value2, Qt::KeyboardModifiers )
{
  if ( !mSourceLayer || value1 == 0.0 || value2 == 0.0 )
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
        message += QStringLiteral( "Invalid part %1: '%2'" ).arg( i ).arg( lastError );
        break;
      }
      i++;
    }

    emit messageEmitted( tr( "Generated geometry is not valid: '%1'. " ).arg( mModifiedGeometry.lastError() ) + message, Qgis::MessageLevel::Critical );
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
    emit messageEmitted( QStringLiteral( "Could not apply chamfer/fillet" ), Qgis::MessageLevel::Critical );
  }

  deleteRubberBandAndGeometry();
  deleteUserInputWidget();
  destLayer->triggerRepaint();
  mSourceLayer = nullptr;
}

void QgsMapToolChamferFillet::cancel()
{
  deleteUserInputWidget();
  deleteRubberBandAndGeometry();
  mSourceLayer = nullptr;
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

        // Set maximum fillet radius based on geometry
        if ( mUserInputWidget && mUserInputWidget->operation() == QgsGeometry::Fillet && mVertexIndex >= 0 )
        {
          // Get the segments around the vertex
          const QgsPoint vertexBefore = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex - 1 );
          const QgsPoint vertex = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex );
          const QgsPoint vertexAfter = mManipulatedGeometryInSourceLayerCrs.vertexAt( mVertexIndex + 1 );

          if ( !vertexBefore.isEmpty() && !vertex.isEmpty() && !vertexAfter.isEmpty() )
          {
            const double maxRadius = QgsGeometryUtils::maxFilletRadius( vertexBefore, vertex, vertex, vertexAfter );
            if ( maxRadius > 0 )
            {
              mUserInputWidget->setMaximumValue1( maxRadius );
            }
          }
        }

        const bool hasZ = QgsWkbTypes::hasZ( mSourceLayer->wkbType() );
        const bool hasM = QgsWkbTypes::hasZ( mSourceLayer->wkbType() );
        if ( hasZ || hasM )
        {
          emit messageEmitted( QStringLiteral( "layer %1 has %2%3%4 geometry. %2%3%4 values be set to 0 when using chamfer/fillet tool." ).arg( mSourceLayer->name(), hasZ ? QStringLiteral( "Z" ) : QString(), hasZ && hasM ? QStringLiteral( "/" ) : QString(), hasM ? QStringLiteral( "M" ) : QString() ), Qgis::MessageLevel::Warning );
        }
      }
    }

    if ( mOriginalGeometryInSourceLayerCrs.isNull() )
    {
      emit messageEmitted( tr( "Could not find a nearby feature in any vector layer." ) );
      cancel();
    }

    // force rubberband update
    canvasMoveEvent( e );
  }
  else
  {
    // second click - apply changes
    double value1, value2;
    calculateDistances( e->mapPoint(), value1, value2 );
    QgsGeometry::ChamferFilletOperationType op = qgsEnumKeyToValue( QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value(), QgsGeometry::Chamfer );
    if ( op == QgsGeometry::Chamfer )
    {
      if ( e->modifiers() & Qt::ShiftModifier )
      {
        value1 = ( value1 + value2 ) / 2.0;
        value2 = value1;
      }
    }
    else
    {
      value1 = ( value1 + value2 ) / 2.0;
    }

    applyOperation( value1, value2, e->modifiers() );
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

  double value1, value2;
  calculateDistances( mapPoint, value1, value2 );

  QgsGeometry::ChamferFilletOperationType op = qgsEnumKeyToValue( QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value(), QgsGeometry::Chamfer );
  if ( op == QgsGeometry::Chamfer )
  {
    if ( e->modifiers() & Qt::ShiftModifier )
    {
      value1 = ( value1 + value2 ) / 2.0;
      value2 = value1;
    }
  }
  else
  {
    value1 = ( value1 + value2 ) / 2.0;
  }

  bool locked = QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock1->value();
  if ( locked )
    value1 = QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue1->value();

  locked = QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock2->value();
  if ( locked )
    value2 = QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue2->value();

  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    if ( op == QgsGeometry::Chamfer )
    {
      mUserInputWidget->setValue1( value1 );
      mUserInputWidget->setValue2( value2 );
    }
    else
    {
      mUserInputWidget->setValue1( value1 );
    }
    mUserInputWidget->blockSignals( false );
    mUserInputWidget->setFocus( Qt::TabFocusReason );
    mUserInputWidget->editor()->selectAll();
  }

  //create chamfer geometry using geos
  updateGeometryAndRubberBand( value1, value2 );
}

bool QgsMapToolChamferFillet::prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  const QgsVectorLayer *vl = match.layer();
  if ( !vl )
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
    emit messageEmitted( tr( "Chamfer/fillet: input geometry is invalid!" ), Qgis::MessageLevel::Critical );
    return false;
  }

  const Qgis::WkbType geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Line && QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Polygon )
    return false;

  if ( !match.hasEdge() && !match.hasVertex() )
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
    updateGeometryAndRubberBand( mUserInputWidget->value1(), mUserInputWidget->value2() );
}

void QgsMapToolChamferFillet::updateGeometryAndRubberBand( double value1, double value2 )
{
  if ( !mRubberBand || mOriginalGeometryInSourceLayerCrs.isNull() )
  {
    return;
  }

  if ( !mSourceLayer )
  {
    return;
  }

  QgsGeometry newGeom;
  const QgsGeometry::ChamferFilletOperationType op = qgsEnumKeyToValue( QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value(), QgsGeometry::Chamfer );
  const int segments = QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->value();

  if ( op == QgsGeometry::Chamfer )
  {
    QgsDebugMsgLevel( QStringLiteral( "will chamfer %1 / %2" ).arg( value1 ).arg( value2 ), 3 );
    newGeom = mManipulatedGeometryInSourceLayerCrs.chamfer( mVertexIndex, value1, value2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "will fillet %1 / %2" ).arg( value1 ).arg( segments ), 3 );
    newGeom = mManipulatedGeometryInSourceLayerCrs.fillet( mVertexIndex, value1, segments );
  }

  if ( newGeom.isNull() )
  {
    deleteRubberBandAndGeometry();
    deleteUserInputWidget();
    mSourceLayer = nullptr;
    mGeometryModified = false;
    emit messageDiscarded();
    emit messageEmitted( tr( "Creating chamfer/fillet geometry failed: %1" ).arg( mManipulatedGeometryInSourceLayerCrs.lastError() ), Qgis::MessageLevel::Critical );
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
  mOperationComboBox->addItem( tr( "Chamfer" ), QgsGeometry::Chamfer );
  mOperationComboBox->addItem( tr( "Fillet" ), QgsGeometry::Fillet );

  bool ok;
  QgsGeometry::ChamferFilletOperationType op = qgsEnumKeyToValue( QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value(), QgsGeometry::Chamfer, true, &ok );
  if ( !ok )
  {
    op = QgsGeometry::Chamfer;
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->setValue( qgsEnumValueToKey( op ) );
  }

  mOperationComboBox->setCurrentIndex( mOperationComboBox->findData( op ) );

  auto updateLabels = [this]( const QgsGeometry::ChamferFilletOperationType &op ) {
    if ( op == QgsGeometry::Chamfer )
    {
      mVal1Label->setText( tr( "Distance 1" ) );
      mValue1SpinBox->setDecimals( 6 );
      mValue1SpinBox->setClearValue( 0.001 );
      const double value1 = QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue1->value();
      mValue1SpinBox->setValue( value1 );

      mVal2Label->setText( tr( "Distance 2" ) );
      mValue2SpinBox->setDecimals( 6 );
      mValue2SpinBox->setClearValue( 0.001 );
      const double value2 = QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue2->value();
      mValue2SpinBox->setValue( value2 );

      mVal2Locker->setEnabled( true );
    }
    else
    {
      mVal1Label->setText( tr( "Radius" ) );
      mValue1SpinBox->setDecimals( 6 );
      mValue1SpinBox->setClearValue( 0.001 );
      const double value1 = QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue1->value();
      mValue1SpinBox->setValue( value1 );

      mVal2Label->setText( tr( "Fillet segments" ) );
      mValue2SpinBox->setDecimals( 0 );
      mValue2SpinBox->setClearValue( 6.0 );
      const int segments = QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->value();
      mValue2SpinBox->setValue( segments );

      mVal2Locker->setEnabled( false );
    }

    bool checked = QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock1->value();
    mVal1Locker->setChecked( checked );

    checked = QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock2->value();
    mVal2Locker->setChecked( checked );
  };

  updateLabels( op );

  connect( mOperationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [this, updateLabels] {
    QgsGeometry::ChamferFilletOperationType op = operation();
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->setValue( qgsEnumValueToKey( op ) );
    updateLabels( op );

    emit distanceConfigChanged();
  } );

  connect( mValue1SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue1->setValue( value );
    emit distanceConfigChanged();
  } );

  connect( mValue2SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    if ( operation() == QgsGeometry::Fillet )
      QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->setValue( static_cast<int>( value ) );
    else
      QgsSettingsRegistryCore::settingsDigitizingChamferFilletValue2->setValue( value );
    emit distanceConfigChanged();
  } );

  connect( mVal1Locker, &QPushButton::clicked, this, [this]( bool checked ) {
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock1->setValue( checked );
    emit distanceConfigChanged(); } );

  connect( mVal2Locker, &QPushButton::clicked, this, [this]( bool checked ) {
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletLock2->setValue( checked );
    emit distanceConfigChanged(); } );

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
