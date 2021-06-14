
#ifndef QGSVIRTUALRASTERPROVIDER_H
#define QGSVIRTUALRASTERPROVIDER_H

#include <QObject>

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprovidermetadata.h"


class QgsVirtualRasterProvider : public QgsRasterDataProvider
{
    Q_OBJECT
public:

    QgsVirtualRasterProvider ( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions);
    //QgsVirtualRasterProvider ( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    virtual ~QgsVirtualRasterProvider() override = default;

    QString dataSourceUri( bool expandAuthConfig = false ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;


     //since it was not useful at the moment i keep it away from the class
   //bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) override
    //    { Q_UNUSED( bandNo ) Q_UNUSED( xBlock ); Q_UNUSED( yBlock ); Q_UNUSED( data ); return true; }
    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override
        { Q_UNUSED( bandNo ) Q_UNUSED( viewExtent ); Q_UNUSED( width ); Q_UNUSED( height ); Q_UNUSED( data ); Q_UNUSED( feedback ); return true; }



    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    virtual QString name() const override;
    virtual QString description() const override;

    int xBlockSize() const override;
    int yBlockSize() const override;
    int bandCount() const override;
    QgsVirtualRasterProvider *clone() const override;
    int capabilities() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;

    int xSize() const override;
    int ySize() const override;

    QString htmlMetadata() override;
    QString lastErrorTitle() override;
    QString lastError() override;

    static QString providerKey();


private:

    QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other);

    bool mValid = false;

    QgsCoordinateReferenceSystem mCrs;
    //Needs extent and pixel size/nr of row and columns (Sets gdal 6 parameters array from mOutputRectangle, mNumOutputColumns, mNumOutputRows)
    //QgsRectangle mExtent;
    //int mWidth= 0;
    //int mHeight= 0;

    //another hardcoded ex.
    QgsRectangle mExtent = QgsRectangle(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
    int mWidth= 373;
    int mHeight= 350;


/*
    //rect to show also added hardcoded layer (only for this case)
    QgsRectangle mExtent = QgsRectangle(1,2,3,4);
    int mWidth = mExtent.width();
    int mHeight = mExtent.height();
*/

    int mBandCount = 1;
    int mXBlockSize = 0;
    int mYBlockSize = 0;

    //! Data type for each band
    std::vector<Qgis::DataType> mDataTypes;
    //! Data size in bytes for each band
    std::vector<int> mDataSizes;

};

class QgsVirtualRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsVirtualRasterProviderMetadata();
    //QVariantMap decodeUri( const QString &uri ) const override;
    //QgsVirtualRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options);//, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QgsVirtualRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
   /*
    QgsVirtualRasterProvider *createRasterDataProvider(
          const QString &uri,
          const QString &format,
          int nBands,
          Qgis::DataType type,
          int width,
          int height,
          double *geoTransform,
          const QgsCoordinateReferenceSystem &crs,
          const QStringList &createOptions ) override;
    */
    //QString encodeUri( const QVariantMap &parts ) const override;
};

#endif // QGSVIRTUALRASTERPROVIDER_H

