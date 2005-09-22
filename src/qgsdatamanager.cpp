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



bool QgsDataManager::openVector( QString const & name )
{
  // find the default provider that can handle the given name
  
  // note that we may already have the provider
  
  // create a QgsDataSourceLayer for the provider
  return false;
} // QgsDataManager::openVector



bool QgsDataManager::openVector( QString const & name, QgsDataProvider & provider )
{
  return false;
} // QgsDataManager::openVector



bool QgsDataManager::openRaster( QString const & name )
{
  return false;
} // QgsDataManager::openRaster



bool QgsDataManager::openRaster( QString const & name, QgsDataProvider & provider )
{
  return false;
} // QgsDataManager::openRaster


