/***************************************************************************
    qgsmeshrendereractivedatasetwidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrendereractivedatasetwidget.h"

#include <QDateTime>
#include <QIcon>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshtimeformatdialog.h"

QgsMeshRendererActiveDatasetWidget::QgsMeshRendererActiveDatasetWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererActiveDatasetWidget::onActiveTimeChanged );
  connect( mDatasetSlider, &QSlider::valueChanged, mTimeComboBox, &QComboBox::setCurrentIndex );

  connect( mTimeFormatButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onTimeSettingsClicked );
  connect( mFirstDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onFirstTimeClicked );
  connect( mPreviousDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onPreviousTimeClicked );
  connect( mNextDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onNextTimeClicked );
  connect( mLastDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onLastTimeClicked );
  connect( mDatasetPlaybackButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onDatasetPlaybackClicked );

  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeScalarGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged );
  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeVectorGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged );

  mDatasetPlaybackTimer = new QTimer( this );
  connect( mDatasetPlaybackTimer, &QTimer::timeout,
           this,  qgis::overload<>::of( &QgsMeshRendererActiveDatasetWidget::datasetPlaybackTick ) );

  mDatasetPlaybackButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionPlay.svg" ) ) );
  mFirstDatasetButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFirst.svg" ) ) );
  mPreviousDatasetButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionPrevious.svg" ) ) );
  mNextDatasetButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionNext.svg" ) ) );
  mLastDatasetButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionLast.svg" ) ) );
}

QgsMeshRendererActiveDatasetWidget::~QgsMeshRendererActiveDatasetWidget()
{
  mDatasetPlaybackTimer->stop();
}

void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
  mDatasetGroupTreeView->setLayer( layer );
  setTimeRange();

  if ( layer )
  {
    connect( mMeshLayer, &QgsMeshLayer::timeSettingsChanged, this, &QgsMeshRendererActiveDatasetWidget::setTimeRange );
  }
}

int QgsMeshRendererActiveDatasetWidget::activeScalarDatasetGroup() const
{
  return mActiveScalarDatasetGroup;
}

int QgsMeshRendererActiveDatasetWidget::activeVectorDatasetGroup() const
{
  return mActiveVectorDatasetGroup;
}

QgsMeshDatasetIndex QgsMeshRendererActiveDatasetWidget::activeScalarDataset() const
{
  return mActiveScalarDataset;
}

QgsMeshDatasetIndex QgsMeshRendererActiveDatasetWidget::activeVectorDataset() const
{
  return mActiveVectorDataset;
}

void QgsMeshRendererActiveDatasetWidget::setTimeRange()
{
  // figure out which dataset group contains the greatest number of datasets.
  // this group will be used to initialize the time combo box.
  int datasetCount = 0;
  int groupWithMaximumDatasets = -1;
  if ( mMeshLayer && mMeshLayer->dataProvider() )
  {
    for ( int i = 0; i < mMeshLayer->dataProvider()->datasetGroupCount(); ++i )
    {
      int currentCount = mMeshLayer->dataProvider()->datasetCount( i );
      if ( currentCount > datasetCount )
      {
        datasetCount = currentCount;
        groupWithMaximumDatasets = i;
      }
    }
  }

  // update slider
  mDatasetSlider->blockSignals( true );
  mDatasetSlider->setMinimum( 0 );
  mDatasetSlider->setMaximum( datasetCount - 1 );
  mDatasetSlider->blockSignals( false );

  // update combobox
  mTimeComboBox->blockSignals( true );
  mTimeComboBox->clear();
  if ( groupWithMaximumDatasets > -1 )
  {
    for ( int i = 0; i < datasetCount; ++i )
    {
      QgsMeshDatasetIndex index( groupWithMaximumDatasets, i );
      QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( index );
      double time = meta.time();
      mTimeComboBox->addItem( mMeshLayer->formatTime( time ), time );
    }
  }
  mTimeComboBox->blockSignals( false );

  // enable/disable time controls depending on whether the data set is time varying
  enableTimeControls();
}

void QgsMeshRendererActiveDatasetWidget::enableTimeControls()
{
  const int scalarDatesets = mMeshLayer->dataProvider()->datasetCount( mActiveScalarDatasetGroup );
  const int vectorDatesets = mMeshLayer->dataProvider()->datasetCount( mActiveVectorDatasetGroup );
  const bool isTimeVarying = ( scalarDatesets > 1 ) || ( vectorDatesets > 1 );
  mTimeComboBox->setEnabled( isTimeVarying );
  mDatasetSlider->setEnabled( isTimeVarying );
  mTimeFormatButton->setEnabled( isTimeVarying );
  mFirstDatasetButton->setEnabled( isTimeVarying );
  mPreviousDatasetButton->setEnabled( isTimeVarying );
  mNextDatasetButton->setEnabled( isTimeVarying );
  mLastDatasetButton->setEnabled( isTimeVarying );
  mDatasetPlaybackButton->setEnabled( isTimeVarying );
}

void QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveScalarDatasetGroup )
    return;

  mActiveScalarDatasetGroup = groupIndex;

  // enable/disable time slider controls
  enableTimeControls();

  // keep the same timestep if possible
  onActiveTimeChanged( mTimeComboBox->currentIndex() );
  emit activeScalarGroupChanged( mActiveScalarDatasetGroup );
}

void QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveVectorDatasetGroup )
    return;

  mActiveVectorDatasetGroup = groupIndex;
  // enable/disable time slider controls
  enableTimeControls();

  // keep the same timestep if possible
  onActiveTimeChanged( mTimeComboBox->currentIndex() );
  emit activeVectorGroupChanged( mActiveVectorDatasetGroup );
}

void QgsMeshRendererActiveDatasetWidget::onActiveTimeChanged( int value )
{
  if ( !mMeshLayer || !mMeshLayer->dataProvider() )
    return;

  bool changed = false;

  QgsMeshDatasetIndex activeScalarDataset(
    mActiveScalarDatasetGroup,
    std::min( value, mMeshLayer->dataProvider()->datasetCount( mActiveScalarDatasetGroup ) - 1 )
  );
  if ( activeScalarDataset != mActiveScalarDataset )
  {
    mActiveScalarDataset = activeScalarDataset;
    changed = true;
  }

  QgsMeshDatasetIndex activeVectorDataset(
    mActiveVectorDatasetGroup,
    std::min( value, mMeshLayer->dataProvider()->datasetCount( mActiveVectorDatasetGroup ) - 1 )
  );
  if ( activeVectorDataset != mActiveVectorDataset )
  {
    mActiveVectorDataset = activeVectorDataset;
    changed = true;
  }

  if ( changed )
  {
    whileBlocking( mDatasetSlider )->setValue( value );
    updateMetadata();
    emit widgetChanged();
  }
}

void QgsMeshRendererActiveDatasetWidget::onTimeSettingsClicked()
{
  if ( !mMeshLayer )
    return;
  QgsMeshTimeFormatDialog dlg( mMeshLayer );
  dlg.setModal( true );
  dlg.exec();
}

void QgsMeshRendererActiveDatasetWidget::onFirstTimeClicked()
{
  mTimeComboBox->setCurrentIndex( 0 );
}

void QgsMeshRendererActiveDatasetWidget::onPreviousTimeClicked()
{
  int idx = mTimeComboBox->currentIndex() - 1;
  if ( idx >= 0 )
    mTimeComboBox->setCurrentIndex( idx );
}

void QgsMeshRendererActiveDatasetWidget::onNextTimeClicked()
{
  int idx = mTimeComboBox->currentIndex() + 1;
  if ( idx < mTimeComboBox->count() )
    mTimeComboBox->setCurrentIndex( idx );
}

void QgsMeshRendererActiveDatasetWidget::onLastTimeClicked()
{
  mTimeComboBox->setCurrentIndex( mTimeComboBox->count() - 1 );
}

void QgsMeshRendererActiveDatasetWidget::onDatasetPlaybackClicked()
{
  if ( mDatasetIsPlaying )
  {
    // stop playing
    mDatasetIsPlaying = false;
    mTimeComboBox->setEnabled( true );
    mTimeFormatButton->setEnabled( true );
    mDatasetSlider->setEnabled( true );
    mFirstDatasetButton->setEnabled( true );
    mPreviousDatasetButton->setEnabled( true );
    mNextDatasetButton->setEnabled( true );
    mLastDatasetButton->setEnabled( true );
    mDatasetPlaybackTimer->stop();
    mDatasetPlaybackButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionPlay.svg" ) ) );
  }
  else
  {
    // start
    mDatasetIsPlaying = true;
    mTimeComboBox->setEnabled( false );
    mTimeFormatButton->setEnabled( false );
    mDatasetSlider->setEnabled( false );
    mFirstDatasetButton->setEnabled( false );
    mPreviousDatasetButton->setEnabled( false );
    mNextDatasetButton->setEnabled( false );
    mLastDatasetButton->setEnabled( false );
    int intervalMs = 3000;
    if ( mMeshLayer )
    {
      intervalMs = static_cast<int>( mMeshLayer->timeSettings().datasetPlaybackInterval() * 1000 );
    }
    datasetPlaybackTick();
    mDatasetPlaybackTimer->start( intervalMs );
    mDatasetPlaybackButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionStop.svg" ) ) );
  }
}

void QgsMeshRendererActiveDatasetWidget::datasetPlaybackTick()
{
  int nextIdx = mTimeComboBox->currentIndex() + 1;
  if ( nextIdx >= mTimeComboBox->count() )
    nextIdx = 0;

  mTimeComboBox->setCurrentIndex( nextIdx );
}

void QgsMeshRendererActiveDatasetWidget::updateMetadata()
{
  QString msg;

  if ( !mMeshLayer ||
       !mMeshLayer->dataProvider() )
  {
    msg += tr( "Invalid mesh layer selected" );
  }
  else
  {
    if ( mActiveScalarDataset.isValid() )
    {
      if ( mActiveVectorDataset.isValid() )
      {
        if ( mActiveScalarDataset == mActiveVectorDataset )
        {
          msg += metadata( mActiveScalarDataset );
        }
        else
        {
          msg += QStringLiteral( "<p> <h3> %1 </h3> " ).arg( tr( "Scalar dataset" ) );
          msg += metadata( mActiveScalarDataset );
          msg += QStringLiteral( "</p> <p> <h3> %1 </h3>" ).arg( tr( "Vector dataset" ) );
          msg += metadata( mActiveVectorDataset );
          msg += QStringLiteral( "</p>" );
        }
      }
      else
      {
        msg += metadata( mActiveScalarDataset );
      }
    }
    else
    {
      if ( mActiveVectorDataset.isValid() )
      {
        msg += metadata( mActiveVectorDataset );
      }
      else
      {
        msg += tr( "No mesh dataset selected" );
      }
    }
  }

  mActiveDatasetMetadata->setText( msg );
}


QString QgsMeshRendererActiveDatasetWidget::metadata( QgsMeshDatasetIndex datasetIndex )
{

  QString msg;
  msg += QStringLiteral( "<table>" );

  const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Is valid" ) )
         .arg( meta.isValid() ? tr( "Yes" ) : tr( "No" ) );

  const double time = meta.time();
  msg += QStringLiteral( "<tr><td>%1</td><td>%2 (%3)</td></tr>" )
         .arg( tr( "Time" ) )
         .arg( mMeshLayer->formatTime( time ) )
         .arg( time );

  const QgsMeshDatasetGroupMetadata gmeta = mMeshLayer->dataProvider()->datasetGroupMetadata( datasetIndex );
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Data Type" ) )
         .arg( gmeta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? tr( "Defined on vertices" ) : tr( "Defined on faces" ) );

  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Is vector" ) )
         .arg( gmeta.isVector() ? tr( "Yes" ) : tr( "No" ) );

  const auto options = gmeta.extraOptions();
  for ( auto it = options.constBegin(); it != options.constEnd(); ++it )
  {
    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( it.key() ).arg( it.value() );
  }

  msg += QStringLiteral( "</table>" );

  return msg;
}

void QgsMeshRendererActiveDatasetWidget::syncToLayer()
{
  setEnabled( mMeshLayer );

  whileBlocking( mDatasetGroupTreeView )->syncToLayer();

  if ( mMeshLayer )
  {
    const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
    mActiveScalarDatasetGroup = mDatasetGroupTreeView->activeScalarGroup();
    mActiveVectorDatasetGroup = mDatasetGroupTreeView->activeVectorGroup();
    mActiveScalarDataset = rendererSettings.activeScalarDataset();
    mActiveVectorDataset = rendererSettings.activeVectorDataset();
  }
  else
  {
    mActiveScalarDatasetGroup = -1;
    mActiveVectorDatasetGroup = -1;
    mActiveScalarDataset = QgsMeshDatasetIndex();
    mActiveVectorDataset = QgsMeshDatasetIndex();
  }

  setTimeRange();

  int val = 0;
  if ( mActiveScalarDataset.isValid() )
    val = mActiveScalarDataset.dataset();
  else if ( mActiveVectorDataset.isValid() )
    val = mActiveVectorDataset.dataset();

  whileBlocking( mTimeComboBox )->setCurrentIndex( val );
  whileBlocking( mDatasetSlider )->setValue( val );
  updateMetadata();
}
