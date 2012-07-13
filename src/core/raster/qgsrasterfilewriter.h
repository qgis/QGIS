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

    WriterError writeRaster( QgsRasterIterator* iter, int nCols, QgsRectangle outputExtent,
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

  private:
    QgsRasterFileWriter(); //forbidden
    WriterError writeDataRaster( QgsRasterIterator* iter, int nCols, const QgsRectangle& outputExtent,
                                 const QgsCoordinateReferenceSystem& crs );
    WriterError writeImageRaster( QgsRasterIterator* iter, int nCols, const QgsRectangle& outputExtent,
                                  const QgsCoordinateReferenceSystem& crs );

    //initialize vrt member variables
    void createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform );
    //write vrt document to disk
    bool writeVRT( const QString& file );
    //add file entry to vrt
    void addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset );
    void buildPyramides( const QString& filename );

    static int pyramidesProgress( double dfComplete, const char *pszMessage, void* pData );

    /**Create provider and datasource for a part image (vrt mode)*/
    QgsRasterDataProvider* createPartProvider( const QgsRectangle& extent, int nCols, int iterCols, int iterRows,
        int iterLeft, int iterTop,
        const QString& outputUrl, int fileIndex, int nBands, QgsRasterInterface::DataType type,
        const QgsCoordinateReferenceSystem& crs );

    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
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
