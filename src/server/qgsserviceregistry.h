/***************************************************************************
                          qgsserviceregstry.h

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
#ifndef QGSSERVICEREGISTRY_H
#define QGSSERVICEREGISTRY_H

#include "qgsconfig.h"

#include <QHash>
#include <QString>

#include "qgsservicenativeloader.h"
#include "qgsservicepythonloader.h"
#include <memory>

class QgsService;

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
class SERVER_EXPORT QgsServiceRegistry
{

  public:

    //! Constructor
    QgsServiceRegistry();

    //! Destructor
   ~QgsServiceRegistry();

    /**
     * Retrieve a service from its name
     * @param name the name of the service
     * @param version the version string (optional) 
     * @return QgsService
     *
     * If the version is not provided the higher version of the service is returned 
     */
    QgsService* getService( const QString& name, const QString& version = QString() );

    /**
     * Register a service by its name and version
     * 
     * This method is intended to  be called by modules for registering
     * services. A module may register multiple services.
     *
     * The registry gain ownership of services and will call 'delete' on cleanup
     *
     * @param service a QgsServerResponse to be registered
     */
    void registerService( QgsService* service );

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

  private:
    typedef QHash<QString, std::shared_ptr<QgsService> > ServiceTable;
    typedef QHash<QString, QPair<QString, QString> > VersionTable;

    QgsServiceNativeLoader mNativeLoader;
    QgsServicePythonLoader mPythonLoader;

    ServiceTable mServices;
    VersionTable mVersions;
};

#endif

