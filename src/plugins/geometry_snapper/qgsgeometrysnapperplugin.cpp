/***************************************************************************
 *  qgsgeometrysnapperplugin.cpp                                           *
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

#include "qgsgeometrysnapperplugin.h"
#include "qgisinterface.h"

QgsGeometrySnapperPlugin::QgsGeometrySnapperPlugin( QgisInterface* iface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
    , mIface( iface )
    , mDialog( 0 )
    , mMenuAction( 0 )
{
}

void QgsGeometrySnapperPlugin::initGui()
{
  mDialog = new QgsGeometrySnapperDialog( mIface );
  mMenuAction = new QAction( QIcon( ":/geometrysnapper/icons/geometrysnapper.png" ), QApplication::translate( "QgsGeometrySnapperPlugin", "Snap geometries" ), this );
  connect( mMenuAction, SIGNAL( triggered() ), mDialog, SLOT( show() ) );
  mIface->addPluginToVectorMenu( QApplication::translate( "QgsGeometrySnapperPlugin", "G&eometry Tools" ), mMenuAction );
}

void QgsGeometrySnapperPlugin::unload()
{
  delete mDialog;
  mDialog = 0;
  delete mMenuAction;
  mMenuAction = 0;
  mIface->removePluginVectorMenu( QApplication::translate( "QgsGeometrySnapperPlugin", "G&eometry Tools" ), mMenuAction );
}
