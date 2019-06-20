/***************************************************************************
  qgsdataitemproviderregistry.h
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMPROVIDERREGISTRY_H
#define QGSDATAITEMPROVIDERREGISTRY_H

#include <QList>
#include "qgis_sip.h"

#include "qgis_core.h"

class QgsDataItemProvider;

/**
 * \ingroup core
 * This class keeps a list of data item providers that may add items to the browser tree.
 * When created, it automatically adds providers from provider plugins (e.g. PostGIS, WMS, ...)
 *
 * QgsDataItemProviderRegistry is not usually directly created, but rather accessed through
 * QgsApplication::dataItemProviderRegistry().
 *
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsDataItemProviderRegistry
{
  public:

    QgsDataItemProviderRegistry();

    ~QgsDataItemProviderRegistry();

    //! QgsDataItemProviderRegistry cannot be copied.
    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry &rh ) = delete;
    //! QgsDataItemProviderRegistry cannot be copied.
    QgsDataItemProviderRegistry &operator=( const QgsDataItemProviderRegistry &rh ) = delete;

    /**
     * Returns the list of available providers.
     */
    QList<QgsDataItemProvider *> providers() const { return mProviders; }

    /**
     * Adds a \a provider implementation to the registry. Ownership of the provider
     * is transferred to the registry.
     */
    void addProvider( QgsDataItemProvider *provider SIP_TRANSFER );

    /**
     * Removes a \a provider implementation from the registry.
     * The provider object is automatically deleted.
     */
    void removeProvider( QgsDataItemProvider *provider );

  private:
#ifdef SIP_RUN
    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry &rh );
#endif

    //! Available providers, owned by this class
    QList<QgsDataItemProvider *> mProviders;

};

#endif // QGSDATAITEMPROVIDERREGISTRY_H
