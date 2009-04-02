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
  QgsDataSourceURI(QString uri);
   
  //! connection info
  QString connectionInfo() const;

  //! complete uri
  QString uri() const;

  //! quoted table name
  QString quotedTablename() const;
  
  //! Set all connection related members at once
  void setConnection(const QString& aHost,
                     const QString& aPort,
                     const QString& aDatabase,
                     const QString& aUsername,
                     const QString& aPassword,
                     SSLmode sslmode );
  
  //! Set all data source related members at once
  void setDataSource(const QString& aSchema,
                     const QString& aTable,
                     const QString& aGeometryColumn,
                     const QString& aSql = QString());
  
  QString username() const;
  QString schema() const;
  QString table() const;
  QString sql() const;
  QString geometryColumn() const;

  void setSql(QString sql);
};
