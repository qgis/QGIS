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


    static void createFeatureSinkPython(
      QgsFeatureSink **sink /Out,TransferBack/,
      QString &destination /In,Out/,
      QgsProcessingContext &context,
      const QgsFields &fields,
      QgsWkbTypes::Type geometryType,
      const QgsCoordinateReferenceSystem &crs,
      const QVariantMap &createOptions = QVariantMap() ) /PyName=createFeatureSink/;
%Docstring
 Creates a feature sink ready for adding features. The ``destination`` specifies a destination
 URI for the resultant layer. It may be updated in place to reflect the actual destination
 for the layer.

 Sink parameters such as desired ``fields``, ``geometryType`` and ``crs`` must be specified.

 The ``createOptions`` map can be used to specify additional sink creation options, which
 are passed to the underlying provider when creating new layers. Known options also
 include 'fileEncoding', which is used to specify a file encoding to use for created
 files. If 'fileEncoding' is not specified, the default encoding from the ``context`` will be used.

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

    static QVariant generateIteratingDestination( const QVariant &input, const QVariant &id, QgsProcessingContext &context );
%Docstring
 Converts an ``input`` parameter value for use in source iterating mode, where one individual sink
 is created per input feature.
 The ``id`` parameter represents the unique ID for this output, which is embedded into the resulting
 parameter value.
 :rtype: QVariant
%End

    static QString tempFolder();
%Docstring
 Returns a session specific processing temporary folder for use in processing algorithms.
.. seealso:: generateTempFileName()
 :rtype: str
%End

    static QString generateTempFilename( const QString &basename );
%Docstring
 Returns a temporary filename for a given file, putting it into
 a temporary folder (creating that folder in the process),
 but not changing the ``basename``.
.. seealso:: tempFolder()
 :rtype: str
%End

    static QString formatHelpMapAsHtml( const QVariantMap &map, const QgsProcessingAlgorithm *algorithm );
%Docstring
 Returns a HTML formatted version of the help text encoded in a variant ``map`` for
 a specified ``algorithm``.
 :rtype: str
%End

};

class QgsProcessingFeatureSource : QgsFeatureSource
{
%Docstring
 QgsFeatureSource subclass which proxies methods to an underlying QgsFeatureSource, modifying
 results according to the settings in a QgsProcessingContext.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingutils.h"
%End
  public:

    enum Flag
    {
      FlagSkipGeometryValidityChecks,
    };
    typedef QFlags<QgsProcessingFeatureSource::Flag> Flags;


    QgsProcessingFeatureSource( QgsFeatureSource *originalSource, const QgsProcessingContext &context, bool ownsOriginalSource = false );
%Docstring
 Constructor for QgsProcessingFeatureSource, accepting an original feature source ``originalSource``
 and processing ``context``.
 Ownership of ``originalSource`` is dictated by ``ownsOriginalSource``. If ``ownsOriginalSource`` is false,
 ownership is not transferred, and callers must ensure that ``originalSource`` exists for the lifetime of this object.
 If ``ownsOriginalSource`` is true, then this object will take ownership of ``originalSource``.
%End

    ~QgsProcessingFeatureSource();

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request, Flags flags ) const;
%Docstring
 Returns an iterator for the features in the source, respecting the supplied feature ``flags``.
 An optional ``request`` can be used to optimise the returned
 iterator, eg by restricting the returned attributes or geometry.
 :rtype: QgsFeatureIterator
%End

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;

    virtual QgsCoordinateReferenceSystem sourceCrs() const;

    virtual QgsFields fields() const;

    virtual QgsWkbTypes::Type wkbType() const;

    virtual long featureCount() const;

    virtual QString sourceName() const;


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
