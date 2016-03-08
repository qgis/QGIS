/***************************************************************************
   qgscolorbuttonplugin.cpp
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
#include "qgscolorbuttonplugin.h"
#include "qgscolorbutton.h"


QgsColorButtonPlugin::QgsColorButtonPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsColorButtonPlugin::name() const
{
  return "QgsColorButton";
}

QString QgsColorButtonPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsColorButtonPlugin::includeFile() const
{
  return "qgscolorbutton.h";
}

QIcon QgsColorButtonPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsColorButtonPlugin::isContainer() const
{
  return false;
}

QWidget *QgsColorButtonPlugin::createWidget( QWidget *parent )
{
  return new QgsColorButton( parent );
}

bool QgsColorButtonPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsColorButtonPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsColorButtonPlugin::toolTip() const
{
  return tr( "Select color" );
}

QString QgsColorButtonPlugin::whatsThis() const
{
  return "";
}

QString QgsColorButtonPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mColorButton\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>27</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
