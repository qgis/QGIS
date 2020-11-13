/***************************************************************************
    qgspointcloudrendererpropertieswidget.cpp
    ---------------------
    begin                : November 2020
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
#include "qgspointcloudrendererpropertieswidget.h"

#include "qgspointcloudrendererregistry.h"
#include "qgsapplication.h"
#include "qgssymbolwidgetcontext.h"
#include "qgspointcloudrendererwidget.h"
#include "qgspointcloudlayer.h"

static bool _initRenderer( const QString &name, QgsPointCloudRendererWidgetFunc f, const QString &iconName = QString() )
{
  QgsPointCloudRendererRegistry *reg = QgsApplication::pointCloudRendererRegistry();
  QgsPointCloudRendererAbstractMetadata *am = reg->rendererMetadata( name );
  if ( !am )
    return false;
  QgsPointCloudRendererMetadata *m = dynamic_cast<QgsPointCloudRendererMetadata *>( am );
  if ( !m )
    return false;

  m->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    m->setIcon( QgsApplication::getThemeIcon( iconName ) );
  }

  QgsDebugMsgLevel( "Set for " + name, 2 );
  return true;
}

static void _initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  //_initRenderer( QStringLiteral( "rgb" ), QgsSingleSymbolRendererWidget::create, QStringLiteral( "rendererSingleSymbol.svg" ) );

  sInitialized = true;
}

QgsPointCloudRendererPropertiesWidget::QgsPointCloudRendererPropertiesWidget( QgsPointCloudLayer *layer, QgsStyle *style, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, nullptr, parent )
  , mLayer( layer )
  , mStyle( style )
{
  setupUi( this );
  mLayerRenderingGroupBox->setSettingGroup( QStringLiteral( "layerRenderingGroupBox" ) );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  // initialize registry's widget functions
  _initRendererWidgetFunctions();

  QgsPointCloudRendererRegistry *reg = QgsApplication::pointCloudRendererRegistry();
  const QStringList renderers = reg->renderersList();
  for ( const QString &name : renderers )
  {
    if ( QgsPointCloudRendererAbstractMetadata *m = reg->rendererMetadata( name ) )
      cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  // connect layer opacity slider and spin box
  //connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::rendererChanged );

  //syncToLayer();
}


void QgsPointCloudRendererPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mMapCanvas = context.mapCanvas();
  mMessageBar = context.messageBar();
  if ( mActiveWidget )
  {
    mActiveWidget->setContext( context );
  }
}

