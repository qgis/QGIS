/***************************************************************************
   qgsfeaturelistcomboboxplugin.cpp
    --------------------------------------
   Date                 : 26.03.2020
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
#include "qgsfeaturelistcomboboxplugin.h"
#include "moc_qgsfeaturelistcomboboxplugin.cpp"
#include "qgsfeaturelistcombobox.h"


QgsFeatureListComboBoxPlugin::QgsFeatureListComboBoxPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}

QString QgsFeatureListComboBoxPlugin::name() const
{
  return "QgsFeatureListComboBox";
}

QString QgsFeatureListComboBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFeatureListComboBoxPlugin::includeFile() const
{
  return "qgsfeaturelistcombobox.h";
}

QIcon QgsFeatureListComboBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFeatureListComboBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFeatureListComboBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsFeatureListComboBox( parent );
}

bool QgsFeatureListComboBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFeatureListComboBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFeatureListComboBoxPlugin::toolTip() const
{
  return "";
}

QString QgsFeatureListComboBoxPlugin::whatsThis() const
{
  return "";
}

QString QgsFeatureListComboBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mFeatureListComboBox\">\n"
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
