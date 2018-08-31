/***************************************************************************
    qgsgraduatedsymbolrenderer.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsgraduatedsymbolrenderer.h"

#include "qgsattributes.h"
#include "qgscolorramp.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgslogger.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsproperty.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend
#include <limits> // for jenks classification
#include <ctime>


QgsRendererRange::QgsRendererRange( double lowerValue, double upperValue, QgsSymbol *symbol, const QString &label, bool render )
  : mLowerValue( lowerValue )
  , mUpperValue( upperValue )
  , mSymbol( symbol )
  , mLabel( label )
  , mRender( render )
{
}

QgsRendererRange::QgsRendererRange( const QgsRendererRange &range )
  : mLowerValue( range.mLowerValue )
  , mUpperValue( range.mUpperValue )
  , mSymbol( range.mSymbol ? range.mSymbol->clone() : nullptr )
  , mLabel( range.mLabel )
  , mRender( range.mRender )
{
}

// cpy and swap idiom, note that the cpy is done with 'pass by value'
QgsRendererRange &QgsRendererRange::operator=( QgsRendererRange range )
{
  swap( range );
  return *this;
}

bool QgsRendererRange::operator<( const QgsRendererRange &other ) const
{
  return
    lowerValue() < other.lowerValue() ||
    ( qgsDoubleNear( lowerValue(), other.lowerValue() ) && upperValue() < other.upperValue() );
}


void QgsRendererRange::swap( QgsRendererRange &other )
{
  std::swap( mLowerValue, other.mLowerValue );
  std::swap( mUpperValue, other.mUpperValue );
  std::swap( mSymbol, other.mSymbol );
  std::swap( mLabel, other.mLabel );
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

void QgsRendererRange::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props, bool firstRange ) const
{
  if ( !mSymbol || props.value( QStringLiteral( "attribute" ), QString() ).isEmpty() )
    return;

  QString attrName = props[ QStringLiteral( "attribute" )];

  QDomElement ruleElem = doc.createElement( QStringLiteral( "se:Rule" ) );
  element.appendChild( ruleElem );

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
}

///////////

const int QgsRendererRangeLabelFormat::MAX_PRECISION = 15;
const int QgsRendererRangeLabelFormat::MIN_PRECISION = -6;

QgsRendererRangeLabelFormat::QgsRendererRangeLabelFormat()
  : mFormat( QStringLiteral( " %1 - %2 " ) )
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
  precision = qBound( MIN_PRECISION, precision, MAX_PRECISION );
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
    if ( mReNegativeZero.exactMatch( valueStr ) )
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

///////////

QgsGraduatedSymbolRenderer::QgsGraduatedSymbolRenderer( const QString &attrName, const QgsRangeList &ranges )
  : QgsFeatureRenderer( QStringLiteral( "graduatedSymbol" ) )
  , mAttrName( attrName )
{
  // TODO: check ranges for sanity (NULL symbols, invalid ranges)

  //important - we need a deep copy of the ranges list, not a shared copy. This is required because
  //QgsRendererRange::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mRanges BUT that same method CAN be used to modify a symbol in place
  Q_FOREACH ( const QgsRendererRange &range, ranges )
  {
    mRanges << range;
  }

}

QgsGraduatedSymbolRenderer::~QgsGraduatedSymbolRenderer()
{
  mRanges.clear(); // should delete all the symbols
}

QgsSymbol *QgsGraduatedSymbolRenderer::symbolForValue( double value ) const
{
  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    if ( range.lowerValue() <= value && range.upperValue() >= value )
    {
      if ( range.renderState() || mCounting )
        return range.symbol();
      else
        return nullptr;
    }
  }
  // the value is out of the range: return NULL instead of symbol
  return nullptr;
}

QString QgsGraduatedSymbolRenderer::legendKeyForValue( double value ) const
{
  int i = 0;
  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    if ( range.lowerValue() <= value && range.upperValue() >= value )
    {
      if ( range.renderState() || mCounting )
        return QString::number( i );
      else
        return QString();
    }
    i++;
  }
  // the value is out of the range: return NULL
  return QString();
}

QgsSymbol *QgsGraduatedSymbolRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return originalSymbolForFeature( feature, context );
}

QVariant QgsGraduatedSymbolRenderer::valueForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QgsAttributes attrs = feature.attributes();
  QVariant value;
  if ( mAttrNum < 0 || mAttrNum >= attrs.count() )
  {
    value = mExpression->evaluate( &context.expressionContext() );
  }
  else
  {
    value = attrs.at( mAttrNum );
  }

  return value;
}

QgsSymbol *QgsGraduatedSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QVariant value = valueForFeature( feature, context );

  // Null values should not be categorized
  if ( value.isNull() )
    return nullptr;

  // find the right category
  return symbolForValue( value.toDouble() );
}

void QgsGraduatedSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  mCounting = context.rendererScale() == 0.0;

  // find out classification attribute index from name
  mAttrNum = fields.lookupField( mAttrName );

  if ( mAttrNum == -1 )
  {
    mExpression.reset( new QgsExpression( mAttrName ) );
    mExpression->prepare( &context.expressionContext() );
  }

  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    if ( !range.symbol() )
      continue;

    range.symbol()->startRender( context, fields );
  }
}

void QgsGraduatedSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    if ( !range.symbol() )
      continue;

    range.symbol()->stopRender( context );
  }
}

QSet<QString> QgsGraduatedSymbolRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes;

  // mAttrName can contain either attribute name or an expression.
  // Sometimes it is not possible to distinguish between those two,
  // e.g. "a - b" can be both a valid attribute name or expression.
  // Since we do not have access to fields here, try both options.
  attributes << mAttrName;

  QgsExpression testExpr( mAttrName );
  if ( !testExpr.hasParserError() )
    attributes.unite( testExpr.referencedColumns() );

  QgsRangeList::const_iterator range_it = mRanges.constBegin();
  for ( ; range_it != mRanges.constEnd(); ++range_it )
  {
    QgsSymbol *symbol = range_it->symbol();
    if ( symbol )
    {
      attributes.unite( symbol->usedAttributes( context ) );
    }
  }
  return attributes;
}

bool QgsGraduatedSymbolRenderer::updateRangeSymbol( int rangeIndex, QgsSymbol *symbol )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setSymbol( symbol );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeLabel( int rangeIndex, const QString &label )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setLabel( label );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeUpperValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  QgsRendererRange &range = mRanges[rangeIndex];
  bool isDefaultLabel = range.label() == mLabelFormat.labelForRange( range );
  range.setUpperValue( value );
  if ( isDefaultLabel ) range.setLabel( mLabelFormat.labelForRange( range ) );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeLowerValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  QgsRendererRange &range = mRanges[rangeIndex];
  bool isDefaultLabel = range.label() == mLabelFormat.labelForRange( range );
  range.setLowerValue( value );
  if ( isDefaultLabel ) range.setLabel( mLabelFormat.labelForRange( range ) );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeRenderState( int rangeIndex, bool value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setRenderState( value );
  return true;
}

QString QgsGraduatedSymbolRenderer::dump() const
{
  QString s = QStringLiteral( "GRADUATED: attr %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mRanges.count(); i++ )
    s += mRanges[i].dump();
  return s;
}

QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::clone() const
{
  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( mAttrName, mRanges );
  r->setMode( mMode );
  if ( mSourceSymbol )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp )
  {
    r->setSourceColorRamp( mSourceColorRamp->clone() );
  }
  r->setUsingSymbolLevels( usingSymbolLevels() );
  r->setDataDefinedSizeLegend( mDataDefinedSizeLegend ? new QgsDataDefinedSizeLegend( *mDataDefinedSizeLegend ) : nullptr );
  r->setLabelFormat( labelFormat() );
  r->setGraduatedMethod( graduatedMethod() );
  copyRendererData( r );
  return r;
}

void QgsGraduatedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  QgsStringMap newProps = props;
  newProps[ QStringLiteral( "attribute" )] = mAttrName;
  newProps[ QStringLiteral( "method" )] = graduatedMethodStr( mGraduatedMethod );

  // create a Rule for each range
  bool first = true;
  for ( QgsRangeList::const_iterator it = mRanges.constBegin(); it != mRanges.constEnd(); ++it )
  {
    it->toSld( doc, element, newProps, first );
    first = false;
  }
}

QgsSymbolList QgsGraduatedSymbolRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context );
  QgsSymbolList lst;
  lst.reserve( mRanges.count() );
  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    lst.append( range.symbol() );
  }
  return lst;
}

static QList<double> _calcEqualIntervalBreaks( double minimum, double maximum, int classes )
{

  // Equal interval algorithm
  //
  // Returns breaks based on dividing the range ('minimum' to 'maximum')
  // into 'classes' parts.

  double step = ( maximum - minimum ) / classes;

  QList<double> breaks;
  double value = minimum;
  breaks.reserve( classes );
  for ( int i = 0; i < classes; i++ )
  {
    value += step;
    breaks.append( value );
  }

  // floating point arithmetics is not precise:
  // set the last break to be exactly maximum so we do not miss it
  breaks[classes - 1] = maximum;

  return breaks;
}

static QList<double> _calcQuantileBreaks( QList<double> values, int classes )
{
  // q-th quantile of a data set:
  // value where q fraction of data is below and (1-q) fraction is above this value
  // Xq = (1 - r) * X_NI1 + r * X_NI2
  //   NI1 = (int) (q * (n+1))
  //   NI2 = NI1 + 1
  //   r = q * (n+1) - (int) (q * (n+1))
  // (indices of X: 1...n)

  // sort the values first
  std::sort( values.begin(), values.end() );

  QList<double> breaks;

  // If there are no values to process: bail out
  if ( values.isEmpty() )
    return breaks;

  int n = values.count();
  double Xq = n > 0 ? values[0] : 0.0;

  breaks.reserve( classes );
  for ( int i = 1; i < classes; i++ )
  {
    if ( n > 1 )
    {
      double q = i  / static_cast< double >( classes );
      double a = q * ( n - 1 );
      int aa = static_cast<  int >( a );

      double r = a - aa;
      Xq = ( 1 - r ) * values[aa] + r * values[aa + 1];
    }
    breaks.append( Xq );
  }

  breaks.append( values[ n - 1 ] );

  return breaks;
}

static QList<double> _calcStdDevBreaks( QList<double> values, int classes, QList<double> &labels )
{

  // C++ implementation of the standard deviation class interval algorithm
  // as implemented in the 'classInt' package available for the R statistical
  // prgramming language.

  // Returns breaks based on 'prettyBreaks' of the centred and scaled
  // values of 'values', and may have a number of classes different from 'classes'.

  // If there are no values to process: bail out
  if ( values.isEmpty() )
    return QList<double>();

  double mean = 0.0;
  double stdDev = 0.0;
  int n = values.count();
  double minimum = values[0];
  double maximum = values[0];

  for ( int i = 0; i < n; i++ )
  {
    mean += values[i];
    minimum = std::min( values[i], minimum ); // could use precomputed max and min
    maximum = std::max( values[i], maximum ); // but have to go through entire list anyway
  }
  mean = mean / static_cast< double >( n );

  double sd = 0.0;
  for ( int i = 0; i < n; i++ )
  {
    sd = values[i] - mean;
    stdDev += sd * sd;
  }
  stdDev = std::sqrt( stdDev / n );

  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( ( minimum - mean ) / stdDev, ( maximum - mean ) / stdDev, classes );
  for ( int i = 0; i < breaks.count(); i++ )
  {
    labels.append( breaks[i] );
    breaks[i] = ( breaks[i] * stdDev ) + mean;
  }

  return breaks;
} // _calcStdDevBreaks

static QList<double> _calcJenksBreaks( QList<double> values, int classes,
                                       double minimum, double maximum,
                                       int maximumSize = 3000 )
{
  // Jenks Optimal (Natural Breaks) algorithm
  // Based on the Jenks algorithm from the 'classInt' package available for
  // the R statistical prgramming language, and from Python code from here:
  // http://danieljlewis.org/2010/06/07/jenks-natural-breaks-algorithm-in-python/
  // and is based on a JAVA and Fortran code available here:
  // https://stat.ethz.ch/pipermail/r-sig-geo/2006-March/000811.html

  // Returns class breaks such that classes are internally homogeneous while
  // assuring heterogeneity among classes.

  if ( values.isEmpty() )
    return QList<double>();

  if ( classes <= 1 )
  {
    return QList<double>() << maximum;
  }

  if ( classes >= values.size() )
  {
    return values;
  }

  QVector<double> sample;

  // if we have lots of values, we need to take a random sample
  if ( values.size() > maximumSize )
  {
    // for now, sample at least maximumSize values or a 10% sample, whichever
    // is larger. This will produce a more representative sample for very large
    // layers, but could end up being computationally intensive...

    sample.resize( std::max( maximumSize, values.size() / 10 ) );

    QgsDebugMsg( QString( "natural breaks (jenks) sample size: %1" ).arg( sample.size() ) );
    QgsDebugMsg( QString( "values:%1" ).arg( values.size() ) );

    sample[ 0 ] = minimum;
    sample[ 1 ] = maximum;
    for ( int i = 2; i < sample.size(); i++ )
    {
      // pick a random integer from 0 to n
      double r = qrand();
      int j = std::floor( r / RAND_MAX * ( values.size() - 1 ) );
      sample[ i ] = values[ j ];
    }
  }
  else
  {
    sample = values.toVector();
  }

  int n = sample.size();

  // sort the sample values
  std::sort( sample.begin(), sample.end() );

  QVector< QVector<int> > matrixOne( n + 1 );
  QVector< QVector<double> > matrixTwo( n + 1 );

  for ( int i = 0; i <= n; i++ )
  {
    matrixOne[i].resize( classes + 1 );
    matrixTwo[i].resize( classes + 1 );
  }

  for ( int i = 1; i <= classes; i++ )
  {
    matrixOne[0][i] = 1;
    matrixOne[1][i] = 1;
    matrixTwo[0][i] = 0.0;
    for ( int j = 2; j <= n; j++ )
    {
      matrixTwo[j][i] = std::numeric_limits<double>::max();
    }
  }

  for ( int l = 2; l <= n; l++ )
  {
    double s1 = 0.0;
    double s2 = 0.0;
    int w = 0;

    double v = 0.0;

    for ( int m = 1; m <= l; m++ )
    {
      int i3 = l - m + 1;

      double val = sample[ i3 - 1 ];

      s2 += val * val;
      s1 += val;
      w++;

      v = s2 - ( s1 * s1 ) / static_cast< double >( w );
      int i4 = i3 - 1;
      if ( i4 != 0 )
      {
        for ( int j = 2; j <= classes; j++ )
        {
          if ( matrixTwo[l][j] >= v + matrixTwo[i4][j - 1] )
          {
            matrixOne[l][j] = i4;
            matrixTwo[l][j] = v + matrixTwo[i4][j - 1];
          }
        }
      }
    }
    matrixOne[l][1] = 1;
    matrixTwo[l][1] = v;
  }

  QVector<double> breaks( classes );
  breaks[classes - 1] = sample[n - 1];

  for ( int j = classes, k = n; j >= 2; j-- )
  {
    int id = matrixOne[k][j] - 1;
    breaks[j - 2] = sample[id];
    k = matrixOne[k][j] - 1;
  }

  return breaks.toList();
} //_calcJenksBreaks


QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::createRenderer(
  QgsVectorLayer *vlayer,
  const QString &attrName,
  int classes,
  Mode mode,
  QgsSymbol *symbol,
  QgsColorRamp *ramp,
  const QgsRendererRangeLabelFormat &labelFormat
)
{
  QgsRangeList ranges;
  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( attrName, ranges );
  r->setSourceSymbol( symbol->clone() );
  r->setSourceColorRamp( ramp->clone() );
  r->setMode( mode );
  r->setLabelFormat( labelFormat );
  r->updateClasses( vlayer, mode, classes );
  return r;
}

void QgsGraduatedSymbolRenderer::updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses )
{
  if ( mAttrName.isEmpty() )
    return;

  setMode( mode );
  // Custom classes are not recalculated
  if ( mode == Custom )
    return;

  if ( nclasses < 1 )
    nclasses = 1;

  QList<double> values;
  bool valuesLoaded = false;
  double minimum;
  double maximum;

  int attrNum = vlayer->fields().lookupField( mAttrName );

  bool ok;
  if ( attrNum == -1 )
  {
    values = QgsVectorLayerUtils::getDoubleValues( vlayer, mAttrName, ok );
    if ( !ok || values.isEmpty() )
      return;

    auto result = std::minmax_element( values.begin(), values.end() );
    minimum = *result.first;
    maximum = *result.second;
    valuesLoaded = true;
  }
  else
  {
    minimum = vlayer->minimumValue( attrNum ).toDouble();
    maximum = vlayer->maximumValue( attrNum ).toDouble();
  }

  QgsDebugMsg( QString( "min %1 // max %2" ).arg( minimum ).arg( maximum ) );
  QList<double> breaks;
  QList<double> labels;
  if ( mode == EqualInterval )
  {
    breaks = _calcEqualIntervalBreaks( minimum, maximum, nclasses );
  }
  else if ( mode == Pretty )
  {
    breaks = QgsSymbolLayerUtils::prettyBreaks( minimum, maximum, nclasses );
  }
  else if ( mode == Quantile || mode == Jenks || mode == StdDev )
  {
    // get values from layer
    if ( !valuesLoaded )
    {
      values = QgsVectorLayerUtils::getDoubleValues( vlayer, mAttrName, ok );
    }

    // calculate the breaks
    if ( mode == Quantile )
    {
      breaks = _calcQuantileBreaks( values, nclasses );
    }
    else if ( mode == Jenks )
    {
      breaks = _calcJenksBreaks( values, nclasses, minimum, maximum );
    }
    else if ( mode == StdDev )
    {
      breaks = _calcStdDevBreaks( values, nclasses, labels );
    }
  }
  else
  {
    Q_ASSERT( false );
  }

  double lower, upper = minimum;
  QString label;
  deleteAllClasses();

  // "breaks" list contains all values at class breaks plus maximum as last break

  int i = 0;
  for ( QList<double>::iterator it = breaks.begin(); it != breaks.end(); ++it, ++i )
  {
    lower = upper; // upper border from last interval
    upper = *it;

    // Label - either StdDev label or default label for a range
    if ( mode == StdDev )
    {
      if ( i == 0 )
      {
        label = "< " + QString::number( labels[i], 'f', 2 ) + " Std Dev";
      }
      else if ( i == labels.count() - 1 )
      {
        label = ">= " + QString::number( labels[i - 1], 'f', 2 ) + " Std Dev";
      }
      else
      {
        label = QString::number( labels[i - 1], 'f', 2 ) + " Std Dev" + " - " + QString::number( labels[i], 'f', 2 ) + " Std Dev";
      }
    }
    else
    {
      label = mLabelFormat.labelForRange( lower, upper );
    }
    QgsSymbol *newSymbol = mSourceSymbol ? mSourceSymbol->clone() : QgsSymbol::defaultSymbol( vlayer->geometryType() );
    addClass( QgsRendererRange( lower, upper, newSymbol, label ) );
  }
  updateColorRamp( nullptr );
}

QgsFeatureRenderer *QgsGraduatedSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement rangesElem = element.firstChildElement( QStringLiteral( "ranges" ) );
  if ( rangesElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
  QgsRangeList ranges;

  QDomElement rangeElem = rangesElem.firstChildElement();
  while ( !rangeElem.isNull() )
  {
    if ( rangeElem.tagName() == QLatin1String( "range" ) )
    {
      double lowerValue = rangeElem.attribute( QStringLiteral( "lower" ) ).toDouble();
      double upperValue = rangeElem.attribute( QStringLiteral( "upper" ) ).toDouble();
      QString symbolName = rangeElem.attribute( QStringLiteral( "symbol" ) );
      QString label = rangeElem.attribute( QStringLiteral( "label" ) );
      bool render = rangeElem.attribute( QStringLiteral( "render" ), QStringLiteral( "true" ) ) != QLatin1String( "false" );
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbol *symbol = symbolMap.take( symbolName );
        ranges.append( QgsRendererRange( lowerValue, upperValue, symbol, label, render ) );
      }
    }
    rangeElem = rangeElem.nextSiblingElement();
  }

  QString attrName = element.attribute( QStringLiteral( "attr" ) );

  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( attrName, ranges );

  QString attrMethod = element.attribute( QStringLiteral( "graduatedMethod" ) );
  if ( !attrMethod.isEmpty() )
  {
    if ( attrMethod == graduatedMethodStr( GraduatedColor ) )
      r->setGraduatedMethod( GraduatedColor );
    else if ( attrMethod == graduatedMethodStr( GraduatedSize ) )
      r->setGraduatedMethod( GraduatedSize );
  }


  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( QStringLiteral( "source-symbol" ) );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolMap sourceSymbolMap = QgsSymbolLayerUtils::loadSymbols( sourceSymbolElem, context );
    if ( sourceSymbolMap.contains( QStringLiteral( "0" ) ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( QStringLiteral( "0" ) ) );
    }
    QgsSymbolLayerUtils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  // try to load mode
  QDomElement modeElem = element.firstChildElement( QStringLiteral( "mode" ) );
  if ( !modeElem.isNull() )
  {
    QString modeString = modeElem.attribute( QStringLiteral( "name" ) );
    if ( modeString == QLatin1String( "equal" ) )
      r->setMode( EqualInterval );
    else if ( modeString == QLatin1String( "quantile" ) )
      r->setMode( Quantile );
    else if ( modeString == QLatin1String( "jenks" ) )
      r->setMode( Jenks );
    else if ( modeString == QLatin1String( "stddev" ) )
      r->setMode( StdDev );
    else if ( modeString == QLatin1String( "pretty" ) )
      r->setMode( Pretty );
  }

  QDomElement rotationElem = element.firstChildElement( QStringLiteral( "rotation" ) );
  if ( !rotationElem.isNull() && !rotationElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    Q_FOREACH ( const QgsRendererRange &range, r->mRanges )
    {
      convertSymbolRotation( range.symbol(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol )
    {
      convertSymbolRotation( r->mSourceSymbol.get(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
  }

  QDomElement sizeScaleElem = element.firstChildElement( QStringLiteral( "sizescale" ) );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    Q_FOREACH ( const QgsRendererRange &range, r->mRanges )
    {
      convertSymbolSizeScale( range.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol && r->mSourceSymbol->type() == QgsSymbol::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.get(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
  }

  QDomElement labelFormatElem = element.firstChildElement( QStringLiteral( "labelformat" ) );
  if ( ! labelFormatElem.isNull() )
  {
    QgsRendererRangeLabelFormat labelFormat;
    labelFormat.setFromDomElement( labelFormatElem );
    r->setLabelFormat( labelFormat );
  }

  QDomElement ddsLegendSizeElem = element.firstChildElement( QStringLiteral( "data-defined-size-legend" ) );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsGraduatedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "graduatedSymbol" ) );
  rendererElem.setAttribute( QStringLiteral( "symbollevels" ), ( mUsingSymbolLevels ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
  rendererElem.setAttribute( QStringLiteral( "forceraster" ), ( mForceRaster ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
  rendererElem.setAttribute( QStringLiteral( "attr" ), mAttrName );
  rendererElem.setAttribute( QStringLiteral( "graduatedMethod" ), graduatedMethodStr( mGraduatedMethod ) );

  // ranges
  int i = 0;
  QgsSymbolMap symbols;
  QDomElement rangesElem = doc.createElement( QStringLiteral( "ranges" ) );
  QgsRangeList::const_iterator it = mRanges.constBegin();
  for ( ; it != mRanges.constEnd(); ++it )
  {
    const QgsRendererRange &range = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, range.symbol() );

    QDomElement rangeElem = doc.createElement( QStringLiteral( "range" ) );
    rangeElem.setAttribute( QStringLiteral( "lower" ), QString::number( range.lowerValue(), 'f', 15 ) );
    rangeElem.setAttribute( QStringLiteral( "upper" ), QString::number( range.upperValue(), 'f', 15 ) );
    rangeElem.setAttribute( QStringLiteral( "symbol" ), symbolName );
    rangeElem.setAttribute( QStringLiteral( "label" ), range.label() );
    rangeElem.setAttribute( QStringLiteral( "render" ), range.renderState() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    rangesElem.appendChild( rangeElem );
    i++;
  }

  rendererElem.appendChild( rangesElem );

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  rendererElem.appendChild( symbolsElem );

  // save source symbol
  if ( mSourceSymbol )
  {
    QgsSymbolMap sourceSymbols;
    sourceSymbols.insert( QStringLiteral( "0" ), mSourceSymbol.get() );
    QDomElement sourceSymbolElem = QgsSymbolLayerUtils::saveSymbols( sourceSymbols, QStringLiteral( "source-symbol" ), doc, context );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mSourceColorRamp.get(), doc );
    rendererElem.appendChild( colorRampElem );
  }

  // save mode
  QString modeString;
  if ( mMode == EqualInterval )
    modeString = QStringLiteral( "equal" );
  else if ( mMode == Quantile )
    modeString = QStringLiteral( "quantile" );
  else if ( mMode == Jenks )
    modeString = QStringLiteral( "jenks" );
  else if ( mMode == StdDev )
    modeString = QStringLiteral( "stddev" );
  else if ( mMode == Pretty )
    modeString = QStringLiteral( "pretty" );
  if ( !modeString.isEmpty() )
  {
    QDomElement modeElem = doc.createElement( QStringLiteral( "mode" ) );
    modeElem.setAttribute( QStringLiteral( "name" ), modeString );
    rendererElem.appendChild( modeElem );
  }

  QDomElement rotationElem = doc.createElement( QStringLiteral( "rotation" ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( QStringLiteral( "sizescale" ) );
  rendererElem.appendChild( sizeScaleElem );

  QDomElement labelFormatElem = doc.createElement( QStringLiteral( "labelformat" ) );
  mLabelFormat.saveToDomElement( labelFormatElem );
  rendererElem.appendChild( labelFormatElem );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( QStringLiteral( "orderby" ) );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( QStringLiteral( "enableorderby" ), ( mOrderByEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( QStringLiteral( "data-defined-size-legend" ) );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  return rendererElem;
}

QgsLegendSymbolList QgsGraduatedSymbolRenderer::baseLegendSymbolItems() const
{
  QgsLegendSymbolList lst;
  int i = 0;
  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    lst << QgsLegendSymbolItem( range.symbol(), range.label(), QString::number( i++ ), true );
  }
  return lst;
}

QgsLegendSymbolList QgsGraduatedSymbolRenderer::legendSymbolItems() const
{
  if ( mDataDefinedSizeLegend && mSourceSymbol && mSourceSymbol->type() == QgsSymbol::Marker )
  {
    // check that all symbols that have the same size expression
    QgsProperty ddSize;
    Q_FOREACH ( const QgsRendererRange &range, mRanges )
    {
      const QgsMarkerSymbol *symbol = static_cast<const QgsMarkerSymbol *>( range.symbol() );
      if ( ddSize )
      {
        QgsProperty sSize( symbol->dataDefinedSize() );
        if ( sSize && sSize != ddSize )
        {
          // no common size expression
          return baseLegendSymbolItems();
        }
      }
      else
      {
        ddSize = symbol->dataDefinedSize();
      }
    }

    if ( ddSize && ddSize.isActive() )
    {
      QgsLegendSymbolList lst;

      QgsDataDefinedSizeLegend ddSizeLegend( *mDataDefinedSizeLegend );
      ddSizeLegend.updateFromSymbolAndProperty( static_cast<const QgsMarkerSymbol *>( mSourceSymbol.get() ), ddSize );
      lst += ddSizeLegend.legendSymbolList();

      lst += baseLegendSymbolItems();
      return lst;
    }
  }

  return baseLegendSymbolItems();
}

QSet< QString > QgsGraduatedSymbolRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QVariant value = valueForFeature( feature, context );

  // Null values should not be categorized
  if ( value.isNull() )
    return QSet< QString >();

  // find the right category
  QString key = legendKeyForValue( value.toDouble() );
  if ( !key.isNull() )
    return QSet< QString >() << key;
  else
    return QSet< QString >();
}

QgsSymbol *QgsGraduatedSymbolRenderer::sourceSymbol()
{
  return mSourceSymbol.get();
}
void QgsGraduatedSymbolRenderer::setSourceSymbol( QgsSymbol *sym )
{
  mSourceSymbol.reset( sym );
}

QgsColorRamp *QgsGraduatedSymbolRenderer::sourceColorRamp()
{
  return mSourceColorRamp.get();
}

void QgsGraduatedSymbolRenderer::setSourceColorRamp( QgsColorRamp *ramp )
{
  if ( ramp == mSourceColorRamp.get() )
    return;

  mSourceColorRamp.reset( ramp );
}

double QgsGraduatedSymbolRenderer::minSymbolSize() const
{
  double min = std::numeric_limits<double>::max();
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    double sz = 0;
    if ( mRanges[i].symbol()->type() == QgsSymbol::Marker )
      sz = static_cast< QgsMarkerSymbol * >( mRanges[i].symbol() )->size();
    else if ( mRanges[i].symbol()->type() == QgsSymbol::Line )
      sz = static_cast< QgsLineSymbol * >( mRanges[i].symbol() )->width();
    min = std::min( sz, min );
  }
  return min;
}

double QgsGraduatedSymbolRenderer::maxSymbolSize() const
{
  double max = std::numeric_limits<double>::min();
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    double sz = 0;
    if ( mRanges[i].symbol()->type() == QgsSymbol::Marker )
      sz = static_cast< QgsMarkerSymbol * >( mRanges[i].symbol() )->size();
    else if ( mRanges[i].symbol()->type() == QgsSymbol::Line )
      sz = static_cast< QgsLineSymbol * >( mRanges[i].symbol() )->width();
    max = std::max( sz, max );
  }
  return max;
}

void QgsGraduatedSymbolRenderer::setSymbolSizes( double minSize, double maxSize )
{
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    std::unique_ptr<QgsSymbol> symbol( mRanges.at( i ).symbol() ? mRanges.at( i ).symbol()->clone() : nullptr );
    const double size = mRanges.count() > 1
                        ? minSize + i * ( maxSize - minSize ) / ( mRanges.count() - 1 )
                        : .5 * ( maxSize + minSize );
    if ( symbol->type() == QgsSymbol::Marker )
      static_cast< QgsMarkerSymbol * >( symbol.get() )->setSize( size );
    if ( symbol->type() == QgsSymbol::Line )
      static_cast< QgsLineSymbol * >( symbol.get() )->setWidth( size );
    updateRangeSymbol( i, symbol.release() );
  }
}

void QgsGraduatedSymbolRenderer::updateColorRamp( QgsColorRamp *ramp )
{
  int i = 0;
  if ( ramp )
  {
    setSourceColorRamp( ramp );
  }

  if ( mSourceColorRamp )
  {
    Q_FOREACH ( const QgsRendererRange &range, mRanges )
    {
      QgsSymbol *symbol = range.symbol() ? range.symbol()->clone() : nullptr;
      if ( symbol )
      {
        double colorValue;
        colorValue = ( mRanges.count() > 1 ? static_cast< double >( i ) / ( mRanges.count() - 1 ) : 0 );
        symbol->setColor( mSourceColorRamp->color( colorValue ) );
      }
      updateRangeSymbol( i, symbol );
      ++i;
    }
  }

}

void QgsGraduatedSymbolRenderer::updateSymbols( QgsSymbol *sym )
{
  if ( !sym )
    return;

  int i = 0;
  Q_FOREACH ( const QgsRendererRange &range, mRanges )
  {
    std::unique_ptr<QgsSymbol> symbol( sym->clone() );
    if ( mGraduatedMethod == GraduatedColor )
    {
      symbol->setColor( range.symbol()->color() );
    }
    else if ( mGraduatedMethod == GraduatedSize )
    {
      if ( symbol->type() == QgsSymbol::Marker )
        static_cast<QgsMarkerSymbol *>( symbol.get() )->setSize(
          static_cast<QgsMarkerSymbol *>( range.symbol() )->size() );
      else if ( symbol->type() == QgsSymbol::Line )
        static_cast<QgsLineSymbol *>( symbol.get() )->setWidth(
          static_cast<QgsLineSymbol *>( range.symbol() )->width() );
    }
    updateRangeSymbol( i, symbol.release() );
    ++i;
  }
  setSourceSymbol( sym->clone() );
}

bool QgsGraduatedSymbolRenderer::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsGraduatedSymbolRenderer::legendSymbolItemChecked( const QString &key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mRanges.size() )
    return mRanges.at( index ).renderState();
  else
    return true;
}

void QgsGraduatedSymbolRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateRangeRenderState( index, state );
}

void QgsGraduatedSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateRangeSymbol( index, symbol );
  else
    delete symbol;
}

void QgsGraduatedSymbolRenderer::addClass( QgsSymbol *symbol )
{
  QgsSymbol *newSymbol = symbol->clone();
  QString label = QStringLiteral( "0.0 - 0.0" );
  mRanges.insert( 0, QgsRendererRange( 0.0, 0.0, newSymbol, label ) );
}

void QgsGraduatedSymbolRenderer::addClass( double lower, double upper )
{
  QgsSymbol *newSymbol = mSourceSymbol->clone();
  QString label = mLabelFormat.labelForRange( lower, upper );
  mRanges.append( QgsRendererRange( lower, upper, newSymbol, label ) );
}

void QgsGraduatedSymbolRenderer::addBreak( double breakValue, bool updateSymbols )
{
  QMutableListIterator< QgsRendererRange > it( mRanges );
  while ( it.hasNext() )
  {
    QgsRendererRange range = it.next();
    if ( range.lowerValue() < breakValue && range.upperValue() > breakValue )
    {
      QgsRendererRange newRange = QgsRendererRange();
      newRange.setLowerValue( breakValue );
      newRange.setUpperValue( range.upperValue() );
      newRange.setLabel( mLabelFormat.labelForRange( newRange ) );
      newRange.setSymbol( mSourceSymbol->clone() );

      //update old range
      bool isDefaultLabel = range.label() == mLabelFormat.labelForRange( range );
      range.setUpperValue( breakValue );
      if ( isDefaultLabel ) range.setLabel( mLabelFormat.labelForRange( range.lowerValue(), breakValue ) );
      it.setValue( range );

      it.insert( newRange );
      break;
    }
  }

  if ( updateSymbols )
  {
    switch ( mGraduatedMethod )
    {
      case GraduatedColor:
        updateColorRamp( mSourceColorRamp.get() );
        break;
      case GraduatedSize:
        setSymbolSizes( minSymbolSize(), maxSymbolSize() );
        break;
    }
  }
}

void QgsGraduatedSymbolRenderer::addClass( const QgsRendererRange &range )
{
  mRanges.append( range );
}

void QgsGraduatedSymbolRenderer::deleteClass( int idx )
{
  mRanges.removeAt( idx );
}

void QgsGraduatedSymbolRenderer::deleteAllClasses()
{
  mRanges.clear();
}

void QgsGraduatedSymbolRenderer::setLabelFormat( const QgsRendererRangeLabelFormat &labelFormat, bool updateRanges )
{
  if ( updateRanges && labelFormat != mLabelFormat )
  {
    for ( QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it )
    {
      it->setLabel( labelFormat.labelForRange( *it ) );
    }
  }
  mLabelFormat = labelFormat;
}


void QgsGraduatedSymbolRenderer::calculateLabelPrecision( bool updateRanges )
{
  // Find the minimum size of a class
  double minClassRange = 0.0;
  Q_FOREACH ( const QgsRendererRange &rendererRange, mRanges )
  {
    double range = rendererRange.upperValue() - rendererRange.lowerValue();
    if ( range <= 0.0 )
      continue;
    if ( minClassRange == 0.0 || range < minClassRange )
      minClassRange = range;
  }
  if ( minClassRange <= 0.0 )
    return;

  // Now set the number of decimal places to ensure no more than 20% error in
  // representing this range (up to 10% at upper and lower end)

  int ndp = 10;
  double nextDpMinRange = 0.0000000099;
  while ( ndp > 0 && nextDpMinRange < minClassRange )
  {
    ndp--;
    nextDpMinRange *= 10.0;
  }
  mLabelFormat.setPrecision( ndp );
  if ( updateRanges ) setLabelFormat( mLabelFormat, true );
}

void QgsGraduatedSymbolRenderer::moveClass( int from, int to )
{
  if ( from < 0 || from >= mRanges.size() || to < 0 || to >= mRanges.size() )
    return;
  mRanges.move( from, to );
}

bool valueLessThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return r1 < r2;
}

bool valueGreaterThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return !valueLessThan( r1, r2 );
}

void QgsGraduatedSymbolRenderer::sortByValue( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    std::sort( mRanges.begin(), mRanges.end(), valueLessThan );
  }
  else
  {
    std::sort( mRanges.begin(), mRanges.end(), valueGreaterThan );
  }
}

bool QgsGraduatedSymbolRenderer::rangesOverlap() const
{
  QgsRangeList sortedRanges = mRanges;
  std::sort( sortedRanges.begin(), sortedRanges.end(), valueLessThan );

  QgsRangeList::const_iterator it = sortedRanges.constBegin();
  if ( it == sortedRanges.constEnd() )
    return false;

  if ( ( *it ).upperValue() < ( *it ).lowerValue() )
    return true;

  double prevMax = ( *it ).upperValue();
  ++it;

  for ( ; it != sortedRanges.constEnd(); ++it )
  {
    if ( ( *it ).upperValue() < ( *it ).lowerValue() )
      return true;

    if ( ( *it ).lowerValue() < prevMax )
      return true;

    prevMax = ( *it ).upperValue();
  }
  return false;
}

bool QgsGraduatedSymbolRenderer::rangesHaveGaps() const
{
  QgsRangeList sortedRanges = mRanges;
  std::sort( sortedRanges.begin(), sortedRanges.end(), valueLessThan );

  QgsRangeList::const_iterator it = sortedRanges.constBegin();
  if ( it == sortedRanges.constEnd() )
    return false;

  double prevMax = ( *it ).upperValue();
  ++it;

  for ( ; it != sortedRanges.constEnd(); ++it )
  {
    if ( !qgsDoubleNear( ( *it ).lowerValue(), prevMax ) )
      return true;

    prevMax = ( *it ).upperValue();
  }
  return false;
}

bool labelLessThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return QString::localeAwareCompare( r1.label(), r2.label() ) < 0;
}

bool labelGreaterThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return !labelLessThan( r1, r2 );
}

void QgsGraduatedSymbolRenderer::sortByLabel( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    std::sort( mRanges.begin(), mRanges.end(), labelLessThan );
  }
  else
  {
    std::sort( mRanges.begin(), mRanges.end(), labelGreaterThan );
  }
}

QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  QgsGraduatedSymbolRenderer *r = nullptr;
  if ( renderer->type() == QLatin1String( "graduatedSymbol" ) )
  {
    r = dynamic_cast<QgsGraduatedSymbolRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == QLatin1String( "pointDisplacement" ) || renderer->type() == QLatin1String( "pointCluster" ) )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r = convertFromRenderer( pointDistanceRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer );
    if ( invertedPolygonRenderer )
      r = convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  // If not one of the specifically handled renderers, then just grab the symbol from the renderer
  // Could have applied this to specific renderer types (singleSymbol, graduatedSymbol)

  if ( !r )
  {
    r = new QgsGraduatedSymbolRenderer( QString(), QgsRangeList() );
    QgsRenderContext context;
    QgsSymbolList symbols = const_cast<QgsFeatureRenderer *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r->setSourceSymbol( symbols.at( 0 )->clone() );
    }
  }

  r->setOrderBy( renderer->orderBy() );
  r->setOrderByEnabled( renderer->orderByEnabled() );

  return r;
}

void QgsGraduatedSymbolRenderer::setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings )
{
  mDataDefinedSizeLegend.reset( settings );
}

QgsDataDefinedSizeLegend *QgsGraduatedSymbolRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend.get();
}

const char *QgsGraduatedSymbolRenderer::graduatedMethodStr( GraduatedMethod method )
{
  switch ( method )
  {
    case GraduatedColor:
      return "GraduatedColor";
    case GraduatedSize:
      return "GraduatedSize";
  }
  return "";
}


