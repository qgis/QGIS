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

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgspythonutils.h"

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
  QString pythonlibName( u"qgispython"_s );
#if defined( Q_OS_UNIX )
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  const QString version = u"%1.%2.%3"_s.arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 ).arg( Qgis::versionInt() % 100 );
  QgsMessageLog::logMessage( u"load library %1 (%2)"_s.arg( pythonlibName, version ), __FILE__, Qgis::MessageLevel::Info );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      QgsMessageLog::logMessage( u"Couldn't load Python support library: %1"_s.arg( pythonlib.errorString() ) );
      return false;
    }
  }

  QgsMessageLog::logMessage( u"Python support library loaded successfully."_s, __FILE__, Qgis::MessageLevel::Info );
  typedef QgsPythonUtils *( *inst )();
  inst pythonlib_inst = ( inst ) cast_to_fptr( pythonlib.resolve( "instance" ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsDebugError( u"Couldn't resolve python support library's instance() symbol."_s );
    return false;
  }

  QgsDebugMsgLevel( u"Python support library's instance() symbol resolved."_s, 2 );
  sPythonUtils = pythonlib_inst();
  if ( sPythonUtils )
  {
    sPythonUtils->initServerPython( interface );
  }

  if ( sPythonUtils && sPythonUtils->isEnabled() )
  {
    QgsDebugMsgLevel( u"Python support ENABLED :-)"_s, 2 );
  }
  else
  {
    QgsDebugError( u"Python support FAILED :-("_s );
    return false;
  }

  //Init plugins: loads a list of installed plugins and filter them
  //for "server" metadata
  bool atLeastOneEnabled = false;

  const auto constPluginList( sPythonUtils->pluginList() );
  for ( const QString &pluginName : constPluginList )
  {
    const QString pluginService = sPythonUtils->getPluginMetadata( pluginName, u"server"_s );
    if ( pluginService == "True"_L1 )
    {
      if ( sPythonUtils->loadPlugin( pluginName ) )
      {
        if ( sPythonUtils->startServerPlugin( pluginName ) )
        {
          atLeastOneEnabled = true;
          serverPlugins().append( pluginName );
          QgsMessageLog::logMessage( u"Server plugin %1 loaded!"_s.arg( pluginName ), u"Server"_s, Qgis::MessageLevel::Info );
        }
        else
        {
          QgsMessageLog::logMessage( u"Error loading server plugin %1"_s.arg( pluginName ), u"Server"_s, Qgis::MessageLevel::Critical );
        }
      }
      else
      {
        QgsMessageLog::logMessage( u"Error starting server plugin %1"_s.arg( pluginName ), u"Server"_s, Qgis::MessageLevel::Critical );
      }
    }
  }
  return sPythonUtils && sPythonUtils->isEnabled() && atLeastOneEnabled;
}
