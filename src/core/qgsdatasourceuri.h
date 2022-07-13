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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgswkbtypes.h"
#include "qgshttpheaders.h"

#include <QMap>
#include <QSet>

/**
 * \ingroup core
 * \brief Class for storing the component parts of a RDBMS data source URI (e.g. a Postgres data source).
 *
 * This structure stores the database connection information, including host, database,
 * user name, password, schema, password, and SQL where clause.
 */
class CORE_EXPORT QgsDataSourceUri
{
    Q_GADGET
  public:

    /**
     * Available SSL connection modes.
     */
    enum SslMode
    {
      SslPrefer,
      SslDisable,
      SslAllow,
      SslRequire,
      SslVerifyCa,
      SslVerifyFull
    };
    Q_ENUM( SslMode )

    QgsDataSourceUri();

    /**
     * Constructor for QgsDataSourceUri which parses an input \a uri string.
     */
    QgsDataSourceUri( const QString &uri );

    /**
     * Constructor for QgsDataSourceUri which parses an input encoded \a uri).
     * \note not available in Python bindings
     */
    QgsDataSourceUri( const QByteArray &uri ) SIP_SKIP;

    /**
     * Returns the connection part of the URI.
     */
    QString connectionInfo( bool expandAuthConfig = true ) const;

    /**
     * Returns the complete URI as a string.
     */
    QString uri( bool expandAuthConfig = true ) const;

    /**
     * Returns the complete encoded URI as a byte array.
     */
    QByteArray encodedUri() const;

    /**
     * Sets the complete encoded \a uri.
     *
     * \note not available in Python bindings
     */
    void setEncodedUri( const QByteArray &uri ) SIP_SKIP;

    /**
     * Sets the complete encoded \a uri from a string value.
     */
    void setEncodedUri( const QString &uri );

    /**
     * Returns the URI's table name, escaped and quoted.
     */
    QString quotedTablename() const;

    /**
     * Sets a generic parameter \a value on the URI.
     * \note If a parameter with the specified \a key already exists, another is inserted
     * and the existing value is left unchanged.
     */
    void setParam( const QString &key, const QString &value );

    /**
     * Sets a generic parameter list \a value on the URI.
     * \note available in Python as setParamList
     */
    void setParam( const QString &key, const QStringList &value ) SIP_PYNAME( setParamList );

    /**
     * Removes a generic parameter by \a key.
     * \note Calling this method removes all the occurrences of key, and returns the number of parameters removed.
     */
    int removeParam( const QString &key );

    /**
     * Returns a generic parameter value corresponding to the specified \a key.
     */
    QString param( const QString &key ) const;

    /**
     * Returns multiple generic parameter values corresponding to the specified \a key.
     */
    QStringList params( const QString &key ) const;

    /**
     * Returns TRUE if a parameter with the specified \a key exists.
     */
    bool hasParam( const QString &key ) const;

    /**
     * Sets all connection related members at once.
     */
    void setConnection( const QString &aHost,
                        const QString &aPort,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );

    /**
     * Sets all connection related members at once (for a service case).
     */
    void setConnection( const QString &aService,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );

    /**
     * Sets the URI database name.
     */
    void setDatabase( const QString &database );

    /**
     * Sets all data source related members at once.
     *
     * The \a aSql argument represents a subset filter string to be applied to the source, and should take the
     * form of a SQL "where" clause (e.g. "VALUE > 5", "CAT IN (1,2,3)").
     */
    void setDataSource( const QString &aSchema,
                        const QString &aTable,
                        const QString &aGeometryColumn,
                        const QString &aSql = QString(),
                        const QString &aKeyColumn = QString() );

    /**
     * Sets the authentication configuration ID for the URI.
     */
    void setAuthConfigId( const QString &authcfg );

    /**
     * Sets the \a username for the URI.
     */
    void setUsername( const QString &username );

    /**
     * Sets the \a password for the URI.
     */
    void setPassword( const QString &password );

    /**
     * Removes the password element from a URI.
     */
    static QString removePassword( const QString &aUri );

    /**
     * Returns any associated authentication configuration ID stored in the URI.
     */
    QString authConfigId() const;

    //! Returns the username stored in the URI.
    QString username() const;

    //! Returns the schema stored in the URI.
    QString schema() const;

    //! Returns the table name stored in the URI.
    QString table() const;

    /**
     * Returns the SQL filter stored in the URI, if set.
     *
     * This represents a subset filter string to be applied to the source, and takes the
     * form of a SQL "where" clause (e.g. "VALUE > 5", "CAT IN (1,2,3)").
     *
     * \see setSql()
     */
    QString sql() const;

