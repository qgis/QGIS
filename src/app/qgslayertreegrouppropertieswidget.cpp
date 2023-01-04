/***************************************************************************
    qgslayertreegrouppropertieswidget.cpp
    ---------------------
    begin                : Nobember 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreegrouppropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsgui.h"
#include "qgspainteffect.h"
#include "qgsmapcanvas.h"
#include "qgspainteffectregistry.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>

QgsLayerTreeGroupPropertiesWidget::QgsLayerTreeGroupPropertiesWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( nullptr, canvas, parent )
{
  setupUi( this );

  mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  mPaintEffect->setEnabled( false );

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsLayerTreeGroupPropertiesWidget::onLayerPropertyChanged );
  connect( mBlendModeComboBox, qOverload< int >( &QgsBlendModeComboBox::currentIndexChanged ), this, &QgsLayerTreeGroupPropertiesWidget::onLayerPropertyChanged );
  connect( mEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsLayerTreeGroupPropertiesWidget::onLayerPropertyChanged );
  connect( mRenderAsGroupCheckBox, &QGroupBox::toggled, this, &QgsLayerTreeGroupPropertiesWidget::onLayerPropertyChanged );

  setDockMode( true );
}

QgsLayerTreeGroupPropertiesWidget::~QgsLayerTreeGroupPropertiesWidget() = default;

void QgsLayerTreeGroupPropertiesWidget::syncToLayer( QgsMapLayer * )
{
}

void QgsLayerTreeGroupPropertiesWidget::setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context )
{
  QgsMapLayerConfigWidget::setMapLayerConfigWidgetContext( context );
  mLayerTreeGroup = context.layerTreeGroup();

  mBlockLayerUpdates = true;
  if ( QgsGroupLayer *groupLayer = mLayerTreeGroup->groupLayer() )
  {
    mRenderAsGroupCheckBox->setChecked( true );
    mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), groupLayer ) );
    mBlendModeComboBox->setBlendMode( groupLayer->blendMode() );
    mOpacityWidget->setOpacity( groupLayer->opacity() );
    if ( groupLayer->paintEffect() )
    {
      mPaintEffect.reset( groupLayer->paintEffect()->clone() );
      mEffectWidget->setPaintEffect( mPaintEffect.get() );
    }
  }
  else
  {
    mRenderAsGroupCheckBox->setChecked( false );
  }
  mBlockLayerUpdates = false;
}

void QgsLayerTreeGroupPropertiesWidget::setDockMode( bool dockMode )
{
  QgsMapLayerConfigWidget::setDockMode( dockMode );
}

void QgsLayerTreeGroupPropertiesWidget::apply()
{
  if ( !mLayerTreeGroup )
    return;

  QgsGroupLayer *groupLayer = mLayerTreeGroup->groupLayer();
  if ( groupLayer && !mRenderAsGroupCheckBox->isChecked() )
  {
    mLayerTreeGroup->setGroupLayer( nullptr );
    QgsProject::instance()->removeMapLayer( groupLayer );
    groupLayer = nullptr;
  }
  else if ( !groupLayer && mRenderAsGroupCheckBox->isChecked() )
  {
    QgsGroupLayer::LayerOptions options( QgsProject::instance()->transformContext() );
    groupLayer = mLayerTreeGroup->convertToGroupLayer( options );
    QgsProject::instance()->addMapLayer( groupLayer, false );
  }

  if ( groupLayer )
  {
    // set the blend mode and opacity for the layer
    groupLayer->setBlendMode( mBlendModeComboBox->blendMode() );
    groupLayer->setOpacity( mOpacityWidget->opacity() );
    groupLayer->setPaintEffect( mPaintEffect->clone() );

    groupLayer->triggerRepaint();
  }
  else if ( mMapLayerConfigWidgetContext.mapCanvas() )
  {
    mMapLayerConfigWidgetContext.mapCanvas()->refresh();
  }
}

void QgsLayerTreeGroupPropertiesWidget::onLayerPropertyChanged()
{
  if ( mBlockLayerUpdates )
    return;

  emit widgetChanged();
}


//
// QgsLayerTreeGroupPropertiesWidgetFactory
//

QgsLayerTreeGroupPropertiesWidgetFactory::QgsLayerTreeGroupPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ) );
  setTitle( tr( "Symbology" ) );
}

QgsMapLayerConfigWidget *QgsLayerTreeGroupPropertiesWidgetFactory::createWidget( QgsMapLayer *, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsLayerTreeGroupPropertiesWidget( canvas, parent );
}

bool QgsLayerTreeGroupPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return false;
}

bool QgsLayerTreeGroupPropertiesWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsLayerTreeGroupPropertiesWidgetFactory::supportsLayer( QgsMapLayer * ) const
{
  return false;
}

bool QgsLayerTreeGroupPropertiesWidgetFactory::supportsLayerTreeGroup( QgsLayerTreeGroup * ) const
{
  return true;
}

