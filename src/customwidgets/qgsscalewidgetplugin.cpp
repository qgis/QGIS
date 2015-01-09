/***************************************************************************
   qgsscalewidgetplugin.cpp
    --------------------------------------
   Date                 : 08.01.2015
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
#include "qgsscalewidgetplugin.h"
#include "qgsscalewidget.h"


QgsScaleWidgetPlugin::QgsScaleWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsScaleWidgetPlugin::name() const
{
  return "QgsScaleWidget";
}

QString QgsScaleWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsScaleWidgetPlugin::includeFile() const
{
  return "qgsscalewidget.h";
}

QIcon QgsScaleWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsScaleWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsScaleWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsScaleWidget( parent );
}

bool QgsScaleWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsScaleWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsScaleWidgetPlugin::toolTip() const
{
  return tr( "A widget to define the scale" );
}

QString QgsScaleWidgetPlugin::whatsThis() const
{
  return tr( "A widget to define the scale." );
}

QString QgsScaleWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mScaleWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>200</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  "  <property name=\"showCurrentScaleButton\">\n"
                  "   <bool>true</bool>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
