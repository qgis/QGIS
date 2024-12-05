/***************************************************************************
   qgspasswordlineeditplugin.cpp
    --------------------------------------
   Date                 : March 13, 2017
   Copyright            : (C) 2017 Alexander Bruy
   Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgspasswordlineedit.h"
#include "qgspasswordlineeditplugin.h"
#include "moc_qgspasswordlineeditplugin.cpp"


QgsPasswordLineEditPlugin::QgsPasswordLineEditPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsPasswordLineEditPlugin::name() const
{
  return "QgsPasswordLineEdit";
}

QString QgsPasswordLineEditPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsPasswordLineEditPlugin::includeFile() const
{
  return "qgspasswordlineedit.h";
}

QIcon QgsPasswordLineEditPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsPasswordLineEditPlugin::isContainer() const
{
  return false;
}

QWidget *QgsPasswordLineEditPlugin::createWidget( QWidget *parent )
{
  return new QgsPasswordLineEdit( parent );
}

bool QgsPasswordLineEditPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsPasswordLineEditPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsPasswordLineEditPlugin::toolTip() const
{
  return "";
}

QString QgsPasswordLineEditPlugin::whatsThis() const
{
  return "";
}

QString QgsPasswordLineEditPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mLineEdit\">\n"
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
