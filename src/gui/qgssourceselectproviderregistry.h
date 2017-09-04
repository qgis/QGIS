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
#include "qgis.h"

class QgsSourceSelectProvider;

/** \ingroup gui
 * This class keeps a list of source select providers that may add items to the QgsDataSourceManagerDialog
 * When created, it automatically adds providers from data provider plugins (e.g. PostGIS, WMS, ...)
 *
 * QgsSourceSelectProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::sourceSelectProviderRegistry().
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSourceSelectProviderRegistry
{
  public:
    QgsSourceSelectProviderRegistry();

    ~QgsSourceSelectProviderRegistry();

    //! QgsDataItemProviderRegistry cannot be copied.
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh ) = delete;
    //! QgsDataItemProviderRegistry cannot be copied.
    QgsSourceSelectProviderRegistry &operator=( const QgsSourceSelectProviderRegistry &rh ) = delete;

    //! Get list of available providers
    QList< QgsSourceSelectProvider *> providers() const { return mProviders; }

    //! Add a provider implementation. Takes ownership of the object.
    void addProvider( QgsSourceSelectProvider *provider SIP_TRANSFER );

    //! Remove provider implementation from the list (provider object is deleted)
    void removeProvider( QgsSourceSelectProvider *provider );

    //! Return a provider by name or nullptr if not found
    QgsSourceSelectProvider *providerByName( const QString &name ) const;

    //! Return a (possibly empty) list of providers by data provider's key
    QList<QgsSourceSelectProvider *> providersByKey( const QString &providerKey ) const;


  private:
#ifdef SIP_RUN
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh );
#endif

    //! available providers. this class owns the pointers
    QList<QgsSourceSelectProvider *> mProviders;

};

#endif // QGSSOURCESELECTPROVIDERREGISTRY_H
