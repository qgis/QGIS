/***************************************************************************
    qgsdatadefined.cpp - Data defined container class
     --------------------------------------
    Date                 : 9-May-2013
    Copyright            : (C) 2013 by Larry Shaffer
    Email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefined.h"
#include "qgsdatadefined_p.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"

QgsDataDefined::QgsDataDefined( bool active,
                                bool useexpr,
                                const QString& expr,
                                const QString& field )
{
  d = new QgsDataDefinedPrivate( active, useexpr, expr, field );
}

QgsDataDefined::QgsDataDefined( const QgsExpression * expression )
{
  bool active = bool( expression );
  bool useExpression = expression && ! expression->isField();
  d = new QgsDataDefinedPrivate( active,
                                 useExpression,
                                 useExpression ? expression->expression() : QString(),
                                 !useExpression ? ( expression ? expression->expression() : QString() ) : QString() );
}

QgsDataDefined::QgsDataDefined( const QgsDataDefined &other )
    : d( other.d )
{

}

QgsDataDefined* QgsDataDefined::fromMap( const QgsStringMap &map, const QString &baseName )
{
  QString prefix;
  if ( !baseName.isEmpty() )
  {
    prefix.append( QString( "%1_dd_" ).arg( baseName ) );
  }

  if ( !map.contains( QString( "%1expression" ).arg( prefix ) ) )
  {
    //requires at least the expression value
    return 0;
  }

  bool active = ( map.value( QString( "%1active" ).arg( prefix ), "1" ) != QString( "0" ) );
  QString expression = map.value( QString( "%1expression" ).arg( prefix ) );
  bool useExpression = ( map.value( QString( "%1useexpr" ).arg( prefix ), "1" ) != QString( "0" ) );
  QString field = map.value( QString( "%1field" ).arg( prefix ), QString() );

  return new QgsDataDefined( active, useExpression, expression, field );
}

QgsDataDefined::QgsDataDefined( const QString & string )
{
  QgsExpression expression( string );

  bool active = expression.rootNode();
  bool useExpression = active && ! expression.isField();
  d = new QgsDataDefinedPrivate( active,
                                 useExpression,
                                 useExpression ? expression.expression() : QString(),
                                 expression.isField() ? expression.rootNode()->dump() : QString() );
}

QgsDataDefined::~QgsDataDefined()
{

}

bool QgsDataDefined::hasDefaultValues() const
{
  return ( !d->active && !d->useExpression && d->expressionString.isEmpty() && d->field.isEmpty() );
}

bool QgsDataDefined::isActive() const
{
  return d->active;
}

void QgsDataDefined::setActive( bool active )
{
  if ( active == d->active )
    return;

  d.detach();
  d->active = active;
}

bool QgsDataDefined::useExpression() const
{
  return d->useExpression;
}

void QgsDataDefined::setUseExpression( bool use )
{
  if ( use == d->useExpression )
    return;

  d.detach();
  d->useExpression = use;
  d->expressionPrepared = false;
  d->exprRefColumns.clear();
}

QString QgsDataDefined::expressionString() const
{
  return d->expressionString;
}

void QgsDataDefined::setExpressionString( const QString &expr )
{
  if ( expr == d->expressionString )
    return;

  d.detach();

  d->useExpression = true;
  d->expressionString = expr;
  d->expressionPrepared = false;
  d->exprRefColumns.clear();
}

QString QgsDataDefined::expressionOrField() const
{
  return d->useExpression ? d->expressionString : QString( "\"%1\"" ).arg( d->field );
}

QMap<QString, QVariant> QgsDataDefined::expressionParams() const
{
  return d->expressionParams;
}

void QgsDataDefined::setExpressionParams( QMap<QString, QVariant> params )
{
  d.detach();
  d->expressionParams = params;
}

bool QgsDataDefined::prepareExpression( QgsVectorLayer* layer )
{
  if ( layer )
  {
    return prepareExpression( QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), layer->fields() ) );
  }
  else
  {
    //preparing expression without a layer set, so pass empty context
    QgsExpressionContext empty;
    return prepareExpression( empty );
  }
}

bool QgsDataDefined::prepareExpression( const QgsFields &fields )
{
  return prepareExpression( QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), fields ) );
}

bool QgsDataDefined::prepareExpression( const QgsExpressionContext& context )
{
  if ( !d->useExpression || d->expressionString.isEmpty() )
  {
    return false;
  }

  d.detach();
  delete d->expression;
  d->expression = new QgsExpression( d->expressionString );
  if ( d->expression->hasParserError() )
  {
    QgsDebugMsg( "Parser error:" + d->expression->parserErrorString() );
    return false;
  }

  // setup expression parameters
  QVariant scaleV = d->expressionParams.value( "scale" );
  if ( scaleV.isValid() )
  {
    bool ok;
    double scale = scaleV.toDouble( &ok );
    if ( ok )
    {
      d->expression->setScale( scale );
    }
  }

  d->expression->prepare( &context );
  d->exprRefColumns = d->expression->referencedColumns();

  if ( d->expression->hasEvalError() )
  {
    d->expressionPrepared = false;
    QgsDebugMsg( "Prepare error:" + d->expression->evalErrorString() );
    return false;
  }

  d->expressionPrepared = true;

  return true;
}

bool QgsDataDefined::expressionIsPrepared() const
{
  return d->expressionPrepared;
}

QgsExpression *QgsDataDefined::expression()
{
  //Ideally there should be a detach here, but that causes issues
  //as detaching can create a new expression which will be unprepared
  //TODO - revisit after QgsExpression is made implicitly shared
  //d.detach();
  return d->expression;
}

QStringList QgsDataDefined::referencedColumns( QgsVectorLayer* layer )
{
  if ( layer )
  {
    return referencedColumns( QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), layer->fields() ) );
  }
  else
  {
    QgsExpressionContext empty;
    return referencedColumns( empty );
  }
}

QStringList QgsDataDefined::referencedColumns( const QgsFields &fields )
{
  return referencedColumns( QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), fields ) );
}

QStringList QgsDataDefined::referencedColumns( const QgsExpressionContext& context )
{
  if ( !d->exprRefColumns.isEmpty() )
  {
    return d->exprRefColumns;
  }

  d.detach();
  if ( d->useExpression )
  {
    if ( !d->expression || !d->expressionPrepared )
    {
      prepareExpression( context );
    }
  }
  else if ( !d->field.isEmpty() )
  {
    d->exprRefColumns << d->field;
  }

  return d->exprRefColumns;
}

QString QgsDataDefined::field() const
{
  return d->field;
}

void QgsDataDefined::setField( const QString &field )
{
  if ( field == d->field )
    return;

  d.detach();
  d->useExpression = false;
  d->field = field;
  d->exprRefColumns.clear();
}

void QgsDataDefined::insertExpressionParam( QString key, QVariant param )
{
  d.detach();
  d->expressionParams.insert( key, param );
}

QgsStringMap QgsDataDefined::toMap( const QString &baseName ) const
{
  QgsStringMap map;
  QString prefix;
  if ( !baseName.isEmpty() )
  {
    prefix.append( QString( "%1_dd_" ).arg( baseName ) );
  }

  map.insert( QString( "%1active" ).arg( prefix ), ( d->active ? "1" : "0" ) );
  map.insert( QString( "%1useexpr" ).arg( prefix ), ( d->useExpression ? "1" : "0" ) );
  map.insert( QString( "%1expression" ).arg( prefix ), d->expressionString );
  map.insert( QString( "%1field" ).arg( prefix ), d->field );

  return map;
}

QDomElement QgsDataDefined::toXmlElement( QDomDocument &document, const QString& elementName ) const
{
  QDomElement element = document.createElement( elementName );
  element.setAttribute( "active", d->active ? "true" : "false" );
  element.setAttribute( "useExpr", d->useExpression ? "true" : "false" );
  element.setAttribute( "expr", d->expressionString );
  element.setAttribute( "field", d->field );
  return element;
}

bool QgsDataDefined::setFromXmlElement( const QDomElement &element )
{
  if ( element.isNull() )
  {
    return false;
  }

  d.detach();
  d->active = element.attribute( "active" ).compare( "true", Qt::CaseInsensitive ) == 0;
  d->useExpression = element.attribute( "useExpr" ).compare( "true", Qt::CaseInsensitive ) == 0;
  d->field = element.attribute( "field" );
  setExpressionString( element.attribute( "expr" ) );
  return true;
}

bool QgsDataDefined::operator==( const QgsDataDefined &other ) const
{
  return *( other.d ) == *d;
}

bool QgsDataDefined::operator!=( const QgsDataDefined &other ) const
{
  return !( *this == other );
}

QgsDataDefined &QgsDataDefined::operator=( const QgsDataDefined & rhs )
{
  d.detach();
  d = rhs.d;
  return *this;
}
