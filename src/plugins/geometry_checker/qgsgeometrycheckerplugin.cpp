/***************************************************************************
 *  qgsgeometrycheckerplugin.cpp                                           *
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

#include "qgsgeometrycheckerplugin.h"
#include "qgisinterface.h"
#include "ui/qgsgeometrycheckerdialog.h"

QgsGeometryCheckerPlugin::QgsGeometryCheckerPlugin( QgisInterface* iface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
    , mIface( iface )
    , mDialog( 0 )
    , mMenuAction( 0 )
{
}

void QgsGeometryCheckerPlugin::initGui()
{
  mDialog = new QgsGeometryCheckerDialog( mIface, mIface->mainWindow() );
  mDialog->setWindowModality( Qt::NonModal );
  mMenuAction = new QAction( QIcon( ":/geometrychecker/icons/geometrychecker.png" ), QApplication::translate( "QgsGeometryCheckerPlugin", "Check Geometries" ), this );
  connect( mMenuAction, SIGNAL( triggered() ), mDialog, SLOT( show() ) );
  connect( mMenuAction, SIGNAL( triggered() ), mDialog, SLOT( raise() ) );
  mIface->addPluginToVectorMenu( QApplication::translate( "QgsGeometryCheckerPlugin", "G&eometry Tools" ), mMenuAction );
}

void QgsGeometryCheckerPlugin::unload()
{
  delete mDialog;
  mDialog = 0;
  delete mMenuAction;
  mMenuAction = 0;
  mIface->removePluginVectorMenu( QApplication::translate( "QgsGeometryCheckerPlugin", "G&eometry Tools" ), mMenuAction );
}
