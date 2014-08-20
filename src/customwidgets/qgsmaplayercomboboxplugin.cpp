/***************************************************************************
   qgsmaplayercomboboxplugin.cpp
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
#include "qgsmaplayercombobox.h"
#include "qgsmaplayercomboboxplugin.h"


QgsMapLayerComboBoxPlugin::QgsMapLayerComboBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsMapLayerComboBoxPlugin::name() const
{
  return "QgsMapLayerComboBox";
}

QString QgsMapLayerComboBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsMapLayerComboBoxPlugin::includeFile() const
{
  return "qgsmaplayercombobox.h";
}

QIcon QgsMapLayerComboBoxPlugin::icon() const
{
  return QIcon();
}

bool QgsMapLayerComboBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsMapLayerComboBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsMapLayerComboBox( parent );
}

bool QgsMapLayerComboBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsMapLayerComboBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsMapLayerComboBoxPlugin::toolTip() const
{
  return tr( "A combo box to list the layers" );
}

QString QgsMapLayerComboBoxPlugin::whatsThis() const
{
  return tr( "A combo box to list the layers registered in QGIS. Layers might be filtered according to their type." );
}

QString QgsMapLayerComboBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mMapLayerComboBox\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>160</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
