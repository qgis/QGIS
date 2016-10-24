/***************************************************************************
    qgscheckboxwidgetfactory.cpp
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

#include "qgscheckboxwidgetfactory.h"
#include "qgscheckboxsearchwidgetwrapper.h"
#include "qgscheckboxwidgetwrapper.h"
#include "qgscheckboxconfigdlg.h"

QgsCheckboxWidgetFactory::QgsCheckboxWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsCheckboxWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsCheckboxWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper*QgsCheckboxWidgetFactory::createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsCheckboxSearchWidgetWrapper( vl, fieldIdx, parent );
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

  cfg.insert( QStringLiteral( "CheckedState" ), configElement.attribute( QStringLiteral( "CheckedState" ) ) );
  cfg.insert( QStringLiteral( "UncheckedState" ), configElement.attribute( QStringLiteral( "UncheckedState" ) ) );

  return cfg;
}

void QgsCheckboxWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  configElement.setAttribute( QStringLiteral( "CheckedState" ), config.value( QStringLiteral( "CheckedState" ), "1" ).toString() );
  configElement.setAttribute( QStringLiteral( "UncheckedState" ), config.value( QStringLiteral( "UncheckedState" ), "0" ).toString() );
}

QHash<const char*, int> QgsCheckboxWidgetFactory::supportedWidgetTypes()
{
  QHash<const char*, int> map = QHash<const char*, int>();
  map.insert( QCheckBox::staticMetaObject.className(), 10 );
  map.insert( QGroupBox::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsCheckboxWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  const QVariant::Type type = vl->fields().field( fieldIdx ).type();
  return type == QVariant::Bool ? 20 : 5;
}
