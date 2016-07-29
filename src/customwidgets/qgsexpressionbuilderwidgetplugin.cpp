/***************************************************************************
   qgsexpressionbuilderwidgetplugin.cpp
    --------------------------------------
   Date                 : 03.11.2015
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
#include "qgsexpressionbuilderwidget.h"
#include "qgsexpressionbuilderwidgetplugin.h"


QgsExpressionBuilderWidgetPlugin::QgsExpressionBuilderWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsExpressionBuilderWidgetPlugin::name() const
{
  return "QgsExpressionBuilderWidget";
}

QString QgsExpressionBuilderWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsExpressionBuilderWidgetPlugin::includeFile() const
{
  return "qgsexpressionbuilderwidget.h";
}

QIcon QgsExpressionBuilderWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsExpressionBuilderWidgetPlugin::isContainer() const
{
  return true;
}

QWidget *QgsExpressionBuilderWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsExpressionBuilderWidget( parent );
}

bool QgsExpressionBuilderWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsExpressionBuilderWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsExpressionBuilderWidgetPlugin::toolTip() const
{
  return tr( "Edit expression" );
}

QString QgsExpressionBuilderWidgetPlugin::whatsThis() const
{
  return tr( "Edit expression" );
}

QString QgsExpressionBuilderWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mExpressionBuilderWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>700</width>\n"
                  "    <height>400</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
