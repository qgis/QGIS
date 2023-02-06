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

#include <QList>
#include <QObject>
#include "qgis_gui.h"
#include "qgis_sip.h"

#include "qgsproviderregistry.h"

class QgsSourceSelectProvider;
class QgsProviderGuiRegistry;
class QgsAbstractDataSourceWidget;

/**
 * \ingroup gui
 * \brief This class keeps a list of source select providers that may add items to the QgsDataSourceManagerDialog
 * When created, it automatically adds providers from data provider plugins (e.g. PostGIS, WMS, ...)
 *
 * QgsSourceSelectProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::sourceSelectProviderRegistry().
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSourceSelectProviderRegistry : public QObject
{
    Q_OBJECT

  public:

    QgsSourceSelectProviderRegistry();
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
     * \returns TRUE if the provider was actually removed and deleted
     */
    bool removeProvider( QgsSourceSelectProvider *provider SIP_TRANSFER );

    /**
     * Initializes the registry. The registry needs to be passed explicitly
     * (instead of using singleton) because this gets called from QgsGui constructor.
     * \since QGIS 3.10
     */
    void initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry );

    //! Returns a provider by \a name or NULLPTR if not found
    QgsSourceSelectProvider *providerByName( const QString &name );

    //! Returns a (possibly empty) list of providers by data \a providerkey
    QList<QgsSourceSelectProvider *> providersByKey( const QString &providerKey );

    /**
     * Gets select widget from provider with \a name
     *
     * The function is replacement of  QgsProviderRegistry::createSelectionWidget() from QGIS 3.8
     *
     * \since QGIS 3.10
     */
    QgsAbstractDataSourceWidget *createSelectionWidget(
      const QString &name,
      QWidget *parent,
      Qt::WindowFlags fl,
      QgsProviderRegistry::WidgetMode widgetMode
    );

  signals:

    /**
     * Emitted whenever a provider is added to the registry.
     *
     * \since QGIS 3.30
     */
    void providerAdded( const QString &name );

    /**
     * Emitted whenever a provider is removed from the registry.
     *
     * \since QGIS 3.30
     */
    void providerRemoved( const QString &name );

  private:
#ifdef SIP_RUN
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh );
#endif

    //! available providers. this class owns the pointers
    QList<QgsSourceSelectProvider *> mProviders;

};

#endif // QGSSOURCESELECTPROVIDERREGISTRY_H
