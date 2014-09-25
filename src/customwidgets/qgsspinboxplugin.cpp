/***************************************************************************
   qgsspinboxplugin.cpp
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
#include "qgsspinboxplugin.h"
#include "qgsspinbox.h"


QgsSpinBoxPlugin::QgsSpinBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsSpinBoxPlugin::name() const
{
  return "QgsSpinBox";
}

QString QgsSpinBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsSpinBoxPlugin::includeFile() const
{
  return "qgsspinbox.h";
}

QIcon QgsSpinBoxPlugin::icon() const
{
  return QIcon();
}

bool QgsSpinBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsSpinBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsSpinBox( parent );
}

bool QgsSpinBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsSpinBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsSpinBoxPlugin::toolTip() const
{
  return "";
}

QString QgsSpinBoxPlugin::whatsThis() const
{
  return "";
}

QString QgsSpinBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsSpinBox\">\n"
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
