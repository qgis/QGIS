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
  //! All in a single string
  QString text();
};

