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


class QgsDataProvider;
class QgsProviderMetadata;
class QgsVectorLayer;
class QgsCoordinateReferenceSystem;


/** \ingroup core
  * A registry / canonical manager of data providers.

  This is a Singleton class that manages data provider access.

  Loaded providers may be restricted using QGIS_PROVIDER_FILE environment variable.
  QGIS_PROVIDER_FILE is regexp pattern applied to provider file name (not provider key).
  For example, if the variable is set to gdal|ogr|postgres it will load only providers gdal,
  ogr and postgres.
*/
class CORE_EXPORT QgsProviderRegistry
{

  public:

    /** Means of accessing canonical single instance  */
    static QgsProviderRegistry* instance( QString pluginPath = QString::null );

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

    QWidget *selectWidget( const QString & providerKey,
                           QWidget * parent = 0, Qt::WindowFlags fl = 0 );

#if QT_VERSION >= 0x050000
    /** Get pointer to provider function
        @param providerKey identificator of the provider
        @param functionName name of function
        @return pointer to function or NULL on error
     */
    QFunctionPointer function( const QString & providerKey,
                               const QString & functionName );
#else
    /** Get pointer to provider function
        @param providerKey identificator of the provider
        @param functionName name of function
        @return pointer to function or NULL on error
     */
    void *function( const QString & providerKey,
                    const QString & functionName );
#endif

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

    /** Open the given vector data source

    Similar to open(QString const &), except that the user specifies a data provider
    with which to open the data source instead of using the default data provider
    that QgsDataManager would figure out to use.  This should be useful when (and if)
    there will exist more than one data provider that can handle a given data
    source.  (E.g., use GDAL to open an SDTS file, or a different data provider that uses
    sdts++.)

    Called by QgsDataManager::open().

    @param name could be a file, URI
    @param provider is the key for the dataprovider used to open name
    @return NULL if unable to open vector data source

    Temporarily always returns false until finished implementing.

    Eventually would be nice if could make QgsDataManager smart
    enough to figure out whether the given name mapped to a vector,
    raster, or database source.
    */
    //QgsDataProvider * openVector( QString const & dataSource, QString const & providerKey );


    /** Type for data provider metadata associative container */
    typedef std::map<QString, QgsProviderMetadata*> Providers;

  private:
    /** Ctor private since instance() creates it */
    QgsProviderRegistry( QString pluginPath );

    /** Associative container of provider metadata handles */
    Providers mProviders;

    /** Directory in which provider plugins are installed */
    QDir mLibraryDirectory;

    /** File filter string for vector files

        Built once when registry is constructed by appending strings returned
        from iteratively calling vectorFileFilter() for each visited data
        provider.  The alternative would have been to do this each time
        fileVectorFilters was invoked; instead we only have to build it the
        one time.
     */
    QString mVectorFileFilters;
    /** File filter string for raster files
     */
    QString mRasterFileFilters;
    /** Available database drivers string for vector databases

     This is a string of form:
     DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
        */
    QString mDatabaseDrivers;
    /** Available directory drivers string for vector databases

     This is a string of form:
     DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
        */
    QString mDirectoryDrivers;
    /** Available protocol drivers string for vector databases

     This is a string of form:
     DriverNameToShow,DriverName;DriverNameToShow,DriverName;...
        */
    QString mProtocolDrivers;

}; // class QgsProviderRegistry

#endif //QGSPROVIDERREGISTRY_H

