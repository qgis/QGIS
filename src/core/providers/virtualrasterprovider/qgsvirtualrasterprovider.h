
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
    virtual ~QgsVirtualRasterProvider() override = default;

    QgsRasterBlock *block(Qgis::DataType dataType, int width, int height );
    //QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

    //bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) override;
    //bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;
    bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) override
        { Q_UNUSED( bandNo ) Q_UNUSED( xBlock ); Q_UNUSED( yBlock ); Q_UNUSED( data ); return true; }
    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override
        { Q_UNUSED( bandNo ) Q_UNUSED( viewExtent ); Q_UNUSED( width ); Q_UNUSED( height ); Q_UNUSED( data ); Q_UNUSED( feedback ); true; }


    QgsRectangle extent() const override;
    virtual QString name() const override;
    virtual QString description() const override;

    int xBlockSize() const override;
    int yBlockSize() const override;
    int bandCount() const override;
    //Qgis::DataType dataType( int bandNo ) const override;
    //Qgis::DataType sourceDataType( int bandNo ) const override;

    int xSize() const override;
    int ySize() const override;

    static const QString VR_RASTER_PROVIDER_KEY;
    static const QString VR_RASTER_PROVIDER_DESCRIPTION;

private:
    //I don't really understand it now
    QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other);

    QgsRectangle mExtent;
    int mWidth = 0;
    int mHeight = 0;
    int mBandCount = 1;
    int mXBlockSize = 0;
    int mYBlockSize = 0;

    //! Data type for each band
    std::vector<Qgis::DataType> mDataTypes;
    //! Data size in bytes for each band
    std::vector<int> mDataSizes;
};

#endif // QGSVIRTUALRASTERPROVIDER_H

