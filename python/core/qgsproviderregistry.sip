/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproviderregistry.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/









class QgsProviderRegistry
{
%Docstring
A registry / canonical manager of data providers.

This is a Singleton class that manages data provider access.

Providers can be either loaded via libraries or native providers that
are included in the core QGIS installation and accessed through function pointers.

Loaded providers may be restricted using QGIS_PROVIDER_FILE environment variable.
QGIS_PROVIDER_FILE is regexp pattern applied to provider file name (not provider key).
For example, if the variable is set to gdal|ogr|postgres it will load only providers gdal,
ogr and postgres.
%End

%TypeHeaderCode
#include "qgsproviderregistry.h"
%End
  public:

    enum WidgetMode
    {
      None,
      Embedded,
      Manager,
    };

    static QgsProviderRegistry *instance( const QString &pluginPath = QString() );
%Docstring
Means of accessing canonical single instance

:rtype: QgsProviderRegistry
%End

    virtual ~QgsProviderRegistry();

    QString library( const QString &providerKey ) const;
%Docstring
Return path for the library of the provider.

If the provider uses direct provider function pointers instead of a library an empty string will
be returned.

:rtype: str
%End

    QString pluginList( bool asHtml = false ) const;
%Docstring
Return list of provider plugins found

:rtype: str
%End

    QDir libraryDirectory() const;
%Docstring
Returns the library directory where plugins are found.

:rtype: QDir
%End

    void setLibraryDirectory( const QDir &path );
%Docstring
Set library directory where to search for plugins
%End

    QgsDataProvider *createProvider( const QString &providerKey,
                                     const QString &dataSource ) /Factory/;
%Docstring
Creates a new instance of a provider.
:param providerKey: identificator of the provider
:param dataSource:  string containing data source for the provider

:return: new instance of provider or NULL on error
:rtype: QgsDataProvider

%End

    int providerCapabilities( const QString &providerKey ) const;
%Docstring
Return the provider capabilities
:param providerKey: identificator of the provider

:rtype: int

.. versionadded:: 2.6
%End

    QWidget *createSelectionWidget( const QString &providerKey,
                                    QWidget *parent = 0,
                                    Qt::WindowFlags fl = Qt::WindowFlags(),
                                    QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Returns a new widget for selecting layers from a provider.
Either the ``parent`` widget must be set or the caller becomes
responsible for deleting the returned widget.

:rtype: QWidget
%End

    QFunctionPointer function( const QString &providerKey,
                               const QString &functionName );
%Docstring
Get pointer to provider function
:param providerKey: identificator of the provider
:param functionName: name of function

:return: pointer to function or NULL on error. If the provider uses direct provider
:rtype: QFunctionPointer

function pointers instead of a library None will be returned.
%End

    QLibrary *createProviderLibrary( const QString &providerKey ) const /Factory/;
%Docstring
Returns a new QLibrary for the specified ``providerKey``. Ownership of the returned
object is transferred to the caller and the caller is responsible for deleting it.

If the provider uses direct provider function pointers instead of a library None will
be returned.

:rtype: QLibrary
%End

    QStringList providerList() const;
%Docstring
Return list of available providers by their keys

:rtype: list of str
%End

    const QgsProviderMetadata *providerMetadata( const QString &providerKey ) const;
%Docstring
Return metadata of the provider or NULL if not found

:rtype: QgsProviderMetadata
%End

    virtual QString fileVectorFilters() const;
%Docstring
Return vector file filter string

Returns a string suitable for a QFileDialog of vector file formats
supported by all data providers.

This walks through all data providers appending calls to their
fileVectorFilters to a string, which is then returned.

\note

It'd be nice to eventually be raster/vector neutral.

:rtype: str
%End

    virtual QString fileRasterFilters() const;
%Docstring
Return raster file filter string

Returns a string suitable for a QFileDialog of raster file formats
supported by all data providers.

This walks through all data providers appending calls to their
buildSupportedRasterFileFilter to a string, which is then returned.

:rtype: str

.. note::

   This replaces QgsRasterLayer.buildSupportedRasterFileFilter()
%End
    virtual QString databaseDrivers() const;
%Docstring
Return a string containing the available database drivers

:rtype: str
%End
    virtual QString directoryDrivers() const;
%Docstring
Return a string containing the available directory drivers

:rtype: str
%End
    virtual QString protocolDrivers() const;
%Docstring
Return a string containing the available protocol drivers

:rtype: str
%End

    void registerGuis( QWidget *widget );



  private:
    QgsProviderRegistry( const QString &pluginPath );
}; // class QgsProviderRegistry


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproviderregistry.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
