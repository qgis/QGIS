/***************************************************************************
    qgsinvertedpolygonrendererwidget.cpp
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsinvertedpolygonrendererwidget.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgsrendererv2registry.h"

#include "qgssymbolv2.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

QgsRendererV2Widget* QgsInvertedPolygonRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsInvertedPolygonRendererWidget( layer, style, renderer );
}

QgsInvertedPolygonRendererWidget::QgsInvertedPolygonRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{
  if ( !layer )
  {
    return;
  }

  QgsWKBTypes::Type type = QgsWKBTypes::singleType( QgsWKBTypes::flatType( QGis::fromOldWkbType( layer->wkbType() ) ) );

  // the renderer only applies to polygon vector layers
  if ( type != QgsWKBTypes::Polygon && type != QgsWKBTypes::CurvePolygon )
  {
    //setup blank dialog
    mRenderer.reset( nullptr );
    QGridLayout* layout = new QGridLayout( this );
    QLabel* label = new QLabel( tr( "The inverted polygon renderer only applies to polygon and multipolygon layers. \n"
                                    "'%1' is not a polygon layer and then cannot be displayed" )
                                .arg( layer->name() ), this );
    this->setLayout( layout );
    layout->addWidget( label );
    return;
  }
  setupUi( this );

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")

  if ( renderer )
  {
    mRenderer.reset( QgsInvertedPolygonRenderer::convertFromRenderer( renderer ) );
  }
  if ( ! mRenderer )
  {
    mRenderer.reset( new QgsInvertedPolygonRenderer() );
  }
  mMergePolygonsCheckBox->blockSignals( true );
  mMergePolygonsCheckBox->setCheckState( mRenderer->preprocessingEnabled() ? Qt::Checked : Qt::Unchecked );
  mMergePolygonsCheckBox->blockSignals( false );

  int currentEmbeddedIdx = 0;
  //insert possible renderer types
  QStringList rendererList = QgsRendererV2Registry::instance()->renderersList( QgsRendererV2AbstractMetadata::PolygonLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  int idx = 0;
  mRendererComboBox->blockSignals( true );
  for ( ; it != rendererList.constEnd(); ++it, ++idx )
  {
    if ( *it != "invertedPolygonRenderer" ) //< an inverted renderer cannot contain another inverted renderer
    {
      QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( *it );
      mRendererComboBox->addItem( m->icon(), m->visibleName(), /* data */ *it );
      const QgsFeatureRendererV2* embeddedRenderer = mRenderer->embeddedRenderer();
      if ( embeddedRenderer && embeddedRenderer->type() == m->name() )
      {
        // store the combo box index of the current renderer
        currentEmbeddedIdx = idx;
      }
    }
  }
  mRendererComboBox->blockSignals( false );

  int oldIdx = mRendererComboBox->currentIndex();
  mRendererComboBox->setCurrentIndex( currentEmbeddedIdx );
  if ( oldIdx == currentEmbeddedIdx )
  {
    // force update
    on_mRendererComboBox_currentIndexChanged( currentEmbeddedIdx );
  }
}

QgsFeatureRendererV2* QgsInvertedPolygonRendererWidget::renderer()
{
  if ( mRenderer && mEmbeddedRendererWidget )
  {
    QgsFeatureRendererV2* embeddedRenderer = mEmbeddedRendererWidget->renderer();
    if ( embeddedRenderer )
    {
      mRenderer->setEmbeddedRenderer( embeddedRenderer->clone() );
    }
  }
  return mRenderer.data();
}

void QgsInvertedPolygonRendererWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  QgsRendererV2Widget::setMapCanvas( canvas );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setMapCanvas( canvas );
}

void QgsInvertedPolygonRendererWidget::on_mRendererComboBox_currentIndexChanged( int index )
{
  QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererId );
  if ( m )
  {
    mEmbeddedRendererWidget.reset( m->createRendererWidget( mLayer, mStyle, const_cast<QgsFeatureRendererV2*>( mRenderer->embeddedRenderer() )->clone() ) );
    connect( mEmbeddedRendererWidget.data(), SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
    mEmbeddedRendererWidget->setMapCanvas( mMapCanvas );

    if ( layout()->count() > 2 )
    {
      // remove the current renderer widget
      layout()->takeAt( 2 );
    }
    layout()->addWidget( mEmbeddedRendererWidget.data() );
  }
}

void QgsInvertedPolygonRendererWidget::on_mMergePolygonsCheckBox_stateChanged( int state )
{
  mRenderer->setPreprocessingEnabled( state == Qt::Checked );
  emit widgetChanged();
}
