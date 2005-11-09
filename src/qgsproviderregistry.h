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
 /* $Id$ */
 
#ifndef QGSPROVIDERREGISTRY_H
#define QGSPROVIDERREGISTRY_H

#include <map>

#include <qdir.h>
#include <qstring.h>



class QgsDataProvider;
class QgsProviderMetadata;

/** canonical manager of data providers

  This is a Singleton class that manages data provider access.
*/
class QgsProviderRegistry
{
public:

    /** means of accessing canonical single instance
     */
    static QgsProviderRegistry* instance(QString pluginPath = QString::null);

    QString library(QString const & providerKey) const;

    QString pluginList(bool asHtml = false) const;

    /// return library directory where plugins are found
    QDir const & libraryDirectory() const;

    void setLibraryDirectory(QDir const & path);
 
    QgsDataProvider * getProvider( QString const & providerKey, 
                                   QString const & dataSource );

    /// type for data provider metadata associative container
    typedef std::map<QString,QgsProviderMetadata*> Providers;

    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by all data providers.

      This walks through all data providers appending calls to their
      fileVectorFilters to a string, which is then returned.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    virtual QString fileVectorFilters() const;

  /** open the given vector data source
  
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
  QgsDataProvider * openVector( QString const & dataSource, QString const & providerKey );
  


private:

    /** ctor private since instance() creates it */
    QgsProviderRegistry(QString pluginPath);

    /// pointer to canonical Singleton object
    static QgsProviderRegistry* _instance;

    /// associative container of provider metadata handles
    Providers mProviders;

    /// directory in which provider plugins are installed
    QDir mLibraryDirectory;

    /** file filter string for vector files

        Built once when registry is constructed by appending strings returned
        from iteratively calling vectorFileFilter() for each visited data
        provider.  The alternative would have been to do this each time
        fileVectorFilters was invoked; instead we only have to build it the
        one time.
     */
    QString mVectorFileFilters;

}; // class QgsProviderRegistry

#endif //QGSPROVIDERREGISTRY_H

