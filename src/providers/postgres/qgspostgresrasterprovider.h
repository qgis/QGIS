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

#include <exception>

class QgsPostgresRasterProvider : public QgsRasterDataProvider
{

    Q_OBJECT

  public:

    QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
    explicit QgsPostgresRasterProvider( const QgsPostgresRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions );

    virtual ~QgsPostgresRasterProvider() override;

    // QgsDataProvider interface
  public:
    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QgsRectangle extent() const override;
    virtual bool isValid() const override;
    virtual QString name() const override;
    virtual QString description() const override;

    bool readBlock( int bandNo, QgsRectangle const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    // QgsRasterInterface interface
    virtual Qgis::DataType dataType( int bandNo ) const override;
    virtual int bandCount() const override;
    virtual QgsRasterInterface *clone() const override;
    virtual Qgis::DataType sourceDataType( int bandNo ) const override;

    // QgsRasterDataProvider interface
    virtual QString htmlMetadata() override;
    virtual QString lastErrorTitle() override;
    virtual QString lastError() override;
    int capabilities() const override;

    // Utility functions
    //! Parses a WKB raster and returns information as a variant map
    static QVariantMap parseWkb( const QByteArray &wkb );

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
    //! SQL statement used to limit the features retrieved
    QString mSqlWhereClause;
    //! Rectangle that contains the extent (bounding box) of the layer
    mutable QgsRectangle mExtent;
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
    //! Nodata values for each band
    std::vector<double> mNoDataValues;
    //! Store overviews
    QMap<int, QString> mOverViews;
    //! Band count
    int mBandCount = 0;
    //! If is tiled
    bool mIsTiled = false;
    //! If is out of DB
    bool mIsOutOfDb = false;
    //! Has spatial index
    bool mHasSpatialIndex = false;
    //! Tile size x
    int mWidth = 0;
    //! Tile size y
    int mHeight = 0;
    //! Scale x
    int mScaleX = 0;
    //! Scale y
    int mScaleY = 0;

    QString mDetectedSrid;            //!< Spatial reference detected in the database
    QString mRequestedSrid;           //!< Spatial reference requested in the uri
    QgsPostgresConn *mConnectionRO = nullptr ; //!< Read-only database connection (initially)
    QgsPostgresConn *mConnectionRW = nullptr ; //!< Read-write database connection (on update)

    QgsPostgresConn *connectionRO() const;
    QgsPostgresConn *connectionRW();
    bool hasSufficientPermsAndCapabilities();
    void disconnectDb();
    //! Get SRID and data type, FALSE if it's not a valid raster table
    bool getDetails();
    //! Search for overviews and store a map
    void findOverviews();
    //! Find the overview table name for a given scale
    QString overviewName( const double scale ) const;

    static QString quotedIdentifier( const QString &ident ) { return QgsPostgresConn::quotedIdentifier( ident ); }
    static QString quotedValue( const QVariant &value ) { return QgsPostgresConn::quotedValue( value ); }
    static QString quotedJsonValue( const QVariant &value ) { return QgsPostgresConn::quotedJsonValue( value ); }
    static QString quotedByteaValue( const QVariant &value );


    // QgsRasterInterface interface
  public:
    int xSize() const override;
    int ySize() const override;
};


struct QgsPostgresRasterProviderException: public std::exception
{
  QgsPostgresRasterProviderException( const QString &msg )
    : message( msg )
  {}

  QString message;
};

class QgsPostgresRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsPostgresRasterProviderMetadata();
    QVariantMap decodeUri( const QString &uri ) override;
    QgsPostgresRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options ) override;

};

#endif // QGSPOSTGRESRASTERPROVIDER_H
