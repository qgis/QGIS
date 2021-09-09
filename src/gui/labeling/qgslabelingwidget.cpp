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
#include "qgsreadwritecontext.h"
#include "qgsrulebasedlabelingwidget.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgslabelobstaclesettingswidget.h"

QgsLabelingWidget::QgsLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent, QgsMessageBar *messageBar )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mLayer( layer )
  , mCanvas( canvas )
  , mMessageBar( messageBar )

{
  setupUi( this );

  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingNone.svg" ) ), tr( "No Labels" ), ModeNone );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Single Labels" ), ModeSingle );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingRuleBased.svg" ) ), tr( "Rule-based Labeling" ), ModeRuleBased );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingObstacle.svg" ) ), tr( "Blocking" ), ModeBlocking );

  connect( mEngineSettingsButton, &QAbstractButton::clicked, this, &QgsLabelingWidget::showEngineConfigDialog );

  connect( mLabelModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLabelingWidget::labelModeChanged );
  setLayer( layer );

  const int iconSize16 = QgsGuiUtils::scaleIconSize( 16 );
  mEngineSettingsButton->setIconSize( QSize( iconSize16, iconSize16 ) );
}

QgsLabelingGui *QgsLabelingWidget::labelingGui()
{
  return qobject_cast<QgsLabelingGui *>( mWidget );
}

void QgsLabelingWidget::resetSettings()
{
  if ( mOldSettings )
  {
    mLayer->setLabeling( mOldSettings.release() );
    mLayer->setLabelsEnabled( mOldLabelsEnabled );
  }
  setLayer( mLayer );
}


void QgsLabelingWidget::setLayer( QgsMapLayer *mapLayer )
{
  if ( !mapLayer || mapLayer->type() != QgsMapLayerType::VectorLayer )
  {
    setEnabled( false );
    return;
  }
  else
  {
    setEnabled( true );
  }

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mapLayer );
  mLayer = layer;
  if ( mLayer->labeling() )
  {
    mOldSettings.reset( mLayer->labeling()->clone() );
  }
  else
    mOldSettings.reset();
  mOldLabelsEnabled = mLayer->labelsEnabled();

  adaptToLayer();
}

void QgsLabelingWidget::adaptToLayer()
{
  if ( !mLayer )
    return;

  whileBlocking( mLabelModeComboBox )->setCurrentIndex( -1 );

  // pick the right mode of the layer
  if ( mLayer->labelsEnabled() && mLayer->labeling()->type() == QLatin1String( "rule-based" ) )
  {
    mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( ModeRuleBased ) );
  }
  else if ( mLayer->labelsEnabled() && mLayer->labeling()->type() == QLatin1String( "simple" ) )
  {
    const QgsPalLayerSettings lyr = mLayer->labeling()->settings();

    mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( lyr.drawLabels ? ModeSingle : ModeBlocking ) );
  }
  else
  {
    mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( ModeNone ) );
  }

  if ( QgsLabelingGui *lg = qobject_cast<QgsLabelingGui *>( mWidget ) )
  {
    lg->updateUi();
  }
}

void QgsLabelingWidget::writeSettingsToLayer()
{
  const Mode mode = static_cast< Mode >( mLabelModeComboBox->currentData().toInt() );
  switch ( mode )
  {
    case ModeRuleBased:
    {
      const QgsRuleBasedLabeling::Rule *rootRule = qobject_cast<QgsRuleBasedLabelingWidget *>( mWidget )->rootRule();

      mLayer->setLabeling( new QgsRuleBasedLabeling( rootRule->clone() ) );
      mLayer->setLabelsEnabled( true );
      break;
    }

    case ModeSingle:
    {
      mLayer->setLabeling( new QgsVectorLayerSimpleLabeling( qobject_cast<QgsLabelingGui *>( mWidget )->layerSettings() ) );
      mLayer->setLabelsEnabled( true );
      break;
    }

    case ModeBlocking:
    {
      mLayer->setLabeling( new QgsVectorLayerSimpleLabeling( *mSimpleSettings ) );
      mLayer->setLabelsEnabled( true );
      break;
    }

    case ModeNone:
    {
      mLayer->setLabelsEnabled( false );
      break;
    }
  }
}

void QgsLabelingWidget::apply()
{
  writeSettingsToLayer();
  QgsProject::instance()->setDirty();
  // trigger refresh
  mLayer->triggerRepaint();
}

