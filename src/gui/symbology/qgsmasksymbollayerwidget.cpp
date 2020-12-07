/***************************************************************************
 qgsmasksymbollayerwidget.cpp
 ---------------------
 begin                : July 2019
 copyright            : (C) 2019 by Hugo Mercier / Oslandia
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmasksymbollayerwidget.h"
#include <QVBoxLayout>
#include "qgsmasksymbollayer.h"

#include "qgsvectorlayer.h"

QgsMaskMarkerSymbolLayerWidget::QgsMaskMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent )
  : QgsSymbolLayerWidget( parent, vl )
{
  setupUi( this );
}

void QgsMaskMarkerSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer *layer )
{
  mLayer = static_cast<QgsMaskMarkerSymbolLayer *>( layer );
}


QgsSymbolLayer *QgsMaskMarkerSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}
