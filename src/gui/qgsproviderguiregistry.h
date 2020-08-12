/***************************************************************************
                    qgsproviderrguiegistry.h
                    -------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at google dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERGUIREGISTRY_H
#define QGSPROVIDERGUIREGISTRY_H

#include <map>

#include <QDir>
#include <QString>
#include <QStringList>
#include <QMainWindow>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsProjectStorageGuiProvider;
class QgsProviderGuiMetadata;
class QgsDataItemGuiProvider;
class QgsSourceSelectProvider;
class QgsProjectStorageGuiProvider;

/**
 * \ingroup gui
 * A registry / canonical manager of GUI parts of data providers.
 *
 * QgsProviderGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::providerGuiRegistry().
 *
 * setPluginPath() should be called (once) to load dynamic providers. Static providers are
 * loaded in constructor.
 *
 * \since QGIS 3.10
*/
class GUI_EXPORT QgsProviderGuiRegistry
{
  public:

    //! Creates registry and loads static provider plugins
    QgsProviderGuiRegistry( const QString &pluginPath );

    //! dtor
    virtual ~QgsProviderGuiRegistry();

    //! Returns list of available providers by their keys
    QStringList providerList() const;

    //! Returns metadata of the provider or NULLPTR if not found
    const QgsProviderGuiMetadata *providerMetadata( const QString &providerKey ) const;

    /**
     * Called during GUI initialization - allows providers to do its internal initialization
     * of GUI components, possibly making use of the passed pointer to the QGIS main window.
     */
    void registerGuis( QMainWindow *widget );

    /**
     * Returns all data item gui providers registered in provider with \a providerKey
     * \note Ownership of created data item providers is passed to the caller.
     */
    virtual const QList<QgsDataItemGuiProvider *> dataItemGuiProviders( const QString &providerKey ) SIP_FACTORY;

    /**
     * Returns all source select providers registered in provider with \a providerKey
     * \note Ownership of created source select providers is passed to the caller.
     */
    virtual QList<QgsSourceSelectProvider *> sourceSelectProviders( const QString &providerKey ) SIP_FACTORY;

    /**
     * Returns all project storage gui providers registered in provider with \a providerKey
     * \note Ownership of created project storage gui providers is passed to the caller.
     */
    virtual QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders( const QString &providerKey ) SIP_FACTORY;

    //! Type for data provider metadata associative container
    SIP_SKIP typedef std::map<QString, QgsProviderGuiMetadata *> GuiProviders;

  private:

    /**
     * Loads the dynamic plugins on the given path
     * When QGIS is compiled with FORCE_STATIC_PROVIDERS,
     * the function is no-op
     */
    void loadDynamicProviders( const QString &pluginPath );

    /**
     * Loads the static providers
     * By default only ogr and gdal providers, but when QGIS is compiled with
     * FORCE_STATIC_PROVIDERS, it also loads all the rest of providers
     */
    void loadStaticProviders( );

    //! Associative container of provider metadata handles
    GuiProviders mProviders;
};

#endif
