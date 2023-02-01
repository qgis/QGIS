/***************************************************************************
  qgsmeshlayer3drendererwidget.cpp
  --------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshlayer3drendererwidget.h"

#include "qgsmesh3dsymbol.h"
#include "qgsmesh3dsymbolwidget.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsvscrollarea.h"
#include "qgsmeshlayer.h"

#include <QBoxLayout>
#include <QCheckBox>

QgsMeshLayer3DRendererWidget::QgsMeshLayer3DRendererWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );
  setObjectName( QStringLiteral( "mOptsPage_3DView" ) );

  QgsVScrollArea *scrollArea = new QgsVScrollArea( this );
  scrollArea->setFrameShape( QFrame::NoFrame );
  scrollArea->setFrameShadow( QFrame::Plain );
  scrollArea->setWidgetResizable( true );
  QVBoxLayout *scrollLayout = new QVBoxLayout( this );
  scrollLayout->setContentsMargins( 0, 0, 0, 0 );
  scrollLayout->addWidget( scrollArea );

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  QWidget *widget = new QWidget;
  widget->setLayout( layout );
  scrollArea->setWidget( widget );

  mChkEnabled = new QCheckBox( tr( "Enable 3D Renderer" ), this );
  layout->addWidget( mChkEnabled );

  mWidgetMesh = new QgsMesh3dSymbolWidget( layer, this );
  mWidgetMesh->configureForDataset();
  layout->addWidget( mWidgetMesh );

  connect( mChkEnabled, &QCheckBox::clicked, this, &QgsMeshLayer3DRendererWidget::onEnabledClicked );
  connect( mWidgetMesh, &QgsMesh3dSymbolWidget::changed, this, &QgsMeshLayer3DRendererWidget::widgetChanged );

  setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#d-view-properties" ) );
}

void QgsMeshLayer3DRendererWidget::setRenderer( const QgsMeshLayer3DRenderer *renderer )
{
  mRenderer.reset( renderer ? renderer->clone() : nullptr );
  whileBlocking( mChkEnabled )->setChecked( renderer ? renderer->symbol()->isEnabled() : false );
}

QgsMeshLayer3DRenderer *QgsMeshLayer3DRendererWidget::renderer()
{
  std::unique_ptr< QgsMesh3DSymbol > sym = mWidgetMesh->symbol();
  sym->setEnabled( mChkEnabled->isChecked() );
  mRenderer.reset( new QgsMeshLayer3DRenderer( sym.release() ) );
  mRenderer->setLayer( qobject_cast<QgsMeshLayer *>( mLayer ) );
  return mRenderer.get();
}

void QgsMeshLayer3DRendererWidget::apply()
{
  QgsMeshLayer3DRenderer *r = renderer();
  mLayer->setRenderer3D( r ? r->clone() : nullptr );
}

void QgsMeshLayer3DRendererWidget::onEnabledClicked()
{
  mWidgetMesh->setEnabled( mChkEnabled->isChecked() );
  emit widgetChanged();
}

void QgsMeshLayer3DRendererWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = layer ;
  QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( layer );
  mWidgetMesh->setLayer( meshLayer );
  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "mesh" ) )
  {
    QgsMeshLayer3DRenderer *meshRenderer = static_cast<QgsMeshLayer3DRenderer *>( r );
    setRenderer( meshRenderer );
    mWidgetMesh->setEnabled( meshRenderer->symbol()->isEnabled() );
  }
  else
  {
    setRenderer( nullptr );
    mWidgetMesh->setEnabled( false );
  }
}

QgsMeshLayer3DRendererWidgetFactory::QgsMeshLayer3DRendererWidgetFactory( QObject *parent ):
  QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/3d.svg" ) );
  setTitle( tr( "3D View" ) );
}

QgsMapLayerConfigWidget *QgsMeshLayer3DRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( layer );
  if ( !meshLayer )
    return nullptr;
  return new QgsMeshLayer3DRendererWidget( meshLayer, canvas, parent );
}

bool QgsMeshLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsMeshLayer3DRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::MeshLayer;
}

QString QgsMeshLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Rendering" );
}
