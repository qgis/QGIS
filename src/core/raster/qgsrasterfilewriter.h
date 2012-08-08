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
#include "qgsrectangle.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

class QProgressDialog;
class QgsRasterIterator;

class CORE_EXPORT QgsRasterFileWriter
{
  public:
    enum WriterError
    {
      NoError = 0,
      SourceProviderError = 1,
      DestProviderError = 2,
      CreateDatasourceError = 3,
      WriteError = 4
    };

    QgsRasterFileWriter( const QString& outputUrl );
    ~QgsRasterFileWriter();

    /**Write raster file
        @param iter raster iterator
        @param nCols number of output columns
        @param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
        @param outputExtent extent to output
        @param crs crs to reproject to
        @param p dialog to show progress in */
    WriterError writeRaster( QgsRasterIterator* iter, int nCols, int nRows, QgsRectangle outputExtent,
                             const QgsCoordinateReferenceSystem& crs, QProgressDialog* p = 0 );

    void setOutputFormat( const QString& format ) { mOutputFormat = format; }
    QString outputFormat() const { return mOutputFormat; }

    void setOutputProviderKey( const QString& key ) { mOutputProviderKey = key; }
    QString outputProviderKey() const { return mOutputProviderKey; }

    void setTiledMode( bool t ) { mTiledMode = t; }
    bool tiledMode() const { return mTiledMode; }

    void setMaxTileWidth( int w ) { mMaxTileWidth = w; }
    int maxTileWidth() const { return mMaxTileWidth; }

    void setMaxTileHeight( int h ) { mMaxTileHeight = h; }
    int maxTileHeight() const { return mMaxTileHeight; }

    // for now not putting createOptions in all methods, use createOptions()
    void setCreateOptions( const QStringList& list ) { mCreateOptions = list; }
    QStringList createOptions() const { return mCreateOptions; }

  private:
    QgsRasterFileWriter(); //forbidden
    WriterError writeDataRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
                                 const QgsCoordinateReferenceSystem& crs, QProgressDialog* p = 0 );
    WriterError writeImageRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
                                  const QgsCoordinateReferenceSystem& crs, QProgressDialog* p = 0 );

    //initialize vrt member variables
    void createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform );
    //write vrt document to disk
    bool writeVRT( const QString& file );
    //add file entry to vrt
    void addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset );
    void buildPyramids( const QString& filename );

    static int pyramidsProgress( double dfComplete, const char *pszMessage, void* pData );

    /**Create provider and datasource for a part image (vrt mode)*/
    QgsRasterDataProvider* createPartProvider( const QgsRectangle& extent, int nCols, int iterCols, int iterRows,
        int iterLeft, int iterTop,
        const QString& outputUrl, int fileIndex, int nBands, QgsRasterInterface::DataType type,
        const QgsCoordinateReferenceSystem& crs );

    /**Init VRT (for tiled mode) or create global output provider (single-file mode)*/
    QgsRasterDataProvider* initOutput( int nCols, int nRows, const QgsCoordinateReferenceSystem& crs, double* geoTransform, int nBands,
                                       QgsRasterInterface::DataType type );

    /**Calculate nRows, geotransform and pixel size for output*/
    void globalOutputParameters( const QgsRectangle& extent, int nCols, int& nRows, double* geoTransform, double& pixelSize );

    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
    QStringList mCreateOptions;
    QgsCoordinateReferenceSystem mOutputCRS;

    /**False: Write one file, true: create a directory and add the files numbered*/
    bool mTiledMode;
    double mMaxTileWidth;
    double mMaxTileHeight;

    QDomDocument mVRTDocument;
    QDomElement mVRTRedBand;
    QDomElement mVRTGreenBand;
    QDomElement mVRTBlueBand;
    QDomElement mVRTAlphaBand;

    QProgressDialog* mProgressDialog;
};

#endif // QGSRASTERFILEWRITER_H
