/** \ingroup core
  * A registry / canonical manager of data providers.

  This is a Singleton class that manages data provider access.

  Loaded providers may be restricted using QGIS_PROVIDER_FILE environment variable.
  QGIS_PROVIDER_FILE is regexp pattern applied to provider file name (not provider key).
  For example, if the variable is set to gdal|ogr|postgres it will load only providers gdal,
  ogr and postgres.
*/
class QgsProviderRegistry
{
%TypeHeaderCode
#include <qgsproviderregistry.h>
%End

  public:

    /** Means of accessing canonical single instance  */
    static QgsProviderRegistry* instance( const QString& pluginPath = QString::null );

    /** Virtual dectructor */
    virtual ~QgsProviderRegistry();

    /** Return path for the library of the provider */
    QString library( const QString & providerKey ) const;

    /** Return list of provider plugins found */
    QString pluginList( bool asHtml = false ) const;

    /** Return library directory where plugins are found */
    const QDir & libraryDirectory() const;

    /** Set library directory where to search for plugins */
    void setLibraryDirectory( const QDir & path );

    /** Create an instance of the provider
        @param providerKey identificator of the provider
        @param dataSource  string containing data source for the provider
        @return instance of provider or NULL on error
     */
    QgsDataProvider *provider( const QString & providerKey,
                               const QString & dataSource );

    /** Return the provider capabilities
        @param providerKey identificator of the provider
        @note Added in 2.6
     */
    int providerCapabilities( const QString& providerKey ) const;

    /** Returns a widget for selecting layers from a provider.
     */
    QWidget *selectWidget( const QString & providerKey,
                           QWidget * parent = 0, const Qt::WindowFlags& fl = 0 );

%If (!QT5_SUPPORT)
    /** Get pointer to provider function
     * @param providerKey identificator of the provider
     * @param functionName name of function
     * @return pointer to function or NULL on error
     */
    void *function( const QString & providerKey,
                    const QString & functionName );
%End

    QLibrary *providerLibrary( const QString & providerKey ) const;

    /** Return list of available providers by their keys */
    QStringList providerList() const;

    /** Return metadata of the provider or NULL if not found */
    const QgsProviderMetadata* providerMetadata( const QString& providerKey ) const;

    /** Return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileVectorFilters to a string, which is then returned.

      @note

      It'd be nice to eventually be raster/vector neutral.
     */
    virtual QString fileVectorFilters() const;
    /** Return raster file filter string

      Returns a string suitable for a QFileDialog of raster file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      buildSupportedRasterFileFilter to a string, which is then returned.

      @note This replaces QgsRasterLayer::buildSupportedRasterFileFilter()
     */
    virtual QString fileRasterFilters() const;
    /** Return a string containing the available database drivers */
    virtual QString databaseDrivers() const;
    /** Return a string containing the available directory drivers */
    virtual QString directoryDrivers() const;
    /** Return a string containing the available protocol drivers */
    virtual QString protocolDrivers() const;

    void registerGuis( QWidget *widget );

  private:
    /** Ctor private since instance() creates it */
    QgsProviderRegistry( QString pluginPath );
}; // class QgsProviderRegistry
