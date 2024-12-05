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
#include "moc_qgsmeshlabelingwidget.cpp"

#include "qgslabelinggui.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerlabeling.h"
#include "qgsmeshlayerlabelprovider.h"
#include "qgsproject.h"
#include "qgsapplication.h"

QgsMeshLabelingWidget::QgsMeshLabelingWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent, QgsMessageBar *messageBar )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mLayer( layer )
  , mCanvas( canvas )
  , mMessageBar( messageBar )

{
  setupUi( this );

  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingNone.svg" ) ), tr( "No Labels" ), ModeNone );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Labels on Vertices" ), ModeVertices );
  mLabelModeComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Labels on Faces" ), ModeFaces );

  connect( mLabelModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMeshLabelingWidget::labelModeChanged );
  setLayer( layer );
}

void QgsMeshLabelingWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );

  if ( QgsLabelingGui *l = labelingGui() )
    l->setDockMode( dockMode );
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
  Mode mode = ModeNone;
  if ( mLayer->labelsEnabled() )
  {
    if ( QgsMeshLayerSimpleLabeling *labeling = dynamic_cast<QgsMeshLayerSimpleLabeling *>( mLayer->labeling() ) )
    {
      mode = labeling->provider( mLayer )->labelFaces() ? ModeFaces : ModeVertices;
    }
  }
  mLabelModeComboBox->setCurrentIndex( mLabelModeComboBox->findData( mode ) );

  if ( QgsLabelingGui *lg = qobject_cast<QgsLabelingGui *>( mWidget ) )
  {
    lg->updateUi();
  }
}

void QgsMeshLabelingWidget::writeSettingsToLayer()
{
  const Mode mode = static_cast<Mode>( mLabelModeComboBox->currentData().toInt() );
  switch ( mode )
  {
    case ModeVertices:
    {
      mLayer->setLabeling( new QgsMeshLayerSimpleLabeling( qobject_cast<QgsLabelingGui *>( mWidget )->layerSettings(), false ) );
      mLayer->setLabelsEnabled( true );
      break;
    }

    case ModeFaces:
    {
      mLayer->setLabeling( new QgsMeshLayerSimpleLabeling( qobject_cast<QgsLabelingGui *>( mWidget )->layerSettings(), true ) );
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

  const Mode mode = static_cast<Mode>( mLabelModeComboBox->currentData().toInt() );

  switch ( mode )
  {
    case ModeVertices:
    case ModeFaces:
    {
      QgsMeshLayerSimpleLabeling *labeling = dynamic_cast<QgsMeshLayerSimpleLabeling *>( mLayer->labeling() );
      if ( labeling )
      {
        mSettings.reset( new QgsPalLayerSettings( labeling->settings() ) );
      }
      else
      {
        mSettings = std::make_unique<QgsPalLayerSettings>( QgsAbstractMeshLayerLabeling::defaultSettingsForLayer( mLayer ) );
      }

      QgsSymbolWidgetContext context;
      context.setMapCanvas( mMapCanvas );
      context.setMessageBar( mMessageBar );

      Qgis::GeometryType geomType = mode == ModeFaces ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Point;
      QgsLabelingGui *labelingGui = new QgsLabelingGui( mLayer, mCanvas, *mSettings, this, geomType );
      labelingGui->setLabelMode( QgsLabelingGui::Labels );
      labelingGui->layout()->setContentsMargins( 0, 0, 0, 0 );
      labelingGui->setContext( context );

      labelingGui->setDockMode( dockMode() );
      connect( labelingGui, &QgsLabelingGui::widgetChanged, this, &QgsMeshLabelingWidget::widgetChanged );
      connect( labelingGui, &QgsLabelingGui::auxiliaryFieldCreated, this, &QgsMeshLabelingWidget::auxiliaryFieldCreated );

      mWidget = labelingGui;


      mStackedWidget->addWidget( mWidget );
      mStackedWidget->setCurrentWidget( mWidget );
      break;
    }

    case ModeNone:
      break;
  }
  emit widgetChanged();
}
