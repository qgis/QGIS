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

If the ``sort`` argument is true then the layers will be sorted by their :py:func:`QgsMapLayer.name()`
value.

.. seealso:: :py:func:`compatibleVectorLayers`

.. seealso:: :py:func:`compatibleLayers`
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

If the ``sort`` argument is true then the layers will be sorted by their :py:func:`QgsMapLayer.name()`
value.

.. seealso:: :py:func:`compatibleRasterLayers`

.. seealso:: :py:func:`compatibleLayers`
%End

    static QList< QgsMapLayer * > compatibleLayers( QgsProject *project, bool sort = true );
%Docstring
Returns a list of map layers from a ``project`` which are compatible with the processing
framework.

If the ``sort`` argument is true then the layers will be sorted by their :py:func:`QgsMapLayer.name()`
value.

.. seealso:: :py:func:`compatibleRasterLayers`

.. seealso:: :py:func:`compatibleVectorLayers`
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
%End

    static QgsProcessingFeatureSource *variantToSource( const QVariant &value, QgsProcessingContext &context, const QVariant &fallbackValue = QVariant() ) /Factory/;
%Docstring
Converts a variant ``value`` to a new feature source.

Sources will either be taken from ``context``'s active project, or loaded from external
sources and stored temporarily in the ``context``.

The optional ``fallbackValue`` can be used to specify a "default" value which is used
if ``value`` cannot be successfully converted to a source.

This function creates a new object and the caller takes responsibility for deleting the returned object.
%End

    static QString normalizeLayerSource( const QString &source );
%Docstring
Normalizes a layer ``source`` string for safe comparison across different
operating system environments.
%End

    static QString stringToPythonLiteral( const QString &string );
%Docstring
Converts a string to a Python string literal. E.g. by replacing ' with \'.
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
%End

    static QVariant generateIteratingDestination( const QVariant &input, const QVariant &id, QgsProcessingContext &context );
%Docstring
Converts an ``input`` parameter value for use in source iterating mode, where one individual sink
is created per input feature.
The ``id`` parameter represents the unique ID for this output, which is embedded into the resulting
parameter value.
%End

    static QString tempFolder();
%Docstring
Returns a session specific processing temporary folder for use in processing algorithms.

.. seealso:: :py:func:`generateTempFileName`
%End

    static QString generateTempFilename( const QString &basename );
%Docstring
Returns a temporary filename for a given file, putting it into
a temporary folder (creating that folder in the process),
but not changing the ``basename``.

.. seealso:: :py:func:`tempFolder`
%End

    static QString formatHelpMapAsHtml( const QVariantMap &map, const QgsProcessingAlgorithm *algorithm );
%Docstring
Returns a HTML formatted version of the help text encoded in a variant ``map`` for
a specified ``algorithm``.
%End

    static QString convertToCompatibleFormat( const QgsVectorLayer *layer,
        bool selectedFeaturesOnly,
        const QString &baseName,
        const QStringList &compatibleFormats,
        const QString &preferredFormat,
        QgsProcessingContext &context,
        QgsProcessingFeedback *feedback );
%Docstring
Converts a source vector ``layer`` to a file path to a vector layer of compatible format.

If the specified ``layer`` is not of the format listed in the
``compatibleFormats`` argument, then the layer will first be exported to a compatible format
in a temporary location using ``baseName``. The function will then return the path to that temporary file.

``compatibleFormats`` should consist entirely of lowercase file extensions, e.g. 'shp'.

The ``preferredFormat`` argument is used to specify to desired file extension to use when a temporary
layer export is required. This defaults to shapefiles.
%End

    static QgsFields combineFields( const QgsFields &fieldsA, const QgsFields &fieldsB );
%Docstring
Combines two field lists, avoiding duplicate field names (in a case-insensitive manner).

Duplicate field names will be altered to "name_2", "name_3", etc, finding the first
non-duplicate name.

.. note::

   Some output file formats (e.g. shapefiles) have restrictions on the maximum
length of field names, so be aware that the results of calling this method may
be truncated when saving to these formats.
%End

};

class QgsProcessingFeatureSource : QgsFeatureSource
{
%Docstring
QgsFeatureSource subclass which proxies methods to an underlying :py:class:`QgsFeatureSource`, modifying
results according to the settings in a :py:class:`QgsProcessingContext`.

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
%End

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;

    virtual QgsCoordinateReferenceSystem sourceCrs() const;

    virtual QgsFields fields() const;

    virtual QgsWkbTypes::Type wkbType() const;

    virtual long featureCount() const;

    virtual QString sourceName() const;

    virtual QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const;

    virtual QVariant minimumValue( int fieldIndex ) const;

    virtual QVariant maximumValue( int fieldIndex ) const;

    virtual QgsRectangle sourceExtent() const;

    virtual QgsFeatureIds allFeatureIds() const;


    QgsExpressionContextScope *createExpressionContextScope() const /Factory/;
%Docstring
Returns an expression context scope suitable for this source.
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
