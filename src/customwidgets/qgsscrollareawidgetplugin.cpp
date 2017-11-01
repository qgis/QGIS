/***************************************************************************
   qgsscrollareawidgetplugin.cpp
    --------------------------------------
   Date                 : March 2017
   Copyright            : (C) 2017 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "qgiscustomwidgets.h"
#include "qgsscrollareawidgetplugin.h"
#include "qgsscrollarea.h"


QgsScrollAreaWidgetPlugin::QgsScrollAreaWidgetPlugin( QObject *parent )
  : QObject( parent )
{}

QString QgsScrollAreaWidgetPlugin::name() const
{
  return "QgsScrollArea";
}

QString QgsScrollAreaWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsScrollAreaWidgetPlugin::includeFile() const
{
  return "qgsscrollarea.h";
}

QIcon QgsScrollAreaWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsScrollAreaWidgetPlugin::isContainer() const
{
  return true;
}

QWidget *QgsScrollAreaWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsScrollArea( parent );
}

bool QgsScrollAreaWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsScrollAreaWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsScrollAreaWidgetPlugin::toolTip() const
{
  return tr( "Scroll area" );
}

QString QgsScrollAreaWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsScrollAreaWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mScrollArea\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>300</width>\n"
                  "    <height>100</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  "  <property name=\"widgetResizable\">\n"
                  "   <bool>true</bool>\n"
                  "  </property>\n"
                  "  <widget class=\"QWidget\" name=\"scrollAreaWidgetContents\">\n"
                  "   <property name=\"geometry\">\n"
                  "    <rect>\n"
                  "     <x>0</x>\n"
                  "     <y>0</y>\n"
                  "     <width>118</width>\n"
                  "     <height>78</height>\n"
                  "    </rect>\n"
                  "   </property>\n"
                  "  </widget>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
