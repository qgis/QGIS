/***************************************************************************
    qgsprovidersourcewidgetproviderregistry.h
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERSOURCEWIDGETPROVIDERREGISTRY_H
#define QGSPROVIDERSOURCEWIDGETPROVIDERREGISTRY_H

#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsMapLayer;
class QgsProviderSourceWidget;
class QgsProviderSourceWidgetProvider;
class QgsProviderGuiRegistry;

/**
 * \ingroup gui
 * \brief This class keeps a list of provider source widget providers.
 *
 * QgsProviderSourceWidgetProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::QgsProviderSourceWidgetProviderRegistry().
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsProviderSourceWidgetProviderRegistry
{
  public:
    QgsProviderSourceWidgetProviderRegistry();
    ~QgsProviderSourceWidgetProviderRegistry();

    QgsProviderSourceWidgetProviderRegistry( const QgsProviderSourceWidgetProviderRegistry &rh ) = delete;
    QgsProviderSourceWidgetProviderRegistry &operator=( const QgsProviderSourceWidgetProviderRegistry &rh ) = delete;

    //! Gets list of available providers
    QList<QgsProviderSourceWidgetProvider *> providers();

    //! Add a \a provider implementation. Takes ownership of the object.
    void addProvider( QgsProviderSourceWidgetProvider *provider SIP_TRANSFER );

    /**
     * Remove \a provider implementation from the list (\a provider object is deleted)
     * \returns TRUE if the provider was actually removed and deleted
     */
    bool removeProvider( QgsProviderSourceWidgetProvider *provider SIP_TRANSFER );

    /**
     * Initializes the registry. The registry needs to be passed explicitly
     * (instead of using singleton) because this gets called from QgsGui constructor.
     */
    void initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry );

    //! Returns a provider by \a name or NULLPTR if not found
    QgsProviderSourceWidgetProvider *providerByName( const QString &name );

    //! Returns a (possibly empty) list of providers by data \a providerkey
    QList<QgsProviderSourceWidgetProvider *> providersByKey( const QString &providerKey );

    /**
     * Creates a new widget to configure the source of the specified \a layer.
     * It may return NULLPTR if no provider was found.
     * The returned object must be destroyed by the caller.
     */
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr ) SIP_TRANSFERBACK;

  private:
#ifdef SIP_RUN
    QgsProviderSourceWidgetProviderRegistry( const QgsProviderSourceWidgetProviderRegistry &rh );
#endif

    //! available providers. this class owns the pointers
    QList<QgsProviderSourceWidgetProvider *> mProviders;
};

#endif // QGSPROVIDERSOURCEWIDGETPROVIDERREGISTRY_H
