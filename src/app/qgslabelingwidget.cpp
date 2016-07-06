/***************************************************************************
    qgslabelingwidget.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QDomElement>

#include "qgslabelingwidget.h"

#include "qgslabelengineconfigdialog.h"
#include "qgslabelinggui.h"
#include "qgsrulebasedlabelingwidget.h"
#include "qgsvectorlayerlabeling.h"
#include "qgisapp.h"

QgsLabelingWidget::QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
    : QgsMapLayerConfigWidget( layer, canvas, parent )
    , mLayer( layer )
    , mCanvas( canvas )
    , mWidget( nullptr )
{
  setupUi( this );

  connect( mEngineSettingsButton, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

  mLabelModeComboBox->setCurrentIndex( -1 );
  mLabelGui = new QgsLabelingGui( nullptr, mCanvas, nullptr, this );
  mStackedWidget->addWidget( mLabelGui );

  connect( mLabelModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( labelModeChanged( int ) ) );
  connect( mLabelGui, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
  setLayer( layer );
}

void QgsLabelingWidget::resetSettings()
{
  if ( mOldSettings.data() )
  {
    if ( mOldSettings->type() == "simple" )
    {
      mOldPalSettings.writeToLayer( mLayer );
    }
    mLayer->setLabeling( mOldSettings.take() );
  }
  setLayer( mLayer );
}


void QgsLabelingWidget::setLayer( QgsMapLayer* mapLayer )
{
  if ( !mapLayer || mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    setEnabled( false );
    return;
  }
  else
  {
    setEnabled( true );
  }

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>( mapLayer );
  mLayer = layer;
  if ( mLayer->labeling() )
  {
    QDomDocument doc;
    QDomElement oldSettings = mLayer->labeling()->save( doc );
    mOldSettings.reset( QgsAbstractVectorLayerLabeling::create( oldSettings ) );
    mOldPalSettings.readFromLayer( mLayer );
  }
  else
    mOldSettings.reset();

  adaptToLayer();
}

void QgsLabelingWidget::setDockMode( bool enabled )
{
  QgsPanelWidget::setDockMode( enabled );
  mLabelGui->setDockMode( enabled );
}

void QgsLabelingWidget::adaptToLayer()
{
  if ( !mLayer )
    return;

  QgsDebugMsg( QString( "Setting up for layer %1" ).arg( mLayer->name() ) );

  mLabelModeComboBox->setCurrentIndex( -1 );

  // pick the right mode of the layer
  if ( mLayer->labeling() && mLayer->labeling()->type() == "rule-based" )
  {
    mLabelModeComboBox->setCurrentIndex( 2 );
  }
  else
  {
    // load labeling settings from layer
    QgsPalLayerSettings lyr;
    lyr.readFromLayer( mLayer );

    // enable/disable main options based upon whether layer is being labeled
    if ( !lyr.enabled )
    {
      mLabelModeComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mLabelModeComboBox->setCurrentIndex( lyr.drawLabels ? 1 : 3 );
    }
  }
}

void QgsLabelingWidget::writeSettingsToLayer()
{
  if ( mLabelModeComboBox->currentIndex() == 2 )
  {
    qobject_cast<QgsRuleBasedLabelingWidget*>( mWidget )->writeSettingsToLayer();
  }
  else
  {
    mLabelGui->writeSettingsToLayer();
  }
}

void QgsLabelingWidget::apply()
{
  writeSettingsToLayer();
  QgisApp::instance()->markDirty();
  // trigger refresh
  mLayer->triggerRepaint();
}

void QgsLabelingWidget::labelModeChanged( int index )
{
  if ( index < 0 )
    return;

  if ( index == 2 )
  {
    if ( mWidget )
      mStackedWidget->removeWidget( mWidget );

    delete mWidget;
    mWidget = nullptr;

    QgsRuleBasedLabelingWidget* ruleWidget = new QgsRuleBasedLabelingWidget( mLayer, mCanvas, this );
    ruleWidget->setDockMode( dockMode() );
    connect( ruleWidget, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );
    connect( ruleWidget, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
    mWidget = ruleWidget;
    mStackedWidget->addWidget( mWidget );
    mStackedWidget->setCurrentWidget( mWidget );
  }
  else
  {

    if ( index == 3 )
      mLabelGui->setLabelMode( QgsLabelingGui::ObstaclesOnly );
    else
      mLabelGui->setLabelMode( static_cast< QgsLabelingGui::LabelMode >( index ) );

    mLabelGui->setLayer( mLayer );
    mStackedWidget->setCurrentWidget( mLabelGui );
  }
  emit widgetChanged();
}

void QgsLabelingWidget::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}
