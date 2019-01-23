/***************************************************************************
  qgssourceselectproviderregistry.h - QgsSourceSelectProviderRegistry

 ---------------------
 begin                : 1.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSOURCESELECTPROVIDERREGISTRY_H
#define QGSSOURCESELECTPROVIDERREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsSourceSelectProvider;

/**
 * \ingroup gui
 * This class keeps a list of source select providers that may add items to the QgsDataSourceManagerDialog
 * When created, it automatically adds providers from data provider plugins (e.g. PostGIS, WMS, ...)
 *
 * QgsSourceSelectProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::sourceSelectProviderRegistry().
 *
 * \note This class access to QgsProviderRegistry instance to initialize, but QgsProviderRegistry is
 * typically initialized after QgsGui is constructed, for this reason a delayed initialization has been
 * implemented in the class.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSourceSelectProviderRegistry
{
  public:

    /**
     * Constructor for QgsSourceSelectProviderRegistry.
     */
    QgsSourceSelectProviderRegistry() = default;

    ~QgsSourceSelectProviderRegistry();

    //! QgsDataItemProviderRegistry cannot be copied.
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh ) = delete;
    //! QgsDataItemProviderRegistry cannot be copied.
    QgsSourceSelectProviderRegistry &operator=( const QgsSourceSelectProviderRegistry &rh ) = delete;

    //! Gets list of available providers
    QList< QgsSourceSelectProvider *> providers();

    //! Add a \a provider implementation. Takes ownership of the object.
    void addProvider( QgsSourceSelectProvider *provider SIP_TRANSFER );

    /**
     * Remove \a provider implementation from the list (\a provider object is deleted)
     * \returns true if the provider was actually removed and deleted
     */
    bool removeProvider( QgsSourceSelectProvider *provider SIP_TRANSFER );

    //! Returns a provider by \a name or nullptr if not found
    QgsSourceSelectProvider *providerByName( const QString &name );

    //! Returns a (possibly empty) list of providers by data \a providerkey
    QList<QgsSourceSelectProvider *> providersByKey( const QString &providerKey );


  private:

    /**
     * Populate the providers list, this needs to happen after the data provider
     * registry has been initialized.
     */
    void init();
    bool mInitialized = false;
#ifdef SIP_RUN
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh );
#endif

    //! available providers. this class owns the pointers
    QList<QgsSourceSelectProvider *> mProviders;

};

#endif // QGSSOURCESELECTPROVIDERREGISTRY_H
