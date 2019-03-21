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

#ifndef QGSSERVERINTERFACEIMPL_H
#define QGSSERVERINTERFACEIMPL_H

#define SIP_NO_FILE


#include "qgsserverinterface.h"
#include "qgscapabilitiescache.h"
#include "qgsservercachemanager.h"

/**
 * \ingroup server
 * \class QgsServerInterfaceImpl
 * \brief Interfaces exposed by QGIS Server and made available to plugins.
 * \since QGIS 2.8
 */
class SERVER_EXPORT QgsServerInterfaceImpl : public QgsServerInterface
{
  public:

    //! Constructor
    explicit QgsServerInterfaceImpl( QgsCapabilitiesCache *capCache,
                                     QgsServiceRegistry *srvRegistry,
                                     QgsServerSettings *serverSettings );


    ~QgsServerInterfaceImpl() override;

    void setRequestHandler( QgsRequestHandler *requestHandler ) override;
    void clearRequestHandler() override;
    QgsCapabilitiesCache *capabilitiesCache() override { return mCapabilitiesCache; }
    //! Returns the QgsRequestHandler, to be used only in server plugins
    QgsRequestHandler  *requestHandler() override { return mRequestHandler; }
    void registerFilter( QgsServerFilter *filter, int priority = 0 ) override;
    QgsServerFiltersMap filters() override { return mFilters; }

    //! Register an access control filter
    void registerAccessControl( QgsAccessControlFilter *accessControl, int priority = 0 ) override;

    /**
     * Gets the helper over all the registered access control filters
     * \returns the access control helper
     */
    QgsAccessControl *accessControls() const override { return mAccessControls; }


    /**
     * Registers a server cache filter
     * \param serverCache the server cache to register
     * \param priority the priority used to order them
     * \since QGIS 3.4
     */
    void registerServerCache( QgsServerCacheFilter *serverCache SIP_TRANSFER, int priority = 0 ) override;

    /**
     * Gets the helper over all the registered server cache filters
     * \returns the server cache helper
     * \since QGIS 3.4
     */
    QgsServerCacheManager *cacheManager() const override;

    QString getEnv( const QString &name ) const override;
    QString configFilePath() override { return mConfigFilePath; }
    void setConfigFilePath( const QString &configFilePath ) override;
    void setFilters( QgsServerFiltersMap *filters ) override;
    void removeConfigCacheEntry( const QString &path ) override;

    QgsServiceRegistry *serviceRegistry() override;

    QgsServerSettings *serverSettings() override;

  private:

    QString mConfigFilePath;
    QgsServerFiltersMap mFilters;
    QgsAccessControl *mAccessControls = nullptr;
    std::unique_ptr<QgsServerCacheManager> mCacheManager = nullptr;
    QgsCapabilitiesCache *mCapabilitiesCache = nullptr;
    QgsRequestHandler *mRequestHandler = nullptr;
    QgsServiceRegistry *mServiceRegistry = nullptr;
    QgsServerSettings *mServerSettings = nullptr;
};

#endif // QGSSERVERINTERFACEIMPL_H
