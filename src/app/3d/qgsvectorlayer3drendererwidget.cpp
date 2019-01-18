/***************************************************************************
  qgsvectorlayer3drendererwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer3drendererwidget.h"

#include "qgs3dutils.h"
#include "qgsrulebased3drenderer.h"
#include "qgsrulebased3drendererwidget.h"
#include "qgssymbol3dwidget.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>



QgsSingleSymbol3DRendererWidget::QgsSingleSymbol3DRendererWidget( QWidget *parent )
  : QWidget( parent )
{
  widgetSymbol = new QgsSymbol3DWidget( this );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( widgetSymbol );

  connect( widgetSymbol, &QgsSymbol3DWidget::widgetChanged, this, &QgsSingleSymbol3DRendererWidget::widgetChanged );
}


void QgsSingleSymbol3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QStringLiteral( "vector" ) )
  {
    QgsVectorLayer3DRenderer *vectorRenderer = static_cast<QgsVectorLayer3DRenderer *>( r );
    widgetSymbol->setSymbol( vectorRenderer->symbol(), layer );
  }
  else
  {
    std::unique_ptr<QgsAbstract3DSymbol> sym( Qgs3DUtils::symbolForGeometryType( layer->geometryType() ) );
    widgetSymbol->setSymbol( sym.get(), layer );
  }
}

QgsAbstract3DSymbol *QgsSingleSymbol3DRendererWidget::symbol()
{
  return widgetSymbol->symbol();  // cloned or null
}

// -------

QgsVectorLayer3DRendererWidget::QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );

  cboRendererType = new QComboBox( this );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererNullSymbol.svg" ) ), tr( "No symbols" ) );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererSingleSymbol.svg" ) ), tr( "Single symbol" ) );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererRuleBasedSymbol.svg" ) ), tr( "Rule-based" ) );

  widgetRendererStack = new QStackedWidget( this );
  layout->addWidget( cboRendererType );
  layout->addWidget( widgetRendererStack );

  widgetNoRenderer = new QLabel;
  widgetSingleSymbolRenderer = new QgsSingleSymbol3DRendererWidget( this );
  widgetRuleBasedRenderer = new QgsRuleBased3DRendererWidget( this );

  widgetRendererStack->addWidget( widgetNoRenderer );
  widgetRendererStack->addWidget( widgetSingleSymbolRenderer );
  widgetRendererStack->addWidget( widgetRuleBasedRenderer );

  connect( cboRendererType, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, &QgsVectorLayer3DRendererWidget::onRendererTypeChanged );
  connect( widgetSingleSymbolRenderer, &QgsSingleSymbol3DRendererWidget::widgetChanged, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetRuleBasedRenderer, &QgsRuleBased3DRendererWidget::widgetChanged, this, &QgsVectorLayer3DRendererWidget::widgetChanged );

  connect( widgetRuleBasedRenderer, &QgsRuleBased3DRendererWidget::showPanel, this, &QgsPanelWidget::openPanel );
}


void QgsVectorLayer3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  int pageIndex;
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "vector" ) )
  {
    pageIndex = 1;
    widgetSingleSymbolRenderer->setLayer( layer );
  }
  else if ( r && r->type() == QLatin1String( "rulebased" ) )
  {
    pageIndex = 2;
    widgetRuleBasedRenderer->setLayer( layer );
  }
  else
  {
    pageIndex = 0;
  }
  widgetRendererStack->setCurrentIndex( pageIndex );
  whileBlocking( cboRendererType )->setCurrentIndex( pageIndex );
}

void QgsVectorLayer3DRendererWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  widgetRuleBasedRenderer->setDockMode( dockMode );
}


void QgsVectorLayer3DRendererWidget::apply()
{
  int idx = widgetRendererStack->currentIndex();
  switch ( idx )
  {
    case 0:
      mLayer->setRenderer3D( nullptr );
      break;
    case 1:
    {
      QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer( widgetSingleSymbolRenderer->symbol() );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      mLayer->setRenderer3D( r );
    }
    break;
    case 2:
    {
      QgsRuleBased3DRenderer *r = new QgsRuleBased3DRenderer( widgetRuleBasedRenderer->rootRule()->clone() );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      mLayer->setRenderer3D( r );
    }
    break;
    default:
      Q_ASSERT( false );
  }
}

void QgsVectorLayer3DRendererWidget::onRendererTypeChanged( int index )
{
  widgetRendererStack->setCurrentIndex( index );
  switch ( index )
  {
    case 0:
      break;
    case 1:
      widgetSingleSymbolRenderer->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      break;
    case 2:
      widgetRuleBasedRenderer->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      break;
    default:
      Q_ASSERT( false );
  }
  emit widgetChanged();
}
