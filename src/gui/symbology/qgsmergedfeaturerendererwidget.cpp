/***************************************************************************
    qgsmergedfeaturerendererwidget.cpp
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmergedfeaturerendererwidget.h"
#include "qgsmergedfeaturerenderer.h"
#include "qgsrendererregistry.h"

#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

QgsRendererWidget *QgsMergedFeatureRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsMergedFeatureRendererWidget( layer, style, renderer );
}

QgsMergedFeatureRendererWidget::QgsMergedFeatureRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  if ( !layer )
  {
    return;
  }

  const QgsWkbTypes::GeometryType type = QgsWkbTypes::geometryType( layer->wkbType() );

  // the renderer only applies to line or polygon vector layers
  if ( type != QgsWkbTypes::PolygonGeometry && type != QgsWkbTypes::LineGeometry )
  {
    //setup blank dialog
    mRenderer.reset( nullptr );
    QGridLayout *layout = new QGridLayout( this );
    QLabel *label = new QLabel( tr( "The merged feature renderer only applies to line and polygon layers. \n"
                                    "'%1' is not a line or polygon layer and then cannot be displayed" )
                                .arg( layer->name() ), this );
    this->setLayout( layout );
    layout->addWidget( label );
    return;
  }
  setupUi( this );
  connect( mRendererComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMergedFeatureRendererWidget::mRendererComboBox_currentIndexChanged );

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer.reset( QgsMergedFeatureRenderer::convertFromRenderer( renderer ) );
  }
  if ( ! mRenderer )
  {
    // use default embedded renderer
    mRenderer.reset( new QgsMergedFeatureRenderer( QgsFeatureRenderer::defaultRenderer( type ) ) );
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  int currentEmbeddedIdx = 0;
  //insert possible renderer types
  const QStringList rendererList = QgsApplication::rendererRegistry()->renderersList( type == QgsWkbTypes::PolygonGeometry ? QgsRendererAbstractMetadata::PolygonLayer :  QgsRendererAbstractMetadata::LineLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  int idx = 0;
  mRendererComboBox->blockSignals( true );
  for ( ; it != rendererList.constEnd(); ++it, ++idx )
  {
    if ( *it != QLatin1String( "mergedFeatureRenderer" )
         && *it != QLatin1String( "invertedPolygonRenderer" ) ) //< an merged renderer cannot contain another merged or inverted renderer
    {
      QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( *it );
      mRendererComboBox->addItem( m->icon(), m->visibleName(), /* data */ *it );
      const QgsFeatureRenderer *embeddedRenderer = mRenderer->embeddedRenderer();
      if ( embeddedRenderer && embeddedRenderer->type() == m->name() )
      {
        // store the combo box index of the current renderer
        currentEmbeddedIdx = idx;
      }
    }
  }
  mRendererComboBox->blockSignals( false );

  const int oldIdx = mRendererComboBox->currentIndex();
  mRendererComboBox->setCurrentIndex( currentEmbeddedIdx );
  if ( oldIdx == currentEmbeddedIdx )
  {
    // force update
    mRendererComboBox_currentIndexChanged( currentEmbeddedIdx );
  }
}

QgsMergedFeatureRendererWidget::~QgsMergedFeatureRendererWidget() = default;

QgsFeatureRenderer *QgsMergedFeatureRendererWidget::renderer()
{
  if ( mRenderer && mEmbeddedRendererWidget )
  {
    QgsFeatureRenderer *embeddedRenderer = mEmbeddedRendererWidget->renderer();
    if ( embeddedRenderer )
    {
      mRenderer->setEmbeddedRenderer( embeddedRenderer->clone() );
    }
  }
  return mRenderer.get();
}

void QgsMergedFeatureRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setContext( context );
}

void QgsMergedFeatureRendererWidget::setDockMode( bool dockMode )
{
  QgsRendererWidget::setDockMode( dockMode );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setDockMode( dockMode );
}

void QgsMergedFeatureRendererWidget::mRendererComboBox_currentIndexChanged( int index )
{
  const QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererId );
  if ( m )
  {
    const std::unique_ptr< QgsFeatureRenderer > oldRenderer( mRenderer->embeddedRenderer()->clone() );
    mEmbeddedRendererWidget.reset( m->createRendererWidget( mLayer, mStyle, oldRenderer.get() ) );
    connect( mEmbeddedRendererWidget.get(), &QgsRendererWidget::widgetChanged, this, &QgsMergedFeatureRendererWidget::widgetChanged );
    mEmbeddedRendererWidget->setContext( mContext );
    mEmbeddedRendererWidget->disableSymbolLevels();
    mEmbeddedRendererWidget->setDockMode( this->dockMode() );
    connect( mEmbeddedRendererWidget.get(), &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

    if ( layout()->count() > 2 )
    {
      // remove the current renderer widget
      layout()->takeAt( 2 );
    }
    layout()->addWidget( mEmbeddedRendererWidget.get() );
  }
}
