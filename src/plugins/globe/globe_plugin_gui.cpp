/***************************************************************************
    globe_plugin_gui.cpp

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "globe_plugin_gui.h"

#include "qgscontexthelp.h"

QgsGlobePluginGui::QgsGlobePluginGui( QWidget* parent /*= 0*/, Qt::WFlags fl /*= 0*/ )
  : QDialog( parent, fl )
{
  setupUi( this );
}

QgsGlobePluginGui::~QgsGlobePluginGui()
{
}

void QgsGlobePluginGui::on_buttonBox_accepted()
{
  accept();
}

void QgsGlobePluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsGlobePluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}
