/***************************************************************************
   qgsfilewidgetplugin.cpp
    --------------------------------------
   Date                 : 13.01.2016
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
#include "qgsfilewidgetplugin.h"
#include "qgsfilewidget.h"


QgsFileWidgetPlugin::QgsFileWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}

QString QgsFileWidgetPlugin::name() const
{
  return "QgsFileWidget";
}

QString QgsFileWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFileWidgetPlugin::includeFile() const
{
  return "qgsfilewidget.h";
}

QIcon QgsFileWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFileWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFileWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsFileWidget( parent );
}

bool QgsFileWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFileWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFileWidgetPlugin::toolTip() const
{
  return "";
}

QString QgsFileWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsFileWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsFileWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>90</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
