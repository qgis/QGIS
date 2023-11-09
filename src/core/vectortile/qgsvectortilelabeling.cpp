/***************************************************************************
  qgsvectortilelabeling.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelabeling.h"

#include "qgsvectortilelayer.h"

QgsVectorTileLabelProvider::QgsVectorTileLabelProvider( QgsVectorTileLayer *layer )
  : QgsVectorLayerLabelProvider( Qgis::GeometryType::Unknown, QgsFields(), layer->crs(), layer->id(), nullptr, layer )
{
}
