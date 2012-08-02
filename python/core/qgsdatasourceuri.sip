/** 
\struct QgsDataSourceURI 
\brief Structure for storing the component parts of a PostgreSQL/RDBMS datasource URI.
           
 This structure stores the database connection information, including host, database,
  user name, password, schema, password, and sql where clause
 */
class QgsDataSourceURI
{
%TypeHeaderCode
#include <qgsdatasourceuri.h>
%End

  public:
    //! \note enumeration added in version 1.1
    enum SSLmode { SSLprefer, SSLdisable, SSLallow, SSLrequire };

    //! default constructor
    QgsDataSourceURI();

    //! constructor which parses input URI
    QgsDataSourceURI( QString uri );

    //! return connection part of URI
    QString connectionInfo() const;

    //! return complete uri
    QString uri() const;

    //! return complete encoded uri (generic mode)
    // \note added in 1.9
    QByteArray encodedUri() const;

    //! set complete encoded uri (generic mode)
    // \note added in 1.9
    void setEncodedUri( const QString & uri );

    //! quoted table name
    QString quotedTablename() const;

    //! Set generic param (generic mode)
    // \note if key exists, another is inserted
    // \note added in 1.9
    void setParam( const QString &key, const QString &value );
    void setParamList( const QString &key, const QStringList &value );
%MethodCode
	sipCpp->setParam( *a0, *a1 );
%End

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
    SSLmode sslMode() const;

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
};
