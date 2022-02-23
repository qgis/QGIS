/***************************************************************************
                        qgsrasterlayer.h  -  description
                              -------------------
 begin                : Fri Jun 28 2002
 copyright            : (C) 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
 email                : tim@linfiniti.com
***************************************************************************/
/*
 * Peter J. Ersts - contributed to the refactoring and maintenance of this class
 * B. Morley - added functions to convert this class to a data provider interface
 * Frank Warmerdam - contributed bug fixes and migrated class to use all GDAL_C_API calls
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QPair>
#include <QVector>

#include "qgis_sip.h"
#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsraster.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterviewport.h"
#include "qgsrasterminmaxorigin.h"
#include "qgscontrastenhancement.h"

class QgsMapToPixel;
class QgsRasterRenderer;
class QgsRectangle;
class QgsRasterLayerTemporalProperties;
class QgsRasterPipe;
class QgsRasterResampleFilter;
class QgsBrightnessContrastFilter;
class QgsHueSaturationFilter;
class QgsRasterLayerElevationProperties;

class QImage;
class QPixmap;
class QSlider;

typedef QList < QPair< QString, QColor > > QgsLegendColorList;

/**
 * \ingroup core
 *
 * \brief Represents a raster layer.
 *
 * A QgsRasterLayer is instantiated by specifying the name of a data provider,
 * such as "gdal" or "wms", and a url defining the specific data set to connect to.
 * The raster layer constructor in turn instantiates a QgsRasterDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 *  Sample usage of the QgsRasterLayer class:
 *
 * \code{.py}
 *     my_raster_layer = QgsRasterLayer("/path/to/file.tif", "my layer")
 * \endcode
 */
