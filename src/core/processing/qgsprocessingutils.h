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
#include "qgsmessagelog.h"

class QgsProject;
class QgsProcessingContext;

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


    /**
     * Interprets a \a string as a map layer from a project.
     *
     * This method attempts to match a string to a project map layer, using
     * first the layer ID, then layer names, and finally layer source.
     * If the string matches a normalized version of any layer source
     * for layers in the specified \a project, then those matching layers will be
     * returned.
     * \see mapLayerFromString()
     */
    static QgsMapLayer *mapLayerFromProject( const QString &string, QgsProject *project );

    /**
     * Interprets a string as a map layer. The method will attempt to
     * load a layer matching the passed \a string. E.g. if the string is a file path,
     * then the layer at this file path will be loaded.
     * The caller takes responsibility for deleting the returned map layer.
     * \see mapLayerFromProject()
     */
    static QgsMapLayer *mapLayerFromString( const QString &string ) SIP_FACTORY;

    /**
     * Normalizes a layer \a source string for safe comparison across different
     * operating system environments.
     */
    static QString normalizeLayerSource( const QString &source );

    /**
     * Returns an iterator for the features in a \a layer, respecting
     * the settings from the supplied \a context.
     * An optional base \a request can be used to optimise the returned
     * iterator, eg by restricting the returned attributes or geometry.
     */
    static QgsFeatureIterator getFeatures( QgsVectorLayer *layer, const QgsProcessingContext &context, const QgsFeatureRequest &request = QgsFeatureRequest() );

    /**
     * Returns an approximate feature count for a \a layer, when
     * the settings from the supplied \a context are respected. E.g. if the
     * context is set to only use selected features, then calling this will
     * return the count of selected features in the layer.
     */
    static long featureCount( QgsVectorLayer *layer, const QgsProcessingContext &context );

    /**
     * Returns a list of unique values contained in a single field in a \a layer, when
     * the settings from the supplied \a context are respected. E.g. if the
     * context is set to only use selected features, then calling this will
     * return unique values from selected features in the layer.
     */
    static QList< QVariant > uniqueValues( QgsVectorLayer *layer, int fieldIndex, const QgsProcessingContext &context );

  private:

    static bool canUseLayer( const QgsRasterLayer *layer );
    static bool canUseLayer( const QgsVectorLayer *layer,
                             const QList< QgsWkbTypes::GeometryType > &geometryTypes );

};

#endif // QGSPROCESSINGUTILS_H


