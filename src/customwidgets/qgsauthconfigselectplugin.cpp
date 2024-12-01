/***************************************************************************
   qgsauthconfigselectplugin.cpp
    --------------------------------------
   Date                 : October 2019
   Copyright            : (C) 2019 Denis Rouzaud
   Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgsauthconfigselect.h"
#include "qgsauthconfigselectplugin.h"
#include "moc_qgsauthconfigselectplugin.cpp"


QgsAuthConfigSelectPlugin::QgsAuthConfigSelectPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsAuthConfigSelectPlugin::name() const
{
  return "QgsAuthConfigSelect";
}

QString QgsAuthConfigSelectPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsAuthConfigSelectPlugin::includeFile() const
{
  return "qgsauthconfigselect.h";
}

QIcon QgsAuthConfigSelectPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsAuthConfigSelectPlugin::isContainer() const
{
  return false;
}

QWidget *QgsAuthConfigSelectPlugin::createWidget( QWidget *parent )
{
  return new QgsAuthConfigSelect( parent );
}

bool QgsAuthConfigSelectPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsAuthConfigSelectPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsAuthConfigSelectPlugin::toolTip() const
{
  return "";
}

QString QgsAuthConfigSelectPlugin::whatsThis() const
{
  return "";
}

QString QgsAuthConfigSelectPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mAuthConfigSelect\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>160</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
    .arg( name() );
}
