/***************************************************************************
   qgscheckablecomboboxplugin.cpp
    --------------------------------------
   Date                 : March 22, 2017
   Copyright            : (C) 2017 Alexander Bruy
   Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgscheckablecombobox.h"
#include "qgscheckablecomboboxplugin.h"
#include "moc_qgscheckablecomboboxplugin.cpp"


QgsCheckableComboBoxPlugin::QgsCheckableComboBoxPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsCheckableComboBoxPlugin::name() const
{
  return "QgsCheckableComboBox";
}

QString QgsCheckableComboBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsCheckableComboBoxPlugin::includeFile() const
{
  return "qgscheckablecombobox.h";
}

QIcon QgsCheckableComboBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsCheckableComboBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsCheckableComboBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsCheckableComboBox( parent );
}

bool QgsCheckableComboBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsCheckableComboBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsCheckableComboBoxPlugin::toolTip() const
{
  return "";
}

QString QgsCheckableComboBoxPlugin::whatsThis() const
{
  return "";
}

QString QgsCheckableComboBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mComboBox\">\n"
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
