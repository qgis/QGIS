/***************************************************************************
   qgsdoublespinboxplugin.cpp
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
#include "qgsdoublespinboxplugin.h"
#include "qgsdoublespinbox.h"


QgsDoubleSpinBoxPlugin::QgsDoubleSpinBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsDoubleSpinBoxPlugin::name() const
{
  return "QgsDoubleSpinBox";
}

QString QgsDoubleSpinBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsDoubleSpinBoxPlugin::includeFile() const
{
  return "qgsdoublespinbox.h";
}

QIcon QgsDoubleSpinBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsDoubleSpinBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsDoubleSpinBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsDoubleSpinBox( parent );
}

bool QgsDoubleSpinBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsDoubleSpinBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsDoubleSpinBoxPlugin::toolTip() const
{
  return "";
}

QString QgsDoubleSpinBoxPlugin::whatsThis() const
{
  return "";
}

QString QgsDoubleSpinBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mQgsDoubleSpinBox\">\n"
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
