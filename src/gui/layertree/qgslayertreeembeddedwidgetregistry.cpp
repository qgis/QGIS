/***************************************************************************
  qgslayertreeembeddedwidgetregistry.cpp
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeembeddedwidgetregistry.h"


#include "qgsmaplayer.h"


class TransparencyWidgetProvider : public QgsLayerTreeEmbeddedWidgetProvider
{
  public:
    virtual QString id() const override
    {
      return "transparency";
    }

    virtual TransparencyWidget* createWidget( QgsMapLayer* layer, QMap<QString, QString> properties ) override
    {
      Q_UNUSED( properties );
      return new TransparencyWidget( layer );
    }

    virtual bool supportsLayer( QgsMapLayer *layer ) override
    {
      return layer->type() == QgsMapLayer::VectorLayer || layer->type() == QgsMapLayer::RasterLayer;
    }
};


/////


QgsLayerTreeEmbeddedWidgetRegistry*QgsLayerTreeEmbeddedWidgetRegistry::instance()
{
  static QgsLayerTreeEmbeddedWidgetRegistry* sInstance( new QgsLayerTreeEmbeddedWidgetRegistry() );
  return sInstance;
}

QgsLayerTreeEmbeddedWidgetRegistry::~QgsLayerTreeEmbeddedWidgetRegistry()
{
  Q_FOREACH ( QgsLayerTreeEmbeddedWidgetProvider* provider, mProviders )
  {
    removeProvider( provider->id() );
  }
}

QStringList QgsLayerTreeEmbeddedWidgetRegistry::providers() const
{
  return mProviders.keys();
}

QgsLayerTreeEmbeddedWidgetProvider*QgsLayerTreeEmbeddedWidgetRegistry::provider( const QString& providerId ) const
{
  return mProviders.value( providerId );
}

bool QgsLayerTreeEmbeddedWidgetRegistry::addProvider( QgsLayerTreeEmbeddedWidgetProvider* provider )
{
  if ( mProviders.contains( provider->id() ) )
    return false;

  mProviders.insert( provider->id(), provider );
  return true;
}

bool QgsLayerTreeEmbeddedWidgetRegistry::removeProvider( const QString& providerId )
{
  if ( !mProviders.contains( providerId ) )
    return false;

  delete mProviders.take( providerId );
  return true;
}

QgsLayerTreeEmbeddedWidgetRegistry::QgsLayerTreeEmbeddedWidgetRegistry()
{
  addProvider( new TransparencyWidgetProvider() );
}


/////

#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsvectorlayer.h"

TransparencyWidget::TransparencyWidget( QgsMapLayer* layer )
    : mLayer( layer )
{
  setAutoFillBackground( true ); // override the content from model
  QLabel* l = new QLabel( "Transparency", this );
  mSlider = new QSlider( Qt::Horizontal, this );
  mSlider->setRange( 0, 100 );
  QHBoxLayout* lay = new QHBoxLayout();
  lay->addWidget( l );
  lay->addWidget( mSlider );
  setLayout( lay );

  connect( mSlider, SIGNAL( valueChanged( int ) ), this, SLOT( sliderValueChanged( int ) ) );

  // init from layer
  if ( mLayer->type() == QgsMapLayer::VectorLayer )
  {
    mSlider->setValue( qobject_cast<QgsVectorLayer*>( mLayer )->layerTransparency() );
    connect( mLayer, SIGNAL( layerTransparencyChanged( int ) ), this, SLOT( layerTrChanged() ) );
  }
  else if ( mLayer->type() == QgsMapLayer::RasterLayer )
  {
    mSlider->setValue( 100 - qobject_cast<QgsRasterLayer*>( mLayer )->renderer()->opacity() * 100 );
    // TODO: there is no signal for raster layers
  }
}

QSize TransparencyWidget::sizeHint() const
{
  return QWidget::sizeHint();
  //return QSize(200,200); // horizontal seems ignored, vertical is used for spacing
}

void TransparencyWidget::sliderValueChanged( int value )
{
  if ( mLayer->type() == QgsMapLayer::VectorLayer )
  {
    qobject_cast<QgsVectorLayer*>( mLayer )->setLayerTransparency( value );
  }
  else if ( mLayer->type() == QgsMapLayer::RasterLayer )
  {
    qobject_cast<QgsRasterLayer*>( mLayer )->renderer()->setOpacity( 1 - value / 100. );
  }

  mLayer->triggerRepaint();
}

void TransparencyWidget::layerTrChanged()
{
  mSlider->blockSignals( true );
  mSlider->setValue( qobject_cast<QgsVectorLayer*>( mLayer )->layerTransparency() );
  mSlider->blockSignals( false );
}
