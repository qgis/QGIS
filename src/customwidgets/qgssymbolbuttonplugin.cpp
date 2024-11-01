/***************************************************************************
   qgssymbolbuttonplugin.cpp
    ------------------------
   Date                 : 23.07.2017
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
#include "qgssymbolbuttonplugin.h"
#include "moc_qgssymbolbuttonplugin.cpp"
#include "qgssymbolbutton.h"


QgsSymbolButtonPlugin::QgsSymbolButtonPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsSymbolButtonPlugin::name() const
{
  return "QgsSymbolButton";
}

QString QgsSymbolButtonPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsSymbolButtonPlugin::includeFile() const
{
  return "qgssymbolbutton.h";
}

QIcon QgsSymbolButtonPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsSymbolButtonPlugin::isContainer() const
{
  return false;
}

QWidget *QgsSymbolButtonPlugin::createWidget( QWidget *parent )
{
  return new QgsSymbolButton( parent );
}

bool QgsSymbolButtonPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsSymbolButtonPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsSymbolButtonPlugin::toolTip() const
{
  return tr( "Select symbol" );
}

QString QgsSymbolButtonPlugin::whatsThis() const
{
  return "";
}

QString QgsSymbolButtonPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mSymbolButton\">\n"
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
