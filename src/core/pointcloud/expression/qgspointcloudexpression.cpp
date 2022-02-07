/***************************************************************************
                         qgspointcouldexpression.cpp
                         ---------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnodeimpl.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudexpression_p.h"

#include <QRegularExpression>

// from parser
extern QgsPointcloudExpressionNode *parseExpression( const QString &str, QString &parserErrorMsg, QList<QgsPointcloudExpression::ParserError> &parserErrors );

bool QgsPointcloudExpression::checkExpression( const QString &text, const QgsPointCloudBlock *block, QString &errorMessage )
{
  QgsPointcloudExpression exp( text );
  exp.prepare( block );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsPointcloudExpression::setExpression( const QString &expression )
{
  detach();
  d->mRootNode = ::parseExpression( expression, d->mParserErrorString, d->mParserErrors );
  d->mEvalErrorString = QString();
  d->mExp = expression;
  d->mIsPrepared = false;
}

QString QgsPointcloudExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QString QgsPointcloudExpression::quotedAttributeRef( QString name )
{
  return QStringLiteral( "\"%1\"" ).arg( name.replace( '\"', QLatin1String( "\"\"" ) ) );
}

QString QgsPointcloudExpression::quotedString( QString text )
{
  text.replace( '\'', QLatin1String( "''" ) );
  text.replace( '\\', QLatin1String( "\\\\" ) );
  text.replace( '\n', QLatin1String( "\\n" ) );
  text.replace( '\t', QLatin1String( "\\t" ) );
  return QStringLiteral( "'%1'" ).arg( text );
}

QString QgsPointcloudExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, value.type() );
}

QString QgsPointcloudExpression::quotedValue( const QVariant &value, QVariant::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( type )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );

    case QVariant::List:
    {
      QStringList quotedValues;
      const QVariantList values = value.toList();
      quotedValues.reserve( values.count() );
      for ( const QVariant &v : values )
      {
        quotedValues += quotedValue( v );
      }
      return QStringLiteral( "array( %1 )" ).arg( quotedValues.join( QLatin1String( ", " ) ) );
    }

    default:
    case QVariant::String:
      return quotedString( value.toString() );
  }

}

QgsPointcloudExpression::QgsPointcloudExpression( const QString &expr )
  : d( new QgsPointcloudExpressionPrivate )
{
  d->mRootNode = ::parseExpression( expr, d->mParserErrorString, d->mParserErrors );
  d->mExp = expr;
  Q_ASSERT( !d->mParserErrorString.isNull() || d->mRootNode );
}

QgsPointcloudExpression::QgsPointcloudExpression( const QgsPointcloudExpression &other )
  : d( other.d )
{
  d->ref.ref();
}

QgsPointcloudExpression &QgsPointcloudExpression::operator=( const QgsPointcloudExpression &other )
{
  if ( this != &other )
  {
    if ( !d->ref.deref() )
    {
      delete d;
    }

    d = other.d;
    d->ref.ref();
  }
  return *this;
}

QgsPointcloudExpression::operator QString() const
{
  return d->mExp;
}

QgsPointcloudExpression::QgsPointcloudExpression()
  : d( new QgsPointcloudExpressionPrivate )
{
}

QgsPointcloudExpression::~QgsPointcloudExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

bool QgsPointcloudExpression::operator==( const QgsPointcloudExpression &other ) const
{
  return ( d == other.d || d->mExp == other.d->mExp );
}

bool QgsPointcloudExpression::isValid() const
{
  return d->mRootNode;
}

bool QgsPointcloudExpression::hasParserError() const
{
  return d->mParserErrors.count() > 0;
}

QString QgsPointcloudExpression::parserErrorString() const
{
  return d->mParserErrorString;
}

QList<QgsPointcloudExpression::ParserError> QgsPointcloudExpression::parserErrors() const
{
  return d->mParserErrors;
}

QSet<QString> QgsPointcloudExpression::referencedAttributes() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedAttributes();
}

void QgsPointcloudExpression::detach()
{
  Q_ASSERT( d );

  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();

    d = new QgsPointcloudExpressionPrivate( *d );
  }
}

bool QgsPointcloudExpression::prepare( const QgsPointCloudBlock *block )
{
  detach();
  d->mEvalErrorString = QString();

  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return false;
  }

  d->mIsPrepared = true;
  return d->mRootNode->prepare( this, block );
}

double QgsPointcloudExpression::evaluate( int p )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return 0;
  }

  return d->mRootNode->eval( this, p );
}

bool QgsPointcloudExpression::hasEvalError() const
{
  return !d->mEvalErrorString.isNull();
}

QString QgsPointcloudExpression::evalErrorString() const
{
  return d->mEvalErrorString;
}

void QgsPointcloudExpression::setEvalErrorString( const QString &str )
{
  d->mEvalErrorString = str;
}

QString QgsPointcloudExpression::dump() const
{
  if ( !d->mRootNode )
    return QString();

  return d->mRootNode->dump();
}

const QgsPointcloudExpressionNode *QgsPointcloudExpression::rootNode() const
{
  return d->mRootNode;
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpression::nodes() const
{
  if ( !d->mRootNode )
    return QList<const QgsPointcloudExpressionNode *>();

  return d->mRootNode->nodes();
}



