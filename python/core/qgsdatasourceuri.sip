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
return connection part of URI
 :rtype: str
%End

    QString uri( bool expandAuthConfig = true ) const;
%Docstring
return complete uri
 :rtype: str
%End

    QByteArray encodedUri() const;
%Docstring
return complete encoded uri (generic mode)
 :rtype: QByteArray
%End


    void setEncodedUri( const QString &uri );
%Docstring
set complete encoded uri (generic mode)
%End

    QString quotedTablename() const;
%Docstring
quoted table name
 :rtype: str
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
 :rtype: int
%End

    QString param( const QString &key ) const;
%Docstring
Get generic param (generic mode)
 :rtype: str
%End

    QStringList params( const QString &key ) const;
%Docstring
Get multiple generic param (generic mode)
 :rtype: list of str
%End

    bool hasParam( const QString &key ) const;
%Docstring
Test if param exists (generic mode)
 :rtype: bool
%End

    void setConnection( const QString &aHost,
                        const QString &aPort,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );
%Docstring
Set all connection related members at once
%End

    void setConnection( const QString &aService,
                        const QString &aDatabase,
                        const QString &aUsername,
                        const QString &aPassword,
                        SslMode sslmode = SslPrefer,
                        const QString &authConfigId = QString() );
%Docstring
Set all connection related members at once (for the service case)
%End

    void setDatabase( const QString &database );
%Docstring
Set database
%End

    void setDataSource( const QString &aSchema,
                        const QString &aTable,
                        const QString &aGeometryColumn,
                        const QString &aSql = QString(),
                        const QString &aKeyColumn = QString() );
%Docstring
Set all data source related members at once
%End

    void setAuthConfigId( const QString &authcfg );
%Docstring
Set authentication configuration ID
%End

    void setUsername( const QString &username );
%Docstring
set username
%End

    void setPassword( const QString &password );
%Docstring
set password
%End

    static QString removePassword( const QString &aUri );
%Docstring
Removes password element from uris
 :rtype: str
%End

    QString authConfigId() const;
%Docstring
Any associated authentication configuration ID
 :rtype: str
%End

    QString username() const;
%Docstring
Returns the username
 :rtype: str
%End

    QString schema() const;
%Docstring
Returns the schema
 :rtype: str
%End

    QString table() const;
%Docstring
Returns the table
 :rtype: str
%End

    QString sql() const;
%Docstring
Returns the SQL query
 :rtype: str
%End

    QString geometryColumn() const;
%Docstring
Return the name of the geometry column
 :rtype: str
%End

    void setUseEstimatedMetadata( bool flag );
%Docstring
set use Estimated Metadata
%End

    bool useEstimatedMetadata() const;
%Docstring
Returns true if estimated metadata are used
 :rtype: bool
%End

    void disableSelectAtId( bool flag );
%Docstring
Set to true to disable selection by id
%End
    bool selectAtIdDisabled() const;
%Docstring
Returns whether the selection by id is disabled
 :rtype: bool
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
 :rtype: str
%End
    QString database() const;
%Docstring
Returns the database
 :rtype: str
%End
    QString port() const;
%Docstring
Returns the port
 :rtype: str
%End
    QString driver() const;
%Docstring
 :rtype: str
%End
    void setDriver( const QString &driver );
    QString password() const;
%Docstring
Returns the password
 :rtype: str
%End
    SslMode sslMode() const;
%Docstring
Returns the SSL mode
 :rtype: SslMode
%End

    QString service() const;
%Docstring
Returns the service name
 :rtype: str
%End

    QString keyColumn() const;
%Docstring
Returns the name of the (primary) key column
 :rtype: str
%End
    void setKeyColumn( const QString &column );
%Docstring
Sets the name of the (primary) key column
%End

    QgsWkbTypes::Type wkbType() const;
%Docstring
 The wkb type.
 :rtype: QgsWkbTypes.Type
%End

    void setWkbType( QgsWkbTypes::Type type );
%Docstring
Sets the wkb type
%End

    QString srid() const;
%Docstring
Returns the srid
 :rtype: str
%End
    void setSrid( const QString &srid );
%Docstring
Sets the srid
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatasourceuri.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