void QgsLabelingWidget::labelModeChanged( int index )
{
  if ( mWidget )
    mStackedWidget->removeWidget( mWidget );

  delete mWidget;
  mWidget = nullptr;

  if ( index < 0 )
    return;

  const Mode mode = static_cast< Mode >( mLabelModeComboBox->currentData().toInt() );

  switch ( mode )
  {
    case ModeRuleBased:
    {
      // note - QgsRuleBasedLabelingWidget handles conversion of existing non-rule based labels to rule based
      QgsRuleBasedLabelingWidget *ruleWidget = new QgsRuleBasedLabelingWidget( mLayer, mCanvas, this );
      ruleWidget->setDockMode( dockMode() );
      connect( ruleWidget, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
      connect( ruleWidget, &QgsRuleBasedLabelingWidget::widgetChanged, this, &QgsLabelingWidget::widgetChanged );
      mWidget = ruleWidget;
      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }

    case ModeSingle:
    case ModeBlocking:
    {
      mSimpleSettings.reset();
      if ( mLayer->labeling() && mLayer->labeling()->type() == QLatin1String( "simple" ) )
      {
        mSimpleSettings.reset( new QgsPalLayerSettings( mLayer->labeling()->settings() ) );
      }
      else if ( mLayer->labeling() && mLayer->labeling()->type() == QLatin1String( "rule-based" ) )
      {
        // changing from rule-based to simple labels... grab first rule, and copy settings
        const QgsRuleBasedLabeling *rl = static_cast<const QgsRuleBasedLabeling *>( mLayer->labeling() );
        if ( const QgsRuleBasedLabeling::Rule *rootRule = rl->rootRule() )
        {
          if ( const QgsRuleBasedLabeling::Rule *firstChild = rootRule->children().value( 0 ) )
          {
            if ( firstChild->settings() )
              mSimpleSettings.reset( new QgsPalLayerSettings( *firstChild->settings() ) );
          }
        }
      }

      if ( !mSimpleSettings )
      {
        mSimpleSettings = std::make_unique< QgsPalLayerSettings >( QgsAbstractVectorLayerLabeling::defaultSettingsForLayer( mLayer ) );
      }

      if ( mSimpleSettings->fieldName.isEmpty() )
        mSimpleSettings->fieldName = mLayer->displayField();

      QgsSymbolWidgetContext context;
      context.setMapCanvas( mMapCanvas );
      context.setMessageBar( mMessageBar );

      switch ( mode )
      {
        case ModeSingle:
        {
          QgsLabelingGui *simpleWidget = new QgsLabelingGui( mLayer, mCanvas, *mSimpleSettings, this );
          simpleWidget->setContext( context );

          simpleWidget->setDockMode( dockMode() );
          connect( simpleWidget, &QgsTextFormatWidget::widgetChanged, this, &QgsLabelingWidget::widgetChanged );
          connect( simpleWidget, &QgsLabelingGui::auxiliaryFieldCreated, this, &QgsLabelingWidget::auxiliaryFieldCreated );

          simpleWidget->setLabelMode( QgsLabelingGui::Labels );

          mWidget = simpleWidget;
          break;
        }
        case ModeBlocking:
        {
          QgsLabelObstacleSettingsWidget *obstacleWidget = new QgsLabelObstacleSettingsWidget( this, mLayer );
          obstacleWidget->setContext( context );
          obstacleWidget->setGeometryType( mLayer ? mLayer->geometryType() : QgsWkbTypes::UnknownGeometry );
          obstacleWidget->setDockMode( dockMode() );
          obstacleWidget->setSettings( mSimpleSettings->obstacleSettings() );
          obstacleWidget->setDataDefinedProperties( mSimpleSettings->dataDefinedProperties() );

          mSimpleSettings->obstacleSettings().setIsObstacle( true );
          mSimpleSettings->drawLabels = false;

          connect( obstacleWidget, &QgsLabelSettingsWidgetBase::changed, this, [ = ]
          {
            mSimpleSettings->setObstacleSettings( obstacleWidget->settings() );
            obstacleWidget->updateDataDefinedProperties( mSimpleSettings->dataDefinedProperties() );
            emit widgetChanged();
          } );
          connect( obstacleWidget, &QgsLabelSettingsWidgetBase::auxiliaryFieldCreated, this, &QgsLabelingWidget::auxiliaryFieldCreated );

          mWidget = obstacleWidget;
          break;
        }

        case ModeRuleBased:
        case ModeNone:
          break;
      }

      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }

    case ModeNone:
      break;
  }
  emit widgetChanged();
}

void QgsLabelingWidget::showEngineConfigDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsLabelEngineConfigWidget *widget = new QgsLabelEngineConfigWidget( mCanvas );
    connect( widget, &QgsLabelEngineConfigWidget::widgetChanged, widget, &QgsLabelEngineConfigWidget::apply );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelEngineConfigDialog dialog( mCanvas, this );
    dialog.exec();
    // reactivate button's window
    activateWindow();
  }
}
