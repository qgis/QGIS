/***************************************************************************
   qgsscalerangewidgetplugin.cpp
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
#include "qgsscalerangewidgetplugin.h"
#include "qgsscalerangewidget.h"


QgsScaleRangeWidgetPlugin::QgsScaleRangeWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsScaleRangeWidgetPlugin::name() const
{
  return "QgsScaleRangeWidget";
}

QString QgsScaleRangeWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsScaleRangeWidgetPlugin::includeFile() const
{
  return "qgsscalerangewidget.h";
}

QIcon QgsScaleRangeWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsScaleRangeWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsScaleRangeWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsScaleRangeWidget( parent );
}

bool QgsScaleRangeWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsScaleRangeWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsScaleRangeWidgetPlugin::toolTip() const
{
  return tr( "A widget to define the scale range" );
}

QString QgsScaleRangeWidgetPlugin::whatsThis() const
{
  return tr( "A widget to define the scale range." );
}

QString QgsScaleRangeWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mScaleRangeWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>400</width>\n"
                  "    <height>100</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
