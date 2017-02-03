/***************************************************************************
   %CLASSLOWERCASE%plugin.cpp
    --------------------------------------
   Date                 : %DATE%
   Copyright            : (C) %YEAR% %AUTHOR%
   Email                : %EMAIL%
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "%CLASSLOWERCASE%plugin.h"
#include "%CLASSLOWERCASE%.h"


%CLASSMIXEDCASE%Plugin::%CLASSMIXEDCASE%Plugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}

QString %CLASSMIXEDCASE%Plugin::name() const
{
  return "%CLASSMIXEDCASE%";
}

QString %CLASSMIXEDCASE%Plugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString %CLASSMIXEDCASE%Plugin::includeFile() const
{
  return "%CLASSLOWERCASE%.h";
}

QIcon %CLASSMIXEDCASE%Plugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool %CLASSMIXEDCASE%Plugin::isContainer() const
{
  return false;
}

QWidget *%CLASSMIXEDCASE%Plugin::createWidget( QWidget *parent )
{
  return new %CLASSMIXEDCASE%( parent );
}

bool %CLASSMIXEDCASE%Plugin::isInitialized() const
{
  return mInitialized;
}

void %CLASSMIXEDCASE%Plugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString %CLASSMIXEDCASE%Plugin::toolTip() const
{
  return "";
}

QString %CLASSMIXEDCASE%Plugin::whatsThis() const
{
  return "";
}

QString %CLASSMIXEDCASE%Plugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"m%CLASSMIXEDCASE%\">\n"
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
