/***************************************************************************
   qgsexternalresourcewidgetplugin.cpp
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
#include "qgsexternalresourcewidgetplugin.h"
#include "qgsexternalresourcewidget.h"


QgsExternalResourceWidgetPlugin::QgsExternalResourceWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}

QString QgsExternalResourceWidgetPlugin::name() const
{
  return "QgsExternalResourceWidget";
}

QString QgsExternalResourceWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsExternalResourceWidgetPlugin::includeFile() const
{
  return "qgsexternalresourcewidget.h";
}

QIcon QgsExternalResourceWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsExternalResourceWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsExternalResourceWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsExternalResourceWidget( parent );
}

bool QgsExternalResourceWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsExternalResourceWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsExternalResourceWidgetPlugin::toolTip() const
{
  return "";
}

QString QgsExternalResourceWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsExternalResourceWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsExternalResourceWidget\">\n"
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
