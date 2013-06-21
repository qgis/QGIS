
/** canonical manager of data providers

  This is a Singleton class that manages data provider access.
*/
class QgsProviderRegistry
{
%TypeHeaderCode
#include <qgsproviderregistry.h>
%End

  public:

    /** means of accessing canonical single instance  */
    static QgsProviderRegistry* instance( QString pluginPath = QString::null );

    /** Virtual dectructor */
    virtual ~QgsProviderRegistry();

    /** Return path for the library of the provider */
    QString library( const QString & providerKey ) const;

    /** Return list of provider plugins found */
    QString pluginList( bool asHtml = false ) const;

    /** return library directory where plugins are found */
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

    QWidget *selectWidget( const QString & providerKey,
                           QWidget * parent = 0, Qt::WFlags fl = 0 );

    /** Get pointer to provider function
        @param providerKey identificator of the provider
        @param functionName name of function
        @return pointer to function or NULL on error
     */
    void *function( const QString & providerKey,
                    const QString & functionName );

    QLibrary *providerLibrary( const QString & providerKey ) const;

    /** Return list of available providers by their keys */
    QStringList providerList() const;

    /** Return metadata of the provider or NULL if not found */
    const QgsProviderMetadata* providerMetadata( const QString& providerKey ) const;

    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileVectorFilters to a string, which is then returned.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileVectorFilters() const;
    /** return raster file filter string

      Returns a string suitable for a QFileDialog of raster file formats
      supported by all data providers.

      @note this method was added in QGIS 2.0
      @note This replaces QgsRasterLayer::buildSupportedRasterFileFilter()
    */
    virtual QString fileRasterFilters() const;
    /** return a string containing the available database drivers
    * @note this method was added in QGIS 1.1
    */
    virtual QString databaseDrivers() const;
    /** return a string containing the available directory drivers
     * @note this method was added in QGIS 1.1
     */
    virtual QString directoryDrivers() const;
    /** return a string containing the available protocol drivers
     * @note this method was added in QGIS 1.1
     */
    virtual QString protocolDrivers() const;

    void registerGuis( QWidget *widget );

  private:
    /** ctor private since instance() creates it */
    QgsProviderRegistry( QString pluginPath );
}; // class QgsProviderRegistry

