/***************************************************************************
                          qgsserverinterface.h

  Class defining the interface made available to QGIS Server plugins.
  -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSERVERINTERFACE_H
#define QGSSERVERINTERFACE_H

#include "qgsconfig.h"
#include "qgis_sip.h"
#include "qgscapabilitiescache.h"
#include "qgsrequesthandler.h"
#include "qgsserverfilter.h"
#include "qgsserversettings.h"
#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrolfilter.h"
#include "qgsaccesscontrol.h"
#include "qgsservercachefilter.h"
#include "qgsservercachemanager.h"
#else
class QgsAccessControl;
class QgsAccessControlFilter;
class QgsServerCacheManager;
class QgsServerCacheFilter;
#endif
#include "qgsserviceregistry.h"
#include "qgis_server.h"
#include "qgis_sip.h"

SIP_IF_MODULE( HAVE_SERVER_PYTHON_PLUGINS )


/**
 * \ingroup server
 * QgsServerInterface
 * Class defining interfaces exposed by QGIS Server and
 * made available to plugins.
 *
 * This class provides methods to access the request handler and
 * the capabilities cache. A method to read the environment
 * variables set in the main FCGI loop is also available.
 * Plugins can add listeners (instances of QgsServerFilter) with
 * a certain priority through the registerFilter( QgsServerFilter* , int) method.
 *
 */
class SERVER_EXPORT QgsServerInterface
{
  public:

    //! Constructor
    QgsServerInterface() SIP_SKIP;

    virtual ~QgsServerInterface() = default;

    /**
     * Set the request handler
     * \param requestHandler request handler
     * \note not available in Python bindings
     */
    virtual void setRequestHandler( QgsRequestHandler *requestHandler ) = 0 SIP_SKIP;

    /**
     * Clear the request handler
     *
     * \note not available in Python bindings
     */
    virtual void clearRequestHandler() = 0 SIP_SKIP;

    /**
     * Gets pointer to the capabiblities cache
     * \returns QgsCapabilitiesCache
     */
    virtual QgsCapabilitiesCache *capabilitiesCache() = 0 SIP_KEEPREFERENCE;

    /**
     * Gets pointer to the request handler
     * \returns QgsRequestHandler
     */
    virtual QgsRequestHandler *requestHandler() = 0 SIP_KEEPREFERENCE;

    /**
     * Register a QgsServerFilter
     * \param filter the QgsServerFilter to add
     * \param priority an optional priority for the filter order
     */
    virtual void registerFilter( QgsServerFilter *filter SIP_TRANSFER, int priority = 0 ) = 0;

    /**
     * Set the filters map
     * \param filters the QgsServerFiltersMap
     */
    virtual void setFilters( QgsServerFiltersMap *filters SIP_TRANSFER ) = 0;

    /**
     * Returns the list of current QgsServerFilter
     * \returns QgsServerFiltersMap list of QgsServerFilter
     */
    virtual QgsServerFiltersMap filters() = 0;

    /**
     * Register an access control filter
     * \param accessControl the access control to register
     * \param priority the priority used to order them
     */
    virtual void registerAccessControl( QgsAccessControlFilter *accessControl SIP_TRANSFER, int priority = 0 ) = 0;

    //! Gets the registered access control filters
    virtual QgsAccessControl *accessControls() const = 0;

    /**
     * Register a server cache filter
     * \param serverCache the server cache to register
     * \param priority the priority used to order them
     * \since QGIS 3.4
     */
    virtual void registerServerCache( QgsServerCacheFilter *serverCache SIP_TRANSFER, int priority = 0 ) = 0;

    /**
     * Gets the registered server cache filters
     * \since QGIS 3.4
     */
    virtual QgsServerCacheManager *cacheManager() const = 0;

    //! Returns an enrironment variable, used to pass  environment variables to Python
    virtual QString getEnv( const QString &name ) const = 0;

    /**
     * Returns the configuration file path
     * \returns QString containing the configuration file path
     */
    virtual QString configFilePath() = 0;

    /**
     * Set the configuration file path
     * \param configFilePath QString with the configuration file path
     */
    virtual void setConfigFilePath( const QString &configFilePath ) = 0;

    /**
     * Remove entry from config cache
     * \param path the path of the file to remove
     */
    virtual void removeConfigCacheEntry( const QString &path ) = 0;

    /**
     * Returns the service registry
     * \returns QgsServiceResgistry
     */
    virtual QgsServiceRegistry *serviceRegistry() = 0 SIP_KEEPREFERENCE;

    /**
     * Returns the server settings
     * \returns QgsServerSettings
     *
     * \note not available in Python bindings
     */
    virtual QgsServerSettings *serverSettings() = 0 SIP_SKIP;

  private:
#ifdef SIP_RUN
    QgsServerInterface();
#endif

    QString mConfigFilePath;
};

#endif // QGSSERVERINTERFACE_H
