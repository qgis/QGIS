/***************************************************************************
  qgsquickplugin.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKPLUGIN_H
#define QGSQUICKPLUGIN_H

#include <QQmlExtensionPlugin>
#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

class QgisQuickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.qt-project.Qt.QQmlExtensionInterface" )

  public:
    void registerTypes( const char *uri );
};

#endif // QGSQUICKPLUGIN_H

