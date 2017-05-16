/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsProcessingUtils
{
%Docstring
 Utility functions for use with processing classes.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingutils.h"
%End
  public:

    static QList< QgsRasterLayer * > compatibleRasterLayers( QgsProject *project, bool sort = true );
%Docstring
 Returns a list of raster layers from a ``project`` which are compatible with the processing
 framework.

 If the ``sort`` argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
.. seealso:: compatibleVectorLayers()
.. seealso:: compatibleLayers()
 :rtype: list of QgsRasterLayer
%End

    static QList< QgsVectorLayer * > compatibleVectorLayers( QgsProject *project,
        const QList< QgsWkbTypes::GeometryType > &geometryTypes = QList< QgsWkbTypes::GeometryType >(),
        bool sort = true );
%Docstring
 Returns a list of vector layers from a ``project`` which are compatible with the processing
 framework.

 If the ``geometryTypes`` list is non-empty then the layers will be sorted so that only
 layers with geometry types included in the list will be returned. Leaving the ``geometryTypes``
 list empty will cause all vector layers, regardless of their geometry type, to be returned.

 If the ``sort`` argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
.. seealso:: compatibleRasterLayers()
.. seealso:: compatibleLayers()
 :rtype: list of QgsVectorLayer
%End

    static QList< QgsMapLayer * > compatibleLayers( QgsProject *project, bool sort = true );
%Docstring
 Returns a list of map layers from a ``project`` which are compatible with the processing
 framework.

 If the ``sort`` argument is true then the layers will be sorted by their QgsMapLayer.name()
 value.
.. seealso:: compatibleRasterLayers()
.. seealso:: compatibleVectorLayers()
 :rtype: list of QgsMapLayer
%End

    static QgsMapLayer *mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers = true );
%Docstring
 Interprets a string as a map layer within the supplied ``context``.

 The method will attempt to
 load a layer matching the passed ``string``. E.g. if the string matches a layer ID or name
 within the context's project or temporary layer store then this layer will be returned.
 If the string is a file path and ``allowLoadingNewLayers`` is true, then the layer at this
 file path will be loaded and added to the context's temporary layer store.
 Ownership of the layer remains with the ``context`` or the context's current project.
 :rtype: QgsMapLayer
%End

    static QString normalizeLayerSource( const QString &source );
%Docstring
 Normalizes a layer ``source`` string for safe comparison across different
 operating system environments.
 :rtype: str
%End

    static QgsFeatureIterator getFeatures( QgsVectorLayer *layer, const QgsProcessingContext &context, const QgsFeatureRequest &request = QgsFeatureRequest() );
%Docstring
 Returns an iterator for the features in a ``layer``, respecting
 the settings from the supplied ``context``.
 An optional base ``request`` can be used to optimise the returned
 iterator, eg by restricting the returned attributes or geometry.
 :rtype: QgsFeatureIterator
%End

    static long featureCount( QgsVectorLayer *layer, const QgsProcessingContext &context );
%Docstring
 Returns an approximate feature count for a ``layer``, when
 the settings from the supplied ``context`` are respected. E.g. if the
 context is set to only use selected features, then calling this will
 return the count of selected features in the layer.
 :rtype: long
%End

    static QgsSpatialIndex createSpatialIndex( QgsVectorLayer *layer, const QgsProcessingContext &context );
%Docstring
 Creates a spatial index for a layer, when
 the settings from the supplied ``context`` are respected. E.g. if the
 context is set to only use selected features, then calling this will
 return an index containing only selected features in the layer.
 :rtype: QgsSpatialIndex
%End

    static QList< QVariant > uniqueValues( QgsVectorLayer *layer, int fieldIndex, const QgsProcessingContext &context );
%Docstring
 Returns a list of unique values contained in a single field in a ``layer``, when
 the settings from the supplied ``context`` are respected. E.g. if the
 context is set to only use selected features, then calling this will
 return unique values from selected features in the layer.
 :rtype: list of QVariant
%End


    static void createFeatureSinkPython(
      QgsFeatureSink **sink /Out,TransferBack/,
      QString &destination /In,Out/,
      const QString &encoding,
      const QgsFields &fields,
      QgsWkbTypes::Type geometryType,
      const QgsCoordinateReferenceSystem &crs,
      QgsProcessingContext &context ) /PyName=createFeatureSink/;
%Docstring
 Creates a feature sink ready for adding features. The ``destination`` specifies a destination
 URI for the resultant layer. It may be updated in place to reflect the actual destination
 for the layer.

 Sink parameters such as desired ``encoding``, ``fields``, ``geometryType`` and ``crs`` must be specified.

 If the ``encoding`` is not specified, the default encoding from the ``context`` will be used.

 If a layer is created for the feature sink, the layer will automatically be added to the ``context``'s
 temporary layer store.

.. note::

   this version of the createFeatureSink() function has an API designed around use from the
 SIP bindings. c++ code should call the other createFeatureSink() version.
.. note::

   available in Python bindings as createFeatureSink()
%End

    static QgsRectangle combineLayerExtents( const QList< QgsMapLayer *> layers, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
 Combines the extent of several map ``layers``. If specified, the target ``crs``
 will be used to transform the layer's extent to the desired output reference system.
 :rtype: QgsRectangle
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