    //! Returns the name of the geometry column stored in the URI, if set.
    QString geometryColumn() const;

    //! Sets whether estimated metadata should be used for the connection.
    void setUseEstimatedMetadata( bool flag );

    //! Returns TRUE if estimated metadata should be used for the connection.
    bool useEstimatedMetadata() const;

    //! Set to TRUE to disable selection by feature ID.
    void disableSelectAtId( bool flag );

    //! Returns whether the selection by feature ID is disabled.
    bool selectAtIdDisabled() const;

    //! Clears the schema stored in the URI.
    void clearSchema();

    /**
     * Sets the \a scheme for the URI.
     * \since QGIS 2.12
     */
    void setSchema( const QString &schema );

    /**
     * Sets the \a sql filter for the URI.
     *
     * The \a sql represents a subset filter string to be applied to the source, and should take the
     * form of a SQL "where" clause (e.g. "VALUE > 5", "CAT IN (1,2,3)").
     *
     * \see sql()
     */
    void setSql( const QString &sql );

    //! Returns the host name stored in the URI.
    QString host() const;
    //! Returns the database name stored in the URI.
    QString database() const;
    //! Returns the port stored in the URI.
    QString port() const;

    /**
     * Returns the driver name stored in the URI
     * \since QGIS 2.16
     */
    QString driver() const;

    /**
     * Sets the \a driver name stored in the URI.
     * \since QGIS 2.16
     */
    void setDriver( const QString &driver );

    //! Returns the password stored in the URI.
    QString password() const;

    //! Returns the SSL mode associated with the URI.
    SslMode sslMode() const;

    //! Returns the service name associated with the URI.
    QString service() const;

    //! Returns the name of the (primary) key column for the referenced table.
    QString keyColumn() const;

    //! Sets the name of the (primary) key \a column.
    void setKeyColumn( const QString &column );

    /**
     * Returns the WKB type associated with the URI.
     */
    QgsWkbTypes::Type wkbType() const;

    //! Sets the WKB \a type associated with the URI.
    void setWkbType( QgsWkbTypes::Type type );

    //! Returns the spatial reference ID associated with the URI.
    QString srid() const;

    //! Sets the spatial reference ID associated with the URI.
    void setSrid( const QString &srid );

    /**
     * Decodes SSL mode string into enum value. If the string is not recognized, SslPrefer is returned.
     * \since QGIS 3.2
     */
    static SslMode decodeSslMode( const QString &sslMode );

    /**
     * Encodes SSL mode enum value into a string.
     * \since QGIS 3.2
     */
    static QString encodeSslMode( SslMode sslMode );

    /**
     * Sets table to \a table
     * \since QGIS 3.10
     */
    void setTable( const QString &table );

    /**
     * Sets geometry column name to \a geometryColumn
     * \since QGIS 3.10
     */
    void setGeometryColumn( const QString &geometryColumn );

    /**
     * Returns parameter keys used in the uri: specialized ones ("table", "schema", etc.) or generic parameters.
     * \since QGIS 3.26
     */
    QSet<QString> parameterKeys() const;

#ifndef SIP_RUN
    //! Returns http headers
    QgsHttpHeaders httpHeaders() const { return mHttpHeaders; }
#endif

    /**
     * Returns http headers
     * \since QGIS 3.26
     */
    QgsHttpHeaders &httpHeaders() { return mHttpHeaders; }

    /**
     * Returns the http header value according to \a key
     * \since QGIS 3.26
     */
    QString httpHeader( const QString &key ) { return mHttpHeaders[key].toString(); }

    /**
     * Sets headers to \a headers
     * \since QGIS 3.26
     */
    void setHttpHeaders( const QgsHttpHeaders &headers ) { mHttpHeaders = headers; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsDataSourceUri: %1>" ).arg( sipCpp->uri( false ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

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
    SslMode mSSLmode = SslPrefer;
    //! key column
    QString mKeyColumn;
    //! Use estimated metadata flag
    bool mUseEstimatedMetadata = false;
    //! Disable SelectAtId capability (e.g., to trigger the attribute table memory model for expensive views)
    bool mSelectAtIdDisabled = false;
    //! Whether mSelectAtIdDisabled has been explicitly set to true or false
    bool mSelectAtIdDisabledSet = false;
    //! geometry type (or QgsWkbTypes::Unknown if not specified)
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    //! SRID or a null string if not specified
    QString mSrid;
    //! Generic params store
    QMultiMap<QString, QString> mParams;
    //! http header store
    QgsHttpHeaders mHttpHeaders;
};

#endif //QGSDATASOURCEURI_H
