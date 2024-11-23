/***************************************************************************
    qgsrendererrange.cpp
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrendererrange.h"
#include "qgsclassificationmethod.h"
#include "qgssymbol.h"

#include <QLocale>
#include <QUuid>


QgsRendererRange::QgsRendererRange( const QgsClassificationRange &range, QgsSymbol *symbol, bool render, const QString &uuid )
  : mLowerValue( range.lowerBound() )
  , mUpperValue( range.upperBound() )
  , mSymbol( symbol )
  , mLabel( range.label() )
  , mRender( render )
{
  mUuid = !uuid.isEmpty() ? uuid : QUuid::createUuid().toString();
}

QgsRendererRange::QgsRendererRange( double lowerValue, double upperValue, QgsSymbol *symbol, const QString &label, bool render, const QString &uuid )
  : mLowerValue( lowerValue )
  , mUpperValue( upperValue )
  , mSymbol( symbol )
  , mLabel( label )
  , mRender( render )
{
  mUuid = !uuid.isEmpty() ? uuid : QUuid::createUuid().toString();
}

QgsRendererRange::QgsRendererRange( const QgsRendererRange &range )
  : mLowerValue( range.mLowerValue )
  , mUpperValue( range.mUpperValue )
  , mSymbol( range.mSymbol ? range.mSymbol->clone() : nullptr )
  , mLabel( range.mLabel )
  , mRender( range.mRender )
  , mUuid( range.mUuid )
{}

QgsRendererRange::~QgsRendererRange() = default;

QgsRendererRange &QgsRendererRange::operator=( QgsRendererRange range )
{
  mLowerValue = range.mLowerValue;
  mUpperValue = range.mUpperValue;
  mSymbol.reset( range.mSymbol ? range.mSymbol->clone() : nullptr );
  mLabel = range.mLabel;
  mRender = range.mRender;
  mUuid = range.mUuid;
  return *this;
}

bool QgsRendererRange::operator<( const QgsRendererRange &other ) const
{
  return
    lowerValue() < other.lowerValue() ||
    ( qgsDoubleNear( lowerValue(), other.lowerValue() ) && upperValue() < other.upperValue() );
}

QString QgsRendererRange::uuid() const
{
  return mUuid;
}

double QgsRendererRange::lowerValue() const
{
  return mLowerValue;
}

double QgsRendererRange::upperValue() const
{
  return mUpperValue;
}

QgsSymbol *QgsRendererRange::symbol() const
{
  return mSymbol.get();
}

QString QgsRendererRange::label() const
{
  return mLabel;
}

void QgsRendererRange::setSymbol( QgsSymbol *s )
{
  if ( mSymbol.get() != s ) mSymbol.reset( s );
}

void QgsRendererRange::setLabel( const QString &label )
{
  mLabel = label;
}

void QgsRendererRange::setUpperValue( double upperValue )
{
  mUpperValue = upperValue;
}

void QgsRendererRange::setLowerValue( double lowerValue )
{
  mLowerValue = lowerValue;
}

bool QgsRendererRange::renderState() const
{
  return mRender;
}

void QgsRendererRange::setRenderState( bool render )
{
  mRender = render;
}

QString QgsRendererRange::dump() const
{
  return QStringLiteral( "%1 - %2::%3::%4\n" ).arg( mLowerValue ).arg( mUpperValue ).arg( mLabel, mSymbol ? mSymbol->dump() : QStringLiteral( "(no symbol)" ) );
}

void QgsRendererRange::toSld( QDomDocument &doc, QDomElement &element, QVariantMap props, bool firstRange ) const
{
  if ( !mSymbol || props.value( QStringLiteral( "attribute" ), QString() ).toString().isEmpty() )
    return;

  QString attrName = props[ QStringLiteral( "attribute" )].toString();

  QDomElement ruleElem = doc.createElement( QStringLiteral( "se:Rule" ) );

  QDomElement nameElem = doc.createElement( QStringLiteral( "se:Name" ) );
  nameElem.appendChild( doc.createTextNode( mLabel ) );
  ruleElem.appendChild( nameElem );

  QDomElement descrElem = doc.createElement( QStringLiteral( "se:Description" ) );
  QDomElement titleElem = doc.createElement( QStringLiteral( "se:Title" ) );
  QString descrStr = QStringLiteral( "range: %1 - %2" ).arg( qgsDoubleToString( mLowerValue ), qgsDoubleToString( mUpperValue ) );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc = QStringLiteral( "\"%1\" %2 %3 AND \"%1\" <= %4" )
                       .arg( attrName.replace( '\"', QLatin1String( "\"\"" ) ),
                             firstRange ? QStringLiteral( ">=" ) : QStringLiteral( ">" ),
                             qgsDoubleToString( mLowerValue ),
                             qgsDoubleToString( mUpperValue ) );
  QgsSymbolLayerUtils::createFunctionElement( doc, ruleElem, filterFunc );

  mSymbol->toSld( doc, ruleElem, props );
  if ( !QgsSymbolLayerUtils::hasSldSymbolizer( ruleElem ) )
  {
    // symbol could not be converted to SLD, or is an "empty" symbol. In this case we do not generate a rule, as
    // SLD spec requires a Symbolizer element to be present
    return;
  }

  element.appendChild( ruleElem );
}

//////////


const int QgsRendererRangeLabelFormat::MAX_PRECISION = 15;
const int QgsRendererRangeLabelFormat::MIN_PRECISION = -6;

QgsRendererRangeLabelFormat::QgsRendererRangeLabelFormat()
  : mFormat( QStringLiteral( "%1 - %2" ) )
  , mReTrailingZeroes( "[.,]?0*$" )
  , mReNegativeZero( "^\\-0(?:[.,]0*)?$" )
{
}

QgsRendererRangeLabelFormat::QgsRendererRangeLabelFormat( const QString &format, int precision, bool trimTrailingZeroes )
  : mReTrailingZeroes( "[.,]?0*$" )
  , mReNegativeZero( "^\\-0(?:[.,]0*)?$" )
{
  setFormat( format );
  setPrecision( precision );
  setTrimTrailingZeroes( trimTrailingZeroes );
}


bool QgsRendererRangeLabelFormat::operator==( const QgsRendererRangeLabelFormat &other ) const
{
  return
    format() == other.format() &&
    precision() == other.precision() &&
    trimTrailingZeroes() == other.trimTrailingZeroes();
}

bool QgsRendererRangeLabelFormat::operator!=( const QgsRendererRangeLabelFormat &other ) const
{
  return !( *this == other );
}

void QgsRendererRangeLabelFormat::setPrecision( int precision )
{
  // Limit the range of decimal places to a reasonable range
  precision = std::clamp( precision, MIN_PRECISION, MAX_PRECISION );
  mPrecision = precision;
  mNumberScale = 1.0;
  mNumberSuffix.clear();
  while ( precision < 0 )
  {
    precision++;
    mNumberScale /= 10.0;
    mNumberSuffix.append( '0' );
  }
}

QString QgsRendererRangeLabelFormat::labelForRange( const QgsRendererRange &range ) const
{
  return labelForRange( range.lowerValue(), range.upperValue() );
}

QString QgsRendererRangeLabelFormat::formatNumber( double value ) const
{
  if ( mPrecision > 0 )
  {
    QString valueStr = QLocale().toString( value, 'f', mPrecision );
    if ( mTrimTrailingZeroes )
      valueStr = valueStr.remove( mReTrailingZeroes );
    if ( mReNegativeZero.match( valueStr ).hasMatch() )
      valueStr = valueStr.mid( 1 );
    return valueStr;
  }
  else
  {
    QString valueStr = QLocale().toString( value * mNumberScale, 'f', 0 );
    if ( valueStr == QLatin1String( "-0" ) )
      valueStr = '0';
    if ( valueStr != QLatin1String( "0" ) )
      valueStr = valueStr + mNumberSuffix;
    return valueStr;
  }
}

QString QgsRendererRangeLabelFormat::labelForRange( double lower, double upper ) const
{
  QString lowerStr = formatNumber( lower );
  QString upperStr = formatNumber( upper );

  QString legend( mFormat );
  return legend.replace( QLatin1String( "%1" ), lowerStr ).replace( QLatin1String( "%2" ), upperStr );
}

void QgsRendererRangeLabelFormat::setFromDomElement( QDomElement &element )
{
  mFormat = element.attribute( QStringLiteral( "format" ),
                               element.attribute( QStringLiteral( "prefix" ), QStringLiteral( " " ) ) + "%1" +
                               element.attribute( QStringLiteral( "separator" ), QStringLiteral( " - " ) ) + "%2" +
                               element.attribute( QStringLiteral( "suffix" ), QStringLiteral( " " ) )
                             );
  setPrecision( element.attribute( QStringLiteral( "decimalplaces" ), QStringLiteral( "4" ) ).toInt() );
  mTrimTrailingZeroes = element.attribute( QStringLiteral( "trimtrailingzeroes" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );
}

void QgsRendererRangeLabelFormat::saveToDomElement( QDomElement &element )
{
  element.setAttribute( QStringLiteral( "format" ), mFormat );
  element.setAttribute( QStringLiteral( "decimalplaces" ), mPrecision );
  element.setAttribute( QStringLiteral( "trimtrailingzeroes" ), mTrimTrailingZeroes ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
}

