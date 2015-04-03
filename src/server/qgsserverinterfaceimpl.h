/***************************************************************************
                          qgsseerversinterface.h
 Interface class for exposing functions in Qgis Mapserver for use by plugins
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
 * Class defining interfaces exposed by Qgis Mapserver and
 * made available to plugins.
 *
 */

class QgsServerInterfaceImpl : public QgsServerInterface
{

  public:

    /** Constructor */
    QgsServerInterfaceImpl( QgsCapabilitiesCache *capCache );

    /** Destructor */
    ~QgsServerInterfaceImpl();

    void setRequestHandler( QgsRequestHandler* requestHandler ) override;
    QgsCapabilitiesCache* capabiblitiesCache() override { return mCapabilitiesCache; }
    QgsRequestHandler*  requestHandler( ) override { return mRequestHandler; }
    void registerFilter( QgsServerFilter *filter, int priority = 0 ) override;
    QgsServerFiltersMap filters( ) override { return mFilters; }
    QString getEnv( const QString& name ) const override;

  private:

    QgsServerFiltersMap mFilters;
    QgsCapabilitiesCache* mCapabilitiesCache;
    QgsRequestHandler* mRequestHandler;

};

#endif // QGSSERVERINTERFACEIMPL_H
