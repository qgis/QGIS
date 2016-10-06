/***************************************************************************
   qgsfieldexpressionwidgetplugin.cpp
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
#include "qgsfieldexpressionwidgetplugin.h"
#include "qgsfieldexpressionwidget.h"


QgsFieldExpressionWidgetPlugin::QgsFieldExpressionWidgetPlugin( QObject *parent )
    : QObject( parent )
    , mInitialized( false )
{
}


QString QgsFieldExpressionWidgetPlugin::name() const
{
  return "QgsFieldExpressionWidget";
}

QString QgsFieldExpressionWidgetPlugin::group() const
{
  return QgisCustomWidgets::groupName();
}

QString QgsFieldExpressionWidgetPlugin::includeFile() const
{
  return "qgsfieldexpressionwidget.h";
}

QIcon QgsFieldExpressionWidgetPlugin::icon() const
{
  return QIcon( ":/images/icons/qgis-icon-60x60.png" );
}

bool QgsFieldExpressionWidgetPlugin::isContainer() const
{
  return false;
}

QWidget *QgsFieldExpressionWidgetPlugin::createWidget( QWidget *parent )
{
  return new QgsFieldExpressionWidget( parent );
}

bool QgsFieldExpressionWidgetPlugin::isInitialized() const
{
  return mInitialized;
}

void QgsFieldExpressionWidgetPlugin::initialize( QDesignerFormEditorInterface *core )
{
  Q_UNUSED( core );
  if ( mInitialized )
    return;
  mInitialized = true;
}


QString QgsFieldExpressionWidgetPlugin::toolTip() const
{
  return tr( "An editable combo box to enter an expression" );
}

QString QgsFieldExpressionWidgetPlugin::whatsThis() const
{
  return tr( "An editable combo box to enter an expression. A button allows opening the expression dialog. Expression are evaluated to detect errors." );
}

QString QgsFieldExpressionWidgetPlugin::domXml() const
{
  return QString( "<ui language=\"c++\">\n"
                  " <widget class=\"%1\" name=\"mFieldExpressionWidget\">\n"
                  "  <property name=\"geometry\">\n"
                  "   <rect>\n"
                  "    <x>0</x>\n"
                  "    <y>0</y>\n"
                  "    <width>200</width>\n"
                  "    <height>27</height>\n"
                  "   </rect>\n"
                  "  </property>\n"
                  " </widget>\n"
                  "</ui>\n" )
         .arg( name() );
}
