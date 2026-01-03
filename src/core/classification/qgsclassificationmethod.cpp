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

#include "qgsclassificationmethod.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsclassificationmethodregistry.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsxmlutils.h"

#include <QRegularExpression>

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
  , mLabelFormat( u"%1 - %2"_s )
{
}

QgsClassificationMethod::~QgsClassificationMethod()
{
  qDeleteAll( mParameters );
}

void QgsClassificationMethod::copyBase( QgsClassificationMethod *c ) const
{
  c->setSymmetricMode( mSymmetricEnabled, mSymmetryPoint, mSymmetryAstride );
  c->setLabelFormat( mLabelFormat );
  c->setLabelPrecision( mLabelPrecision );
  c->setLabelTrimTrailingZeroes( mLabelTrimTrailingZeroes );
  c->setParameterValues( mParameterValues );
}

std::unique_ptr< QgsClassificationMethod > QgsClassificationMethod::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString methodId = element.attribute( u"id"_s );
  std::unique_ptr< QgsClassificationMethod > method = QgsApplication::classificationMethodRegistry()->method( methodId );

  // symmetric
  QDomElement symmetricModeElem = element.firstChildElement( u"symmetricMode"_s );
  if ( !symmetricModeElem.isNull() )
  {
    bool symmetricEnabled = symmetricModeElem.attribute( u"enabled"_s ).toInt() == 1;
    double symmetricPoint = symmetricModeElem.attribute( u"symmetrypoint"_s ).toDouble();
    bool astride = symmetricModeElem.attribute( u"astride"_s ).toInt() == 1;
    method->setSymmetricMode( symmetricEnabled, symmetricPoint, astride );
  }

  // label format
  QDomElement labelFormatElem = element.firstChildElement( u"labelFormat"_s );
  if ( !labelFormatElem.isNull() )
  {
    QString format = labelFormatElem.attribute( u"format"_s, "%1" + u" - "_s + "%2" );
    int precision = labelFormatElem.attribute( u"labelprecision"_s, u"4"_s ).toInt();
    bool trimTrailingZeroes = labelFormatElem.attribute( u"trimtrailingzeroes"_s, u"false"_s ) == "true"_L1;
    method->setLabelFormat( format );
    method->setLabelPrecision( precision );
    method->setLabelTrimTrailingZeroes( trimTrailingZeroes );
  }

  // parameters (processing parameters)
  QDomElement parametersElem = element.firstChildElement( u"parameters"_s );
  const QVariantMap parameterValues = QgsXmlUtils::readVariant( parametersElem.firstChildElement() ).toMap();
  method->setParameterValues( parameterValues );

  // Read specific properties from the implementation
  QDomElement extraElem = element.firstChildElement( u"extraInformation"_s );
  if ( !extraElem.isNull() )
    method->readXml( extraElem, context );

  return method;
}

QDomElement QgsClassificationMethod::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement methodElem = doc.createElement( u"classificationMethod"_s );

  methodElem.setAttribute( u"id"_s, id() );

  // symmetric
  QDomElement symmetricModeElem = doc.createElement( u"symmetricMode"_s );
  symmetricModeElem.setAttribute( u"enabled"_s, symmetricModeEnabled() ? 1 : 0 );
  symmetricModeElem.setAttribute( u"symmetrypoint"_s, symmetryPoint() );
  symmetricModeElem.setAttribute( u"astride"_s, mSymmetryAstride ? 1 : 0 );
  methodElem.appendChild( symmetricModeElem );

  // label format
  QDomElement labelFormatElem = doc.createElement( u"labelFormat"_s );
  labelFormatElem.setAttribute( u"format"_s, labelFormat() );
  labelFormatElem.setAttribute( u"labelprecision"_s, labelPrecision() );
  labelFormatElem.setAttribute( u"trimtrailingzeroes"_s, labelTrimTrailingZeroes() ? 1 : 0 );
  methodElem.appendChild( labelFormatElem );

  // parameters (processing parameters)
  QDomElement parametersElem = doc.createElement( u"parameters"_s );
  parametersElem.appendChild( QgsXmlUtils::writeVariant( mParameterValues, doc ) );
  methodElem.appendChild( parametersElem );

  // extra information
  QDomElement extraElem = doc.createElement( u"extraInformation"_s );
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
  precision = std::clamp( precision, MIN_PRECISION, MAX_PRECISION );
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
    if ( valueStr == "-0"_L1 )
      valueStr = '0';
    if ( valueStr != "0"_L1 )
      valueStr = valueStr + mLabelNumberSuffix;
    return valueStr;
  }
}

void QgsClassificationMethod::addParameter( QgsProcessingParameterDefinition *definition )
{
  mParameters.append( definition );
}

const QgsProcessingParameterDefinition *QgsClassificationMethod::parameterDefinition( const QString &parameterName ) const
{
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->name() == parameterName )
      return def;
  }
  QgsMessageLog::logMessage( u"No parameter definition found for %1 in %2 method."_s.arg( parameterName ).arg( name() ) );
  return nullptr;
}

void QgsClassificationMethod::setParameterValues( const QVariantMap &values )
{
  mParameterValues = values;
  for ( auto it = mParameterValues.constBegin(); it != mParameterValues.constEnd(); ++it )
  {
    if ( !parameterDefinition( it.key() ) )
    {
      QgsMessageLog::logMessage( name(), QObject::tr( "Parameter %1 does not exist in the method" ).arg( it.key() ) );
    }
  }
}

QList<QgsClassificationRange> QgsClassificationMethod::classes( const QgsVectorLayer *layer, const QString &expression, int nclasses )
{
  QString error;
  return classesV2( layer, expression, nclasses, error );
}

QList<QgsClassificationRange> QgsClassificationMethod::classesV2( const QgsVectorLayer *layer, const QString &expression, int nclasses, QString &error )
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
    QVariant minVal;
    QVariant maxVal;
    layer->minimumAndMaximumValue( fieldIndex, minVal, maxVal );
    minimum = minVal.toDouble();
    maximum = maxVal.toDouble();
  }

  // get the breaks, minimum and maximum might be updated by implementation
  QList<double> breaks = calculateBreaks( minimum, maximum, values, nclasses, error );
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
  QString error;
  QList<double> breaks = calculateBreaks( minimum, maximum, values, nclasses, error );
  ( void )error;

  breaks.insert( 0, minimum );
  // create classes
  return breaksToClasses( breaks );
}

QList<QgsClassificationRange> QgsClassificationMethod::classes( double minimum, double maximum, int nclasses )
{
  if ( valuesRequired() )
  {
    QgsDebugError( u"The classification method %1 tries to calculate classes without values while they are required."_s.arg( name() ) );
  }

  // get the breaks
  QString error;
  QList<double> breaks = calculateBreaks( minimum, maximum, QList<double>(), nclasses, error );
  ( void )error;

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
  if ( astride )
  {
    breaks.removeAll( symmetryPoint );
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

  return labelFormat().replace( "%1"_L1, lowerLabel ).replace( "%2"_L1, upperLabel );
}
