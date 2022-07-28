/***************************************************************************
    qgsapplayerhandling.h
    -------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPLAYERHANDLING_H
#define QGSAPPLAYERHANDLING_H

#include "qgis.h"
#include "qgsconfig.h"

class QgsMapLayer;

/**
 * Contains logic related to general layer handling in QGIS app.
 */
class QgsAppLayerHandling
{

  public:


    static void postProcessAddedLayer( QgsMapLayer *layer );



};

#endif // QGSAPPLAYERHANDLING_H
