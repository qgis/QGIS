/***************************************************************************
    qgsrasterfilewriter.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERFILEWRITER_H
#define QGSRASTERFILEWRITER_H

#include "qgscoordinatereferencesystem.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterpipe.h"
#include "qgsrectangle.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

class QProgressDialog;
class QgsRasterIterator;

/** \ingroup core
 * The raster file writer which allows you to save a raster to a new file.
 */
class CORE_EXPORT QgsRasterFileWriter
{
  public:
    enum Mode
    {
      Raw = 0, // Raw data
      Image = 1 // Rendered image
    };
    enum WriterError
    {
      NoError = 0,
      SourceProviderError = 1,
      DestProviderError = 2,
      CreateDatasourceError = 3,
      WriteError = 4,
      // Internal error if a value used for 'no data' was found in input
      NoDataConflict = 5
    };

    QgsRasterFileWriter( const QString& outputUrl );
    ~QgsRasterFileWriter();

    /**Write raster file
        @param pipe raster pipe
        @param nCols number of output columns
        @param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
        @param outputExtent extent to output
        @param crs crs to reproject to
        @param p dialog to show progress in */
    WriterError writeRaster( const QgsRasterPipe* pipe, int nCols, int nRows, QgsRectangle outputExtent,
                             const QgsCoordinateReferenceSystem& crs, QProgressDialog* p = 0 );

    void setOutputFormat( const QString& format ) { mOutputFormat = format; }
    QString outputFormat() const { return mOutputFormat; }

    void setOutputProviderKey( const QString& key ) { mOutputProviderKey = key; }
    QString outputProviderKey() const { return mOutputProviderKey; }

    void setTiledMode( bool t ) { mTiledMode = t; }
    bool tiledMode() const { return mTiledMode; }

    void setMaxTileWidth( int w ) { mMaxTileWidth = w; }
    int maxTileWidth() const { return mMaxTileWidth; }

    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const { return mBuildPyramidsFlag; }
    void setBuildPyramidsFlag( QgsRaster::RasterBuildPyramids f ) { mBuildPyramidsFlag = f; }

    QList< int > pyramidsList() const { return mPyramidsList; }
    void setPyramidsList( const QList< int > & list ) { mPyramidsList = list; }

    QString pyramidsResampling() const { return mPyramidsResampling; }
    void setPyramidsResampling( const QString & str ) { mPyramidsResampling = str; }

    QgsRaster::RasterPyramidsFormat pyramidsFormat() const { return mPyramidsFormat; }
    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat f ) { mPyramidsFormat = f; }

    void setMaxTileHeight( int h ) { mMaxTileHeight = h; }
    int maxTileHeight() const { return mMaxTileHeight; }

    void setCreateOptions( const QStringList& list ) { mCreateOptions = list; }
    QStringList createOptions() const { return mCreateOptions; }

    void setPyramidsConfigOptions( const QStringList& list ) { mPyramidsConfigOptions = list; }
    QStringList pyramidsConfigOptions() const { return mPyramidsConfigOptions; }

  private:
    QgsRasterFileWriter(); //forbidden
    WriterError writeDataRaster( const QgsRasterPipe* pipe, QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
                                 const QgsCoordinateReferenceSystem& crs, QProgressDialog* progressDialog = 0 );

    // Helper method used by previous one
    WriterError writeDataRaster( const QgsRasterPipe* pipe,
                                 QgsRasterIterator* iter,
                                 int nCols, int nRows,
                                 const QgsRectangle& outputExtent,
                                 const QgsCoordinateReferenceSystem& crs,
                                 QGis::DataType destDataType,
                                 QList<bool> destHasNoDataValueList,
                                 QList<double> destNoDataValueList,
                                 QgsRasterDataProvider* destProvider,
                                 QProgressDialog* progressDialog );

    WriterError writeImageRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
                                  const QgsCoordinateReferenceSystem& crs, QProgressDialog* progressDialog = 0 );

    /** \brief Initialize vrt member variables
     *  @param destHasNoDataValueList true if destination has no data value, indexed from 0
     *  @param destNoDataValueList no data value, indexed from 0
     */
    void createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform, QGis::DataType type, QList<bool> destHasNoDataValueList, QList<double> destNoDataValueList );
    //write vrt document to disk
    bool writeVRT( const QString& file );
    //add file entry to vrt
    void addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset );
    void buildPyramids( const QString& filename );

    /**Create provider and datasource for a part image (vrt mode)*/
    QgsRasterDataProvider* createPartProvider( const QgsRectangle& extent, int nCols, int iterCols, int iterRows,
        int iterLeft, int iterTop,
        const QString& outputUrl, int fileIndex, int nBands, QGis::DataType type,
        const QgsCoordinateReferenceSystem& crs );

    /** \brie Init VRT (for tiled mode) or create global output provider (single-file mode)
     *  @param destHasNoDataValueList true if destination has no data value, indexed from 0
     *  @param destNoDataValueList no data value, indexed from 0
     */
    QgsRasterDataProvider* initOutput( int nCols, int nRows,
                                       const QgsCoordinateReferenceSystem& crs, double* geoTransform, int nBands,
                                       QGis::DataType type,
                                       QList<bool> destHasNoDataValueList = QList<bool>(), QList<double> destNoDataValueList = QList<double>() );

    /**Calculate nRows, geotransform and pixel size for output*/
    void globalOutputParameters( const QgsRectangle& extent, int nCols, int& nRows, double* geoTransform, double& pixelSize );

    QString partFileName( int fileIndex );
    QString vrtFileName();

    Mode mMode;
    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
    QStringList mCreateOptions;
    QgsCoordinateReferenceSystem mOutputCRS;

    /**False: Write one file, true: create a directory and add the files numbered*/
    bool mTiledMode;
    double mMaxTileWidth;
    double mMaxTileHeight;

    QList< int > mPyramidsList;
    QString mPyramidsResampling;
    QgsRaster::RasterBuildPyramids mBuildPyramidsFlag;
    QgsRaster::RasterPyramidsFormat mPyramidsFormat;
    QStringList mPyramidsConfigOptions;

    QDomDocument mVRTDocument;
    QList<QDomElement> mVRTBands;

    QProgressDialog* mProgressDialog;

    const QgsRasterPipe* mPipe;
    const QgsRasterInterface* mInput;
};

#endif // QGSRASTERFILEWRITER_H
