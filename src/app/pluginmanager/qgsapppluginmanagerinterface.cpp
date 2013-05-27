/***************************************************************************
    qgsapppluginmanagerinterface.cpp
     --------------------------------------
    Date                 : 15-May-2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapppluginmanagerinterface.h"
#include <qgslogger.h>


QgsAppPluginManagerInterface::QgsAppPluginManagerInterface( QgsPluginManager * pluginManager )
    : mPluginManager( pluginManager )
{
}


QgsAppPluginManagerInterface::~QgsAppPluginManagerInterface()
{
}


//! show the Plugin Manager window and optionally open tab tabIndex
void QgsAppPluginManagerInterface::showPluginManager( int tabIndex )
{
  mPluginManager->getCppPluginDescriptions();
  mPluginManager->reloadModelData();

  //! switch to tab, if specified ( -1 means not specified )
  if ( tabIndex > -1 )
  {
    mPluginManager->selectTabItem( tabIndex );
  }

  mPluginManager->exec();
}


//! remove python plugins from the metadata registry (c++ plugins stay)
void QgsAppPluginManagerInterface::clearPythonPluginMetadata()
{
  mPluginManager->clearPythonPluginMetadata();
}


//! add a single plugin to the metadata registry
void QgsAppPluginManagerInterface::addPluginMetadata( QMap<QString, QString> metadata )
{
  if ( metadata.isEmpty() || !metadata.contains( "id" ) )
  {
    QgsDebugMsg( "Warning: incomplete metadata" );
    return;
  }
  mPluginManager->addPluginMetadata( metadata.value( "id" ), metadata );
}


//! refresh plugin list model (and metadata browser content if necessary)
void QgsAppPluginManagerInterface::reloadModel()
{
  mPluginManager->reloadModelData();
}


//! return given plugin metadata
QMap<QString, QString> * QgsAppPluginManagerInterface::pluginMetadata( QString key )
{
  return mPluginManager->pluginMetadata( key );
}


//! clear the repository listWidget
void QgsAppPluginManagerInterface::clearRepositoryList()
{
  mPluginManager->clearRepositoryList();
}


//! add repository to the repository listWidget
void QgsAppPluginManagerInterface::addToRepositoryList( QMap<QString, QString> repository )
{
  mPluginManager->addToRepositoryList( repository );
}
