/***************************************************************************
   qgsscalevisibilitywidgetplugin.cpp
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
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
#include "qgsscalevisibilitywidgetplugin.h"
#include "qgsscalevisibilitywidget.h"


QgsScaleVisibilityWidgetPlugin::QgsScaleVisibilityWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsScaleVisibilityWidgetPlugin::name() const
{
  return "QgsScaleVisibilityWidget";
}

QString QgsScaleVisibilityWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsScaleVisibilityWidgetPlugin::includeFile() const
{
  return "qgsscalevisibilitywidget.h";
}

QIcon QgsScaleVisibilityWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsScaleVisibilityWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsScaleVisibilityWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsScaleVisibilityWidget( parent );
}

bool QgsScaleVisibilityWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsScaleVisibilityWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsScaleVisibilityWidgetPlugin::toolTip() const
{
  return "A widget to define the scale visibility";
}

QString QgsScaleVisibilityWidgetPlugin::whatsThis() const
{
  return "A widget to define the scale visibility";
}

QString QgsScaleVisibilityWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mScaleVisibilityWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>400</width>\n"
                  "    <height>100</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  "  <property name=\"toolTip\" >\n"
                  "   <string></string>\n"
                  "  </property>\n"
                  "  <property name=\"whatsThis\" >\n"
                  "   <string></string>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
