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
    //! \note enumeration added in version 1.1
    enum SSLmode { SSLprefer, SSLdisable, SSLallow, SSLrequire };

    //! default constructor
    QgsDataSourceURI();

    //! constructor which parses input URI
    QgsDataSourceURI( QString uri );

    //! constructor which parses input encoded URI (generic mode)
    // \note added in 1.9
    QgsDataSourceURI( const QByteArray & uri );

    //! return connection part of URI
    QString connectionInfo() const;

    //! return complete uri
    QString uri() const;

    //! return complete encoded uri (generic mode)
    // \note added in 1.9
    QByteArray encodedUri() const;

    //! set complete encoded uri (generic mode)
    // \note added in 1.9
    void setEncodedUri( const QByteArray & uri );

    //! set complete encoded uri (generic mode)
    // \note added in 1.9
    void setEncodedUri( const QString & uri );

    //! quoted table name
    QString quotedTablename() const;

    //! Set generic param (generic mode)
    // \note if key exists, another is inserted
    // \note added in 1.9
    void setParam( const QString &key, const QString &value );
    void setParam( const QString &key, const QStringList &value );

    //! Get generic param (generic mode)
    // \note added in 1.9
    QString param( const QString &key ) const;

    //! Get multiple generic param (generic mode)
    // \note added in 1.9
    QStringList params( const QString &key ) const;

    //! Test if param exists (generic mode)
    // \note added in 1.9
    bool hasParam( const QString &key ) const;

    //! Set all connection related members at once
    //! \note This optional sslmode parameter has been added in version 1.1
    void setConnection( const QString& aHost,
                        const QString& aPort,
                        const QString& aDatabase,
                        const QString& aUsername,
                        const QString& aPassword,
                        SSLmode sslmode = SSLprefer );

    //! Set all connection related members at once (for the service case)
    //! \note This optional sslmode parameter has been added in version 1.7
    void setConnection( const QString& aService,
                        const QString& aDatabase,
                        const QString& aUsername,
                        const QString& aPassword,
                        SSLmode sslmode = SSLprefer );

    //! Set database
    // \note added in 1.4
    void setDatabase( const QString &database );

    //! Set all data source related members at once
    void setDataSource( const QString& aSchema,
                        const QString& aTable,
                        const QString& aGeometryColumn,
                        const QString& aSql = QString(),
                        const QString& aKeyColumn = QString() );

    //! set username
    // added in 1.5
    void setUsername( QString username );

    //! set password
    // added in 1.5
    void setPassword( QString password );

    //! Removes password element from uris
    static QString removePassword( const QString& aUri );

    QString username() const;
    QString schema() const;
    QString table() const;
    QString sql() const;
    QString geometryColumn() const;

    //! set use Estimated Metadata
    // added in 1.5
    void setUseEstimatedMetadata( bool theFlag );
    bool useEstimatedMetadata() const;

    void disableSelectAtId( bool theFlag );
    bool selectAtIdDisabled() const;

    void clearSchema();
    void setSql( QString sql );

    // added in version 1.1
    QString host() const;
    QString database() const;
    QString port() const;
    QString password() const;
    enum SSLmode sslMode() const;

    // added in 1.7
    QString service() const;

    // added in version 1.2
    QString keyColumn() const;
    void setKeyColumn( QString column );

    // added in 1.9
    QGis::WkbType wkbType() const;
    void setWkbType( QGis::WkbType type );

    QString srid() const;
    void setSrid( QString srid );

  private:
    void skipBlanks( const QString &uri, int &i );
    QString getValue( const QString &uri, int &i );
    QString escape( const QString &uri, QChar delim ) const;

    /* data */

    //! host name
    QString mHost;
    //! port the database server listens on
    QString mPort;
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
    QGis::WkbType mWkbType;
    //! SRID or a null string if not specified
    QString mSrid;
    //! Generic params store
    QMap<QString, QString> mParams;
};

#endif //QGSDATASOURCEURI_H

