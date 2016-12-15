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
 * Class defining the registry manager for QGIS server services
 *
 * This class provides methods for registering and retrieving
 * services.
 *
 * IMPORTANT: The registry hold ownership of registered services and
 * will call 'delete'  on cleanup
 *
 */
class QgsServiceRegistry
{
%TypeHeaderCode
#include "qgsserviceregistry.h"
#include "qgsservice.h"
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
     * @param service a QgsServerResponse to be registered
     */
    void registerService( QgsService* service /Transfer/ );

    /**
     * Unregister service from its name and version 
     *
     * @param name the tame of the service
     * @param version (optional) the specific version to unload
     * @return the number of unregstered services
     *
     * If the version is not specified then all versions from the specified service 
     * are unloaded
     */
    int unRegisterService( const QString& name, const QString& version = QString() );

    /**
     * Initialize registry, load modules and auto register services
     * @param nativeModulepath the native module path
     */
    void init( const QString& nativeModulepath );

    /**
     * Clean up registered service and unregister modules
     */
    void cleanUp();
};


