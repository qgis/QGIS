/***************************************************************************
   qgspropertyoverridebuttonplugin.cpp
    ----------------------------------
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
#include "qgspropertyoverridebuttonplugin.h"
#include "moc_qgspropertyoverridebuttonplugin.cpp"
#include "qgspropertyoverridebutton.h"


QgsPropertyOverrideButtonPlugin::QgsPropertyOverrideButtonPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsPropertyOverrideButtonPlugin::name() const
{
  return "QgsPropertyOverrideButton";
}

QString QgsPropertyOverrideButtonPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsPropertyOverrideButtonPlugin::includeFile() const
{
  return "qgspropertyoverridebutton.h";
}

QIcon QgsPropertyOverrideButtonPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsPropertyOverrideButtonPlugin::isContainer() const
{
  return false;
}

QWidget *QgsPropertyOverrideButtonPlugin::createWidget( QWidget *parent )
{
  return new QgsPropertyOverrideButton( parent );
}

bool QgsPropertyOverrideButtonPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsPropertyOverrideButtonPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsPropertyOverrideButtonPlugin::toolTip() const
{
  return tr( "A widget to define override for a corresponding property" );
}

QString QgsPropertyOverrideButtonPlugin::whatsThis() const
{
  return tr( "A widget to define override for a corresponding property." );
}

QString QgsPropertyOverrideButtonPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mPropertyOverrideButton\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>27</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
    .arg( name() );
}
