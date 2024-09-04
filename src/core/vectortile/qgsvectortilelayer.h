/***************************************************************************
  qgsvectortilelayer.h
  --------------------------------------
  Date                 : March 2020
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

#ifndef QGSVECTORTILELAYER_H
#define QGSVECTORTILELAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include "qgsvectortilematrixset.h"
#include "qgsfeatureid.h"

class QgsVectorTileRenderer;
class QgsVectorTileLabeling;
class QgsFeature;
class QgsGeometry;
class QgsSelectionContext;
class QgsVectorTileRawData;

/**
 * \ingroup core
 * \brief Implements a map layer that is dedicated to rendering of vector tiles.
 *
 * Vector tiles compared to "ordinary" vector layers are pre-processed data
 * optimized for fast rendering. A dataset is provided with a series of zoom levels
 * for different map scales. Each zoom level has a matrix of tiles that contain
 * actual data. A single vector tile may be a a file stored on a local drive,
 * requested over HTTP request or retrieved from a database.
 *
 * Content of a vector tile is divided into one or more named sub-layers. Each such
 * sub-layer may contain many features which consist of geometry and attributes.
 * Contrary to traditional vector layers, these sub-layers do not need to have a rigid
 * schema where geometry type and attributes are the same for all features. A single
 * sub-layer may have multiple geometry types in a single tile or have some attributes
 * defined only at particular zoom levels.
 *
 * Vector tile layer currently does not use the concept of data providers that other
 * layer types use. The process of rendering of vector tiles looks like this:
 *
 * +--------+                +------+                 +---------+
 * |  DATA  |                |  RAW |                 | DECODED |
 * |        | --> LOADER --> |      | --> DECODER --> |         | --> RENDERER
 * | SOURCE |                | TILE |                 |  TILE   |
 * +--------+                +------+                 +---------+
 *
 * Data source is a place from where tiles are fetched from (URL for HTTP access, local
 * files, MBTiles file, GeoPackage file or others. Loader (QgsVectorTileLoader) class
 * takes care of loading data from the data source. The "raw tile" data is just a blob
 * (QByteArray) that is encoded in some way. There are multiple ways how vector tiles
 * are encoded just like there are different formats how to store images. For example,
 * tiles can be encoded using Mapbox Vector Tiles (MVT) format or in GeoJSON. Decoder
 * (QgsVectorTileDecoder) takes care of decoding raw tile data into QgsFeature objects.
 * A decoded tile is essentially an array of vector features for each sub-layer found
 * in the tile - this is what vector tile renderer (QgsVectorTileRenderer) expects
 * and does the map rendering.
 *
 * To construct a vector tile layer, it is best to use QgsDataSourceUri class and set
 * the following parameters to get a valid encoded URI:
 *
 * - "type" - what kind of data source will be used
 * - "url" - URL or path of the data source (specific to each data source type, see below)
 *
 * Currently supported data source types:
 *
 * - "xyz" - the "url" should be a template like http://example.com/{z}/{x}/{y}.pbf where
 *   {x},{y},{z} will be replaced by tile coordinates
 * - "mbtiles" - tiles read from a MBTiles file (a SQLite database)
 *
 * Currently supported decoders:
 *
 * - MVT - following Mapbox Vector Tiles specification
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileLayer : public QgsMapLayer
{
    Q_OBJECT

  public:


    /**
     * Setting options for loading vector tile layers.
     *
     * \since QGIS 3.22
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with optional \a transformContext.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext( ) )
        : transformContext( transformContext )
      {}

      //! Coordinate transform context
      QgsCoordinateTransformContext transformContext;
    };

    //! Constructs a new vector tile layer
    explicit QgsVectorTileLayer( const QString &path = QString(), const QString &baseName = QString(), const QgsVectorTileLayer::LayerOptions &options = QgsVectorTileLayer::LayerOptions() );
    ~QgsVectorTileLayer() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsVectorTileLayer: '%1'>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    // implementation of virtual functions from QgsMapLayer

    QgsVectorTileLayer *clone() const override SIP_FACTORY;
    QgsDataProvider *dataProvider() override;
    const QgsDataProvider *dataProvider() const override SIP_SKIP;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                         StyleCategories categories = AllStyleCategories ) const override;
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;
    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) override;
    Qgis::MapLayerProperties properties() const override;

    /**
     * Loads the default style for the layer, and returns TRUE if the style was
     * successfully loaded.
     *
     * The \a error string will be filled with a translated error message if an error
     * occurs during the style load. The \a warnings list will be populated with any
     * warning messages generated during the style load (e.g. default style properties
     * which could not be converted).
     *
     * \since QGIS 3.16
     */
    bool loadDefaultStyle( QString &error, QStringList &warnings ) SIP_SKIP;

    /**
     * Loads the default style for the layer, and returns TRUE if the style was
     * successfully loaded. Also loads any sub layers (such as raster terrain layers) associated
     * with the layer's default style.
     *
     * The \a error string will be filled with a translated error message if an error
     * occurs during the style load. The \a warnings list will be populated with any
     * warning messages generated during the style load (e.g. default style properties
     * which could not be converted).
     *
     * Ownership of the \a subLayers is transferrred to the caller.
     *
     * \since QGIS 3.28
     */
    bool loadDefaultStyleAndSubLayers( QString &error, QStringList &warnings, QList< QgsMapLayer * > &subLayers SIP_OUT SIP_TRANSFERBACK );

    QString loadDefaultMetadata( bool &resultFlag SIP_OUT ) override;

    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const FINAL;
    QString decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const FINAL;
    QString htmlMetadata() const override;

    // new methods

    /**
     * Returns the vector tile matrix set.
     *
     * \since QGIS 3.22.6
     */
    QgsVectorTileMatrixSet &tileMatrixSet() { return mMatrixSet; }

    //! Returns type of the data source
    QString sourceType() const { return mSourceType; }
    //! Returns URL/path of the data source (syntax different to each data source type)
    QString sourcePath() const;

    //! Returns minimum zoom level at which source has any valid tiles (negative = unconstrained)
    int sourceMinZoom() const { return mMatrixSet.minimumZoom(); }
    //! Returns maximum zoom level at which source has any valid tiles (negative = unconstrained)
    int sourceMaxZoom() const { return mMatrixSet.maximumZoom(); }

    /**
     * Fetches raw tile data for the give tile coordinates. If failed to fetch tile data,
     * it will return an empty byte array.
     *
     * \note This call may issue a network request (depending on the source type) and will block
     * the caller until the request is finished.
     */
    QgsVectorTileRawData getRawTile( QgsTileXYZ tileID ) SIP_SKIP;

    /**
     * Sets renderer for the map layer.
     * \note Takes ownership of the passed renderer
     */
    void setRenderer( QgsVectorTileRenderer *r SIP_TRANSFER );
    //! Returns currently assigned renderer
    QgsVectorTileRenderer *renderer() const;

    /**
     * Sets labeling for the map layer.
     * \note Takes ownership of the passed labeling
     */
    void setLabeling( QgsVectorTileLabeling *labeling SIP_TRANSFER );
    //! Returns currently assigned labeling
    QgsVectorTileLabeling *labeling() const;

    /**
     * Returns whether the layer contains labels which are enabled and should be drawn.
     * \returns TRUE if layer contains enabled labels
     *
     * \see setLabelsEnabled()
     * \since QGIS 3.34
     */
    bool labelsEnabled() const;

    /**
     * Sets whether labels should be \a enabled for the layer.
     *
     * \note Labels will only be rendered if labelsEnabled() is TRUE and a labeling
     * object is returned by labeling().
     *
     * \see labelsEnabled()
     * \see labeling()
     * \since QGIS 3.34
     */
    void setLabelsEnabled( bool enabled );

    //! Sets whether to render also borders of tiles (useful for debugging)
    void setTileBorderRenderingEnabled( bool enabled ) { mTileBorderRendering = enabled; }
    //! Returns whether to render also borders of tiles (useful for debugging)
    bool isTileBorderRenderingEnabled() const { return mTileBorderRendering; }

    /**
     * Returns the list of features currently selected in the layer.
     *
     * \see selectedFeatureCount()
     * \see selectByGeometry()
     * \see removeSelection()
     * \see selectionChanged()
     * \since QGIS 3.28
     */
    QList< QgsFeature > selectedFeatures() const;

    /**
     * Returns the number of features that are selected in this layer.
     *
     * \see selectedFeatures()
     * \see selectByGeometry()
     * \see removeSelection()
     * \see selectionChanged()
     * \since QGIS 3.28
     */
    int selectedFeatureCount() const;

    /**
     * Selects features found within the search \a geometry (in layer's coordinates).
     *
     * A render context can optionally be specified in order to avoid selecting features which are
     * not currently rendered.
     *
     * \see selectedFeatures()
     * \see removeSelection()
     * \see selectionChanged()
     * \since QGIS 3.28
     */
    void selectByGeometry( const QgsGeometry &geometry, const QgsSelectionContext &context,
                           Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection,
                           Qgis::SelectGeometryRelationship relationship = Qgis::SelectGeometryRelationship::Intersect,
                           Qgis::SelectionFlags flags = Qgis::SelectionFlags(),
                           QgsRenderContext *renderContext = nullptr );

  public slots:

    /**
     * Clear selection
     *
     * \see selectByGeometry()
     * \see selectionChanged()
     * \since QGIS 3.28
     */
    void removeSelection();

  signals:

    /**
     * Emitted whenever the selected features in the layer are changed.
     *
     * \since QGIS 3.28
     */
    void selectionChanged();

  private:
    bool loadDataSource();

  private:
    //! Type of the data source
    QString mSourceType;

    QgsVectorTileMatrixSet mMatrixSet;

    //! Renderer assigned to the layer to draw map
    std::unique_ptr<QgsVectorTileRenderer> mRenderer;
    //! Labeling assigned to the layer to produce labels
    std::unique_ptr<QgsVectorTileLabeling> mLabeling;
    //! True if labels are enabled
    bool mLabelsEnabled = true;
    //! Whether we draw borders of tiles
    bool mTileBorderRendering = false;

    QgsCoordinateTransformContext mTransformContext;

    std::unique_ptr< QgsDataProvider > mDataProvider;

    QHash< QgsFeatureId, QgsFeature > mSelectedFeatures;

    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
                               const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags ) override;

    bool loadDefaultStyleAndSubLayersPrivate( QString &error, QStringList &warnings, QList< QgsMapLayer * > *subLayers );

};

#endif // QGSVECTORTILELAYER_H
