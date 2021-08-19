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

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include "qgsmaplayer.h"

///@cond PRIVATE

QgsLayerTreeOpacityWidget::QgsLayerTreeOpacityWidget( QgsMapLayer *layer )
  : mLayer( layer )
{
  setAutoFillBackground( true ); // override the content from model
  QLabel *l = new QLabel( tr( "Opacity" ), this );
  mSlider = new QSlider( Qt::Horizontal, this );
  mSlider->setRange( 0, 1000 );
  const int sliderW = static_cast< int >( QFontMetricsF( font() ).horizontalAdvance( 'X' ) * 16 * Qgis::UI_SCALE_FACTOR );
  mSlider->setMinimumWidth( sliderW / 2 );
  mSlider->setMaximumWidth( sliderW );
  QHBoxLayout *lay = new QHBoxLayout();
  QSpacerItem *spacerItem = new QSpacerItem( 1, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  lay->addWidget( l );
  lay->addWidget( mSlider );
  lay->addItem( spacerItem );
  setLayout( lay );

  // timer for delayed transparency update - for more responsive GUI
  mTimer = new QTimer( this );
  mTimer->setSingleShot( true );
  mTimer->setInterval( 100 );
  connect( mTimer, &QTimer::timeout, this, &QgsLayerTreeOpacityWidget::updateOpacityFromSlider );

  connect( mSlider, &QAbstractSlider::valueChanged, this, &QgsLayerTreeOpacityWidget::sliderValueChanged );

  // init from layer
  mSlider->setValue( mLayer->opacity() * 1000.0 );
  connect( mLayer, &QgsMapLayer::opacityChanged, this, &QgsLayerTreeOpacityWidget::layerTrChanged );
}

QSize QgsLayerTreeOpacityWidget::sizeHint() const
{
  return QWidget::sizeHint();
  //return QSize(200,200); // horizontal seems ignored, vertical is used for spacing
}

void QgsLayerTreeOpacityWidget::sliderValueChanged( int value )
{
  Q_UNUSED( value )

  if ( mTimer->isActive() )
    return;
  mTimer->start();
}

void QgsLayerTreeOpacityWidget::updateOpacityFromSlider()
{
  if ( !mLayer )
    return;
  const int value = mSlider->value();
  mLayer->setOpacity( value / 1000.0 );
  mLayer->triggerRepaint();
}

void QgsLayerTreeOpacityWidget::layerTrChanged()
{
  if ( !mLayer )
    return;
  mSlider->blockSignals( true );
  mSlider->setValue( mLayer->opacity() * 1000.0 );
  mSlider->blockSignals( false );
}

//

QString QgsLayerTreeOpacityWidget::Provider::id() const
{
  return QStringLiteral( "transparency" );
}

QString QgsLayerTreeOpacityWidget::Provider::name() const
{
  return tr( "Opacity slider" );
}

QgsLayerTreeOpacityWidget *QgsLayerTreeOpacityWidget::Provider::createWidget( QgsMapLayer *layer, int widgetIndex )
{
  Q_UNUSED( widgetIndex )
  return new QgsLayerTreeOpacityWidget( layer );
}

bool QgsLayerTreeOpacityWidget::Provider::supportsLayer( QgsMapLayer * )
{
  return true;
}

///@endcond
