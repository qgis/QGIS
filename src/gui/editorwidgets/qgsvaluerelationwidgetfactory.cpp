/***************************************************************************
    qgsvaluerelationwidgetfactory.cpp
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

#include "qgsvaluerelationwidgetfactory.h"

#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsvaluerelationconfigdlg.h"
#include "qgsvaluerelationsearchwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvaluerelationwidgetwrapper.h"

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

  xml2config( configElement, cfg, QStringLiteral( "Layer" ) );
  xml2config( configElement, cfg, QStringLiteral( "Key" ) );
  xml2config( configElement, cfg, QStringLiteral( "Value" ) );
  xml2config( configElement, cfg, QStringLiteral( "FilterExpression" ) );
  xml2config( configElement, cfg, QStringLiteral( "OrderByValue" ) );
  xml2config( configElement, cfg, QStringLiteral( "AllowMulti" ) );
  xml2config( configElement, cfg, QStringLiteral( "AllowNull" ) );
  xml2config( configElement, cfg, QStringLiteral( "UseCompleter" ) );

  return cfg;
}

void QgsValueRelationWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  config2xml( config, configElement, QStringLiteral( "Layer" ) );
  config2xml( config, configElement, QStringLiteral( "Key" ) );
  config2xml( config, configElement, QStringLiteral( "Value" ) );
  config2xml( config, configElement, QStringLiteral( "FilterExpression" ) );
  config2xml( config, configElement, QStringLiteral( "OrderByValue" ) );
  config2xml( config, configElement, QStringLiteral( "AllowMulti" ) );
  config2xml( config, configElement, QStringLiteral( "AllowNull" ) );
  config2xml( config, configElement, QStringLiteral( "UseCompleter" ) );
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

  if ( config.value( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    QStringList keyList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );
    QStringList valueList;

    Q_FOREACH ( const QgsValueRelationWidgetWrapper::ValueRelationItem& item, vrCache )
    {
      if ( keyList.contains( item.first.toString() ) )
      {
        valueList << item.second;
      }
    }

    return valueList.join( QStringLiteral( ", " ) ).prepend( '{' ).append( '}' );
  }
  else
  {
    if ( value.isNull() )
    {
      QSettings settings;
      return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
    }

    Q_FOREACH ( const QgsValueRelationWidgetWrapper::ValueRelationItem& item, vrCache )
    {
      if ( item.first == value )
      {
        return item.second;
      }
    }
  }

  return QStringLiteral( "(%1)" ).arg( value.toString() );
}

QVariant QgsValueRelationWidgetFactory::sortValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( vl, fieldIdx, config, cache, value );
}

Qt::AlignmentFlag QgsValueRelationWidgetFactory::alignmentFlag( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );

  return Qt::AlignLeft;
}

QVariant QgsValueRelationWidgetFactory::createCache( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config )
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )

  return QVariant::fromValue<QgsValueRelationWidgetWrapper::ValueRelationCache>( QgsValueRelationWidgetWrapper::createCache( config ) );
}

