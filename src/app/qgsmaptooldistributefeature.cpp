/***************************************************************************
 qgsmaptooldistributefeature.cpp  -  map tool for copying and distributing features by mouse drag
 ---------------------
 begin                : November 2025
 copyright            : (C) 2025 by Jacky Volpes
 email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgisapp.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsmaptooldistributefeature.h"
#include "moc_qgsmaptooldistributefeature.cpp"
#include "qgsadvanceddigitizingdockwidget.h"

#include "qgssettingstree.h"
#include "qgssnappingutils.h"

const QgsSettingsEntryEnumFlag<QgsMapToolDistributeFeature::DistributeMode> *QgsMapToolDistributeFeature::settingsMode = new QgsSettingsEntryEnumFlag<QgsMapToolDistributeFeature::DistributeMode>( QStringLiteral( "distributefeature-mode" ), QgsSettingsTree::sTreeDigitizing, QgsMapToolDistributeFeature::DistributeMode::FeatureCount );
const QgsSettingsEntryInteger *QgsMapToolDistributeFeature::settingsFeatureCount = new QgsSettingsEntryInteger( QStringLiteral( "distributefeature-feature-count" ), QgsSettingsTree::sTreeDigitizing, 0 );
const QgsSettingsEntryDouble *QgsMapToolDistributeFeature::settingsFeatureSpacing = new QgsSettingsEntryDouble( QStringLiteral( "distributefeature-feature-spacing" ), QgsSettingsTree::sTreeDigitizing, 0 );

QgsMapToolDistributeFeature::QgsMapToolDistributeFeature( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
{
  mToolName = tr( "Copy and distribute feature" );

  mMode = QgsMapToolDistributeFeature::settingsMode->value();
  mFeatureCount = QgsMapToolDistributeFeature::settingsFeatureCount->value();
  mFeatureSpacing = QgsMapToolDistributeFeature::settingsFeatureSpacing->value();
  mFeatureLayer = nullptr;

  setUseSnappingIndicator( true );
}

void QgsMapToolDistributeFeature::activate()
{
  QgsMapToolAdvancedDigitizing::activate();

  mUserInputWidget = std::make_unique<QgsDistributeFeatureUserWidget>();
  connect( mUserInputWidget.get(), &QgsDistributeFeatureUserWidget::modeChanged, this, [this]( QgsMapToolDistributeFeature::DistributeMode mode ) { mMode = mode; } );
  connect( mUserInputWidget.get(), &QgsDistributeFeatureUserWidget::featureCountChanged, this, [this]( int value ) { mFeatureCount = value; } );
  connect( mUserInputWidget.get(), &QgsDistributeFeatureUserWidget::featureSpacingChanged, this, [this]( int value ) { mFeatureSpacing = value; } );

  setMode( QgsMapToolDistributeFeature::settingsMode->value() );
  setFeatureCount( QgsMapToolDistributeFeature::settingsFeatureCount->value() );
  setFeatureSpacing( QgsMapToolDistributeFeature::settingsFeatureSpacing->value() );

  QgisApp::instance()->addUserInputWidget( mUserInputWidget.get() );
}

void QgsMapToolDistributeFeature::deactivate()
{
  deleteRubberbands();
  mUserInputWidget.reset();
  QgsMapToolDistributeFeature::settingsMode->setValue( mMode );
  QgsMapToolDistributeFeature::settingsFeatureCount->setValue( mFeatureCount );
  QgsMapToolDistributeFeature::settingsFeatureSpacing->setValue( mFeatureSpacing );
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolDistributeFeature::setMode( QgsMapToolDistributeFeature::DistributeMode mode )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setMode( mode );
    mUserInputWidget->blockSignals( false );
  }

  mMode = mode;
}

void QgsMapToolDistributeFeature::setFeatureSpacing( double featureSpacing )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setFeatureSpacing( featureSpacing );
    mUserInputWidget->blockSignals( false );
  }

  mFeatureSpacing = featureSpacing;
}

void QgsMapToolDistributeFeature::setFeatureCount( int featureCount )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setFeatureCount( featureCount );
    mUserInputWidget->blockSignals( false );
  }

  mFeatureCount = featureCount;
}

void QgsMapToolDistributeFeature::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mRubberBand )
  {
    mEndPointMapCoords = e->mapPoint();
    updateRubberband();
  }
}

void QgsMapToolDistributeFeature::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    deleteRubberbands();
    return;
  }
}

void QgsMapToolDistributeFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    deleteRubberbands();
    return;
  }

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer || !vlayer->isEditable() )
  {
    deleteRubberbands();
    cadDockWidget()->clear();
    notifyNotEditableLayer();
    return;
  }

  if ( !mRubberBand )
  {
    // No rubberband means no copy is in progress, find the closest feature to begin a new copy

    // Get the selected feature or find the closest feature
    QgsFeature f;
    if ( vlayer->selectedFeatureCount() == 1 )
      f = vlayer->selectedFeatures()[0];
    else
    {
      QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::All );
      if ( !match.isValid() )
      {
        cadDockWidget()->clear();
        return;
      }
      f = vlayer->getFeature( match.featureId() );
    }

    // Feature found: store internal information and create rubberband
    mFeatureId = f.id();
    mFeatureGeom = f.geometry();
    mStartPointMapCoords = e->mapPoint();
    mFeatureLayer = vlayer;
    updateRubberband();
  }
  else
  {
    // A rubberband exists: create the new features
    mEndPointMapCoords = e->mapPoint();
    const int count = featureCount();
    if ( count >= 0 && mStartPointMapCoords != mEndPointMapCoords )
    {
      // Save features
      QString errorMsg;
      QString childrenInfoMsg;
      const QgsPointXY startPointLayerCoords = toLayerCoordinates( mFeatureLayer, mStartPointMapCoords );
      const QgsPointXY firstFeatureLayerPoint = toLayerCoordinates( mFeatureLayer, firstFeatureMapPoint() );
      const double dx = firstFeatureLayerPoint.x() - startPointLayerCoords.x();
      const double dy = firstFeatureLayerPoint.y() - startPointLayerCoords.y();
      vlayer->beginEditCommand( tr( "Features distributed and copied" ) );
      for ( int i = 1; i < count + 1; ++i )
      {
        QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( { mFeatureId } );
        if ( !QgisApp::instance()->vectorLayerTools()->copyMoveFeatures(
               vlayer, request, dx * i, dy * i, &errorMsg, QgsProject::instance()->topologicalEditing(), nullptr, &childrenInfoMsg
             ) )
        {
          emit messageEmitted( errorMsg, Qgis::MessageLevel::Critical );
          deleteRubberbands();
          vlayer->deleteFeatures( request.filterFids() );
          vlayer->destroyEditCommand();
          return;
        }
        if ( !childrenInfoMsg.isEmpty() )
        {
          emit messageEmitted( childrenInfoMsg, Qgis::MessageLevel::Info );
        }
      }
      vlayer->endEditCommand();
      vlayer->triggerRepaint();
    }
    deleteRubberbands();
  }
}

QgsPointXY QgsMapToolDistributeFeature::firstFeatureMapPoint() const
{
  if ( mStartPointMapCoords.isEmpty() || mEndPointMapCoords.isEmpty() )
    return QgsPointXY();

  return QgsGeometryUtils::pointOnLineWithDistance( QgsPoint( mStartPointMapCoords ), QgsPoint( mEndPointMapCoords ), featureSpacing() );
}

void QgsMapToolDistributeFeature::deleteRubberbands()
{
  mRubberBand.reset();
  mStartPointMapCoords = QgsPointXY();
  mEndPointMapCoords = QgsPointXY();
}

void QgsMapToolDistributeFeature::updateRubberband()
{
  if ( mFeatureGeom.isNull() || mStartPointMapCoords.isEmpty() )
    return;

  mRubberBand.reset( createRubberBand( mFeatureGeom.type() ) );

  const int count = featureCount();
  if ( count == 0 )
    return;

  QgsGeometry geom = mFeatureGeom;

  // The rubberband is in map coordinates, so we transform the feature geometry if needed
  if ( mFeatureLayer->crs() != canvas()->mapSettings().destinationCrs() )
  {
    if ( geom.transform(
           QgsCoordinateTransform( mFeatureLayer->crs(), canvas()->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() )
         )
         != Qgis::GeometryOperationResult::Success )
    {
      emit messageEmitted( tr( "Unable to transform coordinates between layer and map CRS" ), Qgis::MessageLevel::Warning );
      return;
    }
  }

  if ( mEndPointMapCoords.isEmpty() )
  {
    // No end point: the mouse has not moved yet
    mRubberBand->addGeometry( geom );
  }
  else
  {
    // We have a start and an end point: create all the features along the start-end line
    const QgsPointXY firstFeaturePoint = firstFeatureMapPoint();
    const double dx = firstFeaturePoint.x() - mStartPointMapCoords.x();
    const double dy = firstFeaturePoint.y() - mStartPointMapCoords.y();
    for ( int i = 0; i < count; ++i )
    {
      Qgis::GeometryOperationResult translateResult = geom.translate( dx, dy );
      if ( translateResult != Qgis::GeometryOperationResult::Success )
      {
        emit messageEmitted( tr( "Unable to copy and translate feature preview" ), Qgis::MessageLevel::Warning );
        return;
      }
      mRubberBand->addGeometry( geom );
    }
  }

  mRubberBand->show();
}

double QgsMapToolDistributeFeature::featureSpacing() const
{
  switch ( mode() )
  {
    case DistributeMode::FeatureSpacing:
    case DistributeMode::FeatureCountAndSpacing:
    {
      return mFeatureSpacing;
    }
    case DistributeMode::FeatureCount:
    {
      if ( mEndPointMapCoords.isEmpty() )
        return 0;
      const QgsLineString currentLine( { mStartPointMapCoords, mEndPointMapCoords } );
      return currentLine.length() / featureCount();
    }
  }
  return 0; // should not happen
}

int QgsMapToolDistributeFeature::featureCount() const
{
  switch ( mode() )
  {
    case DistributeMode::FeatureCount:
    case DistributeMode::FeatureCountAndSpacing:
    {
      return mFeatureCount;
    }
    case DistributeMode::FeatureSpacing:
    {
      if ( mEndPointMapCoords.isEmpty() )
        return 0;
      const QgsLineString currentLine( { mStartPointMapCoords, mEndPointMapCoords } );
      return std::floor( currentLine.length() / featureSpacing() );
    }
  }
  return 0; // should not happen
}

QgsDistributeFeatureUserWidget::QgsDistributeFeatureUserWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModeComboBox->addItem( tr( "Feature Count" ), QVariant::fromValue( QgsMapToolDistributeFeature::DistributeMode::FeatureCount ) );
  mModeComboBox->addItem( tr( "Spacing" ), QVariant::fromValue( QgsMapToolDistributeFeature::DistributeMode::FeatureSpacing ) );
  mModeComboBox->addItem( tr( "Spacing and Feature Count" ), QVariant::fromValue( QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing ) );

  updateUi();

  connect( mModeComboBox, &QComboBox::currentIndexChanged, this, &QgsDistributeFeatureUserWidget::updateUi );
  connect( mModeComboBox, &QComboBox::currentIndexChanged, this, [this]() { emit modeChanged( mode() ); } );
  connect( mFeatureCountSpinBox, &QSpinBox::valueChanged, this, [this]( int value ) { emit featureCountChanged( value ); } );
  connect( mFeatureSpacingSpinBox, &QDoubleSpinBox::valueChanged, this, [this]( double value ) { emit featureSpacingChanged( value ); } );
}

void QgsDistributeFeatureUserWidget::updateUi()
{
  const QgsMapToolDistributeFeature::DistributeMode currentMode = mode();

  mFeatureCountLabel->setVisible(
    currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCount
    || currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing
  );
  mFeatureCountSpinBox->setVisible(
    currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCount
    || currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing
  );

  mFeatureSpacingLabel->setVisible(
    currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureSpacing
    || currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing
  );
  mFeatureSpacingSpinBox->setVisible(
    currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureSpacing
    || currentMode == QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing
  );
}

void QgsDistributeFeatureUserWidget::setMode( QgsMapToolDistributeFeature::DistributeMode mode )
{
  mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( mode ) ) );
}

QgsMapToolDistributeFeature::DistributeMode QgsDistributeFeatureUserWidget::mode() const
{
  return mModeComboBox->currentData().value<QgsMapToolDistributeFeature::DistributeMode>();
}

void QgsDistributeFeatureUserWidget::setFeatureCount( int featureCount )
{
  mFeatureCountSpinBox->setValue( featureCount );
}

int QgsDistributeFeatureUserWidget::featureCount() const
{
  return mFeatureCountSpinBox->value();
}

void QgsDistributeFeatureUserWidget::setFeatureSpacing( double featureSpacing )
{
  mFeatureSpacingSpinBox->setValue( featureSpacing );
}

double QgsDistributeFeatureUserWidget::featureSpacing() const
{
  return mFeatureSpacingSpinBox->value();
}
