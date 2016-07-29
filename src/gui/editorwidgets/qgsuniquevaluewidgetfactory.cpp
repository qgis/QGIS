/***************************************************************************
    qgsuniquevaluewidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsuniquevaluewidgetfactory.h"

#include "qgsuniquevaluewidgetwrapper.h"
#include "qgsuniquevaluesconfigdlg.h"

QgsUniqueValueWidgetFactory::QgsUniqueValueWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper* QgsUniqueValueWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsUniqueValuesWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsUniqueValueWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsUniqueValuesConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsUniqueValueWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  cfg.insert( "Editable", configElement.attribute( "Editable", "0" ) == "1" );

  return cfg;
}

void QgsUniqueValueWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )
  configElement.setAttribute( "Editable", config.value( "Editable", false ).toBool() );
}
