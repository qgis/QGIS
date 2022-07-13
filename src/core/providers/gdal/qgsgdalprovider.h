/***************************************************************************
      qgsgdalprovider.h  -  QGIS Data provider for
                           GDAL rasters
                             -------------------
    begin                : November, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALPROVIDER_H
#define QGSGDALPROVIDER_H

#include "qgscoordinatereferencesystem.h"
#include "qgsrasterdataprovider.h"
#include "qgsgdalproviderbase.h"
#include "qgsrectangle.h"
#include "qgscolorrampshader.h"
#include "qgsrasterbandstats.h"
#include "qgsprovidermetadata.h"
#include "qgsprovidersublayerdetails.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QMutex;

class QgsRasterPyramid;

/**
 * \ingroup core
 * \brief A call back function for showing progress of gdal operations.
 */
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char *pszMessage,
                                  void *pProgressArg );


class QgsCoordinateTransform;

/**
 *
 * \brief Data provider for GDAL layers.
 *
 * This provider implements the interface defined in the QgsDataProvider class
 * to provide access to spatial data residing in a GDAL layers.
 *
*/
class QgsGdalProvider final: public QgsRasterDataProvider, QgsGdalProviderBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param   uri         file name
     * \param   update      whether to open in update mode
     * \param   newDataset  handle of newly created dataset.
     *
     */
    QgsGdalProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, bool update = false, GDALDatasetH newDataset = nullptr );

    //! Create invalid provider with error
    QgsGdalProvider( const QString &uri, const QgsError &error );

    ~QgsGdalProvider() override;

    /**
     * Gets the data source specification. This may be a path or a protocol
     * connection string
     * \param expandAuthConfig Whether to expand any assigned authentication configuration
     * \returns data source specification
     * \note The default authentication configuration expansion is FALSE. This keeps credentials
     * out of layer data source URIs and project files. Expansion should be specifically done
     * only when needed within a provider
     */
    QString dataSourceUri( bool expandAuthConfig = false ) const override;

    /**
     * Clone the provider.
     *
     * The underlying GDAL dataset is shared among the main provider and its
     * clones.
     */
    QgsGdalProvider *clone() const override;

    QString name() const override;

    /**
     * GDAL data provider key
     * \since QGIS 3.10
     */
    static QString providerKey();

    /**
     * This helper checks to see whether the file name appears to be a valid
     * raster file name.  If the file name looks like it could be valid,
     * but some sort of error occurs in processing the file, the error is
     * returned in \a retErrMsg.
     * \since QGIS 3.10
     */
    static bool isValidRasterFileName( QString const &fileNameQString, QString &retErrMsg );

    /**
     * Gets creation options metadata for a given format
     * \since QGIS 3.10
     */
    static QString helpCreationOptionsFormat( QString format );

    /**
     * Replaces the authcfg part of the string with authentication information
     * \since QGIS 3.22
     */
    static QString expandAuthConfig( const QString &dsName );

    QString description() const override;
    QgsRasterDataProvider::ProviderCapabilities providerCapabilities() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;
    QgsRasterIdentifyResult identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    double sample( const QgsPointXY &point, int band, bool *ok = nullptr, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    QString lastErrorTitle() override;
    QString lastError() override;
    int capabilities() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;
    int bandCount() const override;
    int colorInterpretation( int bandNo ) const override;
    int xBlockSize() const override;
    int yBlockSize() const override;
    int xSize() const override;
    int ySize() const override;
    QString generateBandName( int bandNumber ) const override;

    // Reimplemented from QgsRasterDataProvider to bypass second resampling (more efficient for local file based sources)
    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

    bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) override;
    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    double bandScale( int bandNo ) const override;
    double bandOffset( int bandNo ) const override;
    QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo )const override;
    QString htmlMetadata() override;
    QStringList subLayers() const override;

    static QList< QgsProviderSublayerDetails > sublayerDetails( GDALDatasetH dataset, const QString &baseUri );

    bool hasStatistics( int bandNo,
                        int stats = QgsRasterBandStats::All,
                        const QgsRectangle &boundingBox = QgsRectangle(),
                        int sampleSize = 0 ) override;

    QgsRasterBandStats bandStatistics( int bandNo,
                                       int stats = QgsRasterBandStats::All,
                                       const QgsRectangle &boundingBox = QgsRectangle(),
                                       int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) override;

    bool hasHistogram( int bandNo,
                       int binCount = 0,
                       double minimum = std::numeric_limits<double>::quiet_NaN(),
                       double maximum = std::numeric_limits<double>::quiet_NaN(),
                       const QgsRectangle &boundingBox = QgsRectangle(),
                       int sampleSize = 0,
                       bool includeOutOfRange = false ) override;

    QgsRasterHistogram histogram( int bandNo,
                                  int binCount = 0,
                                  double minimum = std::numeric_limits<double>::quiet_NaN(),
                                  double maximum = std::numeric_limits<double>::quiet_NaN(),
                                  const QgsRectangle &boundingBox = QgsRectangle(),
                                  int sampleSize = 0,
                                  bool includeOutOfRange = false, QgsRasterBlockFeedback *feedback = nullptr ) override;

    QString buildPyramids( const QList<QgsRasterPyramid> &rasterPyramidList,
                           const QString &resamplingMethod = "NEAREST",
                           QgsRaster::RasterPyramidsFormat format = QgsRaster::PyramidsGTiff,
                           const QStringList &createOptions = QStringList(),
                           QgsRasterBlockFeedback *feedback = nullptr ) override;
    QList<QgsRasterPyramid> buildPyramidList( const QList<int> &overviewList = QList<int>() ) override;

    static QMap<QString, QString> supportedMimes();

    bool isEditable() const override;
    bool setEditable( bool enabled ) override;
    bool write( void *data, int band, int width, int height, int xOffset, int yOffset ) override;

    bool setNoDataValue( int bandNo, double noDataValue ) override;
    bool remove() override;

    QString validateCreationOptions( const QStringList &createOptions, const QString &format ) override;
    QString validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
                                           const QStringList &configOptions, const QString &fileFormat ) override;

    QgsPoint transformCoordinates( const QgsPoint &point, TransformType type ) override;

    bool enableProviderResampling( bool enable ) override { mProviderResamplingEnabled = enable; return true; }
    bool setZoomedInResamplingMethod( ResamplingMethod method ) override { mZoomedInResamplingMethod = method; return true; }
    bool setZoomedOutResamplingMethod( ResamplingMethod method ) override { mZoomedOutResamplingMethod = method; return true; }
    bool setMaxOversampling( double factor ) override { mMaxOversampling = factor; return true; }

  private:
    QgsGdalProvider( const QgsGdalProvider &other );
    QgsGdalProvider &operator=( const QgsGdalProvider & ) = delete;

    //! Whether mGdalDataset and mGdalBaseDataset have been attempted to be set
    bool mHasInit = false;

    //! Open mGdalDataset/mGdalBaseDataset if needed.
    bool initIfNeeded();

    // There are 2 cloning mechanisms.
    // * Either the cloned provider use the same GDAL handles as the main provider
    //   instance, in which case *mpRefCounter is used to count how many providers
    //    use the main provider. And *mpMutex is used to protect access to the
    //   GDAL resource.
    // * Or the cloned provider use its own GDAL handles, but with a cache mechanism
    //   to avoid constant opening/closing of datasets. In that case *mpParent is
    //   used to point to the main provider, and *mpLightRefCounter to count how
    //   many providers point to that main provider.

    // reference counter to know how many main and shared provider instances are linked
    QAtomicInt *mpRefCounter = nullptr;

    // mutex to protect access to mGdalDataset among main and shared provider instances
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex *mpMutex = nullptr;
#else
    QRecursiveMutex *mpMutex = nullptr;
