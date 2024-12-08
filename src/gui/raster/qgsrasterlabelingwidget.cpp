/***************************************************************************
    qgsrasterlabelingwidget.cpp
    ---------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlabelingwidget.h"
#include "moc_qgsrasterlabelingwidget.cpp"

#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsrasterlabelsettingswidget.h"
#include "qgslabelingwidget.h"

QgsRasterLabelingWidget::QgsRasterLabelingWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent, QgsMessageBar *messageBar )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mCanvas( canvas )
  , mMessageBar( messageBar )

{
  setupUi( this );

  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingNone.svg" ) ), tr( "No Labels" ), QStringLiteral( "none" ) );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Label with Pixel Values" ), QStringLiteral( "simple" ) );

  connect( mLabelModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterLabelingWidget::labelModeChanged );
  setLayer( layer );

  connect( mLabelRulesButton, &QAbstractButton::clicked, this, &QgsRasterLabelingWidget::showLabelingEngineRulesPrivate );
  connect( mEngineSettingsButton, &QAbstractButton::clicked, this, &QgsRasterLabelingWidget::showEngineConfigDialogPrivate );

  const int iconSize16 = QgsGuiUtils::scaleIconSize( 16 );
  mEngineSettingsButton->setIconSize( QSize( iconSize16, iconSize16 ) );
  mLabelRulesButton->setIconSize( QSize( iconSize16, iconSize16 ) );
}

void QgsRasterLabelingWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );

  if ( QgsRasterLabelSettingsWidget *l = qobject_cast<QgsRasterLabelSettingsWidget *>( mWidget ) )
  {
    l->setDockMode( dockMode );
  }
}

void QgsRasterLabelingWidget::setLayer( QgsMapLayer *mapLayer )
{
  if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Raster )
  {
    setEnabled( false );
    return;
  }
  else
  {
    setEnabled( true );
  }

  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( mapLayer );
  mLayer = layer;

  adaptToLayer();
}

void QgsRasterLabelingWidget::adaptToLayer()
{
  if ( !mLayer )
    return;

  int index = -1;
  QgsAbstractRasterLayerLabeling *labeling = mLayer->labeling();
  if ( mLayer->labelsEnabled() && labeling )
  {
    index = mLabelModeComboBox->findData( labeling->type() );
    if ( QgsRasterLabelSettingsWidget *settingsWidget = qobject_cast<QgsRasterLabelSettingsWidget *>( mWidget ) )
    {
      settingsWidget->setLayer( mLayer );
      settingsWidget->setLabeling( labeling );
    }
  }
  else
  {
    index = 0;
  }

  if ( index != mLabelModeComboBox->currentIndex() )
  {
    mLabelModeComboBox->setCurrentIndex( index );
  }
}

void QgsRasterLabelingWidget::writeSettingsToLayer()
{
  const QString mode = mLabelModeComboBox->currentData().toString();
  if ( mode == QLatin1String( "simple" ) )
  {
    std::unique_ptr<QgsRasterLayerSimpleLabeling> labeling = std::make_unique<QgsRasterLayerSimpleLabeling>();
    if ( QgsRasterLabelSettingsWidget *settingsWidget = qobject_cast<QgsRasterLabelSettingsWidget *>( mWidget ) )
    {
      settingsWidget->updateLabeling( labeling.get() );
    }
    mLayer->setLabeling( labeling.release() );
    mLayer->setLabelsEnabled( true );
  }
  else
  {
    mLayer->setLabelsEnabled( false );
  }
}

void QgsRasterLabelingWidget::apply()
{
  writeSettingsToLayer();
  QgsProject::instance()->setDirty();
  // trigger refresh
  mLayer->triggerRepaint();
}

void QgsRasterLabelingWidget::labelModeChanged( int index )
{
  if ( mWidget )
    mStackedWidget->removeWidget( mWidget );

  delete mWidget;
  mWidget = nullptr;

  if ( index < 0 )
    return;

  const QString mode = mLabelModeComboBox->currentData().toString();
  if ( mode == QLatin1String( "simple" ) )
  {
    QgsSymbolWidgetContext context;
    context.setMapCanvas( mMapCanvas );
    context.setMessageBar( mMessageBar );

    QgsRasterLabelSettingsWidget *settingsWidget = new QgsRasterLabelSettingsWidget( mLayer, mCanvas, this );
    settingsWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
    settingsWidget->setContext( context );

    settingsWidget->setDockMode( dockMode() );
    connect( settingsWidget, &QgsLabelingGui::widgetChanged, this, &QgsRasterLabelingWidget::widgetChanged );

    mWidget = settingsWidget;
    if ( !dynamic_cast<QgsRasterLayerSimpleLabeling *>( mLayer->labeling() ) )
    {
      std::unique_ptr<QgsRasterLayerSimpleLabeling> labeling = std::make_unique<QgsRasterLayerSimpleLabeling>();
      settingsWidget->setLabeling( labeling.get() );
      mLayer->setLabeling( labeling.release() );
    }
    else
    {
      settingsWidget->setLabeling( mLayer->labeling() );
    }

    mStackedWidget->addWidget( mWidget );
    mStackedWidget->setCurrentWidget( mWidget );
  }

  emit widgetChanged();
}

void QgsRasterLabelingWidget::showLabelingEngineRulesPrivate()
{
  QgsLabelingWidget::showLabelingEngineRules( this, mCanvas );
}

void QgsRasterLabelingWidget::showEngineConfigDialogPrivate()
{
  QgsLabelingWidget::showEngineConfiguration( this, mCanvas );
}
