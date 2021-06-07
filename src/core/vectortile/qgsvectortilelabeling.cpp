/***************************************************************************
  qgsvectortilelabeling.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelabeling.h"

#include "qgsvectortilelayer.h"

QgsVectorTileLabelProvider::QgsVectorTileLabelProvider( QgsVectorTileLayer *layer )
  : QgsVectorLayerLabelProvider( QgsWkbTypes::UnknownGeometry, QgsFields(), layer->crs(), layer->id(), nullptr, layer )
{
}
