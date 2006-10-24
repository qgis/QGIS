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

#include "qgsproviderregistry.h"
#include "qgswfssourceselect.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgswfsplugin.h"
#include "qgswfsprovider.h"

#include "mIconAddWfsLayer.xpm"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

QgsWFSPlugin::QgsWFSPlugin(QgisApp* app, QgisIface* iface): QgisPlugin("WFS plugin", "A plugin to add WFS layers to the QGIS canvas", "Version 0.0001", QgisPlugin::MAPLAYER), mApp(app), mIface(iface), mWfsDialogAction(0)
{

}

QgsWFSPlugin::~QgsWFSPlugin()
{
  delete mWfsDialogAction;
}

void QgsWFSPlugin::initGui()
{
  if(mIface)
    {
      mWfsDialogAction = new QAction(QIcon(mIconAddWfsLayer), tr("&Add WFS layer"), 0);
      QObject::connect(mWfsDialogAction, SIGNAL(activated()), this, SLOT(showSourceDialog()));
      mIface->addToolBarIcon(mWfsDialogAction);
      mIface->addPluginMenu(tr("&Add WFS layer"), mWfsDialogAction);
    }
}

void QgsWFSPlugin::unload()
{
  mIface->removeToolBarIcon(mWfsDialogAction);
  mIface->removePluginMenu(tr("&Add WFS layer"), mWfsDialogAction);
  delete mWfsDialogAction;
  mWfsDialogAction = 0;
}

void QgsWFSPlugin::showSourceDialog()
{
  QgsWFSSourceSelect serverDialog(0, mIface);
  serverDialog.exec();
}

QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsWFSPlugin(theQGisAppPointer, theQgisInterfacePointer);
}

QGISEXTERN QString name()
{
  return QString("WFS plugin");
}

QGISEXTERN QString description()
{
  return QString("A plugin to add WFS layers to the QGIS canvas");
}

QGISEXTERN QString version()
{
  return QString("Version 0.0001");
}

QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

QGISEXTERN void unload(QgisPlugin* theQgsWFSPluginPointer)
{
  delete theQgsWFSPluginPointer;
}
