/***************************************************************************
                             QgsLayerChooserwidget.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslayerchoosercombo.h"
#include "qgsmaplayerregistry.h"

#include <QComboBox>


QgsLayerChooserCombo::QgsLayerChooserCombo( QObject *parent )
    : QgsLayerChooserWidget( parent )
    , mWidget( 0 )
{
}

bool QgsLayerChooserCombo::initWidget( QWidget *widget )
{
  mWidget = 0;
  QComboBox* cb = dynamic_cast<QComboBox*>( widget );
  if ( !cb )
    return false;

  connect( cb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( currentIndexChanged( int ) ) );
  mWidget = cb;

  populateLayers();
  mWidget->setCurrentIndex( -1 );

  return true;
}

QgsMapLayer* QgsLayerChooserCombo::getLayer() const
{
  QgsMapLayer* layer = 0;
  if ( !mWidget )
    return layer;

  int idx = mWidget->currentIndex();
  QString layerId = mWidget->itemData( idx ).toString();
  layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  return layer;
}

void QgsLayerChooserCombo::setLayer( QgsMapLayer* layer )
{
  if ( !mWidget )
    return;

  int idx = mWidget->findData( layer->id() );
  mWidget->setCurrentIndex( idx );
}

void QgsLayerChooserCombo::clearWidget()
{
  if ( mWidget )
    mWidget->clear();
}

void QgsLayerChooserCombo::addLayer( QgsMapLayer* layer, DisplayStatus display )
{
  if ( !mWidget )
    return;
  QString layerName = layer->name();
  mWidget->addItem( layerName, QVariant( layer->id() ) );
  if ( display == disabled )
  {
    // dirty trick to disable an item in a combo box
    int i = mWidget->count() - 1;
    QModelIndex j = mWidget->model()->index( i, 0 );
    mWidget->model()->setData( j, 0, Qt::UserRole - 1 );
  }
}

void QgsLayerChooserCombo::currentIndexChanged( int idx )
{
  if ( !mWidget )
    return;
  QString layerId = mWidget->itemData( idx ).toString();
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  emit layerChanged( layer );
}
