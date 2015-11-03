/***************************************************************************
   qgscolorbuttonv2plugin.cpp
    --------------------------------------
   Date                 : 18.08.2014
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
#include "qgscolorbuttonv2plugin.h"
#include "qgscolorbuttonv2.h"


QgsColorButtonV2Plugin::QgsColorButtonV2Plugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsColorButtonV2Plugin::name() const
{
  return "QgsColorButtonV2";
}

QString QgsColorButtonV2Plugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsColorButtonV2Plugin::includeFile() const
{
  return "qgscolorbuttonv2.h";
}

QIcon QgsColorButtonV2Plugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsColorButtonV2Plugin::isContainer() const
{
  return false;
}

QWidget *QgsColorButtonV2Plugin::createWidget( QWidget *parent )
{
  return new QgsColorButtonV2( parent );
}

bool QgsColorButtonV2Plugin::isInitialized() const
{
  return mInitialized;
}

void QgsColorButtonV2Plugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsColorButtonV2Plugin::toolTip() const
{
  return tr( "Select color" );
}

QString QgsColorButtonV2Plugin::whatsThis() const
{
  return "";
}

QString QgsColorButtonV2Plugin::domXml() const
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
