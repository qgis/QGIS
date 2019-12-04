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

#include <memory>

#include "qgsmeshrenderer3daveragingwidget.h"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmesh3daveraging.h"

QgsMeshRenderer3dAveragingWidget::QgsMeshRenderer3dAveragingWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );
  connect( mAveragingMethodComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRenderer3dAveragingWidget::onAveragingMethodChanged );
  connect( mVerticalLayerIndexSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ),
           this, &QgsMeshRenderer3dAveragingWidget::widgetChanged );
}

void QgsMeshRenderer3dAveragingWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

std::unique_ptr<QgsMesh3dAveragingMethod> QgsMeshRenderer3dAveragingWidget::averagingMethod() const
{
  std::unique_ptr<QgsMesh3dAveragingMethod> averaging;

  QgsMesh3dAveragingMethod::Method method = static_cast<QgsMesh3dAveragingMethod::Method>( mAveragingMethodComboBox->currentIndex() );

  switch ( method )
  {
    case QgsMesh3dAveragingMethod::SingleLevelAverageMethod:
    {
      const int verticalLevel = mVerticalLayerIndexSpinBox->value();
      averaging.reset( new QgsMeshSingleLevelAveragingMethod( verticalLevel ) );
    }
  }
  return averaging;
}

void QgsMeshRenderer3dAveragingWidget::syncToLayer( )
{
  if ( !mMeshLayer )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMesh3dAveragingMethod *method = rendererSettings.averagingMethod();
  if ( method )
  {
    const QgsMesh3dAveragingMethod::Method type = method->method();
    whileBlocking( mAveragingMethodComboBox )->setCurrentIndex( type );

    switch ( type )
    {
      case QgsMesh3dAveragingMethod::SingleLevelAverageMethod:
        // Single Vertical Layer settings
        const QgsMeshSingleLevelAveragingMethod *singleAveragingMethod = static_cast<const QgsMeshSingleLevelAveragingMethod *>( method );
        whileBlocking( mVerticalLayerIndexSpinBox )->setValue( singleAveragingMethod->verticalLevel() );
        break;
    }

  }
}

void QgsMeshRenderer3dAveragingWidget::onAveragingMethodChanged( int methodIndex )
{
  whileBlocking( mAveragingMethodStackedWidget )->setCurrentIndex( methodIndex );
  emit widgetChanged();
}
