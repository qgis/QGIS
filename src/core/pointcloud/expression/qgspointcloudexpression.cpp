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

#include "qgsexpression.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnodeimpl.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudexpression_p.h"


bool QgsPointCloudExpression::checkExpression( const QgsExpression &expression, const QgsPointCloudBlock *block, QString &errorMessage )
{
  QgsPointCloudExpression exp( expression );
  exp.prepare( block );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsPointCloudExpression::setExpression( const QgsExpression &expression )
{
  detach();
  QString error;
  d->mRootNode = QgsPointCloudExpressionNode::convert( expression.rootNode(), error );
  d->mEvalErrorString = QString();
  d->mExp = expression;
  d->mIsPrepared = false;
}

QString QgsPointCloudExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QgsPointCloudExpression::QgsPointCloudExpression( const QgsExpression &expr )
  : d( new QgsPointCloudExpressionPrivate )
{
  QString error;
  d->mRootNode = QgsPointCloudExpressionNode::convert( expr.rootNode(), error ); //::parseExpression( expr, d->mParserErrorString, d->mParserErrors );
  d->mExp = expr.dump();
  //Q_ASSERT( !d->mParserErrorString.isNull() || d->mRootNode );
}

QgsPointCloudExpression::QgsPointCloudExpression( const QgsPointCloudExpression &other )
  : d( other.d )
{
  d->ref.ref();
}

QgsPointCloudExpression &QgsPointCloudExpression::operator=( const QgsPointCloudExpression &other )
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

QgsPointCloudExpression::operator QString() const
{
  return d->mExp;
}

QgsPointCloudExpression::QgsPointCloudExpression()
  : d( new QgsPointCloudExpressionPrivate )
{
}

QgsPointCloudExpression::~QgsPointCloudExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

bool QgsPointCloudExpression::operator==( const QgsPointCloudExpression &other ) const
{
  return ( d == other.d || d->mExp == other.d->mExp );
}

bool QgsPointCloudExpression::isValid() const
{
  return d->mRootNode;
}

bool QgsPointCloudExpression::hasParserError() const
{
  return d->mParserErrors.count() > 0;
}

QString QgsPointCloudExpression::parserErrorString() const
{
  return d->mParserErrorString;
}

QList<QgsPointCloudExpression::ParserError> QgsPointCloudExpression::parserErrors() const
{
  return d->mParserErrors;
}

QSet<QString> QgsPointCloudExpression::referencedAttributes() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedAttributes();
}

void QgsPointCloudExpression::detach()
{
  Q_ASSERT( d );

  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();

    d = new QgsPointCloudExpressionPrivate( *d );
  }
}

bool QgsPointCloudExpression::prepare( const QgsPointCloudBlock *block )
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

double QgsPointCloudExpression::evaluate( int p )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return 0;
  }

  return d->mRootNode->eval( this, p );
}

bool QgsPointCloudExpression::hasEvalError() const
{
  return !d->mEvalErrorString.isNull();
}

QString QgsPointCloudExpression::evalErrorString() const
{
  return d->mEvalErrorString;
}

void QgsPointCloudExpression::setEvalErrorString( const QString &str )
{
  d->mEvalErrorString = str;
}

QString QgsPointCloudExpression::dump() const
{
  if ( !d->mRootNode )
    return QString();

  return d->mRootNode->dump();
}

const QgsPointCloudExpressionNode *QgsPointCloudExpression::rootNode() const
{
  return d->mRootNode;
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpression::nodes() const
{
  if ( !d->mRootNode )
    return QList<const QgsPointCloudExpressionNode *>();

  return d->mRootNode->nodes();
}



