/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatasourceuri.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDataSourceUri
{

%TypeHeaderCode
#include "qgsdatasourceuri.h"
%End
  public:
    static const QMetaObject staticMetaObject;

  public:
    enum SslMode
    {
      SslPrefer,
      SslDisable,
      SslAllow,
      SslRequire,
      SslVerifyCa,
      SslVerifyFull
    };

    QgsDataSourceUri();
%Docstring
default constructor
%End

    QgsDataSourceUri( QString uri );
%Docstring
constructor which parses input URI
%End


    QString connectionInfo( bool expandAuthConfig = true ) const;
%Docstring
Returns connection part of URI
%End

    QString uri( bool expandAuthConfig = true ) const;
%Docstring
Returns complete uri
%End

    QByteArray encodedUri() const;
%Docstring
Returns complete encoded uri (generic mode)
%End


    void setEncodedUri( const QString &uri );
%Docstring
Sets complete encoded uri (generic mode)
%End

    QString quotedTablename() const;
%Docstring
quoted table name
%End

    void setParam( const QString &key, const QString &value );
%Docstring
Set generic param (generic mode)

.. note::

   if key exists, another is inserted
%End
    void setParam( const QString &key, const QStringList &value ) /PyName=setParamList/;
%Docstring

.. note::

   available in Python as setParamList
%End

    int removeParam( const QString &key );
%Docstring
Remove generic param (generic mode)

.. note::

   remove all occurrences of key, returns number of params removed
%End

    QString param( const QString &key ) const;
%Docstring
Gets generic param (generic mode)
%End

    QStringList params( const QString &key ) const;
%Docstring
Gets multiple generic param (generic mode)
%End

    bool hasParam( const QString &key ) const;
%Docstring
Test if param exists (generic mode)
%End

    void setConnection( const QString &aHost,
                        const QString &aPort,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );
%Docstring
Sets all connection related members at once
%End

    void setConnection( const QString &aService,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );
%Docstring
Sets all connection related members at once (for the service case)
%End

    void setDatabase( const QString &database );
%Docstring
Sets database
%End

    void setDataSource( const QString &aSchema,
                        const QString &aTable,
                        const QString &aGeometryColumn,
                        const QString &aSql = QString(),
                        const QString &aKeyColumn = QString() );
%Docstring
Sets all data source related members at once
%End

    void setAuthConfigId( const QString &authcfg );
%Docstring
Sets authentication configuration ID
%End

    void setUsername( const QString &username );
%Docstring
Sets username
%End

    void setPassword( const QString &password );
%Docstring
Sets password
%End

    static QString removePassword( const QString &aUri );
%Docstring
Removes password element from uris
%End

    QString authConfigId() const;
%Docstring
Any associated authentication configuration ID
%End

    QString username() const;
%Docstring
Returns the username
%End

    QString schema() const;
%Docstring
Returns the schema
%End

    QString table() const;
%Docstring
Returns the table
%End

    QString sql() const;
%Docstring
Returns the SQL query
%End

    QString geometryColumn() const;
%Docstring
Returns the name of the geometry column
%End

    void setUseEstimatedMetadata( bool flag );
%Docstring
Sets use Estimated Metadata
%End

    bool useEstimatedMetadata() const;
%Docstring
Returns ``True`` if estimated metadata are used
%End

    void disableSelectAtId( bool flag );
%Docstring
Sets to ``True`` to disable selection by id
%End
    bool selectAtIdDisabled() const;
%Docstring
Returns whether the selection by id is disabled
%End

    void clearSchema();
%Docstring
Clears the schema
%End

    void setSchema( const QString &schema );

    void setSql( const QString &sql );
%Docstring
Sets the SQL query
%End

    QString host() const;
%Docstring
Returns the host
%End
    QString database() const;
%Docstring
Returns the database
%End
    QString port() const;
%Docstring
Returns the port
%End
    QString driver() const;
    void setDriver( const QString &driver );
    QString password() const;
%Docstring
Returns the password
%End
    SslMode sslMode() const;
%Docstring
Returns the SSL mode
%End

    QString service() const;
%Docstring
Returns the service name
%End

    QString keyColumn() const;
%Docstring
Returns the name of the (primary) key column
%End
    void setKeyColumn( const QString &column );
%Docstring
Sets the name of the (primary) key column
%End

    QgsWkbTypes::Type wkbType() const;
%Docstring
The wkb type.
%End

    void setWkbType( QgsWkbTypes::Type type );
%Docstring
Sets the wkb type
%End

    QString srid() const;
%Docstring
Returns the srid
%End
    void setSrid( const QString &srid );
%Docstring
Sets the srid
%End

    static SslMode decodeSslMode( const QString &sslMode );
%Docstring
Decodes SSL mode string into enum value. If the string is not recognized, SslPrefer is returned.

.. versionadded:: 3.2
%End

    static QString encodeSslMode( SslMode sslMode );
%Docstring
Encodes SSL mode enum value into a string.

.. versionadded:: 3.2
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatasourceuri.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
