/** A singleton class to register / unregister existing GPS connections such that the information
 is available to all classes and plugins*/
class QgsGPSConnectionRegistry {
%TypeHeaderCode
#include <qgsgpsconnectionregistry.h>
%End
  public:
    static QgsGPSConnectionRegistry* instance();
    ~QgsGPSConnectionRegistry();

    /** Inserts a connection into the registry. The connection is owned by the registry class until it is unregistered again*/
    void registerConnection( QgsGPSConnection* c );
    /** Unregisters connection. The registry does no longer own the connection*/
    void unregisterConnection( QgsGPSConnection* c );

    QList< QgsGPSConnection *> connectionList() const;

  protected:
    QgsGPSConnectionRegistry();
};
