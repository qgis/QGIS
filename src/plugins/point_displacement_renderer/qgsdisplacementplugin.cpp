/***************************************************************************
                              qgsdisplacementplugin.cpp
                              -------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdisplacementplugin.h"
#include "qgisinterface.h"
#include "qgspointdisplacementrenderer.h"
#include "qgspointdisplacementrendererwidget.h"
#include "qgsrendererv2registry.h"
#include "qgssymbollayerv2registry.h"
#include <QObject>

static const QString name_ = QObject::tr( "Displacement plugin" );
static const QString description_ = QObject::tr( "Adds a new renderer that automatically handles point displacement in case they have the same position" );
static const QString version_ = QObject::tr( "Version 0.1" );

QgsDisplacementPlugin::QgsDisplacementPlugin( QgisInterface* iface ): mIface( iface )
{

}

QgsDisplacementPlugin::~QgsDisplacementPlugin()
{

}

void QgsDisplacementPlugin::initGui()
{
  //Add new renderer to the registry

  QgsRendererV2Registry::instance()->addRenderer( new QgsRendererV2Metadata( "pointDisplacement",
      QObject::tr( "point Displacement" ),
      QgsPointDisplacementRenderer::create, QIcon(),
      QgsPointDisplacementRendererWidget::create ) );
}

void QgsDisplacementPlugin::unload()
{
  //Remove renderer type from the registry
  QgsRendererV2Registry::instance()->removeRenderer( "pointDisplacement" );
}

QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsDisplacementPlugin( theQgisInterfacePointer );
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

QGISEXTERN void unload( QgisPlugin* thePluginPointer )
{
  delete thePluginPointer;
}


