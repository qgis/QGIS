/***************************************************************************
                             qgsfieldchooserwidget.cpp
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


#include "qgsfieldchooserwidget.h"
#include "qgslayerchooserwidget.h"
#include "qgsvectorlayer.h"

QgsFieldChooserWidget::QgsFieldChooserWidget( QgsLayerChooserWidget* layerChooser, QObject *parent )
    : QObject( parent )
    , mLayerChooser( layerChooser )
    , mFilter( new FieldFilter() )
    , mLayer( 0 )
{
  connect( layerChooser, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( setLayer( QgsMapLayer* ) ) );
  /* do not call layerChanged here, or it will
   * cause a pure virtual function call crash
   * we can suppose that the layer will be set
   * in the chooser after the field chooser
   * has been instantiated.
   */
}

QgsFieldChooserWidget::QgsFieldChooserWidget( QObject *parent )
    : QObject( parent )
    , mLayerChooser( 0 )
    , mFilter( new FieldFilter() )
    , mLayer( 0 )
{
  /* do not call setLayer here, or it will
   * cause a pure virtual function call crash
   * setLayer must be called once the field
   *chooser has been instantiated.
   */
}

void QgsFieldChooserWidget::setFilter( FieldFilter* filter )
{
  mFilter = filter;
}

void QgsFieldChooserWidget::setLayer( QgsMapLayer* layer )
{
  if ( mLayer )
  {
    disconnect( mLayer, SIGNAL( updatedFields() ), this, SLOT( layerChanged() ) );
    disconnect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  }
  mLayer = 0;
  clearWidget();

  if ( !layer )
    return;

  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vl )
    return;

  mLayer = vl;

  connect( mLayer, SIGNAL( updatedFields() ), this, SLOT( layerChanged() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );


  const QgsFields &fields = vl->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QString fieldName = fields[idx].name();
    QString fieldAlias = vl->attributeDisplayName( idx );
    DisplayStatus display = mFilter->acceptField( idx );
    if ( display != hidden )
      addField( fieldAlias, fieldName, display );
  }

  unselect();
}

void QgsFieldChooserWidget::layerChanged()
{
  QgsMapLayer* layer;
  if ( mLayerChooser )
    layer = mLayerChooser->getLayer();
  else
    layer = mLayer;
  setLayer( layer );
}

void QgsFieldChooserWidget::layerDeleted()
{
  clearWidget();
  mLayer = 0;
}
