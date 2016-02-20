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
#include "qgslabelingwidget.h"

#include "qgslabelengineconfigdialog.h"
#include "qgslabelinggui.h"
#include "qgsrulebasedlabelingwidget.h"
#include "qgsvectorlayerlabeling.h"
#include "qgisapp.h"

QgsLabelingWidget::QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
    : QWidget( parent )
    , mLayer( layer )
    , mCanvas( canvas )
    , mWidget( nullptr )
{
  setupUi( this );

  connect( mEngineSettingsButton, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

  mLabelModeComboBox->setCurrentIndex( -1 );

  connect( mLabelModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( labelModeChanged( int ) ) );

  adaptToLayer();
}

void QgsLabelingWidget::adaptToLayer()
{
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
    qobject_cast<QgsLabelingGui*>( mWidget )->writeSettingsToLayer();
  }
}

void QgsLabelingWidget::apply()
{
  writeSettingsToLayer();
  QgisApp::instance()->markDirty();
  // trigger refresh
  if ( mCanvas )
  {
    mCanvas->refresh();
  }
}

void QgsLabelingWidget::labelModeChanged( int index )
{
  if ( index < 0 )
    return;

  if ( index != 2 )
  {
    if ( QgsLabelingGui* widgetSimple = qobject_cast<QgsLabelingGui*>( mWidget ) )
    {
      // lighter variant - just change the mode of existing widget
      if ( index == 3 )
        widgetSimple->setLabelMode( QgsLabelingGui::ObstaclesOnly );
      else
        widgetSimple->setLabelMode( static_cast< QgsLabelingGui::LabelMode >( index ) );
      return;
    }
  }

  // in general case we need to recreate the widget

  if ( mWidget )
    mStackedWidget->removeWidget( mWidget );

  delete mWidget;
  mWidget = nullptr;

  if ( index == 2 )
  {
    mWidget = new QgsRuleBasedLabelingWidget( mLayer, mCanvas, this );
  }
  else
  {
    QgsLabelingGui* w = new QgsLabelingGui( mLayer, mCanvas, nullptr, this );

    if ( index == 3 )
      w->setLabelMode( QgsLabelingGui::ObstaclesOnly );
    else
      w->setLabelMode( static_cast< QgsLabelingGui::LabelMode >( index ) );

    w->init();
    mWidget = w;
  }

  mStackedWidget->addWidget( mWidget );
  mStackedWidget->setCurrentWidget( mWidget );
}

void QgsLabelingWidget::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}
