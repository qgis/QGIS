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
#include "qgsserverlogger.h"
#include "qgsmsutils.h"

#include <QLibrary>

QgsServerPlugins::QgsServerPlugins()
{
}

// Initialize static members
QgsPythonUtils* QgsServerPlugins::mPythonUtils;
// Initialize static members
QStringList QgsServerPlugins::mServerPlugins;

// This code is mainly borrowed from QGIS desktop Python plugin initialization
bool QgsServerPlugins::initPlugins( QgsServerInterface *interface )
{

  QString pythonlibName( "qgispython" );
#if defined(Q_WS_MAC) || defined(Q_OS_LINUX)
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = QString( "%1.%2.%3" ).arg( QGis::QGIS_VERSION_INT / 10000 ).arg( QGis::QGIS_VERSION_INT / 100 % 100 ).arg( QGis::QGIS_VERSION_INT % 100 );
  QgsDebugMsg( QString( "load library %1 (%2)" ).arg( pythonlibName ).arg( version ) );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      QgsDebugMsg( QString( "Couldn't load Python support library: %1" ).arg( pythonlib.errorString() ) );
      return FALSE;
    }
  }

  QgsDebugMsg( "Python support library loaded successfully." );
  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = ( inst ) cast_to_fptr( pythonlib.resolve( "instance" ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsDebugMsg( QString( "Couldn't resolve python support library's instance() symbol." ) );
    return FALSE;
  }

  QgsDebugMsg( "Python support library's instance() symbol resolved." );
  mPythonUtils = pythonlib_inst();
  mPythonUtils->initServerPython( interface );

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsDebugMsg( "Python support ENABLED :-)" );
  }
  else
  {
    QgsDebugMsg( "Python support FAILED :-(" );
    return FALSE;
  }

  //Init plugins: loads a list of installed plugins and filter them
  //for "server" metadata
  QListIterator<QString> plugins( mPythonUtils->pluginList() );
  bool atLeastOneEnabled = FALSE;
  while ( plugins.hasNext() )
  {
    QString pluginName = plugins.next();
    QString pluginService = mPythonUtils->getPluginMetadata( pluginName, "server" );
    if ( pluginService == "True" )
    {
      if ( mPythonUtils->loadPlugin( pluginName ) )
      {
        if ( mPythonUtils->startServerPlugin( pluginName ) )
        {
          atLeastOneEnabled = TRUE;
          mServerPlugins.append( pluginName );
          QgsMessageLog::logMessage( QString( "Server plugin %1 loaded!" ).arg( pluginName ), "Server", QgsMessageLog::INFO );
        }
        else
        {
          QgsMessageLog::logMessage( QString( "Error loading server plugin %1" ).arg( pluginName ), "Server", QgsMessageLog::CRITICAL );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QString( "Error starting server plugin %1" ).arg( pluginName ), "Server", QgsMessageLog::CRITICAL );
      }
    }
  }
  return mPythonUtils && mPythonUtils->isEnabled() && atLeastOneEnabled;
}
