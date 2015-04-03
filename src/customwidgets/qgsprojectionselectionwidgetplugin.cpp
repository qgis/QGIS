/***************************************************************************
   qgsprojectionselectionwidgetplugin.cpp
    --------------------------------------
   Date                 : 05.01.2015
   Copyright            : (C) 2015 Denis Rouzaud
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
#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectionwidgetplugin.h"


QgsProjectionSelectionWidgetPlugin::QgsProjectionSelectionWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsProjectionSelectionWidgetPlugin::name() const
{
  return "QgsProjectionSelectionWidget";
}

QString QgsProjectionSelectionWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsProjectionSelectionWidgetPlugin::includeFile() const
{
  return "qgsprojectionselectionwidget.h";
}

QIcon QgsProjectionSelectionWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsProjectionSelectionWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsProjectionSelectionWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsProjectionSelectionWidget( parent );
}

bool QgsProjectionSelectionWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsProjectionSelectionWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsProjectionSelectionWidgetPlugin::toolTip() const
{
  return tr( "A widget to select a generic projection system." );
}

QString QgsProjectionSelectionWidgetPlugin::whatsThis() const
{
  return tr( "A widget to select a generic projection system." );
}

QString QgsProjectionSelectionWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsProjectionSelectionWidget\">\n"
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
