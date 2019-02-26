/***************************************************************************
                    qgsproviderregistry.h  -  Singleton class for
                    registering data providers.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERREGISTRY_H
#define QGSPROVIDERREGISTRY_H

#include <map>

#include <QDir>
#include <QLibrary>
#include <QString>

#include "qgsdataprovider.h"
#include "qgis_core.h"
#include "qgis_sip.h"


class QgsProviderMetadata;
class QgsVectorLayer;
class QgsCoordinateReferenceSystem;


/**
 * \ingroup core
  * A registry / canonical manager of data providers.
  *
  * This is a Singleton class that manages data provider access.
  *
  * Providers can be either loaded via libraries or native providers that
  * are included in the core QGIS installation and accessed through function pointers.
  *
  * Loaded providers may be restricted using QGIS_PROVIDER_FILE environment variable.
  * QGIS_PROVIDER_FILE is regexp pattern applied to provider file name (not provider key).
  * For example, if the variable is set to gdal|ogr|postgres it will load only providers gdal,
  * ogr and postgres.
*/
class CORE_EXPORT QgsProviderRegistry
{

  public:

    /**
     * Different ways a source select dialog can be used
     * (embedded is for the data source manager dialog)
     */
    enum WidgetMode
    {
      None,
      Embedded,
      Manager,
    };

    //! Means of accessing canonical single instance
    static QgsProviderRegistry *instance( const QString &pluginPath = QString() );

    virtual ~QgsProviderRegistry();

    /**
     * Returns path for the library of the provider.
     *
     * If the provider uses direct provider function pointers instead of a library an empty string will
     * be returned.
     */
    QString library( const QString &providerKey ) const;

    //! Returns list of provider plugins found
    QString pluginList( bool asHtml = false ) const;

    /**
     * Returns the library directory where plugins are found.
     */
    QDir libraryDirectory() const;

    //! Sets library directory where to search for plugins
    void setLibraryDirectory( const QDir &path );

    /**
     * Creates a new instance of a provider.
     * \param providerKey identifier of the provider
     * \param dataSource  string containing data source for the provider
     * \param options provider options
     * \returns new instance of provider or NULLPTR on error
     */
    QgsDataProvider *createProvider( const QString &providerKey,
                                     const QString &dataSource,
                                     const QgsDataProvider::ProviderOptions &options = QgsDataProvider::ProviderOptions() ) SIP_FACTORY;

    /**
     * Returns the provider capabilities
        \param providerKey identifier of the provider
        \since QGIS 2.6
     */
    int providerCapabilities( const QString &providerKey ) const;

    /**
     * Breaks a provider data source URI into its component paths (e.g. file path, layer name).
     * \param providerKey identifier of the provider
     * \param uri uri string
     * \returns map containing components. Standard components include "path", "layerName", "url".
     * \note this function may not be supported by all providers, an empty map will be returned in such case
     * \since QGIS 3.4
     */
    QVariantMap decodeUri( const QString &providerKey, const QString &uri );

    /**
     * Returns a new widget for selecting layers from a provider.
     * Either the \a parent widget must be set or the caller becomes
     * responsible for deleting the returned widget.
     */
    QWidget *createSelectionWidget( const QString &providerKey,
                                    QWidget *parent = nullptr,
                                    Qt::WindowFlags fl = Qt::WindowFlags(),
                                    QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    /**
     * Gets pointer to provider function
     * \param providerKey identifier of the provider
     * \param functionName name of function
     * \returns pointer to function or NULLPTR on error. If the provider uses direct provider
     * function pointers instead of a library NULLPTR will be returned.
     */
    QFunctionPointer function( const QString &providerKey,
                               const QString &functionName );

    /**
     * Returns a new QLibrary for the specified \a providerKey. Ownership of the returned
     * object is transferred to the caller and the caller is responsible for deleting it.
     *
     * If the provider uses direct provider function pointers instead of a library NULLPTR will
     * be returned.
     */
    QLibrary *createProviderLibrary( const QString &providerKey ) const SIP_FACTORY;

    //! Returns list of available providers by their keys
    QStringList providerList() const;

    //! Returns metadata of the provider or NULLPTR if not found
    const QgsProviderMetadata *providerMetadata( const QString &providerKey ) const;

    /**
     * Returns vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileVectorFilters to a string, which is then returned.

      \note

      It'd be nice to eventually be raster/vector neutral.
     */
    virtual QString fileVectorFilters() const;

    /**
     * Returns raster file filter string

      Returns a string suitable for a QFileDialog of raster file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      buildSupportedRasterFileFilter to a string, which is then returned.

      \note This replaces QgsRasterLayer::buildSupportedRasterFileFilter()
     */
    virtual QString fileRasterFilters() const;

    /**
     * Returns mesh file filter string

      Returns a string suitable for a QFileDialog of mesh file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileMeshFilters to a string, which is then returned.

      \see fileMeshDatasetFilters()

      \since QGIS 3.6
     */
    virtual QString fileMeshFilters() const;

    /**
     * Returns mesh's dataset file filter string

      Returns a string suitable for a QFileDialog of mesh datasets file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileMeshFilters to a string, which is then returned.

      \see fileMeshFilters()

      \since QGIS 3.6
     */
    virtual QString fileMeshDatasetFilters() const;

    //! Returns a string containing the available database drivers
    virtual QString databaseDrivers() const;
    //! Returns a string containing the available directory drivers
    virtual QString directoryDrivers() const;
    //! Returns a string containing the available protocol drivers
    virtual QString protocolDrivers() const;

    void registerGuis( QWidget *widget );

    /**
     * \brief register a new vector data provider from its \a providerMetadata
     * \return TRUE on success, FALSE if a provider with the same key was already registered
     * \note ownership of the QgsProviderMetadata instance is transferred to the registry
     * \since QGIS 3.2
     */
    bool registerProvider( QgsProviderMetadata *providerMetadata SIP_TRANSFER );

    //! Type for data provider metadata associative container
    SIP_SKIP typedef std::map<QString, QgsProviderMetadata *> Providers;

  private:
    //! Ctor private since instance() creates it
    QgsProviderRegistry( const QString &pluginPath );

#ifdef SIP_RUN
    QgsProviderRegistry( const QString &pluginPath );
#endif

    void init();
    void clean();

    //! Associative container of provider metadata handles
    Providers mProviders;

    //! Directory in which provider plugins are installed
    QDir mLibraryDirectory;

    /**
     * File filter string for vector files
     *
     * Built once when registry is constructed by appending strings returned
     * from iteratively calling vectorFileFilter() for each visited data
     * provider.  The alternative would have been to do this each time
     * fileVectorFilters was invoked; instead we only have to build it the
     * one time.
     */
    QString mVectorFileFilters;

    /**
     * File filter string for raster files
     */
    QString mRasterFileFilters;

    /**
     * File filter string for raster files
     */
    QString mMeshFileFilters;

    /**
     * File filter string for raster files
     */
    QString mMeshDatasetFileFilters;

    /**
     * Available database drivers string for vector databases
     *
     * This is a string of form:
     * DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
     */
    QString mDatabaseDrivers;

    /**
     * Available directory drivers string for vector databases
     * This is a string of form:
     * DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
     */
    QString mDirectoryDrivers;

    /**
     * Available protocol drivers string for vector databases
     *
     * This is a string of form:
     * DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
     */
    QString mProtocolDrivers;

}; // class QgsProviderRegistry

#endif //QGSPROVIDERREGISTRY_H

