/***************************************************************************
 qgsmaptoolfeaturearray.cpp  -  map tool for copying a feature in an array of features by mouse drag
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


#include "qgsmaptoolfeaturearray.h"

#include "qgisapp.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgssettingstree.h"
#include "qgssnappingutils.h"

#include <QString>

#include "moc_qgsmaptoolfeaturearray.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryEnumFlag<QgsMapToolFeatureArray::ArrayMode> *QgsMapToolFeatureArray::settingsMode = new QgsSettingsEntryEnumFlag<QgsMapToolFeatureArray::ArrayMode>( u"featurearray-mode"_s, QgsSettingsTree::sTreeDigitizing, QgsMapToolFeatureArray::ArrayMode::FeatureCount );
const QgsSettingsEntryInteger *QgsMapToolFeatureArray::settingsFeatureCount = new QgsSettingsEntryInteger( u"featurearray-feature-count"_s, QgsSettingsTree::sTreeDigitizing, 0 );
const QgsSettingsEntryDouble *QgsMapToolFeatureArray::settingsFeatureSpacing = new QgsSettingsEntryDouble( u"featurearray-feature-spacing"_s, QgsSettingsTree::sTreeDigitizing, 0 );

QgsMapToolFeatureArray::QgsMapToolFeatureArray( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
{
  mToolName = tr( "Copy in an array of features" );
  setUseSnappingIndicator( true );
}

void QgsMapToolFeatureArray::activate()
{
  QgsMapToolAdvancedDigitizing::activate();

  mUserInputWidget.reset( new QgsFeatureArrayUserWidget() );
  connect( mUserInputWidget.get(), &QgsFeatureArrayUserWidget::modeChanged, this, [this]( QgsMapToolFeatureArray::ArrayMode mode ) { mMode = mode; } );
  connect( mUserInputWidget.get(), &QgsFeatureArrayUserWidget::featureCountChanged, this, [this]( int value ) { mFeatureCount = value; updateRubberband(); } );
  connect( mUserInputWidget.get(), &QgsFeatureArrayUserWidget::featureSpacingChanged, this, [this]( int value ) { mFeatureSpacing = value; } );

  setMode( QgsMapToolFeatureArray::settingsMode->value() );
  setFeatureCount( QgsMapToolFeatureArray::settingsFeatureCount->value() );
  setFeatureSpacing( QgsMapToolFeatureArray::settingsFeatureSpacing->value() );

  QgisApp::instance()->addUserInputWidget( mUserInputWidget.get() );
}

void QgsMapToolFeatureArray::deactivate()
{
  // Delete rubberbands
  deleteRubberbands();

  // Remove user input widget
  if ( mUserInputWidget )
    mUserInputWidget.reset();

  // Save current values in QgsSettings
  QgsMapToolFeatureArray::settingsMode->setValue( mMode );
  QgsMapToolFeatureArray::settingsFeatureCount->setValue( mFeatureCount );
  QgsMapToolFeatureArray::settingsFeatureSpacing->setValue( mFeatureSpacing );
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolFeatureArray::setMode( QgsMapToolFeatureArray::ArrayMode mode )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setMode( mode );
    mUserInputWidget->blockSignals( false );
  }

  mMode = mode;
}

void QgsMapToolFeatureArray::setFeatureSpacing( double featureSpacing )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setFeatureSpacing( featureSpacing );
    mUserInputWidget->blockSignals( false );
  }

  mFeatureSpacing = featureSpacing;
}

void QgsMapToolFeatureArray::setFeatureCount( int featureCount )
{
  if ( mUserInputWidget )
  {
    mUserInputWidget->blockSignals( true );
    mUserInputWidget->setFeatureCount( featureCount );
    mUserInputWidget->blockSignals( false );
  }

  mFeatureCount = featureCount;
}

void QgsMapToolFeatureArray::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mStartPointMapCoords.isEmpty() )
  {
    mEndPointMapCoords = e->mapPoint();
    updateRubberband();
  }
}

void QgsMapToolFeatureArray::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolFeatureArray::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
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

  if ( mStartPointMapCoords.isEmpty() )
  {
    // No start point means no copy is in progress, find the closest feature to begin a new copy

    // Get the selected feature or find the closest feature
    if ( vlayer->selectedFeatureCount() > 0 )
      mFeatureList = vlayer->selectedFeatures();
    else
    {
      QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::All );
      if ( !match.isValid() )
      {
        cadDockWidget()->clear();
        return;
      }
      mFeatureList = { vlayer->getFeature( match.featureId() ) };
    }

    // Feature found: store internal information and create rubberband
    mStartPointMapCoords = e->mapPoint();
    mFeatureLayer = vlayer;
    updateRubberband();
  }
  else
  {
    // A rubberband exists: create the new features
    mEndPointMapCoords = e->mapPoint();
    const int count = featureCount();
    if ( count > 0 && mStartPointMapCoords != mEndPointMapCoords )
    {
      // Save features
      QString errorMsg;
      QString childrenInfoMsg;
      const QgsPointXY startPointLayerCoords = toLayerCoordinates( mFeatureLayer, mStartPointMapCoords );
      const QgsPointXY firstFeatureLayerPoint = toLayerCoordinates( mFeatureLayer, firstFeatureMapPoint() );
      const double dx = firstFeatureLayerPoint.x() - startPointLayerCoords.x();
      const double dy = firstFeatureLayerPoint.y() - startPointLayerCoords.y();
      vlayer->beginEditCommand( tr( "Feature array created" ) );
      for ( int i = 1; i < count + 1; ++i )
      {
        QgsFeatureIds fids;
        std::for_each( mFeatureList.begin(), mFeatureList.end(), [&fids]( QgsFeature f ) { fids << f.id(); } );
        QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( fids );
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

QgsPointXY QgsMapToolFeatureArray::firstFeatureMapPoint() const
{
  if ( mStartPointMapCoords.isEmpty() || mEndPointMapCoords.isEmpty() )
    return QgsPointXY();

  return QgsGeometryUtils::pointOnLineWithDistance( QgsPoint( mStartPointMapCoords ), QgsPoint( mEndPointMapCoords ), featureSpacing() );
}

void QgsMapToolFeatureArray::deleteRubberbands()
{
  mRubberBand.reset();
  mStartPointMapCoords = QgsPointXY();
  mEndPointMapCoords = QgsPointXY();
  mFeatureList.clear();
  mFeatureLayer = nullptr;
}

void QgsMapToolFeatureArray::updateRubberband()
{
  const int count = featureCount();
  if ( !mFeatureLayer || mFeatureList.count() == 0 || mStartPointMapCoords.isEmpty() || count == 0 )
    return;

  bool firstFeature = true;
  for ( const QgsFeature &feature : mFeatureList )
  {
    QgsGeometry geom = feature.geometry();
    if ( geom.isNull() )
      return;

    if ( firstFeature )
    {
      mRubberBand.reset( createRubberBand( geom.type() ) );
      firstFeature = false;
    }

    // The rubberband is in map coordinates, so we transform the feature geometry if needed
    if ( mFeatureLayer->crs() != canvas()->mapSettings().destinationCrs() )
    {
      Qgis::GeometryOperationResult result = Qgis::GeometryOperationResult::NothingHappened;
      try
      {
        result = geom.transform( QgsCoordinateTransform( mFeatureLayer->crs(), canvas()->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() ) );
      }
      catch ( QgsCsException & )
      {
        ; // result will remain NothingHappened
      }
      if ( result != Qgis::GeometryOperationResult::Success )
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
  }

  mRubberBand->show();
}

double QgsMapToolFeatureArray::featureSpacing() const
{
  switch ( mode() )
  {
    case ArrayMode::FeatureSpacing:
    case ArrayMode::FeatureCountAndSpacing:
    {
      return mFeatureSpacing;
    }
    case ArrayMode::FeatureCount:
    {
      if ( mEndPointMapCoords.isEmpty() )
        return 0;
      const QgsLineString currentLine( { mStartPointMapCoords, mEndPointMapCoords } );
      return currentLine.length() / featureCount();
    }
  }
  return 0; // should not happen
}

int QgsMapToolFeatureArray::featureCount() const
{
  switch ( mode() )
  {
    case ArrayMode::FeatureCount:
    case ArrayMode::FeatureCountAndSpacing:
    {
      return mFeatureCount;
    }
    case ArrayMode::FeatureSpacing:
    {
      if ( mEndPointMapCoords.isEmpty() )
        return 0;
      const QgsLineString currentLine( { mStartPointMapCoords, mEndPointMapCoords } );
      return std::floor( currentLine.length() / featureSpacing() );
    }
  }
  return 0; // should not happen
}

QgsFeatureArrayUserWidget::QgsFeatureArrayUserWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModeComboBox->addItem( tr( "Feature Count" ), QVariant::fromValue( QgsMapToolFeatureArray::ArrayMode::FeatureCount ) );
  mModeComboBox->addItem( tr( "Spacing" ), QVariant::fromValue( QgsMapToolFeatureArray::ArrayMode::FeatureSpacing ) );
  mModeComboBox->addItem( tr( "Spacing and Feature Count" ), QVariant::fromValue( QgsMapToolFeatureArray::ArrayMode::FeatureCountAndSpacing ) );

  updateUi();

  connect( mModeComboBox, &QComboBox::currentIndexChanged, this, &QgsFeatureArrayUserWidget::updateUi );
  connect( mModeComboBox, &QComboBox::currentIndexChanged, this, [this]() { emit modeChanged( mode() ); } );
  connect( mFeatureCountSpinBox, &QSpinBox::valueChanged, this, [this]( int value ) { emit featureCountChanged( value ); } );
  connect( mFeatureSpacingSpinBox, &QDoubleSpinBox::valueChanged, this, [this]( double value ) { emit featureSpacingChanged( value ); } );
}

void QgsFeatureArrayUserWidget::updateUi()
{
  const QgsMapToolFeatureArray::ArrayMode currentMode = mode();

  mFeatureCountLabel->setVisible(
    currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCount
    || currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCountAndSpacing
  );
  mFeatureCountSpinBox->setVisible(
    currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCount
    || currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCountAndSpacing
  );

  mFeatureSpacingLabel->setVisible(
    currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureSpacing
    || currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCountAndSpacing
  );
  mFeatureSpacingSpinBox->setVisible(
    currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureSpacing
    || currentMode == QgsMapToolFeatureArray::ArrayMode::FeatureCountAndSpacing
  );
}

void QgsFeatureArrayUserWidget::setMode( QgsMapToolFeatureArray::ArrayMode mode )
{
  mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( mode ) ) );
}

QgsMapToolFeatureArray::ArrayMode QgsFeatureArrayUserWidget::mode() const
{
  return mModeComboBox->currentData().value<QgsMapToolFeatureArray::ArrayMode>();
}

void QgsFeatureArrayUserWidget::setFeatureCount( int featureCount )
{
  mFeatureCountSpinBox->setValue( featureCount );
}

int QgsFeatureArrayUserWidget::featureCount() const
{
  return mFeatureCountSpinBox->value();
}

void QgsFeatureArrayUserWidget::setFeatureSpacing( double featureSpacing )
{
  mFeatureSpacingSpinBox->setValue( featureSpacing );
}

double QgsFeatureArrayUserWidget::featureSpacing() const
{
  return mFeatureSpacingSpinBox->value();
}
