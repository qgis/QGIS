/***************************************************************************
                          qgsserviceregistry.h

  Class defining the service manager for QGIS server services.
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * \ingroup server
 * QgsServiceRegistry
 * Class defining the cegistry manager for QGIS server services
 *
 * This class provides methods for registering and retrieving 
 * services. Note that the regstiry does not hord ownership of
 * registered service but vill call the 'release' method on cleanup)
 *
 */
class QgsServiceRegistry
{
%TypeHeaderCode
#include "qgsserviceregistry.h"
#include "qgsservice.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
%End
  public:

    //! Constructor
    QgsServiceRegistry();

    //! Destructor
   ~QgsServiceRegistry();

    /**
     * Retrieve a service from its name
     * @param name the name of the service 
     * @param version the required version (optional)
     * @return QgsService
     */
    QgsService* getService( const QString& name, const QString& version = QString() );

    /**
     * Register a service by its name
     * 
     * This method is intended to  be called by modules for registering
     * services. A module may register multiple services.
     * The registry gain ownership of services.
     *
     * @param name the name of the service
     * @param service a QgsServerResponse to be registered
     * @param version the service version 
     */
    void registerService( const QString& name,  QgsService* service /Transfer/, const QString& version );

    /** 
     * Initialize registry, load modules and auto register services
     * @param nativeModulepath the native module path
     * @param pythonModulePath the python module path
     *
     * If pythonModulePath is not specified the environnement variables QGIS_PYTHON_SERVICE_PATH
     * is examined. 
     */
    void init( const QString& nativeModulepath, const QString& pythonModulePath = QString() );

    /**
     * Clean up registered service and unregister modules
     */
    void cleanUp();
};


