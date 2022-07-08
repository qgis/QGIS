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
#include "qgsmessagelog.h"
#include "qgsspatialindex.h"
#include "qgsprocessing.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"
#include "qgsproxyfeaturesink.h"
#include "qgsremappingproxyfeaturesink.h"

class QgsMeshLayer;
class QgsPluginLayer;
class QgsProject;
class QgsProcessingContext;
class QgsMapLayerStore;
class QgsProcessingFeedback;
class QgsProcessingFeatureSource;
class QgsProcessingAlgorithm;
class QgsVectorTileLayer;
class QgsPointCloudLayer;
class QgsAnnotationLayer;

#include <QString>
#include <QVariant>

/**
 * \class QgsProcessingUtils
 * \ingroup core
 * \brief Utility functions for use with processing classes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingUtils
{
  public:

    /**
     * Returns a list of raster layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleVectorLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePluginLayers()
     * \see compatiblePointCloudLayers()
     * \see compatibleAnnotationLayers()
     * \see compatibleLayers()
     */
    static QList< QgsRasterLayer * > compatibleRasterLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of vector layers from a \a project which are compatible with the processing
     * framework.
     *
     * The \a sourceTypes list should be filled with a list of QgsProcessing::SourceType values.
     * If the \a sourceTypes list is non-empty then the layers will be sorted so that only
     * layers with the specified source type included in the list will be returned. Leaving the \a sourceTypes
     * list empty will cause all vector layers, regardless of their geometry type, to be returned.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleRasterLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePluginLayers()
     * \see compatiblePointCloudLayers()
     * \see compatibleAnnotationLayers()
     * \see compatibleLayers()
     */
    static QList< QgsVectorLayer * > compatibleVectorLayers( QgsProject *project,
        const QList< int > &sourceTypes = QList< int >(),
        bool sort = true );

    /**
     * Returns a list of mesh layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     *
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     * \see compatiblePluginLayers()
     * \see compatiblePointCloudLayers()
     * \see compatibleAnnotationLayers()
     * \see compatibleLayers()
     *
     * \since QGIS 3.6
     */
    static QList<QgsMeshLayer *> compatibleMeshLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of plugin layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     *
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePointCloudLayers()
     * \see compatibleAnnotationLayers()
     * \see compatibleLayers()
     *
     * \since QGIS 3.22
     */
    static QList<QgsPluginLayer *> compatiblePluginLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of point cloud layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     *
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePluginLayers()
     * \see compatibleAnnotationLayers()
     * \see compatibleLayers()
     *
     * \since QGIS 3.22
     */
    static QList<QgsPointCloudLayer *> compatiblePointCloudLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of annotation layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     *
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePluginLayers()
     * \see compatiblePointCloudLayers()
     * \see compatibleLayers()
     *
     * \since QGIS 3.22
     */
    static QList<QgsAnnotationLayer *> compatibleAnnotationLayers( QgsProject *project, bool sort = true );

    /**
     * Returns a list of map layers from a \a project which are compatible with the processing
     * framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     */
    static QList< QgsMapLayer * > compatibleLayers( QgsProject *project, bool sort = true );

    /**
     * Encodes a provider key and layer \a uri to a single string, for use with
     * decodeProviderKeyAndUri()
     *
     * \since QGIS 3.14
     */
    static QString encodeProviderKeyAndUri( const QString &providerKey, const QString &uri );

    /**
     * Decodes a provider key and layer \a uri from an encoded string, for use with
     * encodeProviderKeyAndUri()
     *
     * \param string encoded string, as returned by encodeProviderKeyAndUri()
     * \param providerKey ID key for corresponding data provider
     * \param uri decoded layer uri
     * \returns TRUE if \a string was successfully decoded
     *
     * \since QGIS 3.14
     */
    static bool decodeProviderKeyAndUri( const QString &string, QString &providerKey SIP_OUT, QString &uri SIP_OUT );

    /**
     * Layer type hints.
     * \since QGIS 3.4
     */
    enum class LayerHint SIP_MONKEYPATCH_SCOPEENUM : int
    {
      UnknownType, //!< Unknown layer type
      Vector, //!< Vector layer type
      Raster, //!< Raster layer type
      Mesh, //!< Mesh layer type, since QGIS 3.6
      PointCloud, //!< Point cloud layer type, since QGIS 3.22
      Annotation, //!< Annotation layer type, since QGIS 3.22
    };

    /**
     * Interprets a string as a map layer within the supplied \a context.
     *
     * The method will attempt to
     * load a layer matching the passed \a string. E.g. if the string matches a layer ID or name
     * within the context's project or temporary layer store then this layer will be returned.
     * If the string is a file path and \a allowLoadingNewLayers is TRUE, then the layer at this
     * file path will be loaded and added to the context's temporary layer store.
     * Ownership of the layer remains with the \a context or the context's current project.
     *
     * The \a typeHint can be used to dictate the type of map layer expected.
     */
    static QgsMapLayer *mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers = true, QgsProcessingUtils::LayerHint typeHint = QgsProcessingUtils::LayerHint::UnknownType );

    /**
     * Converts a variant \a value to a new feature source.
     *
     * Sources will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context.
     *
     * The optional \a fallbackValue can be used to specify a "default" value which is used
     * if \a value cannot be successfully converted to a source.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    static QgsProcessingFeatureSource *variantToSource( const QVariant &value, QgsProcessingContext &context, const QVariant &fallbackValue = QVariant() ) SIP_FACTORY;

    /**
     * Converts a variant \a value to a coordinate reference system.
     *
     * The optional \a fallbackValue can be used to specify a "default" value which is used
     * if \a value cannot be successfully converted to a CRS.
     *
     * \since QGIS 3.12
     */
    static QgsCoordinateReferenceSystem variantToCrs( const QVariant &value, QgsProcessingContext &context, const QVariant &fallbackValue = QVariant() );

    /**
     * Normalizes a layer \a source string for safe comparison across different
     * operating system environments.
     */
    static QString normalizeLayerSource( const QString &source );

    /**
     * Converts a variant to a Python literal.
     *
     * \see stringToPythonLiteral()
     * \since QGSIS 3.6
     */
    static QString variantToPythonLiteral( const QVariant &value );

    /**
     * Converts a string to a Python string literal. E.g. by replacing ' with \'.
     *
     * \see variantToPythonLiteral()
     */
    static QString stringToPythonLiteral( const QString &string );

    /**
     * Creates a feature sink ready for adding features. The \a destination specifies a destination
     * URI for the resultant layer. It may be updated in place to reflect the actual destination
     * for the layer.
     *
     * Sink parameters such as desired \a fields, \a geometryType and \a crs must be specified.
     *
     * The \a createOptions map can be used to specify additional sink creation options, which
     * are passed to the underlying provider when creating new layers. Known options also
     * include 'fileEncoding', which is used to specify a file encoding to use for created
     * files. If 'fileEncoding' is not specified, the default encoding from the \a context will be used.
     *
     * If a layer is created for the feature sink, the layer will automatically be added to the \a context's
     * temporary layer store.
     *
     * The caller takes responsibility for deleting the returned sink.
     */
