/***************************************************************************
      qgsdatasourceuri.h  -  Structure to contain the component parts
                             of a data source URI
                             -------------------
    begin                : Dec 5, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATASOURCEURI_H
#define QGSDATASOURCEURI_H

#include "qgis.h"

#include <QMap>

/** \ingroup core
 * Class for storing the component parts of a PostgreSQL/RDBMS datasource URI.
 * This structure stores the database connection information, including host, database,
 * user name, password, schema, password, and sql where clause
 *
 * Extended to support generic params so that it may be used by any provider.
 * The 2 modes (the old - RDMS specific and the new generic) may not yet be mixed.
 * (Radim Blazek 4/2012)
 */
class CORE_EXPORT QgsDataSourceURI
{
  public:
    enum SSLmode { SSLprefer, SSLdisable, SSLallow, SSLrequire, SSLverifyCA, SSLverifyFull };

    //! default constructor
    QgsDataSourceURI();

    //! constructor which parses input URI
    QgsDataSourceURI( QString uri );

    //! constructor which parses input encoded URI (generic mode)
    QgsDataSourceURI( const QByteArray & uri );

    //! return connection part of URI
    QString connectionInfo( bool expandAuthConfig = true ) const;

    //! return complete uri
    QString uri( bool expandAuthConfig = true ) const;

    //! return complete encoded uri (generic mode)
    QByteArray encodedUri() const;

    //! set complete encoded uri (generic mode)
    // \note not available in python bindings
    void setEncodedUri( const QByteArray & uri );

    //! set complete encoded uri (generic mode)
    void setEncodedUri( const QString & uri );

    //! quoted table name
    QString quotedTablename() const;

    //! Set generic param (generic mode)
    // \note if key exists, another is inserted
    void setParam( const QString &key, const QString &value );
    //! @note available in python as setParamList
    void setParam( const QString &key, const QStringList &value );

    //! Remove generic param (generic mode)
    // \note remove all occurrences of key, returns number of params removed
    int removeParam( const QString &key );

    //! Get generic param (generic mode)
    QString param( const QString &key ) const;

    //! Get multiple generic param (generic mode)
    QStringList params( const QString &key ) const;

    //! Test if param exists (generic mode)
    bool hasParam( const QString &key ) const;

    //! Set all connection related members at once
    void setConnection( const QString& aHost,
                        const QString& aPort,
                        const QString& aDatabase,
                        const QString& aUsername,
                        const QString& aPassword,
                        SSLmode sslmode = SSLprefer,
                        const QString& authConfigId = QString() );

    //! Set all connection related members at once (for the service case)
    void setConnection( const QString& aService,
                        const QString& aDatabase,
                        const QString& aUsername,
                        const QString& aPassword,
                        SSLmode sslmode = SSLprefer,
                        const QString& authConfigId = QString() );

    //! Set database
    void setDatabase( const QString &database );

    //! Set all data source related members at once
    void setDataSource( const QString& aSchema,
                        const QString& aTable,
                        const QString& aGeometryColumn,
                        const QString& aSql = QString(),
                        const QString& aKeyColumn = QString() );

    //! Set authentication configuration ID
    void setAuthConfigId( const QString& authcfg );

    //! set username
    void setUsername( const QString& username );

    //! set password
    void setPassword( const QString& password );

    //! Removes password element from uris
    static QString removePassword( const QString& aUri );

    //! Any associated authentication configuration ID
    QString authConfigId() const;

    //! Returns the username
    QString username() const;

    //! Returns the schema
    QString schema() const;

    //! Returns the table
    QString table() const;

    //! Returns the SQL query
    QString sql() const;

    //! Return the name of the geometry column
    QString geometryColumn() const;

    //! set use Estimated Metadata
    void setUseEstimatedMetadata( bool theFlag );

    //! Returns true if estimated metadata are used
    bool useEstimatedMetadata() const;

    //! Set to true to disable selection by id
    void disableSelectAtId( bool theFlag );
    //! Returns whether the selection by id is disabled
    bool selectAtIdDisabled() const;

    //! Clears the schema
    void clearSchema();

    //! set the table schema
    // @note added in 2.11
    void setSchema( const QString& schema );

    //! Sets the SQL query
    void setSql( const QString& sql );

    //! Returns the host
    QString host() const;
    //! Returns the database
    QString database() const;
    //! Returns the port
    QString port() const;
    //! Returns the driver
    // @note added in QGIS 2.16
    QString driver() const;
    //! Sets the driver name
    // @note added in QGIS 2.16
    void setDriver( const QString& driver );
    //! Returns the password
    QString password() const;
    //! Returns the SSL mode
    enum SSLmode sslMode() const;

    //! Returns the service name
    QString service() const;

    //! Returns the name of the (primary) key column
    QString keyColumn() const;
    //! Sets the name of the (primary) key column
    void setKeyColumn( const QString& column );

    /** The (old) wkb type.
        @deprecated Will return QgsWKBTypes::Type in 3.0. Prefer newWkbType() in the meantime */
    Q_DECL_DEPRECATED QGis::WkbType wkbType() const;

    /** The wkb type.
        @note Will be removed in 3.0 */
    QgsWKBTypes::Type newWkbType() const;

    //! @deprecated See setWkbType( QgsWKBTypes::Type )
    Q_DECL_DEPRECATED void setWkbType( QGis::WkbType type );

    //! Sets the wkb type
    void setWkbType( QgsWKBTypes::Type type );

    //! Returns the srid
    QString srid() const;
    //! Sets the srid
    void setSrid( const QString& srid );

  private:
    void skipBlanks( const QString &uri, int &i );
    QString getValue( const QString &uri, int &i );
    QString escape( const QString &uri, QChar delim ) const;

    /* data */

    //! host name
    QString mHost;
    //! port the database server listens on
    QString mPort;
    //! device driver for ODBC
    QString mDriver;
    //! service name
    QString mService;
    //! database name
    QString mDatabase;
    //! schema
    QString mSchema;
    //! spatial table
    QString mTable;
    //! geometry column
    QString mGeometryColumn;
    //! SQL query or where clause used to limit features returned from the layer
    QString mSql;
    //! authentication configuration ID
    QString mAuthConfigId;
    //! username
    QString mUsername;
    //! password
    QString mPassword;
    //! ssl mode
    enum SSLmode mSSLmode;
    //! key column
    QString mKeyColumn;
    //! Use estimated metadata flag
    bool mUseEstimatedMetadata;
    //! Disable SelectAtId capability (eg. to trigger the attribute table memory model for expensive views)
    bool mSelectAtIdDisabled;
    //! geometry type (or QGis::WKBUnknown if not specified)
    QgsWKBTypes::Type mWkbType;
    //! SRID or a null string if not specified
    QString mSrid;
    //! Generic params store
    QMap<QString, QString> mParams;
};

#endif //QGSDATASOURCEURI_H

