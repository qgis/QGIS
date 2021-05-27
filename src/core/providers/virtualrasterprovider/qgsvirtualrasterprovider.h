#ifndef QGSVIRTUALRASTERPROVIDER_H
#define QGSVIRTUALRASTERPROVIDER_H

#include <QObject>

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprovidermetadata.h"

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE
class QgsVirtualRasterProvider : public QgsRasterDataProvider
{
    Q_OBJECT
public:
    //QgsVirtualRasterProvider ( const QString &uri);
    QgsVirtualRasterProvider ( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    //explicit QgsVirtualRasterProvider(const QgsVirtualRasterProvider &other);
    explicit QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    virtual ~QgsVirtualRasterProvider() override = default;

public:
    // QgsDataProvider interface
    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QgsRectangle extent() const override;
    virtual bool isValid() const override;
    virtual QString name() const override;
    virtual QString description() const override;

    bool readBlock( int bandNo, QgsRectangle const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    // QgsRasterInterface interface
    virtual Qgis::DataType dataType( int bandNo ) const override;
    virtual int bandCount() const override;
    virtual QgsVirtualRasterProvider *clone() const override;
    virtual Qgis::DataType sourceDataType( int bandNo ) const override;
    virtual int xBlockSize() const override;
    virtual int yBlockSize() const override;
    //virtual QgsRasterBandStats bandStatistics( int bandNo, int stats, const QgsRectangle &extent, int sampleSize, QgsRasterBlockFeedback *feedback ) override;

    // QgsRasterInterface interface
    int xSize() const override;
    int ySize() const override;

    // QgsRasterDataProvider interface
    virtual QString htmlMetadata() override;
    virtual QString lastErrorTitle() override; //not useful at the mom
    virtual QString lastError() override; //not useful at the mom
    int capabilities() const override;
    //QgsFields fields() const override;

    static const QString VR_RASTER_PROVIDER_KEY;
    static const QString VR_RASTER_PROVIDER_DESCRIPTION;

signals:

private:

    //QImage draw( const QgsRectangle &viewExtent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr );
    QImage *draw(int width, int height);

    QgsCoordinateReferenceSystem mCrs;
    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;
    bool mValid = false;
    //! Error information
    QString mError;
    //! Error title
    QString mErrorTitle;
    //! Data type for each band
    std::vector<Qgis::DataType> mDataTypes;
    //! Data size in bytes for each band
    std::vector<int> mDataSizes;
    //! Store overviews
    QMap<unsigned int, QString> mOverViews;
    //! Band count
    int mBandCount = 0;
    //! If is tiled
    bool mIsTiled = false;
    //! Raster size x
    long mWidth = 0;
    //! Raster size y
    long mHeight = 0;
    //! Raster tile size x
     int mTileWidth = 0;
     //! Raster tile size y
     int mTileHeight = 0;
     //! Scale x
     double mScaleX = 0;
     //! Scale y
     double mScaleY = 0;

};

class QgsVirtualRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsVirtualRasterProviderMetadata();
    //QVariantMap decodeUri( const QString &uri ) const override;
    QgsVirtualRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;

    //QString encodeUri( const QVariantMap &parts ) const override;
};


#endif // QGSVIRTUALRASTERPROVIDER_H