#ifndef SIP_RUN
    static QgsFeatureSink *createFeatureSink( QString &destination,
        QgsProcessingContext &context,
        const QgsFields &fields,
        QgsWkbTypes::Type geometryType,
        const QgsCoordinateReferenceSystem &crs,
        const QVariantMap &createOptions = QVariantMap(),
        const QStringList &datasourceOptions = QStringList(),
        const QStringList &layerOptions = QStringList(),
        QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(),
        QgsRemappingSinkDefinition *remappingDefinition = nullptr ) SIP_FACTORY;
#endif

    /**
     * Creates a feature sink ready for adding features. The \a destination specifies a destination
     * URI for the resultant layer. It may be updated in place to reflect the actual destination
     * for the layer.
     *
     * Sink parameters such as desired \a fields, \a geometryType and \a crs must be specified.
     *
     * The \a createOptions map can be used to specify additional sink creation options, which
     * are passed to the underlying provider when creating new layers. Known options also
     * include 'fileEncoding', which is used to specify a file encoding to use for created
     * files. If 'fileEncoding' is not specified, the default encoding from the \a context will be used.
     *
     * If a layer is created for the feature sink, the layer will automatically be added to the \a context's
     * temporary layer store.
     *
     * \note this version of the createFeatureSink() function has an API designed around use from the
     * SIP bindings. c++ code should call the other createFeatureSink() version.
     * \note available in Python bindings as createFeatureSink()
     */
    static void createFeatureSinkPython( QgsFeatureSink **sink SIP_OUT SIP_TRANSFERBACK, QString &destination SIP_INOUT, QgsProcessingContext &context, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, const QVariantMap &createOptions = QVariantMap() ) SIP_THROW( QgsProcessingException ) SIP_PYNAME( createFeatureSink );


    /**
     * Combines the extent of several map \a layers. If specified, the target \a crs
     * will be used to transform the layer's extent to the desired output reference system
     * using the specified \a context.
     * \since QGIS 3.8
     */
    static QgsRectangle combineLayerExtents( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context );

    /**
     * Combines the extent of several map \a layers. If specified, the target \a crs
     * will be used to transform the layer's extent to the desired output reference system.
     * \deprecated Use version with QgsProcessingContext argument instead
     */
    Q_DECL_DEPRECATED static QgsRectangle combineLayerExtents( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) SIP_DEPRECATED;

    /**
     * Converts an \a input parameter value for use in source iterating mode, where one individual sink
     * is created per input feature.
     * The \a id parameter represents the unique ID for this output, which is embedded into the resulting
     * parameter value.
     */
    static QVariant generateIteratingDestination( const QVariant &input, const QVariant &id, QgsProcessingContext &context );

    /**
     * Returns a session specific processing temporary folder for use in processing algorithms.
     * \see generateTempFilename()
     */
    static QString tempFolder();

    /**
     * Returns a temporary filename for a given file, putting it into
     * a temporary folder (creating that folder in the process),
     * but not changing the \a basename.
     * \see tempFolder()
     */
    static QString generateTempFilename( const QString &basename );

    /**
     * Returns a HTML formatted version of the help text encoded in a variant \a map for
     * a specified \a algorithm.
     */
    static QString formatHelpMapAsHtml( const QVariantMap &map, const QgsProcessingAlgorithm *algorithm );

    /**
     * Converts a source vector \a layer to a file path of a vector layer of compatible format.
     *
     * If the specified \a layer is not of the format listed in the
     * \a compatibleFormats argument, then the layer will first be exported to a compatible format
     * in a temporary location using \a baseName. The function will then return the path to that temporary file.
     *
     * \a compatibleFormats should consist entirely of lowercase file extensions, e.g. 'shp'.
     *
     * The \a preferredFormat argument is used to specify to desired file extension to use when a temporary
     * layer export is required. This defaults to shapefiles.
     *
     * The \a featureLimit argument can be used to specify a limit on the number of features read from the layer.
     *
     * When an algorithm is capable of handling multi-layer input files (such as Geopackage), it is preferable
     * to use convertToCompatibleFormatAndLayerName() which may avoid conversion in more situations.
     *
     * \see convertToCompatibleFormatAndLayerName()
     */
    static QString convertToCompatibleFormat( const QgsVectorLayer *layer,
        bool selectedFeaturesOnly,
        const QString &baseName,
        const QStringList &compatibleFormats,
        const QString &preferredFormat,
        QgsProcessingContext &context,
        QgsProcessingFeedback *feedback, long long featureLimit = -1 );

    /**
     * Converts a source vector \a layer to a file path and layer name of a vector layer of compatible format.
     *
     * If the specified \a layer is not of the format listed in the
     * \a compatibleFormats argument, then the layer will first be exported to a compatible format
     * in a temporary location using \a baseName. The function will then return the path to that temporary file.
     *
     * \a compatibleFormats should consist entirely of lowercase file extensions, e.g. 'shp'.
     *
     * The \a featureLimit argument can be used to specify a limit on the number of features read from the layer.
     *
     * The \a preferredFormat argument is used to specify to desired file extension to use when a temporary
     * layer export is required. This defaults to shapefiles.
     *
     * This method should be preferred over convertToCompatibleFormat() when an algorithm is able
     * to correctly handle files with multiple layers. Unlike convertToCompatibleFormat(), it will not force
     * a conversion in this case and will return the target layer name in the \a layerName argument.
     *
     * \param layer source layer to convert (if required)
     * \param selectedFeaturesOnly TRUE if only selected features from the layer should be used
     * \param baseName base file name for converted layer, if required
     * \param compatibleFormats a list of lowercase file extensions compatible with the algorithm
     * \param preferredFormat preferred format extension to use if conversion if required
     * \param context processing context
     * \param feedback feedback object
     * \param layerName will be set to the target layer name for multi-layer sources (e.g. Geopackage)
     * \param featureLimit can be used to place a limit on the maximum number of features read from the layer
     *
     * \returns path to source layer, or nearly converted compatible layer
     *
     * \see convertToCompatibleFormat()
     * \since QGIS 3.10
     */
    static QString convertToCompatibleFormatAndLayerName( const QgsVectorLayer *layer,
        bool selectedFeaturesOnly,
        const QString &baseName,
        const QStringList &compatibleFormats,
        const QString &preferredFormat,
        QgsProcessingContext &context,
        QgsProcessingFeedback *feedback,
        QString &layerName SIP_OUT, long long featureLimit = -1 );

    /**
     * Combines two field lists, avoiding duplicate field names (in a case-insensitive manner).
     *
     * Duplicate field names will be altered to "name_2", "name_3", etc, finding the first
     * non-duplicate name.
     *
     * \note Some output file formats (e.g. shapefiles) have restrictions on the maximum
     * length of field names, so be aware that the results of calling this method may
     * be truncated when saving to these formats.
     */
    static QgsFields combineFields( const QgsFields &fieldsA, const QgsFields &fieldsB, const QString &fieldsBPrefix = QString() );

    /**
     * Returns a list of field indices parsed from the given list of field names. Unknown field names are ignored.
     * If the list of field names is empty, it is assumed that all fields are required.
     * \since QGIS 3.2
     */
    static QList<int> fieldNamesToIndices( const QStringList &fieldNames, const QgsFields &fields );

    /**
     * Returns a subset of fields based on the indices of desired fields.
     * \since QGIS 3.2
     */
    static QgsFields indicesToFields( const QList<int> &indices, const QgsFields &fields );

    /**
     * Returns the default vector extension to use, in the absence of all other constraints (e.g.
     * provider based support for extensions).
     *
     * This method returns the user-set default extension from the processing settings, or
     * a fallback value of "gpkg".
     *
     * \see defaultRasterExtension()
     * \see defaultPointCloudExtension()
     * \since QGIS 3.10
     */
    static QString defaultVectorExtension();

    /**
     * Returns the default raster extension to use, in the absence of all other constraints (e.g.
     * provider based support for extensions).
     *
     * This method returns the user-set default extension from the processing settings, or
     * a fallback value of "tif".
     *
     * \see defaultVectorExtension()
     * \see defaultPointCloudExtension()
     * \since QGIS 3.10
     */
    static QString defaultRasterExtension();

    /**
     * Returns the default point cloud extension to use, in the absence of all other constraints (e.g.
     * provider based support for extensions).
     *
     * This method returns a fallback value of "las".
     *
     * \see defaultVectorExtension()
     * \see defaultRasterExtension()
     * \since QGIS 3.24
     */
    static QString defaultPointCloudExtension();

    /**
     * Removes any raw pointer values from an input \a map, replacing them with
     * appropriate string values where possible.
     *
     * \since QGIS 3.26
     */
    static QVariantMap removePointerValuesFromMap( const QVariantMap &map );

  private:
    static bool canUseLayer( const QgsRasterLayer *layer );
    static bool canUseLayer( const QgsMeshLayer *layer );
    static bool canUseLayer( const QgsPluginLayer *layer );
    static bool canUseLayer( const QgsVectorTileLayer *layer );
    static bool canUseLayer( const QgsPointCloudLayer *layer );
    static bool canUseLayer( const QgsAnnotationLayer *layer );
    static bool canUseLayer( const QgsVectorLayer *layer,
                             const QList< int > &sourceTypes = QList< int >() );

    /**
     * Returns a list of map layers with the given layer type from a \a project which are compatible
     * with the processing framework.
     *
     * If the \a sort argument is TRUE then the layers will be sorted by their QgsMapLayer::name()
     * value.
     * \see compatibleRasterLayers()
     * \see compatibleVectorLayers()
     * \see compatibleMeshLayers()
     * \see compatiblePluginLayers()
     * \since QGIS 3.22
     */
    template< typename T> static QList< T * > compatibleMapLayers( QgsProject *project, bool sort = true );

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
    static QgsMapLayer *mapLayerFromStore( const QString &string, QgsMapLayerStore *store, QgsProcessingUtils::LayerHint typeHint = QgsProcessingUtils::LayerHint::UnknownType );

    /**
     * Interprets a string as a map layer. The method will attempt to
     * load a layer matching the passed \a string using the given coordinate
     * \a transformContext.
     * E.g. if the string is a file path,
     * then the layer at this file path will be loaded.
     * The caller takes responsibility for deleting the returned map layer.
     *
     * \since QGIS 3.8
     */
    static QgsMapLayer *loadMapLayerFromString( const QString &string, const QgsCoordinateTransformContext &transformContext, LayerHint typeHint = LayerHint::UnknownType );

    /**
     * Interprets a string as a map layer. The method will attempt to
     * load a layer matching the passed \a string. E.g. if the string is a file path,
     * then the layer at this file path will be loaded.
     * The caller takes responsibility for deleting the returned map layer.
     *
     * \deprecated use mapLayerFromString() that takes QgsCoordinateTransformContext as an argument instead
     */
    Q_DECL_DEPRECATED static QgsMapLayer *loadMapLayerFromString( const QString &string, LayerHint typeHint = LayerHint::UnknownType ) SIP_DEPRECATED ;

    static void parseDestinationString( QString &destination, QString &providerKey, QString &uri, QString &layerName, QString &format, QMap<QString, QVariant> &options, bool &useWriter, QString &extension );

    friend class TestQgsProcessing;
    friend class QgsProcessingProvider;

};

