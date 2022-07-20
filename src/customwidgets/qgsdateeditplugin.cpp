/***************************************************************************
   qgsdateeditplugin.cpp
    --------------------------------------
   Date                 : 20.07.2022
   Copyright            : (C) 2022 Andrea Giudiceandrea
   Email                : andreaerdna@libero.it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgsdateeditplugin.h"
#include "qgsdateedit.h"


QgsDateEditPlugin::QgsDateEditPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsDateEditPlugin::name() const
{
  return "QgsDateEdit";
}

QString QgsDateEditPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsDateEditPlugin::includeFile() const
{
  return "qgsdateedit.h";
}

QIcon QgsDateEditPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsDateEditPlugin::isContainer() const
{
  return false;
}

QWidget *QgsDateEditPlugin::createWidget( QWidget *parent )
{
  return new QgsDateEdit( parent );
}

bool QgsDateEditPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsDateEditPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsDateEditPlugin::toolTip() const
{
  return tr( "Define date" );
}

QString QgsDateEditPlugin::whatsThis() const
{
  return "";
}

QString QgsDateEditPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mDateEdit\">\n"
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
