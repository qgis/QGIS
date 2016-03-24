/***************************************************************************
   qgsfilterlineeditplugin.cpp
    --------------------------------------
   Date                 : 20.08.2014
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
#include "qgsfilterlineedit.h"
#include "qgsfilterlineeditplugin.h"


QgsFilterLineEditPlugin::QgsFilterLineEditPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsFilterLineEditPlugin::name() const
{
  return "QgsFilterLineEdit";
}

QString QgsFilterLineEditPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFilterLineEditPlugin::includeFile() const
{
  return "qgsfilterlineedit.h";
}

QIcon QgsFilterLineEditPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFilterLineEditPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFilterLineEditPlugin::createWidget( QWidget *parent )
{
  return new QgsFilterLineEdit( parent );
}

bool QgsFilterLineEditPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFilterLineEditPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFilterLineEditPlugin::toolTip() const
{
  return "";
}

QString QgsFilterLineEditPlugin::whatsThis() const
{
  return "";
}

QString QgsFilterLineEditPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mLineEdit\">\n"
                  "  <property name=\"qgisRelation\" >\n"
                  "   <string notr=\"true\"></string>\n"
                  "  </property>\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>60</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
