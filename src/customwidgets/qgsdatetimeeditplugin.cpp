/***************************************************************************
   qgsdatetimeeditplugin.cpp
    --------------------------------------
   Date                 : 01.09.2014
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
#include "qgsdatetimeeditplugin.h"
#include "qgsdatetimeedit.h"


QgsDateTimeEditPlugin::QgsDateTimeEditPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsDateTimeEditPlugin::name() const
{
  return "QgsDateTimeEdit";
}

QString QgsDateTimeEditPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsDateTimeEditPlugin::includeFile() const
{
  return "qgsdatetimeedit.h";
}

QIcon QgsDateTimeEditPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsDateTimeEditPlugin::isContainer() const
{
  return false;
}

QWidget *QgsDateTimeEditPlugin::createWidget( QWidget *parent )
{
  return new QgsDateTimeEdit( parent );
}

bool QgsDateTimeEditPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsDateTimeEditPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsDateTimeEditPlugin::toolTip() const
{
  return tr( "Define date" );
}

QString QgsDateTimeEditPlugin::whatsThis() const
{
  return "";
}

QString QgsDateTimeEditPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mDateTimeEdit\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>90</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
