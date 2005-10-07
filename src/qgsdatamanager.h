/***************************************************************************
    qgsdatamanager.h -  Data manager Singleton class
                             -------------------
    begin                : August 2005
    copyright            : (C) 2005 by Mark Coletti
    email                : mcoletti@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATAMANAGER_H
#define QGSDATAMANAGER_H


class QString;


/** Singleton data manager class

@note

Eventually the data provider and map layer registries will be rolled into this
class.

One thing to consider is that the open{Raster,Vector}( source, key ) could
possibly be consolidated into a single open( source, key ) since presumably
the key would match an appropriate data source provider.

I don't like QgsDataManager since "Data" is fairily generic.  It manages
_both_ layers _and_ their respective data providers.

*/
class QgsDataManager
{
public:

  /// return reference to canonical instance
  QgsDataManager & instance();
  
  ~QgsDataManager();
  
  /** open the given vector data source
  
    @param name could be a file, URI
    @return false if unable to open vector source
    
        
    Temporarily always returns false until finished implementing.

    
    Eventually would be nice if could make QgsDataManager smart
    enough to figure out whether the given name mapped to a vector,
    raster, or database source.
  */
  bool openVector( QString const & dataSource );

  /** open the given vector data source
  
    Similar to open(QString const &), except that the user specifies a data provider 
    with which to open the data source instead of using the default data provider
    that QgsDataManager would figure out to use.  This should be useful when (and if)
    there will exist more than one data provider that can handle a given data
    source.  (E.g., use GDAL to open an SDTS file, or a different data provider that uses
    sdts++.)
  
    @param name could be a file, URI
    @param provider is the key for the dataprovider used to open name
    @return false if unable to open vector source
    
    Temporarily always returns false until finished implementing.
    
    Eventually would be nice if could make QgsDataManager smart
    enough to figure out whether the given name mapped to a vector,
    raster, or database source.
  */
  bool openVector( QString const & dataSource, QString const & providerKey );
  
  /** open the given raster data source
  
    @param name could be a file, URI
    @return false if unable to open raster source
    
    Temporarily always returns false until finished implementing.
    
    @note
    
    Eventually would be nice if could make QgsDataManager smart
    enough to figure out whether the given name mapped to a vector,
    raster, or database source.
  */
  bool openRaster( QString const & dataSource );

  /** open the given raster data source
  
    Similar to openRaster(QString const &), except that the user specifies a data provider 
    with which to open the data source instead of using the default data provider
    that QgsDataManager would figure out to use.  This should be useful when (and if)
    there will exist more than one data provider that can handle a given data
    source.  (E.g., use GDAL to open an SDTS file, or a different data provider that uses
    sdts++.)
  
    Temporarily always returns false until finished implementing.
    
    @param name could be a file, URI
    @param provider is the key to the dataprovider used to open name
    @return false if unable to open vector source
    
    @note
    
    Eventually would be nice if could make QgsDataManager smart
    enough to figure out whether the given name mapped to a vector,
    raster, or database source.
  */
  bool openRaster( QString const & dataSource, QString const & providerKey );


private:

  // private since only instance() can create
  QgsDataManager();

  /// canonical instance
  static QgsDataManager * instance_;
}; // class QgsDataManager

#endif

