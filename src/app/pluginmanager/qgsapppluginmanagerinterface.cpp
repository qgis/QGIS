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
#include "moc_qgsapppluginmanagerinterface.cpp"
#include "qgspluginmanager.h"
#include "qgslogger.h"


QgsAppPluginManagerInterface::QgsAppPluginManagerInterface( QgsPluginManager *pluginManager )
  : mPluginManager( pluginManager )
{
}

void QgsAppPluginManagerInterface::showPluginManager( int tabIndex )
{
  mPluginManager->getCppPluginsMetadata();
  mPluginManager->reloadModelData();

  //! switch to tab, if specified ( -1 means not specified )
  if ( tabIndex > -1 )
  {
    mPluginManager->selectTabItem( tabIndex );
  }

  mPluginManager->exec();
}

void QgsAppPluginManagerInterface::clearPythonPluginMetadata()
{
  mPluginManager->clearPythonPluginMetadata();
}

void QgsAppPluginManagerInterface::addPluginMetadata( const QMap<QString, QString> &metadata )
{
  if ( metadata.isEmpty() || !metadata.contains( QStringLiteral( "id" ) ) )
  {
    QgsDebugError( QStringLiteral( "Warning: incomplete metadata" ) );
    return;
  }
  mPluginManager->addPluginMetadata( metadata.value( QStringLiteral( "id" ) ), metadata );
}

void QgsAppPluginManagerInterface::reloadModel()
{
  mPluginManager->reloadModelData();
}

const QMap<QString, QString> *QgsAppPluginManagerInterface::pluginMetadata( const QString &key ) const
{
  return mPluginManager->pluginMetadata( key );
}

void QgsAppPluginManagerInterface::clearRepositoryList()
{
  mPluginManager->clearRepositoryList();
}

void QgsAppPluginManagerInterface::addToRepositoryList( const QMap<QString, QString> &repository )
{
  mPluginManager->addToRepositoryList( repository );
}

void QgsAppPluginManagerInterface::pushMessage( const QString &text, Qgis::MessageLevel level, int duration )
{
  mPluginManager->pushMessage( text, level, duration );
}
