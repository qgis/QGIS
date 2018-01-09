/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsservicemodule.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsServiceModule
{
%Docstring
QgsServiceModule
Class defining the service module interface for QGIS server services

This class acts as a service registrar for services.

For dynamic modules, a QgsServiceModule instance is returned from the QGS_ServiceModule_Init() entry point
%End

%TypeHeaderCode
#include "qgsservicemodule.h"
%End
  public:

    QgsServiceModule();
%Docstring
Constructor
%End

    virtual ~QgsServiceModule() = 0;

    virtual void registerSelf( QgsServiceRegistry &registry,
                               QgsServerInterface *serverIface = 0 ) = 0;
%Docstring
Ask module to register all provided services

:param registry: :py:class:`QgsServiceRegistry`
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsservicemodule.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
