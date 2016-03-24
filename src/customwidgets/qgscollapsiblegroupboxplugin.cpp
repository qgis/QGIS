/***************************************************************************
   qgscollapsiblegroupboxplugin.cpp
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
#include "qgscollapsiblegroupbox.h"
#include "qgscollapsiblegroupboxplugin.h"


QgsCollapsibleGroupBoxPlugin::QgsCollapsibleGroupBoxPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsCollapsibleGroupBoxPlugin::name() const
{
  return "QgsCollapsibleGroupBox";
}

QString QgsCollapsibleGroupBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsCollapsibleGroupBoxPlugin::includeFile() const
{
  return "qgscollapsiblegroupbox.h";
}

QIcon QgsCollapsibleGroupBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsCollapsibleGroupBoxPlugin::isContainer() const
{
  return true;
}

QWidget *QgsCollapsibleGroupBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsCollapsibleGroupBox( parent );
}

bool QgsCollapsibleGroupBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsCollapsibleGroupBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsCollapsibleGroupBoxPlugin::toolTip() const
{
  return tr( "A collapsible group box" );
}

QString QgsCollapsibleGroupBoxPlugin::whatsThis() const
{
  return tr( "A collapsible group box with save state capability" );
}

QString QgsCollapsibleGroupBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mGroupBox\">\n"
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
