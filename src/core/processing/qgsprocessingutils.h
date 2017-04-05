/***************************************************************************
                         qgsprocessingutils.h
                         ------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGUTILS_H
#define QGSPROCESSINGUTILS_H

#include "qgis_core.h"

#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

class QgsProject;

#include <QString>

/**
 * \class QgsProcessingUtils
 * \ingroup core
 * Utility functions for use with processing classes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingUtils
{

  public:

    /**
     * Returns a list of raster layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is true then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleVectorLayers()
     * \see compatibleLayers()
     */
    static QList< QgsRasterLayer * > compatibleRasterLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of vector layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a geometryTypes list is non-empty then the layers will be sorted so that only
     * layers with geometry types included in the list will be returned. Leaving the \a geometryTypes
     * list empty will cause all vector layers, regardless of their geometry type, to be returned.
     *
     * If the \a sort argument is true then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleRasterLayers()
     * \see compatibleLayers()
     */
    static QList< QgsVectorLayer * > compatibleVectorLayers( QgsProject *project,
        const QList< QgsWkbTypes::GeometryType > &geometryTypes = QList< QgsWkbTypes::GeometryType >(),
        bool sort = true );

    /**
     * Returns a list of map layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is true then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     */
    static QList< QgsMapLayer * > compatibleLayers( QgsProject *project, bool sort = true );

  private:

    static bool canUseLayer( const QgsRasterLayer *layer );
    static bool canUseLayer( const QgsVectorLayer *layer,
                             const QList< QgsWkbTypes::GeometryType > &geometryTypes );

};

#endif // QGSPROCESSINGUTILS_H


