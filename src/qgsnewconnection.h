#ifndef QGSNEWCONNECTION_H
#define QGSNEWCONNECTION_H
#include "qgsnewconnectionbase.h"

class QgsNewConnection : public QgsNewConnectionBase 
{
 public:
    QgsNewConnection();
    ~QgsNewConnection();
    void testConnection();
    void saveConnection();
};

#endif //  QGSNEWCONNECTIONBASE_H
