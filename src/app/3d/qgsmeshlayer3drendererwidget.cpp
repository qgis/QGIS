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

#include "qgsmeshlayer.h"

#include <QBoxLayout>
#include <QCheckBox>

QgsMeshLayer3DRendererWidget::QgsMeshLayer3DRendererWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  chkEnabled = new QCheckBox( tr( "Enable 3D Renderer" ), this );
  layout->addWidget( chkEnabled );
  widgetMesh = new QgsMesh3DSymbolWidget( this );
  layout->addWidget( widgetMesh );

  connect( chkEnabled, &QCheckBox::clicked, this, &QgsMeshLayer3DRendererWidget::onEnabledClicked );
  connect( widgetMesh, &QgsMesh3DSymbolWidget::changed, this, &QgsMeshLayer3DRendererWidget::widgetChanged );
}

void QgsMeshLayer3DRendererWidget::setLayer( QgsMeshLayer *layer )
{
  mLayer = layer;

  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "mesh" ) )
  {
    QgsMeshLayer3DRenderer *meshRenderer = static_cast<QgsMeshLayer3DRenderer *>( r );
    setRenderer( meshRenderer );
  }
  else
  {
    setRenderer( nullptr );
  }
}

void QgsMeshLayer3DRendererWidget::setRenderer( const QgsMeshLayer3DRenderer *renderer )
{
  mRenderer.reset( renderer ? renderer->clone() : nullptr );

  whileBlocking( chkEnabled )->setChecked( ( bool )mRenderer );

  if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == QLatin1String( "mesh" ) )
  {
    whileBlocking( widgetMesh )->setSymbol( *static_cast<const QgsMesh3DSymbol *>( mRenderer->symbol() ), nullptr );
  }
  else
  {
    whileBlocking( widgetMesh )->setSymbol( QgsMesh3DSymbol(), nullptr );
  }
}

QgsMeshLayer3DRenderer *QgsMeshLayer3DRendererWidget::renderer()
{
  if ( chkEnabled->isChecked() )
  {
    QgsMesh3DSymbol *sym = new QgsMesh3DSymbol( widgetMesh->symbol() );
    QgsMeshLayer3DRenderer *r = new QgsMeshLayer3DRenderer( sym );
    r->setLayer( qobject_cast<QgsMeshLayer *>( mLayer ) );
    mRenderer.reset( r );
  }
  else
  {
    mRenderer.reset();
  }

  return mRenderer.get();
}

void QgsMeshLayer3DRendererWidget::apply()
{
  QgsMeshLayer3DRenderer *r = renderer();
  mLayer->setRenderer3D( r ? r->clone() : nullptr );
}

void QgsMeshLayer3DRendererWidget::onEnabledClicked()
{
  widgetMesh->setEnabled( chkEnabled->isChecked() );
  emit widgetChanged();
}
