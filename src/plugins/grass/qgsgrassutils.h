/***************************************************************************
                              qgsgrassutils.h 
                             -------------------
    begin                : March, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSUTILS_H
#define QGSGRASSUTILS_H

#include "qgisiface.h"

/*! \class QgsGrassUtils
 *  \brief Various utilities.
 */
class QgsGrassUtils
{

public:
    //! Constructor
    QgsGrassUtils();

    //! Destructor
    ~QgsGrassUtils();

public:
    // Create vector name from map name, layer name and number of layers
    static QString vectorLayerName( QString map, QString layer, int nLayers );

    // Add all vector layers to QGIS view
    static void QgsGrassUtils::addVectorLayers ( QgisIface *iface,
        QString gisbase, QString location, QString mapset, QString map);
};

#endif // QGSGRASSUTILS_H
