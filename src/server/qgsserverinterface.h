/***************************************************************************
                          qgsseerversinterface.h
 Interface class for exposing functions in QGIS Server for use by plugins
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

#include "qgscapabilitiescache.h"
#include "qgsrequesthandler.h"
#include "qgsserverfilter.h"
#include "qgsaccesscontrolfilter.h"
#include "qgsaccesscontrol.h"

/**
 * \ingroup server
 * QgsServerInterface
 * Class defining interfaces exposed by QGIS Server and
 * made available to plugins.
 *
 */
class SERVER_EXPORT QgsServerInterface
{

  public:

    /** Constructor */
    QgsServerInterface();

    /** Destructor */
    virtual ~QgsServerInterface() = 0;

    /**
     * Set the request handler
     * @param requestHandler request handler
     * @note not available in Python bindings
     */
    virtual void setRequestHandler( QgsRequestHandler* requestHandler ) = 0;

    /**
     * Clear the request handler
     *
     * @note not available in python bindings
     */
    virtual void clearRequestHandler() = 0;

    /**
     * Get pointer to the capabiblities cache
     * @return QgsCapabilitiesCache
     */
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0;

    /**
     * Get pointer to the request handler
     * @return QgsRequestHandler
     */
    virtual QgsRequestHandler* requestHandler() = 0;

    /**
     * Register a QgsServerFilter
     * @param filter the QgsServerFilter to add
     * @param priority an optional priority for the filter order
     */
    virtual void registerFilter( QgsServerFilter* filter, int priority = 0 ) = 0;

    /**
     * Set the filters map
     * @param filters the QgsServerFiltersMap
     */
    virtual void setFilters( QgsServerFiltersMap* filters ) = 0;

    /**
     * Return the list of current QgsServerFilter
     * @return QgsServerFiltersMap list of QgsServerFilter
     */
    virtual QgsServerFiltersMap filters() = 0;
    /** Register an access control filter
     * @param accessControl the access control to register
     * @param priority the priority used to order them
     */
    virtual void registerAccessControl( QgsAccessControlFilter* accessControl, int priority = 0 ) = 0;
    /** Gets the registred access control filters */
    virtual const QgsAccessControl* accessControls() const = 0;

    //! Return an enrironment variable, used to pass  environment variables to python
    virtual QString getEnv( const QString& name ) const = 0;

    /**
     * Return the configuration file path
     * @return QString containing the configuration file path
     */
    virtual QString configFilePath() = 0;

    /**
     * Set the configuration file path
     * @param configFilePath QString with the configuration file path
     */
    virtual void setConfigFilePath( const QString& configFilePath ) = 0;

    /**
     * Remove entry from config cache
     * @param path the path of the file to remove
     */
    virtual void removeConfigCacheEntry( const QString& path ) = 0;

    /**
     * Remove entries from layer cache
     * @param path the path of the project which own the layers to be removed
     */
    virtual void removeProjectLayers( const QString& path ) = 0;




  private:
    QString mConfigFilePath;
};

#endif // QGSSERVERINTERFACE_H
