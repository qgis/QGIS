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
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"

QgsMapToolChamferFillet::QgsMapToolChamferFillet( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mSnapIndicator( std::make_unique<QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Map tool to chamfer" );
}

QgsMapToolChamferFillet::~QgsMapToolChamferFillet()
{
  cancel();
}


void QgsMapToolChamferFillet::applyOperationFromWidget( Qt::KeyboardModifiers modifiers )
{
  if ( mSourceLayer && !mOriginalGeometry.isNull() )
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
    QgsDebugMsgLevel( QStringLiteral( "mModifiedGeometry: %1" ).arg( mModifiedGeometry.asWkt( 2 ) ), 1 );

    QString lastError;
    int i = 0;
    for ( QgsAbstractGeometry::const_part_iterator ite = mModifiedGeometry.const_parts_begin(); ite != mModifiedGeometry.const_parts_end(); ite++ )
    {
      if ( !( *ite )->isValid( lastError ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "Part %1 is not valid: %2" ).arg( i ).arg( ( *ite )->asWkt( 2 ) ), 1 );
        break;
      }
      i++;
    }

    emit messageEmitted( tr( "Generated geometry is not valid (part:%1): %1" ).arg( i ).arg( mModifiedGeometry.lastError() ), Qgis::MessageLevel::Critical );
    // no cancel, continue editing.
    return;
  }

  QgsVectorLayer *destLayer = qobject_cast<QgsVectorLayer *>( canvas()->currentLayer() );
  if ( !destLayer )
    return;

  destLayer->beginEditCommand( tr( "Chamfer curve" ) );

  bool editOk = true;
  editOk = destLayer->changeGeometry( mModifiedFeature, mModifiedGeometry );

  if ( editOk )
  {
    destLayer->endEditCommand();
  }
  else
  {
    destLayer->destroyEditCommand();
    emit messageEmitted( QStringLiteral( "Could not apply chamfer" ), Qgis::MessageLevel::Critical );
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

    QgsVector vect = layerCoords - mVertexPoint;
    QgsVector perpVect = vect.perpVector();

    int beforeVIdx, afterVIdx;
    mManipulatedGeometry.adjacentVertices( mVertexIndex, beforeVIdx, afterVIdx );
    QgsPoint beforeVert = mManipulatedGeometry.vertexAt( beforeVIdx );
    QgsPoint afterVert = mManipulatedGeometry.vertexAt( afterVIdx );

    QgsPoint beforeInter;
    QgsGeometryUtils::lineIntersection( QgsPoint( layerCoords.x(), layerCoords.y() ), perpVect, beforeVert, beforeVert - mVertexPoint, beforeInter );

    QgsPoint afterInter;
    QgsGeometryUtils::lineIntersection( QgsPoint( layerCoords.x(), layerCoords.y() ), perpVect, afterVert, afterVert - mVertexPoint, afterInter );

    value1 = beforeInter.distance( mVertexPoint );
    value2 = afterInter.distance( mVertexPoint );
  }
  QgsDebugMsgLevel( QStringLiteral( "dist between %1 and %2: %3 / %4" ).arg( mapPoint.asWkt() ).arg( mVertexPoint.asWkt( 2 ) ).arg( value1 ).arg( value2 ), 1 );
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
  QgsDebugMsgLevel( "Enter", 1 );

  if ( e->button() == Qt::RightButton )
  {
    cancel();
    return;
  }

  if ( mOriginalGeometry.isNull() )
  {
    QgsDebugMsgLevel( "First click", 1 );
    // first click, get feature to modify
    deleteRubberBandAndGeometry();
    mGeometryModified = false;

    QgsPointLocator::Match match;

    QgsDebugMsgLevel( "using snap on area", 1 );
    match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Types( QgsPointLocator::Vertex ) );

    if ( auto *lLayer = match.layer() )
    {
      mSourceLayer = lLayer;
      QgsFeature fet;
      if ( lLayer->getFeatures( QgsFeatureRequest( match.featureId() ) ).nextFeature( fet ) )
      {
        mSourceFeature = fet;
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
          emit messageEmitted( QStringLiteral( "layer %1 has %2%3%4 geometry. %2%3%4 values be set to 0 when using chamfer/fillet tool." ).arg( mSourceLayer->name(), hasZ ? QStringLiteral( "Z" ) : QString(), hasZ && hasM ? QStringLiteral( "/" ) : QString(), hasM ? QStringLiteral( "M" ) : QString() ), Qgis::MessageLevel::Warning );
        }
        QgsDebugMsgLevel( "First vertex found", 1 );
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
    QgsDebugMsgLevel( "Second click will apply changes", 1 );

    // second click - apply changes
    double value1, value2;
    calculateDistances( e->mapPoint(), value1, value2 );
    const QString op = QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value();
    if ( op == "chamfer" )
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
  QgsDebugMsgLevel( "Exit", 1 );
}

void QgsMapToolChamferFillet::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mOriginalGeometry.isNull() || !mRubberBand )
  {
    QgsDebugMsgLevel( "move without nothing selected", 1 );
    QgsPointLocator::Match match;
    match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Types( QgsPointLocator::Vertex ) );
    mSnapIndicator->setMatch( match );
    return;
  }

  QgsDebugMsgLevel( "move with something selected", 1 );
  mGeometryModified = true;

  const QgsPointXY mapPoint = e->mapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  double value1, value2;
  calculateDistances( mapPoint, value1, value2 );

  const QString op = QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value();
  if ( op == "chamfer" )
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

  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    if ( op == "chamfer" )
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

