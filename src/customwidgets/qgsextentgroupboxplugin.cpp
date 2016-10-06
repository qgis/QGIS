/***************************************************************************
   qgsextentgroupboxplugin.cpp
    --------------------------------------
   Date                 : 28.07.2015
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
#include "qgsextentgroupbox.h"
#include "qgsextentgroupboxplugin.h"


QgsExtentGroupBoxPlugin::QgsExtentGroupBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsExtentGroupBoxPlugin::name() const
{
  return "QgsExtentGroupBox";
}

QString QgsExtentGroupBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsExtentGroupBoxPlugin::includeFile() const
{
  return "qgsextentgroupbox.h";
}

QIcon QgsExtentGroupBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsExtentGroupBoxPlugin::isContainer() const
{
  return true;
}

QWidget *QgsExtentGroupBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsExtentGroupBox( parent );
}

bool QgsExtentGroupBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsExtentGroupBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsExtentGroupBoxPlugin::toolTip() const
{
  return tr( "A group box to enter a map extent" );
}

QString QgsExtentGroupBoxPlugin::whatsThis() const
{
  return tr( "A group box to enter a map extent" );
}

QString QgsExtentGroupBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mExtentGroupBox\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>300</width>\n"
                  "    <height>100</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
