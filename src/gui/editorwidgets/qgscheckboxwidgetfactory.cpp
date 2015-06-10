/***************************************************************************
    qgscheckboxwidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscheckboxwidgetfactory.h"

#include "qgscheckboxwidgetwrapper.h"
#include "qgscheckboxconfigdlg.h"

QgsCheckboxWidgetFactory::QgsCheckboxWidgetFactory( const QString& name ) :
    QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsCheckboxWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsCheckboxWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsCheckboxWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsCheckBoxConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsCheckboxWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  cfg.insert( "CheckedState", configElement.attribute( "CheckedState" ) );
  cfg.insert( "UncheckedState", configElement.attribute( "UncheckedState" ) );

  return cfg;
}

void QgsCheckboxWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  configElement.setAttribute( "CheckedState", config.value( "CheckedState", "1" ).toString() );
  configElement.setAttribute( "UncheckedState", config.value( "UncheckedState", "0" ).toString() );
}