void QgsMapToolChamferFillet::prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature )
{
  QgsVectorLayer *vl = match.layer();
  if ( !vl )
  {
    return;
  }

  mOriginalGeometry = QgsGeometry();
  mManipulatedGeometry = QgsGeometry();

  //assign feature part by vertex number (snap to vertex) or by before vertex number (snap to segment)
  const QgsGeometry geom = snappedFeature.geometry();
  if ( geom.isNull() )
  {
    return;
  }
  mOriginalGeometry = geom;

  const Qgis::WkbType geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Line && QgsWkbTypes::geometryType( geomType ) != Qgis::GeometryType::Polygon )
    return;

  if ( !match.hasEdge() && !match.hasVertex() )
    return;

  mManipulatedGeometry = QgsGeometry( geom.constGet()->clone() );

  mVertexIndex = match.vertexIndex();
  mVertexPoint = geom.vertexAt( mVertexIndex );
}

void QgsMapToolChamferFillet::createUserInputWidget()
{
  deleteUserInputWidget();

  mUserInputWidget = new QgsChamferFilletUserWidget();
  QgisApp::instance()->addUserInputWidget( mUserInputWidget );
  mUserInputWidget->setFocus( Qt::TabFocusReason );

  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingFinished, this, &QgsMapToolChamferFillet::applyOperationFromWidget );
  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingCanceled, this, &QgsMapToolChamferFillet::cancel );
  connect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceConfigChanged, this, //
           [this] { updateGeometryAndRubberBand( mUserInputWidget->value1(), mUserInputWidget->value2() ); } );
}

void QgsMapToolChamferFillet::deleteUserInputWidget()
{
  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingFinished, this, &QgsMapToolChamferFillet::applyOperationFromWidget );
    disconnect( mUserInputWidget, &QgsChamferFilletUserWidget::distanceEditingCanceled, this, &QgsMapToolChamferFillet::cancel );
    mUserInputWidget->releaseKeyboard();
    mUserInputWidget->deleteLater();
  }
  mUserInputWidget = nullptr;
}

void QgsMapToolChamferFillet::deleteRubberBandAndGeometry()
{
  mOriginalGeometry.set( nullptr );
  mManipulatedGeometry.set( nullptr );
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolChamferFillet::updateGeometryAndRubberBand( double value1, double value2 )
{
  if ( !mRubberBand || mOriginalGeometry.isNull() )
  {
    return;
  }

  if ( !mSourceLayer )
  {
    return;
  }

  QgsGeometry newGeom;
  const QString op = QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value();
  const int segments = QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->value();

  if ( op == "chamfer" )
  {
    QgsDebugMsgLevel( QStringLiteral( "will chamfer %1 / %2" ).arg( value1 ).arg( value2 ), 1 );
    newGeom = mManipulatedGeometry.chamfer( mVertexIndex, value1, value2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "will fillet %1 / %2" ).arg( value1 ).arg( segments ), 1 );
    newGeom = mManipulatedGeometry.fillet( mVertexIndex, value1, segments );
  }

  if ( newGeom.isNull() )
  {
    deleteRubberBandAndGeometry();
    deleteUserInputWidget();
    mSourceLayer = nullptr;
    mGeometryModified = false;
    emit messageDiscarded();
    emit messageEmitted( tr( "Creating chamfer/fillet geometry failed: %1" ).arg( mManipulatedGeometry.lastError() ), Qgis::MessageLevel::Critical );
  }
  else
  {
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
  mOperationComboBox->addItem( tr( "Chamfer" ), "chamfer" );
  mOperationComboBox->addItem( tr( "Fillet" ), "fillet" );

  QString op = QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->value();
  if ( op != "chamfer" && op != "fillet" )
  {
    op = "chamfer";
    QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->setValue( op );
    qWarning() << "No op defined!! Default: chamfer";
  }

  mOperationComboBox->setCurrentIndex( mOperationComboBox->findData( op ) );

  auto updateLabels = [this]( const QString &op ) {
    if ( op == "chamfer" )
    {
      mVal1Label->setText( tr( "Distance 1" ) );
      mVal2Label->setText( tr( "Distance 2" ) );
      mValue1SpinBox->setDecimals( 6 );
      mValue1SpinBox->setClearValue( 0.001 );
      mValue2SpinBox->setDecimals( 6 );
      mValue2SpinBox->setClearValue( 0.001 );
    }
    else
    {
      mVal1Label->setText( tr( "Radius" ) );
      mVal2Label->setText( tr( "Fillet segments" ) );
      mValue1SpinBox->setDecimals( 6 );
      mValue1SpinBox->setClearValue( 0.001 );
      mValue2SpinBox->setDecimals( 0 );
      mValue2SpinBox->setClearValue( 6.0 );
      const int segments = QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->value();
      mValue2SpinBox->setValue( segments );
    }
  };

  updateLabels( op );

  // connect signals
  connect( mOperationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, //
           [this, updateLabels] {
             QString op = operation();
             QgsSettingsRegistryCore::settingsDigitizingChamferFilletOperation->setValue( op );
             updateLabels( op );

             emit distanceConfigChanged();
           } );

  connect( mValue1SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, //
           [this]( const double ) { emit distanceConfigChanged(); } );

  connect( mValue2SpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, //
           [this]( const double value ) {
             if ( operation() == "fillet" )
               QgsSettingsRegistryCore::settingsDigitizingChamferFilletSegment->setValue( static_cast<int>( value ) );
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

double QgsChamferFilletUserWidget::value1() const
{
  return mValue1SpinBox->value();
}

double QgsChamferFilletUserWidget::value2() const
{
  return mValue2SpinBox->value();
}

QString QgsChamferFilletUserWidget::operation() const
{
  return mOperationComboBox->currentData().toString();
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
