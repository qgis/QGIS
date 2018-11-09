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

#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"

#include "qgsline3dsymbolwidget.h"
#include "qgspoint3dsymbolwidget.h"
#include "qgspolygon3dsymbolwidget.h"
#include "qgsvectorlayer3drenderer.h"

#include "qgsvectorlayer.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QStackedWidget>

QgsVectorLayer3DRendererWidget::QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  chkEnabled = new QCheckBox( tr( "Enable 3D Renderer" ), this );
  widgetStack = new QStackedWidget( this );
  layout->addWidget( chkEnabled );
  layout->addWidget( widgetStack );

  widgetUnsupported = new QLabel( tr( "Sorry, this layer is not supported." ), this );
  widgetLine = new QgsLine3DSymbolWidget( this );
  widgetPoint = new QgsPoint3DSymbolWidget( this );
  widgetPolygon = new QgsPolygon3DSymbolWidget( this );

  widgetStack->addWidget( widgetUnsupported );
  widgetStack->addWidget( widgetLine );
  widgetStack->addWidget( widgetPoint );
  widgetStack->addWidget( widgetPolygon );

  connect( chkEnabled, &QCheckBox::clicked, this, &QgsVectorLayer3DRendererWidget::onEnabledClicked );
  connect( widgetLine, &QgsLine3DSymbolWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetPoint, &QgsPoint3DSymbolWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetPolygon, &QgsPolygon3DSymbolWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
}

void QgsVectorLayer3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "vector" ) )
  {
    QgsVectorLayer3DRenderer *vectorRenderer = static_cast<QgsVectorLayer3DRenderer *>( r );
    setRenderer( vectorRenderer );
  }
  else
  {
    setRenderer( nullptr );
  }
}

void QgsVectorLayer3DRendererWidget::setRenderer( const QgsVectorLayer3DRenderer *renderer )
{
  mRenderer.reset( renderer ? renderer->clone() : nullptr );

  whileBlocking( chkEnabled )->setChecked( ( bool )mRenderer );
  widgetLine->setEnabled( chkEnabled->isChecked() );
  widgetPoint->setEnabled( chkEnabled->isChecked() );
  widgetPolygon->setEnabled( chkEnabled->isChecked() );

  int pageIndex;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      pageIndex = 2;
      if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == QLatin1String( "point" ) )
      {
        whileBlocking( widgetPoint )->setSymbol( *static_cast<const QgsPoint3DSymbol *>( mRenderer->symbol() ) );
      }
      else
      {
        whileBlocking( widgetPoint )->setSymbol( QgsPoint3DSymbol() );
      }
      break;

    case QgsWkbTypes::LineGeometry:
      pageIndex = 1;
      if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == QLatin1String( "line" ) )
      {
        whileBlocking( widgetLine )->setSymbol( *static_cast<const QgsLine3DSymbol *>( mRenderer->symbol() ) );
      }
      else
      {
        whileBlocking( widgetLine )->setSymbol( QgsLine3DSymbol() );
      }
      break;

    case QgsWkbTypes::PolygonGeometry:
      pageIndex = 3;
      if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == QLatin1String( "polygon" ) )
      {
        whileBlocking( widgetPolygon )->setSymbol( *static_cast<const QgsPolygon3DSymbol *>( mRenderer->symbol() ), vlayer );
      }
      else
      {
        whileBlocking( widgetPolygon )->setSymbol( QgsPolygon3DSymbol(), vlayer );
      }
      break;

    default:
      pageIndex = 0;   // unsupported
      break;
  }
  widgetStack->setCurrentIndex( pageIndex );
}

QgsVectorLayer3DRenderer *QgsVectorLayer3DRendererWidget::renderer()
{
  if ( chkEnabled->isChecked() )
  {
    int pageIndex = widgetStack->currentIndex();
    if ( pageIndex == 1 || pageIndex == 2 || pageIndex == 3 )
    {
      QgsAbstract3DSymbol *sym = nullptr;
      if ( pageIndex == 1 )
        sym = new QgsLine3DSymbol( widgetLine->symbol() );
      else if ( pageIndex == 2 )
        sym = new QgsPoint3DSymbol( widgetPoint->symbol() );
      else
        sym = new QgsPolygon3DSymbol( widgetPolygon->symbol() );
      QgsVectorLayer3DRenderer *r = new QgsVectorLayer3DRenderer( sym );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      mRenderer.reset( r );
    }
    else
    {
      mRenderer.reset();
    }
  }
  else
  {
    mRenderer.reset();
  }

  return mRenderer.get();
}

void QgsVectorLayer3DRendererWidget::apply()
{
  QgsVectorLayer3DRenderer *r = renderer();
  mLayer->setRenderer3D( r ? r->clone() : nullptr );
}

void QgsVectorLayer3DRendererWidget::onEnabledClicked()
{
  widgetLine->setEnabled( chkEnabled->isChecked() );
  widgetPoint->setEnabled( chkEnabled->isChecked() );
  widgetPolygon->setEnabled( chkEnabled->isChecked() );
  emit widgetChanged();
}
