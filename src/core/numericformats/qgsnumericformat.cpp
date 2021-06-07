/***************************************************************************
                             qgsnumericformat.cpp
                             -------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsnumericformat.h"
#include "qgsxmlutils.h"
#include "qgsreadwritecontext.h"

#include <QLocale>

QgsNumericFormatContext::QgsNumericFormatContext()
{
  QLocale l;
  mThousandsSep = l.groupSeparator();
  mDecimalSep = l.decimalPoint();
  mPercent = l.percent();
  mZeroDigit = l.zeroDigit();
  mNegativeSign = l.negativeSign();
  mPositiveSign = l.positiveSign();
  mExponential = l.exponential();
}

int QgsNumericFormat::sortKey()
{
  return 100;
}

double QgsNumericFormat::suggestSampleValue() const
{
  return 1234.56789123456;
}

void QgsNumericFormat::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  const QVariantMap config = configuration( context );
  QDomElement configElement = QgsXmlUtils::writeVariant( config, document );
  element.appendChild( configElement );
  element.setAttribute( QStringLiteral( "id" ), id() );
}

bool QgsNumericFormat::operator==( const QgsNumericFormat &other ) const
{
  return id() == other.id() && configuration( QgsReadWriteContext() ) == other.configuration( QgsReadWriteContext() );
}

bool QgsNumericFormat::operator!=( const QgsNumericFormat &other ) const
{
  return !operator==( other );
}

