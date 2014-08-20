/***************************************************************************
   qgsfieldcomboboxplugin.cpp
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
#include "qgsfieldcombobox.h"
#include "qgsfieldcomboboxplugin.h"


QgsFieldComboBoxPlugin::QgsFieldComboBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsFieldComboBoxPlugin::name() const
{
  return "QgsFieldComboBox";
}

QString QgsFieldComboBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFieldComboBoxPlugin::includeFile() const
{
  return "qgsfieldcombobox.h";
}

QIcon QgsFieldComboBoxPlugin::icon() const
{
  return QIcon();
}

bool QgsFieldComboBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFieldComboBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsFieldComboBox( parent );
}

bool QgsFieldComboBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFieldComboBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFieldComboBoxPlugin::toolTip() const
{
  return tr( "A combo box to list the fields of a layer" );
}

QString QgsFieldComboBoxPlugin::whatsThis() const
{
  return tr( "A combo box to list the field of a layer." );
}

QString QgsFieldComboBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mFieldComboBox\">\n"
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
