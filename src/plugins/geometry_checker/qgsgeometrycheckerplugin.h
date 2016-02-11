/***************************************************************************
 *  qgsgeometrycheckerplugin.h                                             *
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

#ifndef QGS_GEOMETRY_CHECKER_PLUGIN_H
#define QGS_GEOMETRY_CHECKER_PLUGIN_H

#include "qgis.h"
#include "qgisplugin.h"
#include <QAction>
#include <QApplication>

class QgsGeometryCheckerDialog;


class QgsGeometryCheckerPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    explicit QgsGeometryCheckerPlugin( QgisInterface* iface );
    void initGui() override;
    void unload() override;

  private:
    QgisInterface* mIface;
    QgsGeometryCheckerDialog* mDialog;
    QAction* mMenuAction;
};


static const QString sName = QApplication::translate( "QgsGeometryCheckerPlugin", "Geometry Checker" );
static const QString sDescription = QApplication::translate( "QgsGeometryCheckerPlugin", "Check geometries for errors" );
static const QString sCategory = QApplication::translate( "QgsGeometryCheckerPlugin", "Vector" );
static const QString sPluginVersion = QApplication::translate( "QgsGeometryCheckerPlugin", "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/geometrychecker/icons/geometrychecker.png";

#endif // QGS_GEOMETRY_CHECKER_PLUGIN_H