/**
 * \class QgsProcessingFeatureSource
 * \ingroup core
 * \brief QgsFeatureSource subclass which proxies methods to an underlying QgsFeatureSource, modifying
 * results according to the settings in a QgsProcessingContext.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingFeatureSource : public QgsFeatureSource
{
  public:

    //! Flags controlling how QgsProcessingFeatureSource fetches features
    enum Flag
    {
      FlagSkipGeometryValidityChecks = 1 << 1, //!< Invalid geometry checks should always be skipped. This flag can be useful for algorithms which always require invalid geometries, regardless of any user settings (e.g. "repair geometry" type algorithms).
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingFeatureSource, accepting an original feature source \a originalSource
     * and processing \a context.
     * Ownership of \a originalSource is dictated by \a ownsOriginalSource. If \a ownsOriginalSource is FALSE,
     * ownership is not transferred, and callers must ensure that \a originalSource exists for the lifetime of this object.
     * If \a ownsOriginalSource is TRUE, then this object will take ownership of \a originalSource.
     *
     * If \a featureLimit is set to a value > 0, then a limit is placed on the maximum number of features which will be
     * read from the source.
     */
    QgsProcessingFeatureSource( QgsFeatureSource *originalSource, const QgsProcessingContext &context, bool ownsOriginalSource = false,
                                long long featureLimit = -1 );

    ~QgsProcessingFeatureSource() override;

    /**
     * Returns an iterator for the features in the source, respecting the supplied feature \a flags.
     * An optional \a request can be used to optimise the returned
     * iterator, eg by restricting the returned attributes or geometry.
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request, Flags flags ) const;

    QgsFeatureSource::FeatureAvailability hasFeatures() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;
    QgsCoordinateReferenceSystem sourceCrs() const override;
    QgsFields fields() const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QString sourceName() const override;
    QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const override;
    QVariant minimumValue( int fieldIndex ) const override;
    QVariant maximumValue( int fieldIndex ) const override;
    QgsRectangle sourceExtent() const override;
    QgsFeatureIds allFeatureIds() const override;
    SpatialIndexPresence hasSpatialIndex() const override;

    /**
     * Returns an expression context scope suitable for this source.
     */
    QgsExpressionContextScope *createExpressionContextScope() const SIP_FACTORY;

    /**
     * Overrides the default geometry check method for the source.
     *
     * \since QGIS 3.14
     */
    void setInvalidGeometryCheck( QgsFeatureRequest::InvalidGeometryCheck method );

  private:

    QgsFeatureSource *mSource = nullptr;
    bool mOwnsSource = false;
    QgsFeatureRequest::InvalidGeometryCheck mInvalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    std::function< void( const QgsFeature & ) > mTransformErrorCallback;

    std::function< void( const QgsFeature & ) > mInvalidGeometryCallbackSkip;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallbackAbort;

    long long mFeatureLimit = -1;

};

