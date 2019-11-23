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
#ifndef QGSSERVICEREGISTRY_H
#define QGSSERVICEREGISTRY_H

#include "qgsconfig.h"
#include "qgis.h"

#include <QHash>
#include <QString>

#include "qgsservicenativeloader.h"
#include <memory>

class QgsService;
class QgsServerRequest;
class QgsServerApi;
class QgsServerInterface;

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
 * \since QGIS 3.0
 */
class SERVER_EXPORT QgsServiceRegistry
{

  public:

    //! Constructor
    QgsServiceRegistry() = default;

    //! Destructor
    ~QgsServiceRegistry();

    /**
     * Retrieve a service from its name
     * \param name the name of the service
     * \param version the version string (optional)
     * \returns QgsService
     *
     * If the version is not provided the higher version of the service is returned
     */
    QgsService *getService( const QString &name, const QString &version = QString() );

    /**
     * Register a service by its name and version
     *
     * This method is intended to  be called by modules for registering
     * services. A module may register multiple services.
     *
     * The registry takes ownership of services and will call 'delete' on cleanup
     *
     * \param service a QgsService to be registered
     */
    void registerService( QgsService *service SIP_TRANSFER );

    /**
     * Registers the QgsServerApi \a api
     *
     * The registry takes ownership of services and will call 'delete' on cleanup
     * \since QGIS 3.10
     */
    bool registerApi( QgsServerApi *api SIP_TRANSFER );

    /**
     * Unregisters API from its name and version
     *
     * \param name the name of the service
     * \param version (optional) the specific version to unload
     * \returns the number of APIs unregistered
     *
     * If the version is not specified then all versions from the specified API
     * are unloaded
     * \since QGIS 3.10
     */
    int unregisterApi( const QString &name, const QString &version = QString() );

    /**
     * Searches the API register for an API matching the \a request and returns a (possibly NULL) pointer to it.
     * \since QGIS 3.10
     */
    QgsServerApi *apiForRequest( const QgsServerRequest &request ) const SIP_SKIP;

    /**
     * Retrieves an API from its name
     *
     * If the version is not provided the higher version of the service is returned
     *
     * \param name the name of the API
     * \param version the version string (optional)
     * \returns QgsServerApi
     * \since QGIS 3.10
     */
    QgsServerApi *getApi( const QString &name, const QString &version = QString() );

    /**
     * Unregister service from its name and version
     *
     * \param name the name of the service
     * \param version (optional) the specific version to unload
     * \returns the number of services unregistered
     *
     * If the version is not specified then all versions from the specified service
     * are unloaded
     */
    int unregisterService( const QString &name, const QString &version = QString() );

    /**
     * Initialize registry, load modules and auto register services
     * \param serverIface the server interface
     * \param nativeModulepath the native module path
     */
    void init( const QString &nativeModulepath, QgsServerInterface *serverIface = nullptr );

    /**
     * Clean up registered service and unregister modules
     */
    void cleanUp();

  private:

    // XXX consider using QMap because of the few numbers of
    // elements to handle
    typedef QHash<QString, std::shared_ptr<QgsService> > ServiceTable;
    typedef QHash<QString, std::shared_ptr<QgsServerApi> > ApiTable;
    typedef QHash<QString, QPair<QString, QString> > VersionTable;

    QgsServiceNativeLoader mNativeLoader;

    ServiceTable mServices;
    VersionTable mServiceVersions;
    ApiTable mApis;
    VersionTable mApiVersions;

};

#endif

