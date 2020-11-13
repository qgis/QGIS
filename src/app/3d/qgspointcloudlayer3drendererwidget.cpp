/***************************************************************************
  qgspointcloudlayer3drendererwidget.cpp
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayer3drendererwidget.h"

#include "qgspointcloud3dsymbolwidget.h"
#include "qgspointcloudlayer3drenderer.h"

#include "qgspointcloudlayer.h"
#include "qgspointcloud3dsymbol.h"

#include <QBoxLayout>
#include <QCheckBox>

QgsPointCloudLayer3DRendererWidget::QgsPointCloudLayer3DRendererWidget( QgsPointCloudLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );

  // TODO: add enabled checkbox
//  mChkEnabled = new QCheckBox( tr( "Enable 3D Renderer" ), this );
//  layout->addWidget( mChkEnabled );

  mWidgetPointCloudSymbol = new QgsPointCloud3DSymbolWidget( layer, this );
  layout->addWidget( mWidgetPointCloudSymbol );

//  connect( mChkEnabled, &QCheckBox::clicked, this, &QgsPointCloudLayer3DRendererWidget::onEnabledClicked );
  connect( mWidgetPointCloudSymbol, &QgsPointCloud3DSymbolWidget::changed, this, &QgsPointCloudLayer3DRendererWidget::widgetChanged );
}

void QgsPointCloudLayer3DRendererWidget::setRenderer( const QgsPointCloudLayer3DRenderer *renderer )
{
  mRenderer.reset( renderer ? renderer->clone() : nullptr );
  if ( renderer != nullptr )
    mWidgetPointCloudSymbol->setSymbol( const_cast<QgsPointCloud3DSymbol *>( renderer->symbol() ) );
//  whileBlocking( mChkEnabled )->setChecked( renderer ? renderer->symbol()->isEnabled() : false );
}

QgsPointCloudLayer3DRenderer *QgsPointCloudLayer3DRendererWidget::renderer()
{
  // TODO: use unique_ptr for point cloud symbol
  QgsPointCloud3DSymbol *sym = mWidgetPointCloudSymbol->symbol();
  mRenderer.reset( new QgsPointCloudLayer3DRenderer );
  mRenderer->setSymbol( sym );
  delete sym;
  mRenderer->setLayer( qobject_cast<QgsPointCloudLayer *>( mLayer ) );
  return mRenderer.get();
}

void QgsPointCloudLayer3DRendererWidget::apply()
{
  QgsPointCloudLayer3DRenderer *r = renderer();
  if ( r )
    r->setSymbol( mWidgetPointCloudSymbol->symbol() );
  mLayer->setRenderer3D( r ? r->clone() : nullptr );
}

//void QgsPointCloudLayer3DRendererWidget::onEnabledClicked()
//{
//  mWidgetPointCloudSymbol->setEnabled( mChkEnabled->isChecked() );
//  emit widgetChanged();
//}

void QgsPointCloudLayer3DRendererWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = layer ;
  QgsPointCloudLayer *pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( layer );
  mWidgetPointCloudSymbol->setLayer( pointCloudLayer );
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "pointcloud" ) )
  {
    QgsPointCloudLayer3DRenderer *pointCloudRenderer = static_cast<QgsPointCloudLayer3DRenderer *>( r );
    pointCloudRenderer->setSymbol( mWidgetPointCloudSymbol->symbol() );
    setRenderer( pointCloudRenderer );
    mWidgetPointCloudSymbol->setEnabled( pointCloudRenderer->symbol()->isEnabled() );
  }
  else
  {
    setRenderer( nullptr );
    mWidgetPointCloudSymbol->setEnabled( false );
  }
}

QgsPointCloudLayer3DRendererWidgetFactory::QgsPointCloudLayer3DRendererWidgetFactory( QObject *parent ):
  QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/3d.svg" ) );
  setTitle( tr( "3D View" ) );
}

QgsMapLayerConfigWidget *QgsPointCloudLayer3DRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  QgsPointCloudLayer *pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( layer );
  if ( !pointCloudLayer )
    return nullptr;
  return new QgsPointCloudLayer3DRendererWidget( pointCloudLayer, canvas, parent );
}

bool QgsPointCloudLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPointCloudLayer3DRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::PointCloudLayer;
}

QString QgsPointCloudLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Rendering" );
}
