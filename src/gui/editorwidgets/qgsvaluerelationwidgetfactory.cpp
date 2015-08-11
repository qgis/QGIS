/***************************************************************************
    qgsvaluerelationwidgetfactory.cpp
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

#include "qgsvaluerelationwidgetfactory.h"

#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsvaluerelationconfigdlg.h"
#include "qgsvaluerelationsearchwidgetwrapper.h"
#include "qgsvectorlayer.h"

#include <QSettings>

QgsValueRelationWidgetFactory::QgsValueRelationWidgetFactory( const QString& name )
    :  QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsValueRelationWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsValueRelationWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsValueRelationWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsValueRelationSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget* QgsValueRelationWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsValueRelationConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsValueRelationWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  cfg.insert( "Layer", configElement.attribute( "Layer" ) );
  cfg.insert( "Key", configElement.attribute( "Key" ) );
  cfg.insert( "Value", configElement.attribute( "Value" ) );
  cfg.insert( "FilterExpression", configElement.attribute( "FilterExpression" ) );
  cfg.insert( "OrderByValue", configElement.attribute( "OrderByValue" ) );
  cfg.insert( "AllowMulti", configElement.attribute( "AllowMulti" ) );
  cfg.insert( "AllowNull", configElement.attribute( "AllowNull" ) );
  cfg.insert( "UseCompleter", configElement.attribute( "UseCompleter" ) );

  return cfg;
}

void QgsValueRelationWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  configElement.setAttribute( "Layer", config.value( "Layer" ).toString() );
  configElement.setAttribute( "Key", config.value( "Key" ).toString() );
  configElement.setAttribute( "Value", config.value( "Value" ).toString() );
  configElement.setAttribute( "FilterExpression", config.value( "FilterExpression" ).toString() );
  configElement.setAttribute( "OrderByValue", config.value( "OrderByValue" ).toBool() );
  configElement.setAttribute( "AllowMulti", config.value( "AllowMulti" ).toBool() );
  configElement.setAttribute( "AllowNull", config.value( "AllowNull" ).toBool() );
  configElement.setAttribute( "UseCompleter", config.value( "UseCompleter" ).toBool() );
}

QString QgsValueRelationWidgetFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )

  QgsValueRelationWidgetWrapper::ValueRelationCache vrCache;

  if ( cache.isValid() )
  {
    vrCache = cache.value<QgsValueRelationWidgetWrapper::ValueRelationCache>();
  }
  else
  {
    vrCache = QgsValueRelationWidgetWrapper::createCache( config );
  }

  if ( config.value( "AllowMulti" ).toBool() )
  {
    QStringList keyList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( "," );
    QStringList valueList;

    Q_FOREACH ( const QgsValueRelationWidgetWrapper::ValueRelationItem& item, vrCache )
    {
      if ( keyList.contains( item.first.toString() ) )
      {
        valueList << item.second;
      }
    }

    return valueList.join( ", " ).prepend( '{' ).append( '}' );
  }
  else
  {
    if ( value.isNull() )
    {
      QSettings settings;
      return settings.value( "qgis/nullValue", "NULL" ).toString();
    }

    Q_FOREACH ( const QgsValueRelationWidgetWrapper::ValueRelationItem& item, vrCache )
    {
      if ( item.first == value )
      {
        return item.second;
      }
    }
  }

  return QString( "(%1)" ).arg( value.toString() );
}

QVariant QgsValueRelationWidgetFactory::createCache( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config )
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )

  return QVariant::fromValue<QgsValueRelationWidgetWrapper::ValueRelationCache>( QgsValueRelationWidgetWrapper::createCache( config ) );
}

