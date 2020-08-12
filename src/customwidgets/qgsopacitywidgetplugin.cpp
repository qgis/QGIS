/***************************************************************************
   qgsopacitywidgetplugin.cpp
    -------------------------
   Date                 : 30.05.2017
   Copyright            : (C) 2017 Nyall Dawson
   Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgsopacitywidget.h"
#include "qgsopacitywidgetplugin.h"


QgsOpacityWidgetPlugin::QgsOpacityWidgetPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsOpacityWidgetPlugin::name() const
{
  return "QgsOpacityWidget";
}

QString QgsOpacityWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsOpacityWidgetPlugin::includeFile() const
{
  return "qgsopacitywidget.h";
}

QIcon QgsOpacityWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsOpacityWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsOpacityWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsOpacityWidget( parent );
}

bool QgsOpacityWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsOpacityWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsOpacityWidgetPlugin::toolTip() const
{
  return tr( "A widget for specifying an opacity value." );
}

QString QgsOpacityWidgetPlugin::whatsThis() const
{
  return tr( "A widget for specifying an opacity value." );
}

QString QgsOpacityWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mOpacityWidget\">\n"
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
