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
#include "qgsexpressionnodeimpl.h"

#include <QSettings>

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
    QStringList keyList;

    if ( layer->fields().at( fieldIndex ).type() == QVariant::Map )
    {
      //because of json it's stored as QVariantList
      keyList = value.toStringList();
    }
    else
    {
      keyList = valueToStringList( value );
    }

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

  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( config.value( QStringLiteral( "Layer" ) ).toString() );

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
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::formScope() );
  QSet< QString > formVariables = scope->variableNames().toSet();
  const QSet< QString > usedVariables = QgsExpression( expression ).referencedVariables();
  formVariables.intersect( usedVariables );
  return formVariables;
}

bool QgsValueRelationFieldFormatter::expressionRequiresFormScope( const QString &expression )
{
  return !( expressionFormAttributes( expression ).isEmpty() && expressionFormVariables( expression ).isEmpty() );
}

QSet<QString> QgsValueRelationFieldFormatter::expressionFormAttributes( const QString &expression )
{
  QSet<QString> attributes;
  QgsExpression exp( expression );
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::formScope() );
  // List of form function names used in the expression
  const QSet<QString> formFunctions( scope->functionNames()
                                     .toSet()
                                     .intersect( exp.referencedFunctions( ) ) );
  const QList<const QgsExpressionNodeFunction *> expFunctions( exp.findNodes<QgsExpressionNodeFunction>() );
  QgsExpressionContext context;
  for ( const auto &f : expFunctions )
  {
    QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[f->fnIndex()];
    if ( formFunctions.contains( fd->name( ) ) )
    {
      for ( const auto &param : f->args( )->list() )
      {
        attributes.insert( param->eval( &exp, &context ).toString() );
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
