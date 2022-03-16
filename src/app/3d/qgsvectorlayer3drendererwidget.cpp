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
#include "qgsvectorlayer3dpropertieswidget.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsvscrollarea.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>



QgsSingleSymbol3DRendererWidget::QgsSingleSymbol3DRendererWidget( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  QVBoxLayout *scrollLayout = new QVBoxLayout();
  scrollLayout->setContentsMargins( 0, 0, 0, 0 );

  QgsVScrollArea *scrollArea = new QgsVScrollArea( this );
  scrollArea->setFrameShape( QFrame::NoFrame );
  scrollArea->setFrameShadow( QFrame::Plain );
  scrollArea->setWidgetResizable( true );
  scrollLayout->addWidget( scrollArea );

  widgetSymbol = new QgsSymbol3DWidget( mLayer, this );
  scrollArea->setWidget( widgetSymbol );

  setLayout( scrollLayout );

  connect( widgetSymbol, &QgsSymbol3DWidget::widgetChanged, this, &QgsSingleSymbol3DRendererWidget::widgetChanged );
}


void QgsSingleSymbol3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "vector" ) )
  {
    QgsVectorLayer3DRenderer *vectorRenderer = static_cast<QgsVectorLayer3DRenderer *>( r );
    widgetSymbol->setSymbol( vectorRenderer->symbol(), layer );
  }
  else
  {
    const std::unique_ptr<QgsAbstract3DSymbol> sym( QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( layer->geometryType() ) );
    sym->setDefaultPropertiesFromLayer( layer );
    widgetSymbol->setSymbol( sym.get(), layer );
  }
}

std::unique_ptr<QgsAbstract3DSymbol> QgsSingleSymbol3DRendererWidget::symbol()
{
  return widgetSymbol->symbol();  // cloned or null
}

// -------

QgsVectorLayer3DRendererWidget::QgsVectorLayer3DRendererWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );

  cboRendererType = new QComboBox( this );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererNullSymbol.svg" ) ), tr( "No Symbols" ) );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererSingleSymbol.svg" ) ), tr( "Single Symbol" ) );
  cboRendererType->addItem( QgsApplication::getThemeIcon( QStringLiteral( "rendererRuleBasedSymbol.svg" ) ), tr( "Rule-based" ) );

  widgetBaseProperties = new QgsVectorLayer3DPropertiesWidget( this );

  widgetRendererStack = new QStackedWidget( this );
  layout->addWidget( cboRendererType );
  layout->addWidget( widgetRendererStack );
  layout->addWidget( widgetBaseProperties );

  widgetNoRenderer = new QLabel;
  widgetSingleSymbolRenderer = new QgsSingleSymbol3DRendererWidget( qobject_cast< QgsVectorLayer *>( layer ), this );
  widgetRuleBasedRenderer = new QgsRuleBased3DRendererWidget( this );

  widgetRendererStack->addWidget( widgetNoRenderer );
  widgetRendererStack->addWidget( widgetSingleSymbolRenderer );
  widgetRendererStack->addWidget( widgetRuleBasedRenderer );

  connect( cboRendererType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsVectorLayer3DRendererWidget::onRendererTypeChanged );
  connect( widgetSingleSymbolRenderer, &QgsSingleSymbol3DRendererWidget::widgetChanged, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetRuleBasedRenderer, &QgsRuleBased3DRendererWidget::widgetChanged, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetRuleBasedRenderer, &QgsRuleBased3DRendererWidget::showPanel, this, &QgsPanelWidget::openPanel );
  connect( widgetBaseProperties, &QgsVectorLayer3DPropertiesWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );

  syncToLayer( layer );
}


void QgsVectorLayer3DRendererWidget::syncToLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return;
  }
  mLayer = layer;

  int pageIndex;
  QgsAbstract3DRenderer *r = vlayer->renderer3D();
  if ( r && r->type() == QLatin1String( "vector" ) )
  {
    pageIndex = 1;
    widgetSingleSymbolRenderer->setLayer( vlayer );
  }
  else if ( r && r->type() == QLatin1String( "rulebased" ) )
  {
    pageIndex = 2;
    widgetRuleBasedRenderer->setLayer( vlayer );
  }
  else
  {
    pageIndex = 0;
  }
  widgetRendererStack->setCurrentIndex( pageIndex );
  whileBlocking( cboRendererType )->setCurrentIndex( pageIndex );

  if ( r && ( r->type() == QLatin1String( "vector" ) || r->type() == QLatin1String( "rulebased" ) ) )
  {
    widgetBaseProperties->load( static_cast<QgsAbstractVectorLayer3DRenderer *>( r ) );
  }
}

void QgsVectorLayer3DRendererWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  widgetRuleBasedRenderer->setDockMode( dockMode );
}


void QgsVectorLayer3DRendererWidget::apply()
{
  const int idx = widgetRendererStack->currentIndex();
  switch ( idx )
  {
    case 0:
      mLayer->setRenderer3D( nullptr );
      break;
    case 1:
    {
      std::unique_ptr< QgsAbstract3DSymbol > symbol = widgetSingleSymbolRenderer->symbol();
      QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer( symbol ? symbol.release() : nullptr );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      widgetBaseProperties->apply( r );
      mLayer->setRenderer3D( r );
    }
    break;
    case 2:
    {
      QgsRuleBased3DRenderer *r = new QgsRuleBased3DRenderer( widgetRuleBasedRenderer->rootRule()->clone() );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      widgetBaseProperties->apply( r );
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


QgsVectorLayer3DRendererWidgetFactory::QgsVectorLayer3DRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/3d.svg" ) );
  setTitle( tr( "3D View" ) );
}

QgsMapLayerConfigWidget *QgsVectorLayer3DRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  return new QgsVectorLayer3DRendererWidget( layer, canvas, parent );
}

bool QgsVectorLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsVectorLayer3DRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::VectorLayer;
}

QString QgsVectorLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Diagrams" );
}
