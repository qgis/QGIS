/***************************************************************************
    qgsrangewidgetfactory.cpp
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

#include "qgsrangewidgetfactory.h"
#include "qgsrangeconfigdlg.h"
#include "qgsrangewidgetwrapper.h"
#include "qgsvectorlayer.h"
#include <QDial>

QgsRangeWidgetFactory::QgsRangeWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsRangeWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsRangeWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsRangeWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsRangeConfigDlg( vl, fieldIdx, parent );
}

QVariantMap QgsRangeWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QVariantMap cfg;

  xml2config( configElement, cfg, QStringLiteral( "Style" ) );
  xml2config( configElement, cfg, QStringLiteral( "Min" ) );
  xml2config( configElement, cfg, QStringLiteral( "Max" ) );
  xml2config( configElement, cfg, QStringLiteral( "Step" ) );
  xml2config( configElement, cfg, QStringLiteral( "AllowNull" ) );
  xml2config( configElement, cfg, QStringLiteral( "Suffix" ) );

  return cfg;
}

void QgsRangeWidgetFactory::writeConfig( const QVariantMap& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  config2xml( config, configElement, QStringLiteral( "Style" ) );
  config2xml( config, configElement, QStringLiteral( "Min" ) );
  config2xml( config, configElement, QStringLiteral( "Max" ) );
  config2xml( config, configElement, QStringLiteral( "Step" ) );
  config2xml( config, configElement, QStringLiteral( "AllowNull" ) );
  config2xml( config, configElement, QStringLiteral( "Suffix" ) );
}

unsigned int QgsRangeWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().at( fieldIdx );
  if ( field.type() == QVariant::Int || field.type() == QVariant::Double ) return 20;
  if ( field.isNumeric() ) return 5; // widgets used support only signed 32bits (int) and double
  return 0;
}

QHash<const char*, int> QgsRangeWidgetFactory::supportedWidgetTypes()
{
  QHash<const char*, int> map = QHash<const char*, int>();
  map.insert( QSlider::staticMetaObject.className(), 10 );
  map.insert( QDial::staticMetaObject.className(), 10 );
  map.insert( QSpinBox::staticMetaObject.className(), 10 );
  map.insert( QDoubleSpinBox::staticMetaObject.className(), 10 );
  return map;
}
