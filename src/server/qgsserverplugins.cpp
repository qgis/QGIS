/***************************************************************************
                              QgsServerPlugins.cpp
                              -------------------------
  begin                : August 28, 2014
  copyright            : (C) 2014 by Alessandro Pasotti - ItOpen
  email                : apasotti at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsserverplugins.h"
#include "qgsmapserviceexception.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgspythonutils.h"
#include "qgsserverlogger.h"

#include <QLibrary>


// Initialize static members
QgsPythonUtils *QgsServerPlugins::sPythonUtils;

// Construct on first use
QStringList &QgsServerPlugins::serverPlugins()
{
  static QStringList *pluginList = new QStringList();
  return *pluginList;
}


// This code is mainly borrowed from QGIS desktop Python plugin initialization
bool QgsServerPlugins::initPlugins( QgsServerInterface *interface )
{
  QString pythonlibName( QStringLiteral( "qgispython" ) );
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = QStringLiteral( "%1.%2.%3" ).arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 ).arg( Qgis::QGIS_VERSION_INT % 100 );
  QgsMessageLog::logMessage( QStringLiteral( "load library %1 (%2)" ).arg( pythonlibName, version ), __FILE__, Qgis::Info );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Couldn't load Python support library: %1" ).arg( pythonlib.errorString() ) );
      return false;
    }
  }

  QgsMessageLog::logMessage( QStringLiteral( "Python support library loaded successfully." ), __FILE__, Qgis::Info );
  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = ( inst ) cast_to_fptr( pythonlib.resolve( "instance" ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsDebugMsg( QStringLiteral( "Couldn't resolve python support library's instance() symbol." ) );
    return false;
  }

  QgsDebugMsg( "Python support library's instance() symbol resolved." );
  sPythonUtils = pythonlib_inst();
  sPythonUtils->initServerPython( interface );

  if ( sPythonUtils && sPythonUtils->isEnabled() )
  {
    QgsDebugMsg( "Python support ENABLED :-)" );
  }
  else
  {
    QgsDebugMsg( "Python support FAILED :-(" );
    return false;
  }

  //Init plugins: loads a list of installed plugins and filter them
  //for "server" metadata
  bool atLeastOneEnabled = false;
  for ( const QString &pluginName : sPythonUtils->pluginList() )
  {
    QString pluginService = sPythonUtils->getPluginMetadata( pluginName, QStringLiteral( "server" ) );
    if ( pluginService == QLatin1String( "True" ) )
    {
      if ( sPythonUtils->loadPlugin( pluginName ) )
      {
        if ( sPythonUtils->startServerPlugin( pluginName ) )
        {
          atLeastOneEnabled = true;
          serverPlugins().append( pluginName );
          QgsMessageLog::logMessage( QStringLiteral( "Server plugin %1 loaded!" ).arg( pluginName ), QStringLiteral( "Server" ), Qgis::Info );
        }
        else
        {
          QgsMessageLog::logMessage( QStringLiteral( "Error loading server plugin %1" ).arg( pluginName ), QStringLiteral( "Server" ), Qgis::Critical );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error starting server plugin %1" ).arg( pluginName ), QStringLiteral( "Server" ), Qgis::Critical );
      }
    }
  }
  return sPythonUtils && sPythonUtils->isEnabled() && atLeastOneEnabled;
}


