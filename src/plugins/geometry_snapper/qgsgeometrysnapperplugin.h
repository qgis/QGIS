/***************************************************************************
 *  qgsgeometrysnapperplugin.h                                             *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_SNAPPER_PLUGIN_H
#define QGS_GEOMETRY_SNAPPER_PLUGIN_H

#include "qgis.h"
#include "qgisplugin.h"
#include "qgsgeometrysnapperdialog.h"

class QgisInterface;

static const QString sName = QApplication::translate( "QgsGeometrySnapperPlugin", "Geometry Snapper" );
static const QString sDescription = QApplication::translate( "QgsGeometrySnapperPlugin", "Snap geometries to a reference layer" );
static const QString sCategory = QApplication::translate( "QgsGeometrySnapperPlugin", "Vector" );
static const QString sPluginVersion = QApplication::translate( "QgsGeometrySnapperPlugin", "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/geometrysnapper/icons/geometrysnapper.png";


class QgsGeometrySnapperPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    explicit QgsGeometrySnapperPlugin( QgisInterface* iface );
    void initGui() override;
    void unload() override;

  private:
    QgisInterface* mIface;
    QgsGeometrySnapperDialog* mDialog;
    QAction* mMenuAction;
};

#endif // QGS_GEOMETRY_SNAPPER_PLUGIN_H
