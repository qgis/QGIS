/***************************************************************************
                             qgsexpressionbasednumericformat.cpp
                             --------------------------
    begin                : August 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionbasednumericformat.h"
#include "qgis.h"

QgsExpressionBasedNumericFormat::QgsExpressionBasedNumericFormat()
{
  setExpression( QStringLiteral( "@value" ) );
}

QString QgsExpressionBasedNumericFormat::id() const
{
  return QStringLiteral( "expression" );
}

QString QgsExpressionBasedNumericFormat::visibleName() const
{
  return QObject::tr( "Custom Expression" );
}

int QgsExpressionBasedNumericFormat::sortKey()
{
  return 1;
}

QString QgsExpressionBasedNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  QgsExpressionContext expContext = context.expressionContext();
  expContext.setOriginalValueVariable( value );

  return mExpression.evaluate( &expContext ).toString();
}

QgsNumericFormat *QgsExpressionBasedNumericFormat::clone() const
{
  return new QgsExpressionBasedNumericFormat( *this );
}

QgsNumericFormat *QgsExpressionBasedNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  std::unique_ptr< QgsExpressionBasedNumericFormat > res = std::make_unique< QgsExpressionBasedNumericFormat >();
  res->setConfiguration( configuration, context );
  return res.release();
}

QVariantMap QgsExpressionBasedNumericFormat::configuration( const QgsReadWriteContext & ) const
{
  QVariantMap res;
  res.insert( QStringLiteral( "expression" ), mExpressionString );
  return res;
}

void QgsExpressionBasedNumericFormat::setExpression( const QString &expression )
{
  mExpressionString = expression;
  mExpression = QgsExpression( mExpressionString );
}

void QgsExpressionBasedNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext & )
{
  mExpressionString = configuration.value( QStringLiteral( "expression" ), QStringLiteral( "@value" ) ).toString();
  mExpression = QgsExpression( mExpressionString );
}

