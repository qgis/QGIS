/***************************************************************************
   qgsrelationreferencewidgetplugin.cpp
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
#include "qgsrelationreferencewidgetplugin.h"
#include "qgsrelationreferencewidget.h"


QgsRelationReferenceWidgetPlugin::QgsRelationReferenceWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsRelationReferenceWidgetPlugin::name() const
{
  return "QgsRelationReferenceWidget";
}

QString QgsRelationReferenceWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsRelationReferenceWidgetPlugin::includeFile() const
{
  return "qgsrelationreferencewidget.h";
}

QIcon QgsRelationReferenceWidgetPlugin::icon() const
{
  return QIcon();
}

bool QgsRelationReferenceWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsRelationReferenceWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsRelationReferenceWidget( parent );
}

bool QgsRelationReferenceWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsRelationReferenceWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsRelationReferenceWidgetPlugin::toolTip() const
{
  return tr( "Relation reference" );
}

QString QgsRelationReferenceWidgetPlugin::whatsThis() const
{
  return "";
}

QString QgsRelationReferenceWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mRelationReference\">\n"
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
