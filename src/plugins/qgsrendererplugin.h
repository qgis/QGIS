/***************************************************************************
                         qgsrendererplugin.h  -  description
                             -------------------
    begin                : April 2006
    copyright            : (C) 2006 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERERPLUGIN_H
#define QGSRENDERERPLUGIN_H

#include "qgisplugin.h"

class QgsRenderer;
class QDialog;
class QString;

/**Interface class for renderer plugins. This kind of plugins is of plugin type QgisPlugin::RENDERER
 and is capable to return a pointer to a concrete renderer and a dialog which can be embedded into the
vector layer properties dialog in the main qgis application*/
class QgsRendererPlugin: public QgisPlugin
{
  public:
    QgsRendererPlugin( const QString& name, const QString& description, const QString& version ): QgisPlugin( name, description, version, QgisPlugin::RENDERER ) {}
    virtual ~QgsRendererPlugin() {}
    virtual QDialog* rendererDialog() = 0;
    virtual QgsRenderer* renderer() = 0;
};

#endif
