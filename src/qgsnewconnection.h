#ifndef QGSNEWCONNECTION_H
#define QGSNEWCONNECTION_H
#include "qgsnewconnectionbase.h"
/*! \class QgsNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a PostgresQl database
 */
class QgsNewConnection : public QgsNewConnectionBase 
{
 public:
    //! Constructor
    QgsNewConnection(QString connName= QString::null);
    //! Destructor
    ~QgsNewConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
};

#endif //  QGSNEWCONNECTIONBASE_H
