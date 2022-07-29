/***************************************************************************
  qgspostgresrasterprovider.h - QgsPostgresRasterProvider

 ---------------------
 begin                : 20.12.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESRASTERPROVIDER_H
#define QGSPOSTGRESRASTERPROVIDER_H

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprovidermetadata.h"
#include "qgspostgresconn.h"
#include "qgspostgresprovider.h"
#include "qgsogrutils.h"
#include "qgspostgresrastershareddata.h"

#include <exception>

/**
 * The QgsPostgresRasterProvider class implements a raster data provider for PostGIS
 */
class QgsPostgresRasterProvider : public QgsRasterDataProvider
{

    Q_OBJECT

  public:

    QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    explicit QgsPostgresRasterProvider( const QgsPostgresRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    virtual ~QgsPostgresRasterProvider() override = default;

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
    virtual QgsPostgresRasterProvider *clone() const override;
    virtual Qgis::DataType sourceDataType( int bandNo ) const override;
    virtual int xBlockSize() const override;
    virtual int yBlockSize() const override;
    virtual QgsRasterBandStats bandStatistics( int bandNo, int stats, const QgsRectangle &extent, int sampleSize, QgsRasterBlockFeedback *feedback ) override;

    // QgsRasterDataProvider interface
    virtual QString htmlMetadata() override;
    virtual QString lastErrorTitle() override;
    virtual QString lastError() override;
    int capabilities() const override;
    QgsFields fields() const override;

    // QgsRasterInterface interface
    int xSize() const override;
    int ySize() const override;

    static const QString PG_RASTER_PROVIDER_KEY;
    static const QString PG_RASTER_PROVIDER_DESCRIPTION;

  private:

    bool mValid = false;
    QgsCoordinateReferenceSystem mCrs;
    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;
    //! provider references query (instead of a table)
    bool mIsQuery;
    //! Name of the table with no schema
    QString mTableName;
    //! Name of the table or subquery
    QString mQuery;
    //! Name of the raster column
    QString mRasterColumn;
    //! Name of the schema
    QString mSchemaName;
    //! SQL statement used to limit the features retrieved (subset string)
    QString mSqlWhereClause;
    //! Use estimated metadata. Uses fast table counts, geometry type and extent determination
    bool mUseEstimatedMetadata = true;
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
    //! If is out of DB
    bool mIsOutOfDb = false;
    //! Has spatial index
    bool mHasSpatialIndex = false;
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
    //! Temporal field index
    int mTemporalFieldIndex = -1;
    //! Temporal default time
    QDateTime mTemporalDefaultTime;
    //! Keep track of fields
    QgsFields mAttributeFields;
    //! Keeps track of identity fields
    QHash<int, char> mIdentityFields;
    //! Keeps track of default values
    QHash<int, QString> mDefaultValues;
    //! Data comment
    QString mDataComment;
    //! Layer metadata
    QgsLayerMetadata mLayerMetadata;


    QString mDetectedSrid;            //!< Spatial reference detected in the database
    QString mRequestedSrid;           //!< Spatial reference requested in the uri
    QgsPostgresConn *mConnectionRO = nullptr ; //!< Read-only database connection (initially)
    QgsPostgresConn *mConnectionRW = nullptr ; //!< Read-write database connection (on update)

    /**
     * Data type for the primary key
     */
    QgsPostgresPrimaryKeyType mPrimaryKeyType = PktUnknown;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<QString> mPrimaryKeyAttrs;

    //! Mutable data shared between provider and feature sources
    std::shared_ptr<QgsPostgresRasterSharedData> mShared;

    QString mDbName;

    // Methods

    QgsPostgresConn *connectionRO() const;
    QgsPostgresConn *connectionRW();

    bool supportsSubsetString() const override { return true; }

    QString subsetString() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) override;

    //! Subset string with temporal range from request (if any)
    QString subsetStringWithTemporalRange() const;

    //! Subset string with only the temporal default time part
    QString defaultTimeSubsetString( const QDateTime &defaultTime ) const;

    bool hasSufficientPermsAndCapabilities();
    void disconnectDb();
    //! Initialize the raster by fetching metadata and creating spatial indexes.
    bool init();
    //! Initialize fields and temporal capabilities
    bool initFieldsAndTemporal();

    //! Search for overviews and store a map
    void findOverviews();
    //! Initialize spatial indexes

    static QString quotedIdentifier( const QString &ident ) { return QgsPostgresConn::quotedIdentifier( ident ); }
    static QString quotedValue( const QVariant &value ) { return QgsPostgresConn::quotedValue( value ); }
    static QString quotedJsonValue( const QVariant &value ) { return QgsPostgresConn::quotedJsonValue( value ); }
    static QString quotedByteaValue( const QVariant &value );
    QgsPostgresProvider::Relkind relkind() const;
    bool loadFields();

    /**
     * Determine the fields making up the primary key
     */
    bool determinePrimaryKey();

    /**
     * Determine the fields making up the primary key from the uri attribute keyColumn
     *
     * Fills mPrimaryKeyType and mPrimaryKeyAttrs
     * from mUri
     */
    void determinePrimaryKeyFromUriKeyColumn();

    /**
     * Returns the quoted SQL frament to retrieve the PK from the raster table
     */
    QString pkSql();

    /**
     * Returns table comment
     */
    QString dataComment() const override;


    /**
     * Private struct for column type information
     */
    struct PGTypeInfo
    {
      QString typeName;
      QString typeType;
      QString typeElem;
      int typeLen;
    };

    QStringList parseUriKey( const QString &key );

};



struct QgsPostgresRasterProviderException: public std::exception
{

  QgsPostgresRasterProviderException( const QString &msg );

  QString message;
};

class QgsPostgresRasterProviderMetadata: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsPostgresRasterProviderMetadata();
    QIcon icon() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QgsPostgresRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};



#endif // QGSPOSTGRESRASTERPROVIDER_H
