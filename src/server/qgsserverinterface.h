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

/**
 * QgsServerInterface
 * Class defining interfaces exposed by QGIS Server and
 * made available to plugins.
 *
 */
class SERVER_EXPORT QgsServerInterface
{

  public:

    /** Constructor */
    QgsServerInterface( );

    /** Destructor */
    virtual ~QgsServerInterface() = 0;

    /**
     * Set the request handler
     * @param requestHandler request handler
     */
    virtual void setRequestHandler( QgsRequestHandler* requestHandler ) = 0;

    /**
     * Get pointer to the capabiblities cache
     * @return QgsCapabilitiesCache
     */
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0;

    /**
     * Get pointer to the request handler
     * @return QgsRequestHandler
     */
    virtual QgsRequestHandler* requestHandler( ) = 0;

    /**
     * Register a QgsServerFilter
     * @param filter the QgsServerFilter to add
     * @param priority an optional priority for the filter order
     */
    virtual void registerFilter( QgsServerFilter* filter, int priority = 0 ) = 0;

    /**
     * Return the list of current QgsServerFilter
     * @return QgsServerFiltersMap list of QgsServerFilter
     */
    virtual QgsServerFiltersMap filters( ) = 0;

    //! Return an enrironment variable, used to pass  environment variables to python
    virtual QString getEnv( const QString& name ) const = 0;

    /**
     * Return the configuration file path
     * @return QString containing the configuration file path
     */
    virtual QString configFilePath( ) = 0;

    /**
     * Set the configuration file path
     * @param configFilePath QString with the configuration file path
     */
    virtual void setConfigFilePath( QString configFilePath ) = 0;

  private:
    QString mConfigFilePath;
};

#endif // QGSSERVERINTERFACE_H
