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
#include "qgsrendererregistry.h"

#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

QgsRendererWidget *QgsInvertedPolygonRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsInvertedPolygonRendererWidget( layer, style, renderer );
}

QgsInvertedPolygonRendererWidget::QgsInvertedPolygonRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  if ( !layer )
  {
    return;
  }

  QgsWkbTypes::Type type = QgsWkbTypes::singleType( QgsWkbTypes::flatType( layer->wkbType() ) );

  // the renderer only applies to polygon vector layers
  if ( type != QgsWkbTypes::Polygon && type != QgsWkbTypes::CurvePolygon )
  {
    //setup blank dialog
    mRenderer.reset( nullptr );
    QGridLayout *layout = new QGridLayout( this );
    QLabel *label = new QLabel( tr( "The inverted polygon renderer only applies to polygon and multipolygon layers. \n"
                                    "'%1' is not a polygon layer and then cannot be displayed" )
                                .arg( layer->name() ), this );
    this->setLayout( layout );
    layout->addWidget( label );
    return;
  }
  setupUi( this );
  connect( mRendererComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsInvertedPolygonRendererWidget::mRendererComboBox_currentIndexChanged );
  connect( mMergePolygonsCheckBox, &QCheckBox::stateChanged, this, &QgsInvertedPolygonRendererWidget::mMergePolygonsCheckBox_stateChanged );

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
  QStringList rendererList = QgsApplication::rendererRegistry()->renderersList( QgsRendererAbstractMetadata::PolygonLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  int idx = 0;
  mRendererComboBox->blockSignals( true );
  for ( ; it != rendererList.constEnd(); ++it, ++idx )
  {
    if ( *it != QLatin1String( "invertedPolygonRenderer" ) ) //< an inverted renderer cannot contain another inverted renderer
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

  int oldIdx = mRendererComboBox->currentIndex();
  mRendererComboBox->setCurrentIndex( currentEmbeddedIdx );
  if ( oldIdx == currentEmbeddedIdx )
  {
    // force update
    mRendererComboBox_currentIndexChanged( currentEmbeddedIdx );
  }
}

QgsFeatureRenderer *QgsInvertedPolygonRendererWidget::renderer()
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

void QgsInvertedPolygonRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setContext( context );
}

void QgsInvertedPolygonRendererWidget::mRendererComboBox_currentIndexChanged( int index )
{
  QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererId );
  if ( m )
  {
    mEmbeddedRendererWidget.reset( m->createRendererWidget( mLayer, mStyle, const_cast<QgsFeatureRenderer *>( mRenderer->embeddedRenderer() )->clone() ) );
    connect( mEmbeddedRendererWidget.get(), &QgsRendererWidget::widgetChanged, this, &QgsInvertedPolygonRendererWidget::widgetChanged );
    mEmbeddedRendererWidget->setContext( mContext );

    if ( layout()->count() > 2 )
    {
      // remove the current renderer widget
      layout()->takeAt( 2 );
    }
    layout()->addWidget( mEmbeddedRendererWidget.get() );
  }
}

void QgsInvertedPolygonRendererWidget::mMergePolygonsCheckBox_stateChanged( int state )
{
  mRenderer->setPreprocessingEnabled( state == Qt::Checked );
  emit widgetChanged();
}