#endif


    // pointer to a QgsGdalProvider* that is the parent. Note when *mpParent == this, we are the parent.
    QgsGdalProvider **mpParent = nullptr;

    // reference counter to know how many main and related provider instances are linked
    mutable QAtomicInt *mpLightRefCounter = nullptr;

    // update mode
    bool mUpdate;

    //! Do some initialization on the dataset (e.g. handling of south-up datasets)
    void initBaseDataset();

    /**
     * Flag indicating if the layer data source is a valid layer
     */
    bool mValid = false;

    //! \brief Whether this raster has overviews / pyramids or not
    bool mHasPyramids = false;

    /**
     * \brief Gdal data types used to represent data in in QGIS,
     * may be longer than source data type to keep nulls
     * indexed from 0
     */
    QList<GDALDataType> mGdalDataType;

    QgsRectangle mExtent;
    int mWidth = 0;
    int mHeight = 0;
    int mXBlockSize = 0;
    int mYBlockSize = 0;
    int mBandCount = 1;

    //mutable QList<bool> mMinMaxComputed;

    // List of estimated min values, index 0 for band 1
    //mutable QList<double> mMinimum;

    // List of estimated max values, index 0 for band 1
    //mutable QList<double> mMaximum;

    //! \brief Pointer to the gdaldataset
    GDALDatasetH mGdalBaseDataset = nullptr;

    //! \brief Pointer to the gdaldataset (possibly warped vrt)
    GDALDatasetH mGdalDataset = nullptr;

    //! \brief Values for mapping pixel to world coordinates. Contents of this array are the same as the GDAL adfGeoTransform
    double mGeoTransform[6];

    QgsCoordinateReferenceSystem mCrs;

    QList<QgsRasterPyramid> mPyramidList;

    //! \brief sublayers list saved for subsequent access
    QList< QgsProviderSublayerDetails > mSubLayers;

    //! Whether a per-dataset mask band is exposed as an alpha band for the point of view of the rest of the application.
    bool mMaskBandExposedAsAlpha = false;

    //! \brief Driver short name.
    // It is kept in case the driver would be de-registered after the provider has been created.
    // Which is a very dangerous situation (see #29212)
    QString mDriverName;

    //! Wrapper for GDALGetRasterBand() that takes into account mMaskBandExposedAsAlpha.
    GDALRasterBandH getBand( int bandNo ) const;

    //! \brief Close data set and release related data
    void closeDataset();

    //! Pair of GDAL base dataset and "real" dataset handles.
    struct DatasetPair
    {
      GDALDatasetH mGdalBaseDataset;
      GDALDatasetH mGdalDataset;
    };

    // Dataset cache
    static QHash< QgsGdalProvider *, QVector<DatasetPair> > mgDatasetCache;

    // Number of cached datasets in mgDatasetCache ( == sum(iter.value().size() )
    static int mgDatasetCacheSize;

    //! Add handles to the cache if possible for the specified parent provider, in which case true is returned. If false returned, then the handles should be processed appropriately by the caller
    static bool cacheGdalHandlesForLaterReuse( QgsGdalProvider *provider, GDALDatasetH gdalBaseDataset, GDALDatasetH gdalDataset );

    //! Gets cached handles for the specified provider, in which case true is returned and 2 handles are set.
    static bool getCachedGdalHandles( QgsGdalProvider *provider, GDALDatasetH &gdalBaseDataset, GDALDatasetH &gdalDataset );

    //! Close all cached dataset for the specified provider.
    static void closeCachedGdalHandlesFor( QgsGdalProvider *provider );

    /**
     * Converts a world (\a x, \a y) coordinate to a pixel \a row and \a col.
     */
    bool worldToPixel( double x, double y, int &col, int &row ) const;

    bool mStatisticsAreReliable = false;

    /**
     * Closes and reinits dataset
    */
    void reloadProviderData() override;

    //! Instance of GDAL transformer function used in transformCoordinates() for conversion between image and layer coordinates
    void *mGdalTransformerArg = nullptr;

    bool canDoResampling(
      int bandNo,
      const QgsRectangle &reqExtent,
      int bufferWidthPix,
      int bufferHeightPix );
};

/**
 * Metadata of the GDAL data provider
 */
class QgsGdalProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsGdalProviderMetadata();
    QIcon icon() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    bool uriIsBlocklisted( const QString &uri ) const override;
    QgsGdalProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QgsGdalProvider *createRasterDataProvider(
      const QString &uri,
      const QString &format,
      int nBands,
      Qgis::DataType type,
      int width,
      int height,
      double *geoTransform,
      const QgsCoordinateReferenceSystem &crs,
      const QStringList &createOptions ) override;
    QString filters( FilterType type ) override;
    QList<QPair<QString, QString> > pyramidResamplingMethods() override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    ProviderCapabilities providerCapabilities() const override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    QStringList sidecarFilesForUri( const QString &uri ) const override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};

///@endcond
#endif
