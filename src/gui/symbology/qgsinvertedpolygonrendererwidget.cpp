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
#include "moc_qgsinvertedpolygonrendererwidget.cpp"
#include "qgsinvertedpolygonrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

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

  const Qgis::WkbType type = QgsWkbTypes::singleType( QgsWkbTypes::flatType( layer->wkbType() ) );

  // the renderer only applies to polygon vector layers
  if ( type != Qgis::WkbType::Polygon && type != Qgis::WkbType::CurvePolygon )
  {
    //setup blank dialog
    mRenderer.reset( nullptr );
    QGridLayout *layout = new QGridLayout( this );
    QLabel *label = new QLabel( tr( "The inverted polygon renderer only applies to polygon and multipolygon layers. \n"
                                    "'%1' is not a polygon layer and then cannot be displayed" )
                                  .arg( layer->name() ),
                                this );
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
  if ( !mRenderer )
  {
    mRenderer.reset( new QgsInvertedPolygonRenderer() );
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }
  mMergePolygonsCheckBox->blockSignals( true );
  mMergePolygonsCheckBox->setCheckState( mRenderer->preprocessingEnabled() ? Qt::Checked : Qt::Unchecked );
  mMergePolygonsCheckBox->blockSignals( false );

  int currentEmbeddedIdx = 0;
  //insert possible renderer types
  const QStringList rendererList = QgsApplication::rendererRegistry()->renderersList( QgsRendererAbstractMetadata::PolygonLayer );
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

  const int oldIdx = mRendererComboBox->currentIndex();
  mRendererComboBox->setCurrentIndex( currentEmbeddedIdx );
  if ( oldIdx == currentEmbeddedIdx )
  {
    // force update
    mRendererComboBox_currentIndexChanged( currentEmbeddedIdx );
  }
}

QgsInvertedPolygonRendererWidget::~QgsInvertedPolygonRendererWidget() = default;

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

void QgsInvertedPolygonRendererWidget::setDockMode( bool dockMode )
{
  QgsRendererWidget::setDockMode( dockMode );
  if ( mEmbeddedRendererWidget )
    mEmbeddedRendererWidget->setDockMode( dockMode );
}

void QgsInvertedPolygonRendererWidget::mRendererComboBox_currentIndexChanged( int index )
{
  const QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererId );
  if ( m )
  {
    const std::unique_ptr<QgsFeatureRenderer> oldRenderer( mRenderer->embeddedRenderer()->clone() );
    mEmbeddedRendererWidget.reset( m->createRendererWidget( mLayer, mStyle, oldRenderer.get() ) );
    connect( mEmbeddedRendererWidget.get(), &QgsRendererWidget::widgetChanged, this, &QgsInvertedPolygonRendererWidget::widgetChanged );
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

void QgsInvertedPolygonRendererWidget::mMergePolygonsCheckBox_stateChanged( int state )
{
  mRenderer->setPreprocessingEnabled( state == Qt::Checked );
  emit widgetChanged();
}
