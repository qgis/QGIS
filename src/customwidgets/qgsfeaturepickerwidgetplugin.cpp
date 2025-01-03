/***************************************************************************
   qgsfeaturepickerwidgetplugin.cpp
    --------------------------------------
   Date                 : 01.05.2020
   Copyright            : (C) 2020 Denis Rouzaud
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
#include "qgsfeaturepickerwidgetplugin.h"
#include "moc_qgsfeaturepickerwidgetplugin.cpp"
#include "qgsfeaturepickerwidget.h"


QgsFeaturePickerWidgetPlugin::QgsFeaturePickerWidgetPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}

QString QgsFeaturePickerWidgetPlugin::name() const
{
  return "QgsFeaturePickerWidget";
}

QString QgsFeaturePickerWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFeaturePickerWidgetPlugin::includeFile() const
{
  return "qgsfeaturepickerwidget.h";
}

QIcon QgsFeaturePickerWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFeaturePickerWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFeaturePickerWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsFeaturePickerWidget( parent );
}

bool QgsFeaturePickerWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFeaturePickerWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFeaturePickerWidgetPlugin::toolTip() const
{
  return "";
}

QString QgsFeaturePickerWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsFeaturePickerWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mFeaturePickerWidget\">\n"
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
