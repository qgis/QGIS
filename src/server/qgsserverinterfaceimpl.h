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

#include "qgsserverinterface.h"
#include "qgscapabilitiescache.h"
#include "qgsgetrequesthandler.h"
#include "qgspostrequesthandler.h"
#include "qgssoaprequesthandler.h"
#include "qgsmaprenderer.h"

/**
 * QgsServerInterface
 * Class defining interfaces exposed by QGIS Server and
 * made available to plugins.
 *
 */

class QgsServerInterfaceImpl : public QgsServerInterface
{

  public:

    /** Constructor */
    explicit QgsServerInterfaceImpl( QgsCapabilitiesCache *capCache );

    /** Destructor */
    ~QgsServerInterfaceImpl();

    void setRequestHandler( QgsRequestHandler* requestHandler ) override;
    void clearRequestHandler() override;
    QgsCapabilitiesCache* capabiblitiesCache() override { return mCapabilitiesCache; }
    //! Return the QgsRequestHandler, to be used only in server plugins
    QgsRequestHandler*  requestHandler() override { return mRequestHandler; }
    void registerFilter( QgsServerFilter *filter, int priority = 0 ) override;
    QgsServerFiltersMap filters() override { return mFilters; }
    /** Register an access control filter */
    void registerAccessControl( QgsAccessControlFilter *accessControl, int priority = 0 ) override;
    /** Gets the helper over all the registered access control filters
     * @return the access control helper
     */
    const QgsAccessControl* accessControls() const override { return mAccessControls; }
    QString getEnv( const QString& name ) const override;
    QString configFilePath() override { return mConfigFilePath; }
    void setConfigFilePath( const QString& configFilePath ) override;
    void setFilters( QgsServerFiltersMap *filters ) override;
    void removeConfigCacheEntry( const QString& path ) override;
    void removeProjectLayers( const QString& path ) override;

  private:

    QString mConfigFilePath;
    QgsServerFiltersMap mFilters;
    QgsAccessControl* mAccessControls;
    QgsCapabilitiesCache* mCapabilitiesCache;
    QgsRequestHandler* mRequestHandler;

};

#endif // QGSSERVERINTERFACEIMPL_H
