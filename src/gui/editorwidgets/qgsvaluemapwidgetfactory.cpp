/***************************************************************************
    qgsvaluemapwidgetfactory.cpp
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

#include "qgsvaluemapwidgetfactory.h"

#include "qgsvaluemapwidgetwrapper.h"
#include "qgsvaluemapsearchwidgetwrapper.h"
#include "qgsdefaultsearchwidgetwrapper.h"
#include "qgsvaluemapconfigdlg.h"

QgsValueMapWidgetFactory::QgsValueMapWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper* QgsValueMapWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsValueMapWidgetWrapper( vl, fieldIdx, editor, parent );
}


QgsSearchWidgetWrapper* QgsValueMapWidgetFactory::createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsValueMapSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget* QgsValueMapWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsValueMapConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsValueMapWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  QDomNodeList nodes = configElement.elementsByTagName( "value" );

  for ( unsigned int i = 0; i < nodes.length(); ++i )
  {
    QDomElement elem = nodes.at( i ).toElement();
    cfg.insert( elem.attribute( "key" ), elem.attribute( "value" ) );
  }

  return cfg;
}

void QgsValueMapWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig::ConstIterator it = config.constBegin();

  while ( it != config.constEnd() )
  {
    QDomElement elem = doc.createElement( "value" );

    elem.setAttribute( "key", it.key() );
    elem.setAttribute( "value", it.value().toString() );

    configElement.appendChild( elem );

    ++it;
  }
}

QString QgsValueMapWidgetFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( cache )

  return config.key( value, QVariant( QString( "(%1)" ).arg( value.toString() ) ).toString() );
}

QMap<const char*, int> QgsValueMapWidgetFactory::supportedWidgetTypes()
{
  QMap<const char*, int> map = QMap<const char*, int>();
  map.insert( QComboBox::staticMetaObject.className(), 10 );
  return map;
}
