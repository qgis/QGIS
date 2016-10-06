/***************************************************************************
   qgsdockwidgetplugin.cpp
    --------------------------------------
   Date                 : 10.06.2016
   Copyright            : (C) 2016 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgsdockwidget.h"
#include "qgsdockwidgetplugin.h"


QgsDockWidgetPlugin::QgsDockWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsDockWidgetPlugin::name() const
{
  return "QgsDockWidget";
}

QString QgsDockWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsDockWidgetPlugin::includeFile() const
{
  return "qgsdockwidget.h";
}

QIcon QgsDockWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsDockWidgetPlugin::isContainer() const
{
  return true;
}

QWidget *QgsDockWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsDockWidget( parent );
}

bool QgsDockWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsDockWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsDockWidgetPlugin::toolTip() const
{
  return tr( "A dock widget" );
}

QString QgsDockWidgetPlugin::whatsThis() const
{
  return tr( "A dock widget" );
}

QString QgsDockWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mDockWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>300</width>\n"
                  "    <height>500</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
