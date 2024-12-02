/***************************************************************************
   qgsrasterbandcomboboxplugin.cpp
    --------------------------------------
   Date                 : 09.05.2017
   Copyright            : (C) 2017 Nyall Dawson
   Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgiscustomwidgets.h"
#include "qgsrasterbandcombobox.h"
#include "qgsrasterbandcomboboxplugin.h"
#include "moc_qgsrasterbandcomboboxplugin.cpp"


QgsRasterBandComboBoxPlugin::QgsRasterBandComboBoxPlugin( QObject *parent )
  : QObject( parent )
  , mInitialized( false )
{
}


QString QgsRasterBandComboBoxPlugin::name() const
{
  return "QgsRasterBandComboBox";
}

QString QgsRasterBandComboBoxPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsRasterBandComboBoxPlugin::includeFile() const
{
  return "qgsrasterbandcombobox.h";
}

QIcon QgsRasterBandComboBoxPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsRasterBandComboBoxPlugin::isContainer() const
{
  return false;
}

QWidget *QgsRasterBandComboBoxPlugin::createWidget( QWidget *parent )
{
  return new QgsRasterBandComboBox( parent );
}

bool QgsRasterBandComboBoxPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsRasterBandComboBoxPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core )
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsRasterBandComboBoxPlugin::toolTip() const
{
  return tr( "A combo box to list the bands from a raster layer" );
}

QString QgsRasterBandComboBoxPlugin::whatsThis() const
{
  return tr( "A combo box to list the bands from a raster layer." );
}

QString QgsRasterBandComboBoxPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mRasterBandComboBox\">\n"
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
