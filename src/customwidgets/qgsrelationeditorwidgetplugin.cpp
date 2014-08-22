/***************************************************************************
   qgsrelationeditorwidgetplugin.cpp
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
#include "qgsrelationeditorwidget.h"
#include "qgsrelationeditorwidgetplugin.h"


QgsRelationEditorWidgetPlugin::QgsRelationEditorWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsRelationEditorWidgetPlugin::name() const
{
  return "QgsRelationEditorWidget";
}

QString QgsRelationEditorWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsRelationEditorWidgetPlugin::includeFile() const
{
  return "qgsrelationeditorwidget.h";
}

QIcon QgsRelationEditorWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsRelationEditorWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsRelationEditorWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsRelationEditorWidget( parent );
}

bool QgsRelationEditorWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsRelationEditorWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsRelationEditorWidgetPlugin::toolTip() const
{
  return tr( "Relation editor" );
}

QString QgsRelationEditorWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsRelationEditorWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mRelationEditor\">\n"
                  "  <property name=\"qgisRelation\" >\n"
                  "   <string notr=\"true\"></string>\n"
                  "  </property>\n"
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
