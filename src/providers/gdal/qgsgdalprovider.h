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
#include "qgsdataitem.h"
#include "qgsrasterdataprovider.h"
#include "qgsgdalproviderbase.h"
#include "qgsrectangle.h"
#include "qgscolorrampshader.h"
#include "qgsrasterbandstats.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>

class QgsRasterPyramid;

/** \ingroup core
 * A call back function for showing progress of gdal operations.
 */
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char *pszMessage,
                                  void * pProgressArg );


class QgsCoordinateTransform;

/**

  \brief Data provider for GDAL layers.

  This provider implements the interface defined in the QgsDataProvider class
  to provide access to spatial data residing in a GDAL layers.

*/
class QgsGdalProvider : public QgsRasterDataProvider, QgsGdalProviderBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
     *                otherwise we contact the host directly.
     *
     */
    QgsGdalProvider( QString const & uri = QString(), bool update = false );

    //! Create invalid provider with error
    QgsGdalProvider( QString const & uri, QgsError error );


    ~QgsGdalProvider();

    QgsGdalProvider * clone() const override;

    QString name() const override;
    QString description() const override;
    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QgsRectangle extent() const override;
    bool isValid() const override;
    QgsRasterIdentifyResult identify( const QgsPoint & point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
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
    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) override;

    void readBlock( int bandNo, int xBlock, int yBlock, void *data ) override;
    void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, void *data, QgsRasterBlockFeedback* feedback = nullptr ) override;
    double bandScale( int bandNo ) const override;
    double bandOffset( int bandNo ) const override;
    QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo )const override;
    QString metadata() override;
    QStringList subLayers() const override;
    static QStringList subLayers( GDALDatasetH dataset );

    bool hasStatistics( int bandNo,
                        int stats = QgsRasterBandStats::All,
                        const QgsRectangle & boundingBox = QgsRectangle(),
                        int sampleSize = 0 ) override;

    QgsRasterBandStats bandStatistics( int bandNo,
                                       int stats = QgsRasterBandStats::All,
                                       const QgsRectangle & boundingBox = QgsRectangle(),
                                       int sampleSize = 0 ) override;

    bool hasHistogram( int bandNo,
                       int binCount = 0,
                       double minimum = std::numeric_limits<double>::quiet_NaN(),
                       double maximum = std::numeric_limits<double>::quiet_NaN(),
                       const QgsRectangle & boundingBox = QgsRectangle(),
                       int sampleSize = 0,
                       bool includeOutOfRange = false ) override;

    QgsRasterHistogram histogram( int bandNo,
                                  int binCount = 0,
                                  double minimum = std::numeric_limits<double>::quiet_NaN(),
                                  double maximum = std::numeric_limits<double>::quiet_NaN(),
                                  const QgsRectangle & boundingBox = QgsRectangle(),
                                  int sampleSize = 0,
                                  bool includeOutOfRange = false ) override;

    QString buildPyramids( const QList<QgsRasterPyramid> & rasterPyramidList,
                           const QString & resamplingMethod = "NEAREST",
                           QgsRaster::RasterPyramidsFormat format = QgsRaster::PyramidsGTiff,
                           const QStringList & createOptions = QStringList() ) override;
    QList<QgsRasterPyramid> buildPyramidList( QList<int> overviewList = QList<int>() ) override;

    //! \brief Close data set and release related data
    void closeDataset();

    //! Emit a signal to notify of the progress event.
    void emitProgress( int type, double value, const QString &message );
    void emitProgressUpdate( int progress );

    static QMap<QString, QString> supportedMimes();

    bool isEditable() const override;
    bool setEditable( bool enabled ) override;
    bool write( void* data, int band, int width, int height, int xOffset, int yOffset ) override;

    bool setNoDataValue( int bandNo, double noDataValue ) override;
    bool remove() override;

    QString validateCreationOptions( const QStringList& createOptions, const QString& format ) override;
    QString validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
                                           const QStringList & configOptions, const QString & fileFormat ) override;
  private:
    // update mode
    bool mUpdate;

    // initialize CRS from wkt
    bool crsFromWkt( const char *wkt );

    //! Do some initialization on the dataset (e.g. handling of south-up datasets)
    void initBaseDataset();

    /**
     * Flag indicating if the layer data source is a valid layer
     */
    bool mValid;

    //! \brief Whether this raster has overviews / pyramids or not
    bool mHasPyramids;

    /** \brief Gdal data types used to represent data in in QGIS,
     * may be longer than source data type to keep nulls
     * indexed from 0
     */
    QList<GDALDataType> mGdalDataType;

    QgsRectangle mExtent;
    int mWidth;
    int mHeight;
    int mXBlockSize;
    int mYBlockSize;

    //mutable QList<bool> mMinMaxComputed;

    // List of estimated min values, index 0 for band 1
    //mutable QList<double> mMinimum;

    // List of estimated max values, index 0 for band 1
    //mutable QList<double> mMaximum;

    //! \brief Pointer to the gdaldataset
    GDALDatasetH mGdalBaseDataset;

    //! \brief Pointer to the gdaldataset (possibly warped vrt)
    GDALDatasetH mGdalDataset;

    //! \brief Values for mapping pixel to world coordinates. Contents of this array are the same as the GDAL adfGeoTransform
    double mGeoTransform[6];

    QgsCoordinateReferenceSystem mCrs;

    QList<QgsRasterPyramid> mPyramidList;

    //! \brief sublayers list saved for subsequent access
    QStringList mSubLayers;
};

#endif

