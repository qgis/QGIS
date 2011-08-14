
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
    static QgsProviderRegistry* instance(QString pluginPath = QString::null);
    
    /** Virtual dectructor */
    virtual ~QgsProviderRegistry();

    QString library(const QString & providerKey) const;

    QString pluginList(bool asHtml = false) const;

    /** return library directory where plugins are found */
    const QDir & libraryDirectory() const;

    void setLibraryDirectory(const QDir & path);
 
    QgsDataProvider *provider( const QString & providerKey,
                                   const QString & dataSource );

    /** Return list of available providers by their keys */
    QStringList providerList() const;

    /** Return metadata of the provider or NULL if not found */
    const QgsProviderMetadata* providerMetadata(const QString& providerKey) const;

    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileVectorFilters to a string, which is then returned.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileVectorFilters() const;
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

     /** allows to import a vector layer using the provider   
     * @note this method was added in QGIS 1.8
     */
    int importVector( QgsVectorLayer* layer,
                      const QString& providerKey,
                      const QString& uri,
                      const QgsCoordinateReferenceSystem *destCRS,
                      bool onlySelected = FALSE,
                      QString *errorMessage /Out/ = 0,
                      bool skipAttributeCreation = FALSE
                    ) const;
    


  private:

        /** ctor private since instance() creates it */
        QgsProviderRegistry(QString pluginPath) /Default/ ;

}; // class QgsProviderRegistry

