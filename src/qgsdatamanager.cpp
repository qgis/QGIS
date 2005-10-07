/***************************************************************************
                          qgsdatamanager.cpp
                             -------------------
    begin                : 24, August 2005
    copyright            : (C) 2005 by Mark Coletti
    email                : mcoletti -> gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatamanager.h"


#include "qgsvectordataprovider.h"
#include "qgsproviderregistry.h"

#include <list>

using namespace std;



static const char* const ident_ = "$Id$";



QgsDataManager * QgsDataManager::instance_ = 0x0;


QgsDataManager::QgsDataManager()
{
} // QgsDataManager ctor



QgsDataManager::~QgsDataManager()
{
  if ( instance_ )
  {
    delete instance_;
  }
} // QgsDataManager dtor




QgsDataManager &
QgsDataManager::instance()
{
  if ( ! instance_ )
  {
    instance_ = new QgsDataManager;
  }
  
  return *instance_;
} // QgsDataManager::instance()



bool QgsDataManager::openVector( QString const & dataSource )
{
  // find the default provider that can handle the given name
  QgsDataProvider * vectorDataProvider = // XXX ogr temporarily hard-coded
      QgsProviderRegistry::instance()->openVector( dataSource, "ogr" ); 


  if ( ! vectorDataProvider )
  {
      QgsDebug( "Unable to get default vector data provider" );

      return false;
  }

  // now get all the layers corresponding to the data provider

  list<QgsMapLayer*> mapLayers = vectorDataProvider->createLayers();

  if ( mapLayers.empty() )
  {
      QgsDebug( "Unable to get create map layers" );

      return false;
  }

  // and then add them to the map layer registry; each time a layer is added,
  // a signal is emitted which will cause the GUI to be updated with the new
  // layer

  // TODO

  return false;

} // QgsDataManager::openVector



bool QgsDataManager::openVector( QString const & dataSource, 
                                 QString const & providerKey )
{
  // find the default provider that can handle the given name

  QgsDataProvider * vectorDataProvider = 
      QgsProviderRegistry::instance()->openVector( dataSource, providerKey );

  return false;
} // QgsDataManager::openVector



bool QgsDataManager::openRaster( QString const & name )
{
  return false;
} // QgsDataManager::openRaster



bool QgsDataManager::openRaster( QString const & name, QString const & provider )
{
  return false;
} // QgsDataManager::openRaster


