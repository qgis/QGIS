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
#include "qgsspatialindex.h"

class QgsProject;
class QgsProcessingContext;
class QgsMapLayerStore;

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
     * Interprets a string as a map layer within the supplied \a context.
     *
     * The method will attempt to
     * load a layer matching the passed \a string. E.g. if the string matches a layer ID or name
     * within the context's project or temporary layer store then this layer will be returned.
     * If the string is a file path and \a allowLoadingNewLayers is true, then the layer at this
     * file path will be loaded and added to the context's temporary layer store.
     * Ownership of the layer remains with the \a context or the context's current project.
     */
    static QgsMapLayer *mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers = true );

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
     * Creates a spatial index for a layer, when
     * the settings from the supplied \a context are respected. E.g. if the
     * context is set to only use selected features, then calling this will
     * return an index containing only selected features in the layer.
     */
    static QgsSpatialIndex createSpatialIndex( QgsVectorLayer *layer, const QgsProcessingContext &context );

    /**
     * Returns a list of unique values contained in a single field in a \a layer, when
     * the settings from the supplied \a context are respected. E.g. if the
     * context is set to only use selected features, then calling this will
     * return unique values from selected features in the layer.
     */
    static QList< QVariant > uniqueValues( QgsVectorLayer *layer, int fieldIndex, const QgsProcessingContext &context );

    /**
     * Creates a feature sink ready for adding features. The \a destination specifies a destination
     * URI for the resultant layer. It may be updated in place to reflect the actual destination
     * for the layer.
     *
     * Sink parameters such as desired \a encoding, \a fields, \a geometryType and \a crs must be specified.
     *
     * If the \a encoding is not specified, the default encoding from the \a context will be used.
     *
     * If a layer is created for the feature sink, the layer will automatically be added to the \a context's
     * temporary layer store.
     *
     * The caller takes responsibility for deleting the returned sink.
     */
#ifndef SIP_RUN
    static QgsFeatureSink *createFeatureSink(
      QString &destination,
      const QString &encoding,
      const QgsFields &fields,
      QgsWkbTypes::Type geometryType,
      const QgsCoordinateReferenceSystem &crs,
      QgsProcessingContext &context ) SIP_FACTORY;
#endif

    /**
     * Creates a feature sink ready for adding features. The \a destination specifies a destination
     * URI for the resultant layer. It may be updated in place to reflect the actual destination
     * for the layer.
     *
     * Sink parameters such as desired \a encoding, \a fields, \a geometryType and \a crs must be specified.
     *
     * If the \a encoding is not specified, the default encoding from the \a context will be used.
     *
     * If a layer is created for the feature sink, the layer will automatically be added to the \a context's
     * temporary layer store.
     *
     * \note this version of the createFeatureSink() function has an API designed around use from the
     * SIP bindings. c++ code should call the other createFeatureSink() version.
     * \note available in Python bindings as createFeatureSink()
     */
    static void createFeatureSinkPython(
      QgsFeatureSink **sink SIP_OUT SIP_TRANSFERBACK,
      QString &destination SIP_INOUT,
      const QString &encoding,
      const QgsFields &fields,
      QgsWkbTypes::Type geometryType,
      const QgsCoordinateReferenceSystem &crs,
      QgsProcessingContext &context ) SIP_PYNAME( createFeatureSink );


  private:

    static bool canUseLayer( const QgsRasterLayer *layer );
    static bool canUseLayer( const QgsVectorLayer *layer,
                             const QList< QgsWkbTypes::GeometryType > &geometryTypes = QList< QgsWkbTypes::GeometryType >() );

    /**
     * Interprets a \a string as a map layer from a store.
     *
     * This method attempts to match a string to a store map layer, using
     * first the layer ID, then layer names, and finally layer source.
     * If the string matches a normalized version of any layer source
     * for layers in the specified \a store, then those matching layers will be
     * returned.
     * \see mapLayerFromString()
     */
    static QgsMapLayer *mapLayerFromStore( const QString &string, QgsMapLayerStore *store );

    /**
     * Interprets a string as a map layer. The method will attempt to
     * load a layer matching the passed \a string. E.g. if the string is a file path,
     * then the layer at this file path will be loaded.
     * The caller takes responsibility for deleting the returned map layer.
     */
    static QgsMapLayer *loadMapLayerFromString( const QString &string );

    friend class TestQgsProcessing;

};

#endif // QGSPROCESSINGUTILS_H


