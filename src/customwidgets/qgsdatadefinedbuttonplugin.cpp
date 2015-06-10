/***************************************************************************
   qgsdatadefinedbuttonplugin.cpp
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
#include "qgsdatadefinedbuttonplugin.h"
#include "qgsdatadefinedbutton.h"


QgsDataDefinedButtonPlugin::QgsDataDefinedButtonPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsDataDefinedButtonPlugin::name() const
{
  return "QgsDataDefinedButton";
}

QString QgsDataDefinedButtonPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsDataDefinedButtonPlugin::includeFile() const
{
  return "qgsdatadefinedbutton.h";
}

QIcon QgsDataDefinedButtonPlugin::icon() const
{
  return QIcon();
}

bool QgsDataDefinedButtonPlugin::isContainer() const
{
  return false;
}

QWidget *QgsDataDefinedButtonPlugin::createWidget( QWidget *parent )
{
  return new QgsDataDefinedButton( parent );
}

bool QgsDataDefinedButtonPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsDataDefinedButtonPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsDataDefinedButtonPlugin::toolTip() const
{
  return tr( "A widget to define the scale range" );
}

QString QgsDataDefinedButtonPlugin::whatsThis() const
{
  return tr( "A widget to define the scale range." );
}

QString QgsDataDefinedButtonPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mDataDefinedButton\">\n"
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
