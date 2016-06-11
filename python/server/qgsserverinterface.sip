/***************************************************************************
                          qgsserverinterface.sip

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

/**
 * \class QgsServerInterface
 * \brief Class defining the interface made available to server plugins.
 *
 * This class provides methods to access the request handler and
 * the capabilties cache. A method to read the environment
 * variables set in the main FCGI loop is also available.
 * Plugins can add listeners (instances of QgsServerFilter) with
 * a certain priority through the registerFilter( QgsServerFilter* , int) method.
 */


typedef QMultiMap<int, QgsServerFilter*> QgsServerFiltersMap;



class QgsServerInterface
{
%TypeHeaderCode
  #include "qgsserverinterface.h"
%End

  public:
    /** Returns the current request handler*/
    virtual QgsRequestHandler* requestHandler() = 0 /KeepReference/;
    /** Returns the capabilities cache*/
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0 /KeepReference/;
    // Tansfer ownership to avoid garbage collector to call dtor
    /** Register a filter with the given priority. The filter's requestReady()
     * and responseReady() methods will be called from the loop*/
    virtual void registerFilter( QgsServerFilter* filter /Transfer/, int priority = 0 ) = 0;
    /** Set the filters map */
    virtual void setFilters( QgsServerFiltersMap* filters /Transfer/) = 0;
    /** Register a security module with the given priority.*/
    virtual void registerAccessControl( QgsAccessControlFilter* accessControl /Transfer/, int priority = 0 ) = 0;
    /** Gets the registred access control filters */
    virtual const QgsAccessControl* accessControls() const = 0;
    /** Return an environment variable set by FCGI*/
    virtual QString getEnv(const QString& name ) const = 0;
    // Commented because of problems with typedef QgsServerFiltersMap, provided
    // methods to alter the filters map into QgsRequestHandler API
    virtual QgsServerFiltersMap filters() = 0;
    /** Returns the configFilePath as seen by the server, this value is only
     * available after requestReady has been called.*/
    virtual QString configFilePath() = 0;
    /** Set the config file path */
    virtual void setConfigFilePath( const QString& configFilePath) = 0;
    /** Remove entry from config cache */
    virtual void removeConfigCacheEntry( const QString& path ) = 0;
    /** Remove entry from layer cache */
    virtual void removeProjectLayers( const QString& path ) = 0;


private:
    /** Constructor */
    QgsServerInterface();

};

