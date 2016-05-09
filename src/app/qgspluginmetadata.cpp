/***************************************************************************
                    qgspluginmetadata.cpp  -  Metadata class for
                    describing a loaded plugin.
                             -------------------
    begin                : Fri Feb 6 2004
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

#include "../plugins/qgisplugin.h"
#include "qgspluginmetadata.h"
QgsPluginMetadata::QgsPluginMetadata( const QString& _libraryPath,
                                      const QString& _name,
                                      QgisPlugin * _plugin )
    : m_name( _name )
    , libraryPath( _libraryPath )
    , m_plugin( _plugin )
{

}

QString QgsPluginMetadata::name() const
{
  return m_name;
}

QString QgsPluginMetadata::library() const
{
  return libraryPath;
}

QgisPlugin *QgsPluginMetadata::plugin()
{
  return m_plugin;
}
