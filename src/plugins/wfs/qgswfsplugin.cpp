/***************************************************************************
                              qgswfsplugin.h
                              -------------------
  begin                : July 25, 2006
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

#include "qgisinterface.h"
#include "qgsproviderregistry.h"
#include "qgswfssourceselect.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgswfsplugin.h"

#include <QToolBar>



static const QString name_ = QObject::tr( "WFS plugin" );
static const QString description_ = QObject::tr( "Adds WFS layers to the QGIS canvas" );
static const QString version_ = QObject::tr( "Version 0.1" );

QgsWFSPlugin::QgsWFSPlugin( QgisInterface* iface )
    : QgisPlugin( name_, description_, version_, QgisPlugin::MAPLAYER ),
    mIface( iface ), mWfsDialogAction( 0 )
{

}

QgsWFSPlugin::~QgsWFSPlugin()
{
  delete mWfsDialogAction;
}

void QgsWFSPlugin::initGui()
{
  if ( mIface )
  {
    mWfsDialogAction = new QAction( QIcon( ":/mIconAddWfsLayer.png" ), tr( "&Add WFS layer" ), 0 );
    QObject::connect( mWfsDialogAction, SIGNAL( triggered() ), this, SLOT( showSourceDialog() ) );
    mIface->fileToolBar()->addAction( mWfsDialogAction );
    mIface->addPluginToMenu( tr( "&Add WFS layer" ), mWfsDialogAction );
  }
}

void QgsWFSPlugin::unload()
{
  mIface->removeToolBarIcon( mWfsDialogAction );
  mIface->removePluginMenu( tr( "&Add WFS layer" ), mWfsDialogAction );
  delete mWfsDialogAction;
  mWfsDialogAction = 0;
}

void QgsWFSPlugin::showSourceDialog()
{
  QgsWFSSourceSelect serverDialog( 0, mIface );
  serverDialog.exec();
}

QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsWFSPlugin( theQgisInterfacePointer );
}

QGISEXTERN QString name()
{
  return name_;
}

QGISEXTERN QString description()
{
  return description_;
}

QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

QGISEXTERN void unload( QgisPlugin* theQgsWFSPluginPointer )
{
  delete theQgsWFSPluginPointer;
}
