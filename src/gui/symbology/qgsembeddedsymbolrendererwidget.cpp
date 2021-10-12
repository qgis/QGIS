/***************************************************************************
    qgsembeddedsymbolrendererwidget.cpp
    ---------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsembeddedsymbolrendererwidget.h"
#include "qgsembeddedsymbolrenderer.h"
#include "qgsrendererregistry.h"

#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

QgsRendererWidget *QgsEmbeddedSymbolRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsEmbeddedSymbolRendererWidget( layer, style, renderer );
}

QgsEmbeddedSymbolRendererWidget::QgsEmbeddedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  if ( !layer )
  {
    return;
  }

  const QgsWkbTypes::GeometryType type = QgsWkbTypes::geometryType( layer->wkbType() );

  // the renderer only applies to layers with providers supporting embedded symbols
  if ( !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::FeatureSymbology ) )
  {
    //setup blank dialog
    mRenderer.reset( nullptr );
    QGridLayout *layout = new QGridLayout( this );
    QLabel *label = new QLabel( tr( "The embedded symbols renderer can only be used with layers\n"
                                    "containing embedded styling information.\n\n"
                                    "'%1' does not contain embedded styling and cannot be displayed." )
                                .arg( layer->name() ), this );
    this->setLayout( layout );
    layout->addWidget( label );
    mDefaultSymbolToolButton = nullptr;
    return;
  }
  setupUi( this );

  mDefaultSymbolToolButton->setSymbolType( QgsSymbol::symbolTypeForGeometryType( type ) );

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer.reset( QgsEmbeddedSymbolRenderer::convertFromRenderer( renderer ) );
  }
  if ( ! mRenderer )
  {
    // use default embedded renderer
    mRenderer.reset( new QgsEmbeddedSymbolRenderer( QgsSymbol::defaultSymbol( type ) ) );
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  mDefaultSymbolToolButton->setSymbol( mRenderer->defaultSymbol()->clone() );
  mDefaultSymbolToolButton->setDialogTitle( tr( "Default symbol" ) );
  mDefaultSymbolToolButton->setLayer( mLayer );
  mDefaultSymbolToolButton->registerExpressionContextGenerator( this );

  connect( mDefaultSymbolToolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    mRenderer->setDefaultSymbol( mDefaultSymbolToolButton->symbol()->clone() );
    emit widgetChanged();
  } );
}

QgsEmbeddedSymbolRendererWidget::~QgsEmbeddedSymbolRendererWidget() = default;

QgsFeatureRenderer *QgsEmbeddedSymbolRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsEmbeddedSymbolRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( mDefaultSymbolToolButton )
  {
    mDefaultSymbolToolButton->setMapCanvas( context.mapCanvas() );
    mDefaultSymbolToolButton->setMessageBar( context.messageBar() );
  }
}

QgsExpressionContext QgsEmbeddedSymbolRendererWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  if ( QgsExpressionContext *expressionContext = mContext.expressionContext() )
    context = *expressionContext;
  else
    context.appendScopes( mContext.globalProjectAtlasMapLayerScopes( mLayer ) );

  const QList< QgsExpressionContextScope > scopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &s : scopes )
  {
    context << new QgsExpressionContextScope( s );
  }
  return context;
}

