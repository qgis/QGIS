/***************************************************************************
                             qgsnumericformat.cpp
                             -------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnumericformat.h"

#include "qgsreadwritecontext.h"
#include "qgsxmlutils.h"

#include <QLocale>

#include "moc_qgsnumericformat.cpp"

QgsNumericFormatContext::QgsNumericFormatContext()
{
  const QLocale l;
  // With Qt6, these methods return strings to be prepared
  // for utf-16 surrogates
  // Do we care? If yes, we need to switch all members of QgsNumericFormatContext to QString
  mThousandsSep = l.groupSeparator().at( 0 );
  mDecimalSep = l.decimalPoint().at( 0 );
  mPercent = l.percent().at( 0 );
  mZeroDigit = l.zeroDigit().at( 0 );
  mNegativeSign = l.negativeSign().at( 0 );
  mPositiveSign = l.positiveSign().at( 0 );
  mExponential = l.exponential().at( 0 );
}

QgsExpressionContext QgsNumericFormatContext::expressionContext() const
{
  return mExpressionContext;
}

void QgsNumericFormatContext::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}

int QgsNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

double QgsNumericFormat::suggestSampleValue() const
{
  return 1234.56789123456;
}

void QgsNumericFormat::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  const QVariantMap config = configuration( context );
  const QDomElement configElement = QgsXmlUtils::writeVariant( config, document );
  element.appendChild( configElement );
  element.setAttribute( u"id"_s, id() );
}

bool QgsNumericFormat::operator==( const QgsNumericFormat &other ) const
{
  return id() == other.id() && configuration( QgsReadWriteContext() ) == other.configuration( QgsReadWriteContext() );
}

bool QgsNumericFormat::operator!=( const QgsNumericFormat &other ) const
{
  return !operator==( other );
}
