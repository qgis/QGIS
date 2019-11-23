/***************************************************************************
    qgsclassificationmethod.cpp
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

#include <QRegularExpression>

#include "qgis.h"
#include "qgsclassificationmethod.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsapplication.h"
#include "qgsclassificationmethodregistry.h"

const int QgsClassificationMethod::MAX_PRECISION = 15;
const int QgsClassificationMethod::MIN_PRECISION = -6;


QList<double> QgsClassificationMethod::rangesToBreaks( const QList<QgsClassificationRange> &classes )
{
  QList<double> values;
  values.reserve( classes.count() );
  for ( int i = 0 ; i < classes.count(); i++ )
    values << classes.at( i ).upperBound();
  return values;
}

QgsClassificationMethod::QgsClassificationMethod( MethodProperties properties, int codeComplexity )
  : mFlags( properties )
  , mCodeComplexity( codeComplexity )
  , mLabelFormat( QStringLiteral( "%1 - %2 " ) )
{
}

void QgsClassificationMethod::copyBase( QgsClassificationMethod *c ) const
{
  c->setSymmetricMode( mSymmetricEnabled, mSymmetryPoint, mSymmetryAstride );
  c->setLabelFormat( mLabelFormat );
  c->setLabelPrecision( mLabelPrecision );
  c->setLabelTrimTrailingZeroes( mLabelTrimTrailingZeroes );
}

QgsClassificationMethod *QgsClassificationMethod::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString methodId = element.attribute( QStringLiteral( "id" ) );
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );

  // symmetric
  QDomElement symmetricModeElem = element.firstChildElement( QStringLiteral( "symmetricMode" ) );
  if ( !symmetricModeElem.isNull() )
  {
    bool symmetricEnabled = symmetricModeElem.attribute( QStringLiteral( "enabled" ) ).toInt() == 1;
    double symmetricPoint = symmetricModeElem.attribute( QStringLiteral( "symmetrypoint" ) ).toDouble();
    bool astride = symmetricModeElem.attribute( QStringLiteral( "astride" ) ).toInt() == 1;
    method->setSymmetricMode( symmetricEnabled, symmetricPoint, astride );
  }

  // label format
  QDomElement labelFormatElem = element.firstChildElement( QStringLiteral( "labelformat" ) );
  if ( !labelFormatElem.isNull() )
  {
    QString format = labelFormatElem.attribute( QStringLiteral( "format" ), "%1" + QStringLiteral( " - " ) + "%2" );
    int precision = labelFormatElem.attribute( QStringLiteral( "labelprecision" ), QStringLiteral( "4" ) ).toInt();
    bool trimTrailingZeroes = labelFormatElem.attribute( QStringLiteral( "trimtrailingzeroes" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );
    method->setLabelFormat( format );
    method->setLabelPrecision( precision );
    method->setLabelTrimTrailingZeroes( trimTrailingZeroes );
  }

  // Read specific properties from the implementation
  QDomElement extraElem = element.firstChildElement( QStringLiteral( "extraInformation" ) );
  if ( !extraElem.isNull() )
    method->readXml( extraElem, context );

  return method;
}

QDomElement QgsClassificationMethod::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement methodElem = doc.createElement( QStringLiteral( "classificationMethod" ) );

  methodElem.setAttribute( QStringLiteral( "id" ), id() );

  // symmetric
  QDomElement symmetricModeElem = doc.createElement( QStringLiteral( "symmetricMode" ) );
  symmetricModeElem.setAttribute( QStringLiteral( "enabled" ), symmetricModeEnabled() ? 1 : 0 );
  symmetricModeElem.setAttribute( QStringLiteral( "symmetrypoint" ), symmetryPoint() );
  symmetricModeElem.setAttribute( QStringLiteral( "astride" ), mSymmetryAstride ? 1 : 0 );
  methodElem.appendChild( symmetricModeElem );

  // label format
  QDomElement labelFormatElem = doc.createElement( QStringLiteral( "labelFormat" ) );
  labelFormatElem.setAttribute( QStringLiteral( "format" ), labelFormat() );
  labelFormatElem.setAttribute( QStringLiteral( "labelprecision" ), labelPrecision() );
  labelFormatElem.setAttribute( QStringLiteral( "trimtrailingzeroes" ), labelTrimTrailingZeroes() ? 1 : 0 );
  methodElem.appendChild( labelFormatElem );

  // extra information
  QDomElement extraElem = doc.createElement( QStringLiteral( "extraInformation" ) );
  writeXml( extraElem, context );
  methodElem.appendChild( extraElem );

  return methodElem;
}


void QgsClassificationMethod::setSymmetricMode( bool enabled, double symmetryPoint, bool astride )
{
  mSymmetricEnabled = enabled;
  mSymmetryPoint = symmetryPoint;
  mSymmetryAstride = astride;
}

void QgsClassificationMethod::setLabelPrecision( int precision )
{
  // Limit the range of decimal places to a reasonable range
  precision = qBound( MIN_PRECISION, precision, MAX_PRECISION );
  mLabelPrecision = precision;
  mLabelNumberScale = 1.0;
  mLabelNumberSuffix.clear();
  while ( precision < 0 )
  {
    precision++;
    mLabelNumberScale /= 10.0;
    mLabelNumberSuffix.append( '0' );
  }
}

QString QgsClassificationMethod::formatNumber( double value ) const
{
  static const QRegularExpression RE_TRAILING_ZEROES = QRegularExpression( "[.,]?0*$" );
  static const QRegularExpression RE_NEGATIVE_ZERO = QRegularExpression( "^\\-0(?:[.,]0*)?$" );
  if ( mLabelPrecision > 0 )
  {
    QString valueStr = QLocale().toString( value, 'f', mLabelPrecision );
    if ( mLabelTrimTrailingZeroes )
      valueStr = valueStr.remove( RE_TRAILING_ZEROES );
    if ( RE_NEGATIVE_ZERO.match( valueStr ).hasMatch() )
      valueStr = valueStr.mid( 1 );
    return valueStr;
  }
  else
  {
    QString valueStr = QLocale().toString( value * mLabelNumberScale, 'f', 0 );
    if ( valueStr == QLatin1String( "-0" ) )
      valueStr = '0';
    if ( valueStr != QLatin1String( "0" ) )
      valueStr = valueStr + mLabelNumberSuffix;
    return valueStr;
  }
}

QList<QgsClassificationRange> QgsClassificationMethod::classes( const QgsVectorLayer *layer, const QString &expression, int nclasses )
{
  if ( expression.isEmpty() )
    return QList<QgsClassificationRange>();

  if ( nclasses < 1 )
    nclasses = 1;

  QList<double> values;
  double minimum;
  double maximum;

  int fieldIndex = layer->fields().indexFromName( expression );

  bool ok;
  if ( valuesRequired() || fieldIndex == -1 )
  {
    values = QgsVectorLayerUtils::getDoubleValues( layer, expression, ok );
    if ( !ok || values.isEmpty() )
      return QList<QgsClassificationRange>();

    auto result = std::minmax_element( values.begin(), values.end() );
    minimum = *result.first;
    maximum = *result.second;
  }
  else
  {
    minimum = layer->minimumValue( fieldIndex ).toDouble();
    maximum = layer->maximumValue( fieldIndex ).toDouble();
  }

  // get the breaks
  QList<double> breaks = calculateBreaks( minimum, maximum, values, nclasses );
  breaks.insert( 0, minimum );
  // create classes
  return breaksToClasses( breaks );
}

QList<QgsClassificationRange> QgsClassificationMethod::classes( const QList<double> &values, int nclasses )
{
  auto result = std::minmax_element( values.begin(), values.end() );
  double minimum = *result.first;
  double maximum = *result.second;

  // get the breaks
  QList<double> breaks = calculateBreaks( minimum, maximum, values, nclasses );
  breaks.insert( 0, minimum );
  // create classes
  return breaksToClasses( breaks );
}

QList<QgsClassificationRange> QgsClassificationMethod::classes( double minimum, double maximum, int nclasses )
{
  if ( valuesRequired() )
  {
    QgsDebugMsg( QStringLiteral( "The classification method %1 tries to calculate classes without values while they are required." ).arg( name() ) );
  }

  // get the breaks
  QList<double> breaks = calculateBreaks( minimum, maximum, QList<double>(), nclasses );
  breaks.insert( 0, minimum );
  // create classes
  return breaksToClasses( breaks );
}

QList<QgsClassificationRange> QgsClassificationMethod::breaksToClasses( const QList<double> &breaks ) const
{
  QList<QgsClassificationRange> classes;

  for ( int i = 1; i < breaks.count(); i++ )
  {

    const double lowerValue = breaks.at( i - 1 );
    const double upperValue = breaks.at( i );

    ClassPosition pos = Inner;
    if ( i == 1 )
      pos = LowerBound;
    else if ( i == breaks.count() - 1 )
      pos = UpperBound;

    QString label = labelForRange( lowerValue, upperValue, pos );
    classes << QgsClassificationRange( label, lowerValue, upperValue );
  }

  return classes;
}

void QgsClassificationMethod::makeBreaksSymmetric( QList<double> &breaks, double symmetryPoint, bool astride )
{
  // remove the breaks that are above the existing opposite sign classes
  // to keep colors symmetrically balanced around symmetryPoint
  // if astride is true, remove the symmetryPoint break so that
  // the 2 classes form only one

  if ( breaks.count() < 2 )
    return;

  std::sort( breaks.begin(), breaks.end() );
  // breaks contain the maximum of the distrib but not the minimum
  double distBelowSymmetricValue = std::fabs( breaks[0] - symmetryPoint );
  double distAboveSymmetricValue = std::fabs( breaks[ breaks.size() - 2 ] - symmetryPoint ) ;
  double absMin = std::min( distAboveSymmetricValue, distBelowSymmetricValue );

  // make symmetric
  for ( int i = 0; i <= breaks.size() - 2; ++i )
  {
    // part after "absMin" is for doubles rounding issues
    if ( std::fabs( breaks.at( i ) - symmetryPoint ) >= ( absMin - std::fabs( breaks[0] - breaks[1] ) / 100. ) )
    {
      breaks.removeAt( i );
      --i;
    }
  }
  // remove symmetry point
  if ( astride ) // && breaks.indexOf( symmetryPoint ) != -1) // if symmetryPoint is found
  {
    breaks.removeAt( breaks.indexOf( symmetryPoint ) );
  }
}

QString QgsClassificationMethod::labelForRange( const QgsRendererRange &range, QgsClassificationMethod::ClassPosition position ) const
{
  return labelForRange( range.lowerValue(), range.upperValue(), position );
}

QString QgsClassificationMethod::labelForRange( const double lowerValue, const double upperValue, ClassPosition position ) const
{
  Q_UNUSED( position )

  const QString lowerLabel = valueToLabel( lowerValue );
  const QString upperLabel = valueToLabel( upperValue );

  return labelFormat().arg( lowerLabel ).arg( upperLabel );
}
