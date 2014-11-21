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
    QByteArray encodedUri() const;

    //! set complete encoded uri (generic mode)
    void setEncodedUri( const QString & uri );

    //! quoted table name
    QString quotedTablename() const;

    //! Set generic param (generic mode)
    // \note if key exists, another is inserted
    void setParam( const QString &key, const QString &value );
    void setParam( const QString &key, const QStringList &value ) /PyName=setParamList/;

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
                        SSLmode sslmode = SSLprefer );

    //! Set all connection related members at once (for the service case)
    void setConnection( const QString& aService,
                        const QString& aDatabase,
                        const QString& aUsername,
                        const QString& aPassword,
                        SSLmode sslmode = SSLprefer );

    //! Set database
    void setDatabase( const QString &database );

    //! Set all data source related members at once
    void setDataSource( const QString& aSchema,
                        const QString& aTable,
                        const QString& aGeometryColumn,
                        const QString& aSql = QString(),
                        const QString& aKeyColumn = QString() );

    //! set username
    void setUsername( QString username );

    //! set password
    void setPassword( QString password );

    //! Removes password element from uris
    static QString removePassword( const QString& aUri );

    QString username() const;
    QString schema() const;
    QString table() const;
    QString sql() const;
    QString geometryColumn() const;

    //! set use Estimated Metadata
    void setUseEstimatedMetadata( bool theFlag );
    bool useEstimatedMetadata() const;

    void disableSelectAtId( bool theFlag );
    bool selectAtIdDisabled() const;

    void clearSchema();
    void setSql( QString sql );

    QString host() const;
    QString database() const;
    QString port() const;
    QString password() const;
    SSLmode sslMode() const;

    QString service() const;

    QString keyColumn() const;
    void setKeyColumn( QString column );

    QGis::WkbType wkbType() const;
    void setWkbType( QGis::WkbType type );

    QString srid() const;
    void setSrid( QString srid );
};
