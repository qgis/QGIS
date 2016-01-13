/***************************************************************************
   qgsfilepickerwidgetplugin.cpp
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
#include "qgsfilepickerwidgetplugin.h"
#include "qgsfilepickerwidget.h"


QgsFilePickerWidgetPlugin::QgsFilePickerWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}

QString QgsFilePickerWidgetPlugin::name() const
{
  return "QgsFilePickerWidget";
}

QString QgsFilePickerWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFilePickerWidgetPlugin::includeFile() const
{
  return "qgsfilepickerwidget.h";
}

QIcon QgsFilePickerWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFilePickerWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFilePickerWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsFilePickerWidget( parent );
}

bool QgsFilePickerWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFilePickerWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFilePickerWidgetPlugin::toolTip() const
{
  return "";
}

QString QgsFilePickerWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsFilePickerWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsFilePickerWidget\">\n"
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
