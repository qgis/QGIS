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

  //! default constructor
  QgsDataSourceURI();
   
  //! constructor which parses input URI
  QgsDataSourceURI(QString uri);
   
  //! All in a single string
  QString text() const;
  
  //! Set all connection related members at once
  void setConnection(const QString& aHost,
                     const QString& aPort,
                     const QString& aDatabase,
                     const QString& aUsername,
                     const QString& aPassword);
  
  //! Set all data source related members at once
  void setDataSource(const QString& aSchema,
                     const QString& aTable,
                     const QString& aGeometryColumn,
                     const QString& aSql = QString());
  
  /* data */

  //! host name
  QString host;
  //! database name
  QString database;
  //! port the database server listens on
  QString port;
  //! schema
  QString schema;
  //! spatial table
  QString table;
  //! geometry column
  QString geometryColumn;
  //! SQL where clause used to limit features returned from the layer
  QString sql;
  //! username 
  QString username;
  //! password
  QString password;

  //! whole connection info (host, db, port, name, pass)
  QString connInfo;
};

