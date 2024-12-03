/***************************************************************************
   qgsfontbuttonplugin.cpp
    ----------------------
   Date                 : 06.07.2017
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
#include "qgsfontbuttonplugin.h"
#include "moc_qgsfontbuttonplugin.cpp"
#include "qgsfontbutton.h"


QgsFontButtonPlugin::QgsFontButtonPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsFontButtonPlugin::name() const
{
  return "QgsFontButton";
}

QString QgsFontButtonPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFontButtonPlugin::includeFile() const
{
  return "qgsfontbutton.h";
}

QIcon QgsFontButtonPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFontButtonPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFontButtonPlugin::createWidget( QWidget *parent )
{
  return new QgsFontButton( parent );
}

bool QgsFontButtonPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFontButtonPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFontButtonPlugin::toolTip() const
{
  return tr( "Select font" );
}

QString QgsFontButtonPlugin::whatsThis() const
{
  return "";
}

QString QgsFontButtonPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mFontButton\">\n"
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
