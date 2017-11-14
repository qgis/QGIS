/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsconnectionregistry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGPSConnectionRegistry
{
%Docstring
 A class to register / unregister existing GPS connections such that the information
 is available to all classes and plugins.

 QgsGPSConnectionRegistry is not usually directly created, but rather accessed through
 QgsApplication.gpsConnectionRegistry().
%End

%TypeHeaderCode
#include "qgsgpsconnectionregistry.h"
%End
  public:

    QgsGPSConnectionRegistry();
%Docstring
 Constructor for QgsGPSConnectionRegistry.
%End
    ~QgsGPSConnectionRegistry();


    void registerConnection( QgsGPSConnection *c );
%Docstring
Inserts a connection into the registry. The connection is owned by the registry class until it is unregistered again
%End
    void unregisterConnection( QgsGPSConnection *c );
%Docstring
Unregisters connection. The registry does no longer own the connection
%End

    QList< QgsGPSConnection *> connectionList() const;
%Docstring
 :rtype: list of QgsGPSConnection
%End

  private:
    QgsGPSConnectionRegistry( const QgsGPSConnectionRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsconnectionregistry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