#ifndef SIP_RUN

/**
 * \class QgsProcessingFeatureSink
 * \ingroup core
 * \brief QgsProxyFeatureSink subclass which reports feature addition errors to a QgsProcessingContext.
 * \note Not available in Python bindings.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingFeatureSink : public QgsProxyFeatureSink
{
  public:


    /**
     * Constructor for QgsProcessingFeatureSink, accepting an original feature sink \a originalSink
     * and processing \a context. Any added features are added to the \a originalSink, with feature
     * writing errors being reports to \a context.
     *
     * The \a context must exist for the lifetime of this object.
     *
     * The \a sinkName is used to identify the destination sink when reporting errors.
     *
     * Ownership of \a originalSink is dictated by \a ownsOriginalSource. If \a ownsOriginalSink is FALSE,
     * ownership is not transferred, and callers must ensure that \a originalSink exists for the lifetime of this object.
     * If \a ownsOriginalSink is TRUE, then this object will take ownership of \a originalSink.
     */
    QgsProcessingFeatureSink( QgsFeatureSink *originalSink, const QString &sinkName, QgsProcessingContext &context, bool ownsOriginalSink = false );
    ~QgsProcessingFeatureSink() override;
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;

  private:

    QgsProcessingContext &mContext;
    QString mSinkName;
    bool mOwnsSink = false;

};
#endif

#endif // QGSPROCESSINGUTILS_H


