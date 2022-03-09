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


QgsPointCloudExpression::QgsPointCloudExpression()
  : d( new QgsPointCloudExpressionPrivate )
{
}

QgsPointCloudExpression::QgsPointCloudExpression( const QString &subsetString )
  : d( new QgsPointCloudExpressionPrivate )
{
  QgsExpression expression( subsetString );
  setExpression( expression );
}

QgsPointCloudExpression::~QgsPointCloudExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

void QgsPointCloudExpression::setExpression( const QString &subset )
{
  QgsExpression expression( subset );
  if ( expression.hasParserError() )
  {
    d->mRootNode = nullptr;
    d->mParserErrorString = expression.parserErrorString();
    d->mParserErrors = expression.parserErrors();
  }
  else
  {
    d->mParserErrors.clear();
    d->mRootNode = QgsPointCloudExpressionNode::convert( expression.rootNode(), d->mParserErrorString );
    if ( !d->mParserErrorString.isEmpty() )
      d->mParserErrors.append( QgsExpression::ParserError() );
  }
  d->mExp = expression;
  d->mIsPrepared = false;
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

bool QgsPointCloudExpression::operator==( const QgsPointCloudExpression &other ) const
{
  return ( d == other.d || d->mExp == other.d->mExp );
}
QgsPointCloudExpression::operator QString() const
{
  return d->mExp;
}

QString QgsPointCloudExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
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

QList<QgsExpression::ParserError> QgsPointCloudExpression::parserErrors() const
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

double QgsPointCloudExpression::evaluate( int pointIndex )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return 0;
  }

  return d->mRootNode->eval( this, pointIndex );
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

bool QgsPointCloudExpression::checkExpression( const QgsExpression &expression, const QgsPointCloudBlock *block, QString &errorMessage )
{
  QgsPointCloudExpression exp( expression );
  exp.prepare( block );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}
