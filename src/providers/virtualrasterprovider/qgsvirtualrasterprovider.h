
#ifndef QGSVIRTUALRASTERPROVIDER_H
#define QGSVIRTUALRASTERPROVIDER_H

#include <QObject>

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprovidermetadata.h"
#include "qgsrastercalculator.h"

class QgsVirtualRasterProvider : public QgsRasterDataProvider
{
    Q_OBJECT
  public:

    QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
    //QgsVirtualRasterProvider ( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    virtual ~QgsVirtualRasterProvider() override;

    QString dataSourceUri( bool expandAuthConfig = false ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;
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

    QString formulaString();

  private:

    QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other );

    bool mValid = false;

    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    int mWidth;
    int mHeight;
    int mBandCount = 1;
    int mXBlockSize = 0;
    int mYBlockSize = 0;

    //! Data type for each band
    std::vector<Qgis::DataType> mDataTypes;
    //! Data size in bytes for each band
    std::vector<int> mDataSizes;

    //from qgsrastercalc
    QString mFormulaString;
    QVector<QgsRasterCalculatorEntry> mRasterEntries;
    QString mLastError;

    std::unique_ptr< QgsRasterCalcNode > mCalcNode;//( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );
    QVector <QgsRasterLayer *> mRasterLayers;
};

class QgsVirtualRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsVirtualRasterProviderMetadata();
    QgsVirtualRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
};

#endif // QGSVIRTUALRASTERPROVIDER_H

