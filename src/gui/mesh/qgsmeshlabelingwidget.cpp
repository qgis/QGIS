/***************************************************************************
    qgsmeshlabelingwidget.cpp
    ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
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

#include "qgsmeshlabelingwidget.h"

#include "qgslabelengineconfigdialog.h"
#include "qgslabelinggui.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerlabeling.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgslabelobstaclesettingswidget.h"

QgsMeshLabelingWidget::QgsMeshLabelingWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent, QgsMessageBar *messageBar )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mLayer( layer )
  , mCanvas( canvas )
  , mMessageBar( messageBar )

{
  setupUi( this );

  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingNone.svg" ) ), tr( "No Labels" ), ModeNone );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Labels on Vertices" ), ModeVertices );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingRuleBased.svg" ) ), tr( "Labels on Faces" ), ModeFaces );

  //connect( mEngineSettingsButton, &QAbstractButton::clicked, this, &QgsMeshLabelingWidget::showEngineConfigDialog );

  connect( mLabelModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMeshLabelingWidget::labelModeChanged );
  setLayer( layer );

  //const int iconSize16 = QgsGuiUtils::scaleIconSize( 16 );
  //mEngineSettingsButton->setIconSize( QSize( iconSize16, iconSize16 ) );
}

QgsLabelingGui *QgsMeshLabelingWidget::labelingGui()
{
  return qobject_cast<QgsLabelingGui *>( mWidget );
}

void QgsMeshLabelingWidget::resetSettings()
{
  if ( mOldSettings )
  {
    mLayer->setLabeling( mOldSettings.release() );
    mLayer->setLabelsEnabled( mOldLabelsEnabled );
  }
  setLayer( mLayer );
}

void QgsMeshLabelingWidget::setLayer( QgsMapLayer *mapLayer )
{
  if ( !mapLayer || mapLayer->type() != Qgis::LayerType::Mesh )
  {
    setEnabled( false );
    return;
  }
  else
  {
    setEnabled( true );
  }

  QgsMeshLayer *layer = qobject_cast<QgsMeshLayer *>( mapLayer );
  mLayer = layer;
  if ( mLayer->labeling() )
  {
    mOldSettings.reset( mLayer->labeling()->clone() );
  }
  else
  {
    mOldSettings.reset();
  }
  mOldLabelsEnabled = mLayer->labelsEnabled();

  adaptToLayer();
}

void QgsMeshLabelingWidget::adaptToLayer()
{
  if ( !mLayer )
    return;

  whileBlocking( mLabelModeComboBox )->setCurrentIndex( -1 );

  // pick the right mode of the layer
  if ( mLayer->labelsEnabled() && mLayer->labeling()->type() == QLatin1String( "verices" ) )
  {
    mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( ModeVertices ) );
  }
  else if ( mLayer->labelsEnabled() && mLayer->labeling()->type() == QLatin1String( "faces" ) )
  {
    const QgsPalLayerSettings lyr = mLayer->labeling()->settings();
    mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( ModeFaces ) );
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

void QgsMeshLabelingWidget::writeSettingsToLayer()
{
  const Mode mode = static_cast< Mode >( mLabelModeComboBox->currentData().toInt() );
  switch ( mode )
  {
    case ModeVertices:
    {
      mLayer->setLabeling( new QgsMeshLayerSimpleLabeling( qobject_cast<QgsLabelingGui *>( mWidget )->layerSettings() ) );
      mLayer->setLabelsEnabled( true );
      break;
    }

    case ModeFaces:
    {
      mLayer->setLabeling( new QgsMeshLayerSimpleLabeling( qobject_cast<QgsLabelingGui *>( mWidget )->layerSettings() ) );
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

void QgsMeshLabelingWidget::apply()
{
  writeSettingsToLayer();
  QgsProject::instance()->setDirty();
  // trigger refresh
  mLayer->triggerRepaint();
}

void QgsMeshLabelingWidget::labelModeChanged( int index )
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
    case ModeVertices:
    {
      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }

    case ModeFaces:
    {
      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }

    case ModeNone:
      break;
  }
  emit widgetChanged();
}

/*
void QgsMeshLabelingWidget::showEngineConfigDialog()
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
*/
