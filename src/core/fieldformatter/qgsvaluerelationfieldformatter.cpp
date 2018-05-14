/***************************************************************************
  qgsvaluerelationfieldformatter.cpp - QgsValueRelationFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvaluerelationfieldformatter.h"

#include "qgis.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QSettings>

QString QgsValueRelationFieldFormatter::FORM_SCOPE_FUNCTIONS_RE = QStringLiteral( "%1\\s*\\(\\s*'([^']+)'\\s*\\)" );

bool orderByKeyLessThan( const QgsValueRelationFieldFormatter::ValueRelationItem &p1, const QgsValueRelationFieldFormatter::ValueRelationItem &p2 )
{
  return qgsVariantLessThan( p1.key, p2.key );
}

bool orderByValueLessThan( const QgsValueRelationFieldFormatter::ValueRelationItem &p1, const QgsValueRelationFieldFormatter::ValueRelationItem &p2 )
{
  return qgsVariantLessThan( p1.value, p2.value );
}

QString QgsValueRelationFieldFormatter::id() const
{
  return QStringLiteral( "ValueRelation" );
}

QString QgsValueRelationFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )

  ValueRelationCache vrCache;

  if ( cache.isValid() )
  {
    vrCache = cache.value<QgsValueRelationFieldFormatter::ValueRelationCache>();
  }
  else
  {
    vrCache = QgsValueRelationFieldFormatter::createCache( config );
  }

  if ( config.value( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    QStringList keyList = valueToStringList( value );
    QStringList valueList;

    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : qgis::as_const( vrCache ) )
    {
      if ( keyList.contains( item.key.toString() ) )
      {
        valueList << item.value;
      }
    }

    return valueList.join( QStringLiteral( ", " ) ).prepend( '{' ).append( '}' );
  }
  else
  {
    if ( value.isNull() )
    {
      return QgsApplication::nullRepresentation();
    }

    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : qgis::as_const( vrCache ) )
    {
      if ( item.key == value )
      {
        return item.value;
      }
    }
  }

  return QStringLiteral( "(%1)" ).arg( value.toString() );
}

QVariant QgsValueRelationFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariant QgsValueRelationFieldFormatter::createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIndex )
  return QVariant::fromValue<ValueRelationCache>( createCache( config ) );

}

QgsValueRelationFieldFormatter::ValueRelationCache QgsValueRelationFieldFormatter::createCache( const QVariantMap &config, const QgsFeature &formFeature )
{
  ValueRelationCache cache;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( config.value( QStringLiteral( "Layer" ) ).toString() ) );

  if ( !layer )
    return cache;

  QgsFields fields = layer->fields();
  int ki = fields.indexOf( config.value( QStringLiteral( "Key" ) ).toString() );
  int vi = fields.indexOf( config.value( QStringLiteral( "Value" ) ).toString() );

  QgsFeatureRequest request;

  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << ki << vi );

  const QString expression = config.value( QStringLiteral( "FilterExpression" ) ).toString();

  // Skip the filter and build a full cache if the form scope is required and the feature
  // is not valid or the attributes required for the filter have no valid value
  if ( ! expression.isEmpty() && ( ! expressionRequiresFormScope( expression )
                                   || expressionIsUsable( expression, formFeature ) ) )
  {
    QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
    if ( formFeature.isValid( ) )
      context.appendScope( QgsExpressionContextUtils::formScope( formFeature ) );
    request.setExpressionContext( context );
    request.setFilterExpression( expression );
  }

  QgsFeatureIterator fit = layer->getFeatures( request );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    cache.append( ValueRelationItem( f.attribute( ki ), f.attribute( vi ).toString() ) );
  }

  if ( config.value( QStringLiteral( "OrderByValue" ) ).toBool() )
  {
    std::sort( cache.begin(), cache.end(), orderByValueLessThan );
  }
  else
  {
    std::sort( cache.begin(), cache.end(), orderByKeyLessThan );
  }

  return cache;
}

QStringList QgsValueRelationFieldFormatter::valueToStringList( const QVariant &value )
{
  QStringList checkList;
  if ( value.type() == QVariant::StringList )
    checkList = value.toStringList();
  else if ( value.type() == QVariant::String )
    checkList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );
  else if ( value.type() == QVariant::List )
  {
    QVariantList valuesList( value.toList( ) );
    for ( const QVariant &listItem : qgis::as_const( valuesList ) )
    {
      QString v( listItem.toString( ) );
      if ( ! v.isEmpty() )
        checkList.append( v );
    }
  }
  return checkList;
}


QSet<QString> QgsValueRelationFieldFormatter::expressionFormVariables( const QString &expression )
{
  const QStringList formVariables( QgsExpressionContextUtils::formScope()->variableNames() );
  QSet<QString> variables;

  for ( auto const &variable : formVariables )
  {
    if ( expression.contains( variable ) )
    {
      variables.insert( variable );
    }
  }
  return variables;
}

bool QgsValueRelationFieldFormatter::expressionRequiresFormScope( const QString &expression )
{
  return !( expressionFormAttributes( expression ).isEmpty() && expressionFormVariables( expression ).isEmpty() );
}

QSet<QString> QgsValueRelationFieldFormatter::expressionFormAttributes( const QString &expression )
{
  QSet<QString> attributes;
  const QStringList formFunctions( QgsExpressionContextUtils::formScope()->functionNames() );
  QRegularExpression re;
  for ( const auto &fname : formFunctions )
  {
    if ( QgsExpressionContextUtils::formScope()->function( fname )->parameters().count( ) != 0 )
    {
      re.setPattern( QgsValueRelationFieldFormatter::FORM_SCOPE_FUNCTIONS_RE.arg( fname ) );
      QRegularExpressionMatchIterator i = re.globalMatch( expression );
      while ( i.hasNext() )
      {
        QRegularExpressionMatch match = i.next();
        attributes.insert( match.captured( 1 ) );
      }
    }
  }
  return attributes;
}

bool QgsValueRelationFieldFormatter::expressionIsUsable( const QString &expression, const QgsFeature &feature )
{
  const QSet<QString> attrs = expressionFormAttributes( expression );
  for ( auto it = attrs.constBegin() ; it != attrs.constEnd(); it++ )
  {
    if ( ! feature.attribute( *it ).isValid() )
      return false;
  }
  if ( ! expressionFormVariables( expression ).isEmpty() && feature.geometry().isEmpty( ) )
    return false;
  return true;
}
