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

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"

QgsMeshRendererActiveDatasetWidget::QgsMeshRendererActiveDatasetWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererActiveDatasetWidget::onActiveTimeChanged );
  connect( mDatasetSlider, &QSlider::valueChanged, mTimeComboBox, &QComboBox::setCurrentIndex );

  connect( mFirstDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onFirstTimeClicked );
  connect( mPreviousDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onPreviousTimeClicked );
  connect( mNextDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onNextTimeClicked );
  connect( mLastDatasetButton, &QToolButton::clicked, this, &QgsMeshRendererActiveDatasetWidget::onLastTimeClicked );

  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeScalarGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged );
  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeVectorGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged );
}

void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
  mDatasetGroupTreeView->setLayer( layer );
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
      mTimeComboBox->addItem( timeToString( time ), time );
    }
  }
  mTimeComboBox->blockSignals( false );

  // enable/disable time controls depending on whether the data set is time varying
  bool isTimeVarying = datasetCount > 1;
  mTimeComboBox->setEnabled( isTimeVarying );
  mDatasetSlider->setEnabled( isTimeVarying );
  mFirstDatasetButton->setEnabled( isTimeVarying );
  mPreviousDatasetButton->setEnabled( isTimeVarying );
  mNextDatasetButton->setEnabled( isTimeVarying );
  mLastDatasetButton->setEnabled( isTimeVarying );
}

void QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveScalarDatasetGroup )
    return;

  mActiveScalarDatasetGroup = groupIndex;

  // keep the same timestep if possible
  onActiveTimeChanged( mTimeComboBox->currentIndex() );
  emit activeScalarGroupChanged( mActiveScalarDatasetGroup );
}

void QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveVectorDatasetGroup )
    return;

  mActiveVectorDatasetGroup = groupIndex;

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

QString QgsMeshRendererActiveDatasetWidget::timeToString( double val )
{
  // time val should be in hours
  int seconds = static_cast<int>( qgsRound( val * 3600.0, 0 ) );
  QTime t = QTime( 0, 0 ).addSecs( seconds );
  return t.toString();  // the format is "HH:mm:ss"
}

QString QgsMeshRendererActiveDatasetWidget::metadata( QgsMeshDatasetIndex datasetIndex )
{

  QString msg;
  msg += QStringLiteral( "<table>" );

  const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Is valid" ) )
         .arg( meta.isValid() ? tr( "Yes" ) : tr( "No" ) );

  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Time" ) )
         .arg( meta.time() );

  const QgsMeshDatasetGroupMetadata gmeta = mMeshLayer->dataProvider()->datasetGroupMetadata( datasetIndex );
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Data Type" ) )
         .arg( gmeta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ? tr( "Defined on vertices" ) : tr( "Defined on faces" ) );

  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
         .arg( tr( "Is vector" ) )
         .arg( gmeta.isVector() ? tr( "Yes" ) : tr( "No" ) );

  for ( auto it = gmeta.extraOptions().constBegin(); it != gmeta.extraOptions().constEnd(); ++it )
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