class CORE_EXPORT QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
  public:

    //! \brief Default sample size (number of pixels) for estimated statistics/histogram calculation
    static const double SAMPLE_SIZE;

    //! \brief Default enhancement algorithm for single band raster
    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm SINGLE_BAND_ENHANCEMENT_ALGORITHM;

    //! \brief Default enhancement algorithm for multiple band raster of type Byte
    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM;

    //! \brief Default enhancement algorithm for multiple band raster of type different from Byte
    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM;

    //! \brief Default enhancement limits for single band raster
    static const QgsRasterMinMaxOrigin::Limits SINGLE_BAND_MIN_MAX_LIMITS;

    //! \brief Default enhancement limits for multiple band raster of type Byte
    static const QgsRasterMinMaxOrigin::Limits MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS;

    //! \brief Default enhancement limits for multiple band raster of type different from Byte
    static const QgsRasterMinMaxOrigin::Limits MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS;

    //! \brief Constructor. Provider is not set.
    QgsRasterLayer();

    /**
     * Setting options for loading raster layers.
     * \since QGIS 3.0
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( bool loadDefaultStyle = true,
                             const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext() )
        : loadDefaultStyle( loadDefaultStyle )
        , transformContext( transformContext )
      {}

      //! Sets to TRUE if the default layer style should be loaded
      bool loadDefaultStyle = true;

      /**
       * Coordinate transform context
       * \since QGIS 3.8
       */
      QgsCoordinateTransformContext transformContext = QgsCoordinateTransformContext();

      /**
       * Controls whether the layer is allowed to have an invalid/unknown CRS.
       *
       * If TRUE, then no validation will be performed on the layer's CRS and the layer
       * layer's crs() may be invalid() (i.e. the layer will have no georeferencing available
       * and will be treated as having purely numerical coordinates).
       *
       * If FALSE (the default), the layer's CRS will be validated using QgsCoordinateReferenceSystem::validate(),
       * which may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the
       * layer.
       *
       * \since QGIS 3.10
       */
      bool skipCrsValidation = false;

    };

    /**
     * \brief This is the constructor for the RasterLayer class.
     *
     * The main tasks carried out by the constructor are:
     *
     * - Load the rasters default style (.qml) file if it exists
     * - Populate the RasterStatsVector with initial values for each band.
     * - Calculate the layer extents
     * - Determine whether the layer is gray, paletted or multiband.
     * - Assign sensible defaults for the red, green, blue and gray bands.
     *
     */
    explicit QgsRasterLayer( const QString &uri,
                             const QString &baseName = QString(),
                             const QString &providerType = "gdal",
                             const QgsRasterLayer::LayerOptions &options = QgsRasterLayer::LayerOptions() );

    ~QgsRasterLayer() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRasterLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns a new instance equivalent to this one. A new provider is
     *  created for the same data source and renderer is cloned too.
     * \returns a new layer instance
     * \since QGIS 3.0
     */
    QgsRasterLayer *clone() const override SIP_FACTORY;

    //! \brief This enumerator describes the types of shading that can be used
    enum ColorShadingAlgorithm
    {
      UndefinedShader,
      PseudoColorShader,
      FreakOutShader,
      ColorRampShader,
      UserDefinedShader
    };

    //! \brief This enumerator describes the type of raster layer
    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband,
      ColorLayer
    };

    /**
     * This helper checks to see whether the file name appears to be a valid
     * raster file name.  If the file name looks like it could be valid,
     * but some sort of error occurs in processing the file, the error is
     * returned in \a retError.
     */
    static bool isValidRasterFileName( const QString &fileNameQString, QString &retError );
    // TODO QGIS 4.0 - rename fileNameQString to fileName

    static bool isValidRasterFileName( const QString &fileNameQString );

    //! Returns time stamp for given file name
    static QDateTime lastModified( const QString   &name );

    /**
     * Set the data provider.
     * \deprecated Use the version with ProviderOptions instead.
     */
    Q_DECL_DEPRECATED void setDataProvider( const QString &provider ) SIP_DEPRECATED;

    /**
     * Set the data provider.
     * \param provider provider key string, must match a valid QgsRasterDataProvider key. E.g. "gdal", "wms", etc.
     * \param options provider options
     * \param flags provider flags since QGIS 3.16
     * \since QGIS 3.2
     */
    void setDataProvider( const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    /**
     * Returns the raster layer type (which is a read only property).
     */
    LayerType rasterType() { return mRasterType; }

    /**
     * Sets the raster's \a renderer. Takes ownership of the renderer object.
     * \see renderer()
     */
    void setRenderer( QgsRasterRenderer *renderer SIP_TRANSFER );

    /**
     * Returns the raster's renderer.
     *
     * \see setRenderer()
     */
    QgsRasterRenderer *renderer() const;

    /**
     * Returns the raster's resample filter.
     *
     * \see brightnessFilter()
     * \see hueSaturationFilter()
     */
    QgsRasterResampleFilter *resampleFilter() const;

    /**
     * Returns the raster's brightness/contrast filter.
     *
     * \see resampleFilter()
     * \see hueSaturationFilter()
     */
    QgsBrightnessContrastFilter *brightnessFilter() const;

    /**
     * Returns the raster's hue/saturation filter.
     *
     * \see resampleFilter()
     * \see brightnessFilter()
     */
    QgsHueSaturationFilter *hueSaturationFilter() const;

    /**
     * Select which stage of the pipe should apply resampling.
     *
     * \see QgsRasterPipe::setResamplingStage()
     *
     * \since QGIS 3.16
     */
    void setResamplingStage( Qgis::RasterResamplingStage stage );

    /**
     * Returns which stage of the pipe should apply resampling.
     *
     * \see QgsRasterPipe::resamplingStage()
     *
     * \since QGIS 3.16
     */
    Qgis::RasterResamplingStage resamplingStage() const;

    /**
     * Returns the raster pipe.
     */
    QgsRasterPipe *pipe() { return mPipe.get(); }

    /**
     * Returns the width of the (unclipped) raster.
     * \see height()
     */
    int width() const;

    /**
     * Returns the height of the (unclipped) raster.
     * \see width()
     */
    int height() const;

    /**
     * Returns the number of bands in this layer.
     */
    int bandCount() const;

    /**
     * Returns the name of a band given its number.
     */
    QString bandName( int bandNoInt ) const;

    /**
     * Returns the source data provider.
     *
     * This will be NULLPTR if the layer is invalid.
     */
    QgsRasterDataProvider *dataProvider() override;

    /**
     * Returns the source data provider.
     *
     * This will be NULLPTR if the layer is invalid.
     */
    const QgsRasterDataProvider *dataProvider() const SIP_PYNAME( constDataProvider ) override;

    void reload() override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    //! \brief This is an overloaded version of the draw() function that is called by both draw() and thumbnailAsPixmap
    void draw( QPainter *theQPainter,
               QgsRasterViewPort *myRasterViewPort,
               const QgsMapToPixel *qgsMapToPixel = nullptr );

    /**
     * Returns a list with classification items (Text and color).
     *
     * \deprecated use QgsRasterRenderer::createLegendNodes() instead.
     */
    Q_DECL_DEPRECATED QgsLegendColorList legendSymbologyItems() const SIP_DEPRECATED;

    bool isSpatial() const override { return true; }

    QString htmlMetadata() const override;
    Qgis::MapLayerProperties properties() const override;

    /**
     * Returns a 100x100 pixmap of the color palette. If the layer has no palette a white pixmap will be returned
     * \param bandNumber the number of the band to use for generating a pixmap of the associated palette
     */
    QPixmap paletteAsPixmap( int bandNumber = 1 );

    //! \brief [ data provider interface ] Which provider is being used for this Raster Layer?
    QString providerType() const;

    /**
     * Returns the number of raster units per each raster pixel in X axis.
     *
     * In a world file, this is normally the first row (without the sign). (E.g.
     * the value reported by the GDAL geotransform[1]).
     *
     * \see rasterUnitsPerPixelY()
     */
    double rasterUnitsPerPixelX() const;

    /**
     * Returns the number of raster units per each raster pixel in Y axis.
     *
     * In a world file, this is normally the first row (without the sign).
     *
     * \see rasterUnitsPerPixelX()
     */
    double rasterUnitsPerPixelY() const;

    void setOpacity( double opacity ) FINAL;
    double opacity() const FINAL;

    /**
     * \brief Set contrast enhancement algorithm
     *  \param algorithm Contrast enhancement algorithm
     *  \param limits Limits
     *  \param extent Extent used to calculate limits, if empty, use full layer extent
     *  \param sampleSize Size of data sample to calculate limits, if 0, use full resolution
     *  \param generateLookupTableFlag Generate lookup table.
    */
    void setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
                                 QgsRasterMinMaxOrigin::Limits limits = QgsRasterMinMaxOrigin::MinMax,
                                 const QgsRectangle &extent = QgsRectangle(),
                                 int sampleSize = QgsRasterLayer::SAMPLE_SIZE,
                                 bool generateLookupTableFlag = true );

    /**
     * \brief Refresh contrast enhancement with new extent.
     *  \note not available in Python bindings
     */
    void refreshContrastEnhancement( const QgsRectangle &extent ) SIP_SKIP;

    /**
     * \brief Refresh renderer with new extent, if needed
     *  \note not available in Python bindings
     */
    void refreshRendererIfNeeded( QgsRasterRenderer *rasterRenderer, const QgsRectangle &extent ) SIP_SKIP;

    /**
     * Returns the string (typically sql) used to define a subset of the layer.
     * \returns The subset string or null QString if not implemented by the provider
     * \since QGIS 3.12
     */
    virtual QString subsetString() const;

    /**
     * Sets the string (typically sql) used to define a subset of the layer
     * \param subset The subset string. This may be the where clause of a sql statement
     *               or other definition string specific to the underlying dataprovider
     *               and data store.
     * \returns TRUE, when setting the subset string was successful, FALSE otherwise
     * \since QGIS 3.12
     */
    virtual bool setSubsetString( const QString &subset );

    /**
     * Returns default contrast enhancement settings for that type of raster.
     *  \note not available in Python bindings
     */
    bool defaultContrastEnhancementSettings(
      QgsContrastEnhancement::ContrastEnhancementAlgorithm &myAlgorithm,
      QgsRasterMinMaxOrigin::Limits &myLimits ) const SIP_SKIP;

    //! Sets the default contrast enhancement
    void setDefaultContrastEnhancement();

    QStringList subLayers() const override;

    /**
     * \brief Draws a preview of the rasterlayer into a QImage
     * \since QGIS 2.4
    */
    QImage previewAsImage( QSize size, const QColor &bgColor = Qt::white,
                           QImage::Format format = QImage::Format_ARGB32_Premultiplied );

    void setLayerOrder( const QStringList &layers ) override;
    void setSubLayerVisibility( const QString &name, bool vis ) override;
    QDateTime timestamp() const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Writes the symbology of the layer into the document provided in SLD 1.0.0 format
     * \param node the node that will have the style element added to it.
     * \param doc the document that will have the QDomNode added.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param props a open ended set of properties that can drive/inform the SLD encoding
     * \returns TRUE in case of success
     * \since QGIS 3.6
     */
    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QVariantMap &props = QVariantMap() ) const;

    /**
     * If the ignoreExtent flag is set, the layer will also render outside the
     * bounding box reported by the data provider.
     * To be used for example for WMS layers with labels or symbology that happens
     * to be drawn outside the data extent.
     *
     * \since QGIS 3.10
     */
    bool ignoreExtents() const;

    QgsMapLayerTemporalProperties *temporalProperties() override;
    QgsMapLayerElevationProperties *elevationProperties() override;

  public slots:
    void showStatusMessage( const QString &message );

    /**
     * Sets the coordinate transform context to \a transformContext
     *
     * \since QGIS 3.8
     */
    virtual void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

  signals:

    /**
     * Emitted when the layer's subset string has changed.
     * \since QGIS 3.12
     */
    void subsetStringChanged();


  protected:
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) override;
    bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context ) override;
    bool writeSymbology( QDomNode &, QDomDocument &doc, QString &errorMessage,
                         const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                     const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const override;
    QString decodedSource( const QString &source, const QString &provider,  const QgsReadWriteContext &context ) const override;

  private:
    //! \brief Initialize default values
    void init();

    //! \brief Close data provider and clear related members
    void closeDataProvider();

    //! \brief Update the layer if it is outdated
    bool update();

    //! Sets corresponding renderer for style
    void setRendererForDrawingStyle( QgsRaster::DrawingStyle drawingStyle );

    void setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
                                 QgsRasterMinMaxOrigin::Limits limits,
                                 const QgsRectangle &extent,
                                 int sampleSize,
                                 bool generateLookupTableFlag,
                                 QgsRasterRenderer *rasterRenderer );

    //! Refresh renderer
    void refreshRenderer( QgsRasterRenderer *rasterRenderer, const QgsRectangle &extent );

    void computeMinMax( int band,
                        const QgsRasterMinMaxOrigin &mmo,
                        QgsRasterMinMaxOrigin::Limits limits,
                        const QgsRectangle &extent,
                        int sampleSize,
                        double &min, double &max );

    /**
     * Updates the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     * \param dataSource new layer data source
     * \param baseName base name of the layer
     * \param provider provider string
     * \param options provider options
     * \param flags provider read flags
     * \see dataSourceChanged()
     * \since QGIS 3.20
     */
    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags ) override;

    //! \brief  Constant defining flag for XML and a constant that signals property not used
    const QString QSTRING_NOT_SET;
    const QString TRSTRING_NOT_SET;

    //! Pointer to data provider
    QgsRasterDataProvider *mDataProvider = nullptr;

    //! Pointer to temporal properties
    QgsRasterLayerTemporalProperties *mTemporalProperties = nullptr;

    QgsRasterLayerElevationProperties *mElevationProperties = nullptr;

    //! [ data provider interface ] Timestamp, the last modified time of the data source when the layer was created
    QDateTime mLastModified;

    QgsRasterViewPort mLastViewPort;

    LayerType mRasterType = GrayOrUndefined;

    std::unique_ptr< QgsRasterPipe > mPipe;

    //! To save computations and possible infinite cycle of notifications
    QgsRectangle mLastRectangleUsedByRefreshContrastEnhancementIfNeeded;

    QDomDocument mOriginalStyleDocument;
    QDomElement mOriginalStyleElement;
};

// clazy:excludeall=qstring-allocations

#endif
