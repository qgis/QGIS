/***************************************************************************
                         qgsmaplayerrefreshsettingswidget.cpp
                         ---------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerrefreshsettingswidget.h"
#include "moc_qgsmaplayerrefreshsettingswidget.cpp"
#include "qgsmaplayer.h"

QgsMapLayerRefreshSettingsWidget::QgsMapLayerRefreshSettingsWidget( QWidget *parent, QgsMapLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );
  mModeComboBox->addItem( tr( "Reload Data" ), QVariant::fromValue( Qgis::AutoRefreshMode::ReloadData ) );
  mModeComboBox->addItem( tr( "Redraw Layer Only" ), QVariant::fromValue( Qgis::AutoRefreshMode::RedrawOnly ) );
  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMapLayerRefreshSettingsWidget::updateHelp );
  syncToLayer();
}

void QgsMapLayerRefreshSettingsWidget::setLayer( QgsMapLayer *layer )
{
  mLayer = layer;
  syncToLayer();
}

void QgsMapLayerRefreshSettingsWidget::saveToLayer()
{
  mLayer->setAutoRefreshInterval( static_cast<int>( mRefreshLayerIntervalSpinBox->value() * 1000.0 ) );
  if ( !mEnabledGroupBox->isChecked() )
  {
    mLayer->setAutoRefreshMode( Qgis::AutoRefreshMode::Disabled );
  }
  else
  {
    mLayer->setAutoRefreshMode( mModeComboBox->currentData().value<Qgis::AutoRefreshMode>() );
  }
}

void QgsMapLayerRefreshSettingsWidget::syncToLayer()
{
  if ( !mLayer )
    return;

  switch ( mLayer->autoRefreshMode() )
  {
    case Qgis::AutoRefreshMode::Disabled:
      mEnabledGroupBox->setChecked( false );
      break;

    case Qgis::AutoRefreshMode::ReloadData:
    case Qgis::AutoRefreshMode::RedrawOnly:
      mEnabledGroupBox->setChecked( true );
      mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( mLayer->autoRefreshMode() ) ) );
      break;
  }

  mRefreshLayerIntervalSpinBox->setValue( mLayer->autoRefreshInterval() / 1000.0 );
  updateHelp();
}

void QgsMapLayerRefreshSettingsWidget::updateHelp()
{
  QString title;
  QString help;
  switch ( mModeComboBox->currentData().value<Qgis::AutoRefreshMode>() )
  {
    case Qgis::AutoRefreshMode::Disabled:
      break;

    case Qgis::AutoRefreshMode::ReloadData:
      title = tr( "The layer will be completely refreshed." );
      help = tr( "Any cached data will be discarded and refetched from the provider. This mode may result in slower map refreshes." );
      break;

    case Qgis::AutoRefreshMode::RedrawOnly:
      title = tr( "The layer will be redrawn only." );
      help = tr( "This mode is useful for animation or when the layer's style will be updated at regular intervals. Canvas updates are deferred in order to avoid refreshing multiple times if more than one layer has an auto update interval set." );
      break;
  }

  mHelpLabel->setText( QStringLiteral( "<b>%1</b><p>%2" ).arg( title, help ) );
}
