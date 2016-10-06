/***************************************************************************
  qgslayertreeembeddedwidgetsimpl.h
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

#include "qgslayertreeembeddedwidgetsimpl.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>

#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsvectorlayer.h"


///@cond PRIVATE

QgsLayerTreeTransparencyWidget::QgsLayerTreeTransparencyWidget( QgsMapLayer* layer )
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

  // timer for delayed transparency update - for more responsive GUI
  mTimer = new QTimer( this );
  mTimer->setSingleShot( true );
  mTimer->setInterval( 100 );
  connect( mTimer, SIGNAL( timeout() ), this, SLOT( updateTransparencyFromSlider() ) );

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

QSize QgsLayerTreeTransparencyWidget::sizeHint() const
{
  return QWidget::sizeHint();
  //return QSize(200,200); // horizontal seems ignored, vertical is used for spacing
}

void QgsLayerTreeTransparencyWidget::sliderValueChanged( int value )
{
  Q_UNUSED( value );

  if ( mTimer->isActive() )
    return;
  mTimer->start();
}

void QgsLayerTreeTransparencyWidget::updateTransparencyFromSlider()
{
  int value = mSlider->value();

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

void QgsLayerTreeTransparencyWidget::layerTrChanged()
{
  mSlider->blockSignals( true );
  mSlider->setValue( qobject_cast<QgsVectorLayer*>( mLayer )->layerTransparency() );
  mSlider->blockSignals( false );
}

//

QString QgsLayerTreeTransparencyWidget::Provider::id() const
{
  return "transparency";
}

QString QgsLayerTreeTransparencyWidget::Provider::name() const
{
  return tr( "Transparency slider" );
}

QgsLayerTreeTransparencyWidget* QgsLayerTreeTransparencyWidget::Provider::createWidget( QgsMapLayer* layer, int widgetIndex )
{
  Q_UNUSED( widgetIndex );
  return new QgsLayerTreeTransparencyWidget( layer );
}

bool QgsLayerTreeTransparencyWidget::Provider::supportsLayer( QgsMapLayer* layer )
{
  return layer->type() == QgsMapLayer::VectorLayer || layer->type() == QgsMapLayer::RasterLayer;
}

///@endcond
