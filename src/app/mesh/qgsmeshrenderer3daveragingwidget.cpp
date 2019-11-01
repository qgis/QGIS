/***************************************************************************
    qgsmeshrenderer3daveragingwidget.cpp
    ------------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderer3daveragingwidget.h"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"

QgsMeshRenderer3dAveragingWidget::QgsMeshRenderer3dAveragingWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );
  connect( mAveragingMethodComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRenderer3dAveragingWidget::onAveragingMethodChanged );
  QVector<QLineEdit *> widgets;
  connect( mVerticalLayerIndexSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, &QgsMeshRenderer3dAveragingWidget::widgetChanged );
}

void QgsMeshRenderer3dAveragingWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

QgsMeshRenderer3dAveragingSettings QgsMeshRenderer3dAveragingWidget::settings() const
{
  QgsMeshRenderer3dAveragingSettings  settings;
  settings.setAveragingMethod( static_cast<QgsMeshRenderer3dAveragingSettings::Method>( mAveragingMethodComboBox->currentIndex() ) );

  // Single Vertical Layer settings
  QgsMeshRendererAveragingSingleVerticalLayerSettings singleLayerSettings;
  singleLayerSettings.setVerticalLayer( mVerticalLayerIndexSpinBox->value() );
  settings.setSingleVerticalLayerSettings( singleLayerSettings );

  return settings;
}

void QgsMeshRenderer3dAveragingWidget::syncToLayer( )
{
  if ( !mMeshLayer )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMeshRenderer3dAveragingSettings settings = rendererSettings.averagingSettings( );

  whileBlocking( mAveragingMethodComboBox )->setCurrentIndex( settings.averagingMethod() );

  // Single Vertical Layer settings
  const QgsMeshRendererAveragingSingleVerticalLayerSettings singleLayerSettings = settings.singleVerticalLayerSettings();
  whileBlocking( mVerticalLayerIndexSpinBox )->setValue( singleLayerSettings.verticalLayer() );
}

void QgsMeshRenderer3dAveragingWidget::onAveragingMethodChanged( int methodIndex )
{
  whileBlocking( mAveragingMethodStackedWidget )->setCurrentIndex( methodIndex );
  emit widgetChanged();
}
