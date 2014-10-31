
/***************************************************************************
    qgsgraduatedsymbolrendererv2.cpp
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
#include "qgsgraduatedsymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend
#include <limits> // for jenks classification
#include <cmath> // for pretty classification
#include <ctime>

QgsRendererRangeV2::QgsRendererRangeV2()
    : mLowerValue( 0 )
    , mUpperValue( 0 )
    , mSymbol( 0 )
    , mLabel()
{
}

QgsRendererRangeV2::QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label, bool render )
    : mLowerValue( lowerValue )
    , mUpperValue( upperValue )
    , mSymbol( symbol )
    , mLabel( label )
    , mRender( render )
{
}

QgsRendererRangeV2::QgsRendererRangeV2( const QgsRendererRangeV2& range )
    : mLowerValue( range.mLowerValue )
    , mUpperValue( range.mUpperValue )
    , mSymbol( range.mSymbol.data() ? range.mSymbol->clone() : NULL )
    , mLabel( range.mLabel )
    , mRender( range.mRender )
{
}

// cpy and swap idiom, note that the cpy is done with 'pass by value'
QgsRendererRangeV2& QgsRendererRangeV2::operator=( QgsRendererRangeV2 range )
{
  swap( range );
  return *this;
}

bool QgsRendererRangeV2::operator<( const QgsRendererRangeV2 &other ) const
{
  return
    lowerValue() < other.lowerValue() ||
    ( lowerValue() == other.lowerValue() && upperValue() < other.upperValue() );
}


void QgsRendererRangeV2::swap( QgsRendererRangeV2 & other )
{
  qSwap( mLowerValue, other.mLowerValue );
  qSwap( mUpperValue, other.mUpperValue );
  qSwap( mSymbol, other.mSymbol );
  std::swap( mLabel, other.mLabel );
}

double QgsRendererRangeV2::lowerValue() const
{
  return mLowerValue;
}

double QgsRendererRangeV2::upperValue() const
{
  return mUpperValue;
}

QgsSymbolV2* QgsRendererRangeV2::symbol() const
{
  return mSymbol.data();
}

QString QgsRendererRangeV2::label() const
{
  return mLabel;
}

void QgsRendererRangeV2::setSymbol( QgsSymbolV2* s )
{
  if ( mSymbol.data() != s ) mSymbol.reset( s );
}

void QgsRendererRangeV2::setLabel( QString label )
{
  mLabel = label;
}

void QgsRendererRangeV2::setUpperValue( double upperValue )
{
  mUpperValue = upperValue;
}

void QgsRendererRangeV2::setLowerValue( double lowerValue )
{
  mLowerValue = lowerValue;
}

bool QgsRendererRangeV2::renderState() const
{
  return mRender;
}

void QgsRendererRangeV2::setRenderState( bool render )
{
  mRender = render;
}

QString QgsRendererRangeV2::dump() const
{
  return QString( "%1 - %2::%3::%4\n" ).arg( mLowerValue ).arg( mUpperValue ).arg( mLabel ).arg( mSymbol.data() ? mSymbol->dump() : "(no symbol)" );
}

void QgsRendererRangeV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  if ( !mSymbol.data() || props.value( "attribute", "" ).isEmpty() )
    return;

  QString attrName = props[ "attribute" ];

  QDomElement ruleElem = doc.createElement( "se:Rule" );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( mLabel ) );
  ruleElem.appendChild( nameElem );

  QDomElement descrElem = doc.createElement( "se:Description" );
  QDomElement titleElem = doc.createElement( "se:Title" );
  QString descrStr = QString( "range: %1 - %2" ).arg( mLowerValue ).arg( mUpperValue );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc = QString( "%1 > %2 AND %1 <= %3" )
                       .arg( attrName.replace( "\"", "\"\"" ) )
                       .arg( mLowerValue ).arg( mUpperValue );
  QgsSymbolLayerV2Utils::createFunctionElement( doc, ruleElem, filterFunc );

  mSymbol->toSld( doc, ruleElem, props );
}

///////////

int QgsRendererRangeV2LabelFormat::MaxPrecision = 15;
int QgsRendererRangeV2LabelFormat::MinPrecision = -6;

QgsRendererRangeV2LabelFormat::QgsRendererRangeV2LabelFormat():
    mFormat( " %1 - %2 " ),
    mPrecision( 4 ),
    mTrimTrailingZeroes( false ),
    mNumberScale( 1.0 ),
    mNumberSuffix( "" ),
    mReTrailingZeroes( "[.,]?0*$" ),
    mReNegativeZero( "^\\-0(?:[.,]0*)?$" )
{
}

QgsRendererRangeV2LabelFormat::QgsRendererRangeV2LabelFormat( QString format, int precision, bool trimTrailingZeroes ):
    mReTrailingZeroes( "[.,]?0*$" ),
    mReNegativeZero( "^\\-0(?:[.,]0*)?$" )
{
  setFormat( format );
  setPrecision( precision );
  setTrimTrailingZeroes( trimTrailingZeroes );
}


bool QgsRendererRangeV2LabelFormat::operator==( const QgsRendererRangeV2LabelFormat &other ) const
{
  return
    format() == other.format() &&
    precision() == other.precision() &&
    trimTrailingZeroes() == other.trimTrailingZeroes();
}

bool QgsRendererRangeV2LabelFormat::operator!=( const QgsRendererRangeV2LabelFormat &other ) const
{
  return !( *this == other );
}

void QgsRendererRangeV2LabelFormat::setPrecision( int precision )
{
  // Limit the range of decimal places to a reasonable range
  precision = qBound( MinPrecision, precision, MaxPrecision );
  mPrecision = precision;
  mNumberScale = 1.0;
  mNumberSuffix = "";
  while ( precision < 0 )
  {
    precision++;
    mNumberScale /= 10.0;
    mNumberSuffix.append( '0' );
  }
}

QString QgsRendererRangeV2LabelFormat::labelForRange( const QgsRendererRangeV2 &range ) const
{
  return labelForRange( range.lowerValue(), range.upperValue() );
}

QString QgsRendererRangeV2LabelFormat::formatNumber( double value ) const
{
  if ( mPrecision > 0 )
  {
    QString valueStr = QString::number( value, 'f', mPrecision );
    if ( mTrimTrailingZeroes )
      valueStr = valueStr.replace( mReTrailingZeroes, "" );
    if ( mReNegativeZero.exactMatch( valueStr ) )
      valueStr = valueStr.mid( 1 );
    return valueStr;
  }
  else
  {
    QString valueStr = QString::number( value * mNumberScale, 'f', 0 );
    if ( valueStr == "-0" )
      valueStr = "0";
    if ( valueStr != "0" )
      valueStr = valueStr + mNumberSuffix;
    return valueStr;
  }
}

QString QgsRendererRangeV2LabelFormat::labelForRange( double lower, double upper ) const
{
  QString lowerStr = formatNumber( lower );
  QString upperStr = formatNumber( upper );

  QString legend( mFormat );
  return legend.replace( "%1", lowerStr ).replace( "%2", upperStr );
}

void QgsRendererRangeV2LabelFormat::setFromDomElement( QDomElement &element )
{
  mFormat = element.attribute( "format",
                               element.attribute( "prefix", " " ) + "%1" +
                               element.attribute( "separator", " - " ) + "%2" +
                               element.attribute( "suffix", " " )
                             );
  setPrecision( element.attribute( "decimalplaces", "4" ).toInt() );
  mTrimTrailingZeroes = element.attribute( "trimtrailingzeroes", "false" ) == "true";
}

void QgsRendererRangeV2LabelFormat::saveToDomElement( QDomElement &element )
{
  element.setAttribute( "format", mFormat );
  element.setAttribute( "decimalplaces", mPrecision );
  element.setAttribute( "trimtrailingzeroes", mTrimTrailingZeroes ? "true" : "false" );
}

///////////

QgsGraduatedSymbolRendererV2::QgsGraduatedSymbolRendererV2( QString attrName, QgsRangeList ranges )
    : QgsFeatureRendererV2( "graduatedSymbol" )
    , mAttrName( attrName )
    , mRanges( ranges )
    , mMode( Custom )
    , mInvertedColorRamp( false )
    , mScaleMethod( DEFAULT_SCALE_METHOD )
{
  // TODO: check ranges for sanity (NULL symbols, invalid ranges)
}

QgsGraduatedSymbolRendererV2::~QgsGraduatedSymbolRendererV2()
{
  mRanges.clear(); // should delete all the symbols
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForValue( double value )
{
  for ( QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it )
  {
    if ( it->lowerValue() <= value && it->upperValue() >= value )
    {
      if ( it->renderState() || mCounting )
        return it->symbol();
      else
        return NULL;
    }
  }
  // the value is out of the range: return NULL instead of symbol
  return NULL;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  QgsSymbolV2* symbol = originalSymbolForFeature( feature );
  if ( symbol == NULL )
    return NULL;

  if ( !mRotation.data() && !mSizeScale.data() )
    return symbol; // no data-defined rotation/scaling - just return the symbol

  // find out rotation, size scale
  const double rotation = mRotation.data() ? mRotation->evaluate( feature ).toDouble() : 0;
  const double sizeScale = mSizeScale.data() ? mSizeScale->evaluate( feature ).toDouble() : 1.;

  // take a temporary symbol (or create it if doesn't exist)
  QgsSymbolV2* tempSymbol = mTempSymbols[symbol];

  // modify the temporary symbol and return it
  if ( tempSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( tempSymbol );
    if ( mRotation.data() ) markerSymbol->setAngle( rotation );
    markerSymbol->setSize( sizeScale * static_cast<QgsMarkerSymbolV2*>( symbol )->size() );
    markerSymbol->setScaleMethod( mScaleMethod );
  }
  else if ( tempSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( tempSymbol );
    lineSymbol->setWidth( sizeScale * static_cast<QgsLineSymbolV2*>( symbol )->width() );
  }
  return tempSymbol;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::originalSymbolForFeature( QgsFeature& feature )
{
  const QgsAttributes& attrs = feature.attributes();
  QVariant value;
  if ( mAttrNum < 0 || mAttrNum >= attrs.count() )
  {
    value = mExpression->evaluate( &feature );
  }
  else
  {
    value = attrs[mAttrNum];
  }

  // Null values should not be categorized
  if ( value.isNull() )
    return NULL;

  // find the right category
  return symbolForValue( value.toDouble() );
}

void QgsGraduatedSymbolRendererV2::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  mCounting = context.rendererScale() == 0.0;

  // find out classification attribute index from name
  mAttrNum = fields.fieldNameIndex( mAttrName );

  if ( mAttrNum == -1 )
  {
    mExpression.reset( new QgsExpression( mAttrName ) );
    mExpression->prepare( fields );
  }

  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it )
  {
    if( !it->symbol() )
      continue;

    it->symbol()->startRender( context, &fields );

    if ( mRotation.data() || mSizeScale.data() )
    {
      QgsSymbolV2* tempSymbol = it->symbol()->clone();
      tempSymbol->setRenderHints(( mRotation.data() ? QgsSymbolV2::DataDefinedRotation : 0 ) |
                                 ( mSizeScale.data() ? QgsSymbolV2::DataDefinedSizeScale : 0 ) );
      tempSymbol->startRender( context, &fields );
      mTempSymbols[ it->symbol()] = tempSymbol;
    }
  }
}

void QgsGraduatedSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it )
  {
    if( !it->symbol() )
      continue;

    it->symbol()->stopRender( context );
  }

  // cleanup mTempSymbols
  QHash<QgsSymbolV2*, QgsSymbolV2*>::iterator it2 = mTempSymbols.begin();
  for ( ; it2 != mTempSymbols.end(); ++it2 )
  {
    it2.value()->stopRender( context );
    delete it2.value();
  }
  mTempSymbols.clear();
}

QList<QString> QgsGraduatedSymbolRendererV2::usedAttributes()
{
  QSet<QString> attributes;

  // mAttrName can contain either attribute name or an expression.
  // Sometimes it is not possible to distinguish between those two,
  // e.g. "a - b" can be both a valid attribute name or expression.
  // Since we do not have access to fields here, try both options.
  attributes << mAttrName;

  QgsExpression testExpr( mAttrName );
  if ( !testExpr.hasParserError() )
    attributes.unite( testExpr.referencedColumns().toSet() );

  if ( mRotation.data() ) attributes.unite( mRotation->referencedColumns().toSet() );
  if ( mSizeScale.data() ) attributes.unite( mSizeScale->referencedColumns().toSet() );

  QgsRangeList::const_iterator range_it = mRanges.constBegin();
  for ( ; range_it != mRanges.constEnd(); ++range_it )
  {
    QgsSymbolV2* symbol = range_it->symbol();
    if ( symbol )
    {
      attributes.unite( symbol->usedAttributes() );
    }
  }
  return attributes.toList();
}

bool QgsGraduatedSymbolRendererV2::updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setSymbol( symbol );
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeLabel( int rangeIndex, QString label )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setLabel( label );
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeUpperValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  QgsRendererRangeV2 &range = mRanges[rangeIndex];
  bool isDefaultLabel = range.label() == mLabelFormat.labelForRange( range );
  range.setUpperValue( value );
  if ( isDefaultLabel ) range.setLabel( mLabelFormat.labelForRange( range ) );
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeLowerValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  QgsRendererRangeV2 &range = mRanges[rangeIndex];
  bool isDefaultLabel = range.label() == mLabelFormat.labelForRange( range );
  range.setLowerValue( value );
  if ( isDefaultLabel ) range.setLabel( mLabelFormat.labelForRange( range ) );
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeRenderState( int rangeIndex, bool value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setRenderState( value );
  return true;
}

QString QgsGraduatedSymbolRendererV2::dump() const
{
  QString s = QString( "GRADUATED: attr %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mRanges.count(); i++ )
    s += mRanges[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2::clone() const
{
  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( mAttrName, mRanges );
  r->setMode( mMode );
  if ( mSourceSymbol.data() )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp.data() )
  {
    r->setSourceColorRamp( mSourceColorRamp->clone() );
    r->setInvertedColorRamp( mInvertedColorRamp );
  }
  r->setUsingSymbolLevels( usingSymbolLevels() );
  r->setRotationField( rotationField() );
  r->setSizeScaleField( sizeScaleField() );
  r->setScaleMethod( scaleMethod() );
  r->setLabelFormat( labelFormat() );
  return r;
}

void QgsGraduatedSymbolRendererV2::toSld( QDomDocument& doc, QDomElement &element ) const
{
  QgsStringMap props;
  props[ "attribute" ] = mAttrName;
  if ( mRotation.data() )
    props[ "angle" ] = mRotation->expression();
  if ( mSizeScale.data() )
    props[ "scale" ] = mSizeScale->expression();

  // create a Rule for each range
  for ( QgsRangeList::const_iterator it = mRanges.constBegin(); it != mRanges.constEnd(); ++it )
  {
    QgsStringMap catProps( props );
    it->toSld( doc, element, catProps );
  }
}

QgsSymbolV2List QgsGraduatedSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  for ( int i = 0; i < mRanges.count(); i++ )
    lst.append( mRanges[i].symbol() );
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
  for ( int i = 0; i < classes; i++ )
  {
    value += step;
    breaks.append( value );
  }

  // floating point arithmetics is not precise:
  // set the last break to be exactly maximum so we do not miss it
  breaks[classes-1] = maximum;

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
  qSort( values );

  QList<double> breaks;

  // If there are no values to process: bail out
  if ( !values.count() )
    return breaks;

  int n = values.count();
  double Xq = n > 0 ? values[0] : 0.0;

  for ( int i = 1; i < classes; i++ )
  {
    if ( n > 1 )
    {
      double q = i  / ( double ) classes;
      double a = q * ( n - 1 );
      int aa = ( int )( a );

      double r = a - aa;
      Xq = ( 1 - r ) * values[aa] + r * values[aa+1];
    }
    breaks.append( Xq );
  }

  breaks.append( values[ n-1 ] );

  return breaks;
}

static QList<double> _calcPrettyBreaks( double minimum, double maximum, int classes )
{

  // C++ implementation of R's pretty algorithm
  // Based on code for determining optimal tick placement for statistical graphics
  // from the R statistical programming language.
  // Code ported from R implementation from 'labeling' R package
  //
  // Computes a sequence of about 'classes' equally spaced round values
  // which cover the range of values from 'minimum' to 'maximum'.
  // The values are chosen so that they are 1, 2 or 5 times a power of 10.

  QList<double> breaks;
  if ( classes < 1 )
  {
    breaks.append( maximum );
    return breaks;
  }

  int minimumCount = ( int ) classes / 3;
  double shrink = 0.75;
  double highBias = 1.5;
  double adjustBias = 0.5 + 1.5 * highBias;
  int divisions = classes;
  double h = highBias;
  double cell;
  int U;
  bool small = false;
  double dx = maximum - minimum;

  if ( dx == 0 && maximum == 0 )
  {
    cell = 1.0;
    small = true;
    U = 1;
  }
  else
  {
    cell = qMax( qAbs( minimum ), qAbs( maximum ) );
    if ( adjustBias >= 1.5 * h + 0.5 )
    {
      U = 1 + ( 1.0 / ( 1 + h ) );
    }
    else
    {
      U = 1 + ( 1.5 / ( 1 + adjustBias ) );
    }
    small = dx < ( cell * U * qMax( 1, divisions ) * 1e-07 * 3.0 );
  }

  if ( small )
  {
    if ( cell > 10 )
    {
      cell = 9 + cell / 10;
      cell = cell * shrink;
    }
    if ( minimumCount > 1 )
    {
      cell = cell / minimumCount;
    }
  }
  else
  {
    cell = dx;
    if ( divisions > 1 )
    {
      cell = cell / divisions;
    }
  }
  if ( cell < 20 * 1e-07 )
  {
    cell = 20 * 1e-07;
  }

  double base = pow( 10.0, floor( log10( cell ) ) );
  double unit = base;
  if (( 2 * base ) - cell < h *( cell - unit ) )
  {
    unit = 2.0 * base;
    if (( 5 * base ) - cell < adjustBias *( cell - unit ) )
    {
      unit = 5.0 * base;
      if (( 10.0 * base ) - cell < h *( cell - unit ) )
      {
        unit = 10.0 * base;
      }
    }
  }
  // Maybe used to correct for the epsilon here??
  int start = floor( minimum / unit + 1e-07 );
  int end = ceil( maximum / unit - 1e-07 );

  // Extend the range out beyond the data. Does this ever happen??
  while ( start * unit > minimum + ( 1e-07 * unit ) )
  {
    start = start - 1;
  }
  while ( end * unit < maximum - ( 1e-07 * unit ) )
  {
    end = end + 1;
  }
  QgsDebugMsg( QString( "pretty classes: %1" ).arg( end ) );

  // If we don't have quite enough labels, extend the range out
  // to make more (these labels are beyond the data :( )
  int k = floor( 0.5 + end - start );
  if ( k < minimumCount )
  {
    k = minimumCount - k;
    if ( start >= 0 )
    {
      end = end + k / 2;
      start = start - k / 2 + k % 2;
    }
    else
    {
      start = start - k / 2;
      end = end + k / 2 + k % 2;
    }
  }
  double minimumBreak = start * unit;
  //double maximumBreak = end * unit;
  int count = end - start;

  for ( int i = 1; i < count + 1; i++ )
  {
    breaks.append( minimumBreak + i * unit );
  }

  if ( breaks.isEmpty() )
    return breaks;

  if ( breaks.first() < minimum )
  {
    breaks[0] = minimum;
  }
  if ( breaks.last() > maximum )
  {
    breaks[breaks.count()-1] = maximum;
  }

  return breaks;
} // _calcPrettyBreaks


static QList<double> _calcStdDevBreaks( QList<double> values, int classes, QList<double> &labels )
{

  // C++ implementation of the standard deviation class interval algorithm
  // as implemented in the 'classInt' package available for the R statistical
  // prgramming language.

  // Returns breaks based on '_calcPrettyBreaks' of the centred and scaled
  // values of 'values', and may have a number of classes different from 'classes'.

  // If there are no values to process: bail out
  if ( !values.count() )
    return QList<double>();

  double mean = 0.0;
  double stdDev = 0.0;
  int n = values.count();
  double minimum = values[0];
  double maximum = values[0];

  for ( int i = 0; i < n; i++ )
  {
    mean += values[i];
    minimum = qMin( values[i], minimum ); // could use precomputed max and min
    maximum = qMax( values[i], maximum ); // but have to go through entire list anyway
  }
  mean = mean / ( double ) n;

  double sd = 0.0;
  for ( int i = 0; i < n; i++ )
  {
    sd = values[i] - mean;
    stdDev += sd * sd;
  }
  stdDev = sqrt( stdDev / n );

  QList<double> breaks = _calcPrettyBreaks(( minimum - mean ) / stdDev, ( maximum - mean ) / stdDev, classes );
  for ( int i = 0; i < breaks.count(); i++ )
  {
    labels.append( breaks[i] );
    breaks[i] = ( breaks[i] * stdDev ) + mean;
  }

  return breaks;
} // _calcStdDevBreaks

static QList<double> _calcJenksBreaks( QList<double> values, int classes,
                                       double minimum, double maximum,
                                       int maximumSize = 1000 )
{
  // Jenks Optimal (Natural Breaks) algorithm
  // Based on the Jenks algorithm from the 'classInt' package available for
  // the R statistical prgramming language, and from Python code from here:
  // http://danieljlewis.org/2010/06/07/jenks-natural-breaks-algorithm-in-python/
  // and is based on a JAVA and Fortran code available here:
  // https://stat.ethz.ch/pipermail/r-sig-geo/2006-March/000811.html

  // Returns class breaks such that classes are internally homogeneous while
  // assuring heterogeneity among classes.

  if ( !values.count() )
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

    qsrand( time( 0 ) );

    sample.resize( qMax( maximumSize, values.size() / 10 ) );

    QgsDebugMsg( QString( "natural breaks (jenks) sample size: %1" ).arg( sample.size() ) );
    QgsDebugMsg( QString( "values:%1" ).arg( values.size() ) );

    sample[ 0 ] = minimum;
    sample[ 1 ] = maximum;;
    for ( int i = 2; i < sample.size(); i++ )
    {
      // pick a random integer from 0 to n
      double r = qrand();
      int j = floor( r / RAND_MAX * ( values.size() - 1 ) );
      sample[ i ] = values[ j ];
    }
  }
  else
  {
    sample = values.toVector();
  }

  int n = sample.size();

  // sort the sample values
  qSort( sample );

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

      v = s2 - ( s1 * s1 ) / ( double ) w;
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
  breaks[classes-1] = sample[n-1];

  for ( int j = classes, k = n; j >= 2; j-- )
  {
    int id = matrixOne[k][j] - 1;
    breaks[j - 2] = sample[id];
    k = matrixOne[k][j] - 1;
  }

  return breaks.toList();
} //_calcJenksBreaks


QgsGraduatedSymbolRendererV2* QgsGraduatedSymbolRendererV2::createRenderer(
  QgsVectorLayer* vlayer,
  QString attrName,
  int classes,
  Mode mode,
  QgsSymbolV2* symbol,
  QgsVectorColorRampV2* ramp,
  bool inverted,
  QgsRendererRangeV2LabelFormat labelFormat
)
{
  QgsRangeList ranges;
  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( attrName, ranges );
  r->setSourceSymbol( symbol->clone() );
  r->setSourceColorRamp( ramp->clone() );
  r->setInvertedColorRamp( inverted );
  r->setMode( mode );
  r->setLabelFormat( labelFormat );
  r->updateClasses( vlayer, mode, classes );
  return r;
}

QList<double> QgsGraduatedSymbolRendererV2::getDataValues( QgsVectorLayer *vlayer )
{
  QList<double> values;
  QScopedPointer<QgsExpression> expression;
  int attrNum = vlayer->fieldNameIndex( mAttrName );

  if ( attrNum == -1 )
  {
    // try to use expression
    expression.reset( new QgsExpression( mAttrName ) );
    if ( expression->hasParserError() || !expression->prepare( vlayer->pendingFields() ) )
      return values; // should have a means to report errors
  }

  QgsFeature f;
  QStringList lst;
  if ( expression.isNull() )
    lst.append( mAttrName );
  else
    lst = expression->referencedColumns();

  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest()
                                                .setFlags( ( expression && expression->needsGeometry() ) ?
                                                             QgsFeatureRequest::NoFlags :
                                                             QgsFeatureRequest::NoGeometry  )
                                                .setSubsetOfAttributes( lst, vlayer->pendingFields() ) );

  // create list of non-null attribute values
  while ( fit.nextFeature( f ) )
  {
    QVariant v = expression ? expression->evaluate( f ) : f.attribute( attrNum );
    if ( !v.isNull() )
      values.append( v.toDouble() );
  }
  return values;
}

void QgsGraduatedSymbolRendererV2::updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses )
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

  int attrNum = vlayer->fieldNameIndex( mAttrName );

  if ( attrNum == -1 )
  {
    values = getDataValues( vlayer );
    if( values.isEmpty() )
      return;

    qSort( values );
    minimum = values.first();
    maximum = values.last();
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
    breaks = _calcPrettyBreaks( minimum, maximum, nclasses );
  }
  else if ( mode == Quantile || mode == Jenks || mode == StdDev )
  {
    // get values from layer
    if ( !valuesLoaded )
    {
      values = getDataValues( vlayer );
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
        label = ">= " + QString::number( labels[i-1], 'f', 2 ) + " Std Dev";
      }
      else
      {
        label = QString::number( labels[i-1], 'f', 2 ) + " Std Dev" + " - " + QString::number( labels[i], 'f', 2 ) + " Std Dev";
      }
    }
    else
    {
      label = mLabelFormat.labelForRange( lower, upper );
    }
    QgsSymbolV2* newSymbol = mSourceSymbol ? mSourceSymbol->clone() : QgsSymbolV2::defaultSymbol( vlayer->geometryType() );
    addClass( QgsRendererRangeV2( lower, upper, newSymbol, label ) );
  }
  updateColorRamp( 0, mInvertedColorRamp );
}


QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QDomElement rangesElem = element.firstChildElement( "ranges" );
  if ( rangesElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );
  QgsRangeList ranges;

  QDomElement rangeElem = rangesElem.firstChildElement();
  while ( !rangeElem.isNull() )
  {
    if ( rangeElem.tagName() == "range" )
    {
      double lowerValue = rangeElem.attribute( "lower" ).toDouble();
      double upperValue = rangeElem.attribute( "upper" ).toDouble();
      QString symbolName = rangeElem.attribute( "symbol" );
      QString label = rangeElem.attribute( "label" );
      bool render = rangeElem.attribute( "render", "true" ) != "false";
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbolV2* symbol = symbolMap.take( symbolName );
        ranges.append( QgsRendererRangeV2( lowerValue, upperValue, symbol, label, render ) );
      }
    }
    rangeElem = rangeElem.nextSiblingElement();
  }

  QString attrName = element.attribute( "attr" );

  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( attrName, ranges );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( "source-symbol" );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolV2Map sourceSymbolMap = QgsSymbolLayerV2Utils::loadSymbols( sourceSymbolElem );
    if ( sourceSymbolMap.contains( "0" ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( "0" ) );
    }
    QgsSymbolLayerV2Utils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( "colorramp" );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( "name" ) == "[source]" )
  {
    r->setSourceColorRamp( QgsSymbolLayerV2Utils::loadColorRamp( sourceColorRampElem ) );
    QDomElement invertedColorRampElem = element.firstChildElement( "invertedcolorramp" );
    if ( !invertedColorRampElem.isNull() )
      r->setInvertedColorRamp( invertedColorRampElem.attribute( "value" ) == "1" );
  }

  // try to load mode
  QDomElement modeElem = element.firstChildElement( "mode" );
  if ( !modeElem.isNull() )
  {
    QString modeString = modeElem.attribute( "name" );
    if ( modeString == "equal" )
      r->setMode( EqualInterval );
    else if ( modeString == "quantile" )
      r->setMode( Quantile );
    else if ( modeString == "jenks" )
      r->setMode( Jenks );
    else if ( modeString == "stddev" )
      r->setMode( StdDev );
    else if ( modeString == "pretty" )
      r->setMode( Pretty );
  }

  QDomElement rotationElem = element.firstChildElement( "rotation" );
  if ( !rotationElem.isNull() )
    r->setRotationField( rotationElem.attribute( "field" ) );

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() )
    r->setSizeScaleField( sizeScaleElem.attribute( "field" ) );
  r->setScaleMethod( QgsSymbolLayerV2Utils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ) );

  QDomElement labelFormatElem = element.firstChildElement( "labelformat" );
  if ( ! labelFormatElem.isNull() )
  {
    QgsRendererRangeV2LabelFormat labelFormat;
    labelFormat.setFromDomElement( labelFormatElem );
    r->setLabelFormat( labelFormat );
  }
  // TODO: symbol levels
  return r;
}

QDomElement QgsGraduatedSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "graduatedSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );
  rendererElem.setAttribute( "attr", mAttrName );

  // ranges
  int i = 0;
  QgsSymbolV2Map symbols;
  QDomElement rangesElem = doc.createElement( "ranges" );
  QgsRangeList::const_iterator it = mRanges.constBegin();
  for ( ; it != mRanges.constEnd(); ++it )
  {
    const QgsRendererRangeV2& range = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, range.symbol() );

    QDomElement rangeElem = doc.createElement( "range" );
    rangeElem.setAttribute( "lower", QString::number( range.lowerValue(), 'f' ) );
    rangeElem.setAttribute( "upper", QString::number( range.upperValue(), 'f' ) );
    rangeElem.setAttribute( "symbol", symbolName );
    rangeElem.setAttribute( "label", range.label() );
    rangeElem.setAttribute( "render", range.renderState() ? "true" : "false" );
    rangesElem.appendChild( rangeElem );
    i++;
  }

  rendererElem.appendChild( rangesElem );

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  // save source symbol
  if ( mSourceSymbol.data() )
  {
    QgsSymbolV2Map sourceSymbols;
    sourceSymbols.insert( "0", mSourceSymbol.data() );
    QDomElement sourceSymbolElem = QgsSymbolLayerV2Utils::saveSymbols( sourceSymbols, "source-symbol", doc );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp.data() )
  {
    QDomElement colorRampElem = QgsSymbolLayerV2Utils::saveColorRamp( "[source]", mSourceColorRamp.data(), doc );
    rendererElem.appendChild( colorRampElem );
    QDomElement invertedElem = doc.createElement( "invertedcolorramp" );
    invertedElem.setAttribute( "value", mInvertedColorRamp );
    rendererElem.appendChild( invertedElem );
  }

  // save mode
  QString modeString;
  if ( mMode == EqualInterval )
    modeString = "equal";
  else if ( mMode == Quantile )
    modeString = "quantile";
  else if ( mMode == Jenks )
    modeString = "jenks";
  else if ( mMode == StdDev )
    modeString = "stddev";
  else if ( mMode == Pretty )
    modeString = "pretty";
  if ( !modeString.isEmpty() )
  {
    QDomElement modeElem = doc.createElement( "mode" );
    modeElem.setAttribute( "name", modeString );
    rendererElem.appendChild( modeElem );
  }

  QDomElement rotationElem = doc.createElement( "rotation" );
  if ( mRotation.data() )
    rotationElem.setAttribute( "field", QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mRotation.data() ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  if ( mSizeScale.data() )
    sizeScaleElem.setAttribute( "field", QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mSizeScale.data() ) );
  sizeScaleElem.setAttribute( "scalemethod", QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod ) );
  rendererElem.appendChild( sizeScaleElem );

  QDomElement labelFormatElem = doc.createElement( "labelformat" );
  mLabelFormat.saveToDomElement( labelFormatElem );
  rendererElem.appendChild( labelFormatElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsGraduatedSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  int count = ranges().count();
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererRangeV2& range = ranges()[i];
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( range.symbol(), iconSize );
    lst << qMakePair( range.label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsGraduatedSymbolRendererV2::legendSymbolItems( double scaleDenominator, QString rule )
{
  Q_UNUSED( scaleDenominator );
  QgsLegendSymbolList lst;

  foreach ( const QgsRendererRangeV2& range, mRanges )
  {
    if ( rule.isEmpty() || range.label() == rule )
    {
      lst << qMakePair( range.label(), range.symbol() );
    }
  }
  return lst;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::sourceSymbol()
{
  return mSourceSymbol.data();
}
void QgsGraduatedSymbolRendererV2::setSourceSymbol( QgsSymbolV2* sym )
{
  mSourceSymbol.reset( sym );
}

QgsVectorColorRampV2* QgsGraduatedSymbolRendererV2::sourceColorRamp()
{
  return mSourceColorRamp.data();
}

void QgsGraduatedSymbolRendererV2::setSourceColorRamp( QgsVectorColorRampV2* ramp )
{
  mSourceColorRamp.reset( ramp );
}

void QgsGraduatedSymbolRendererV2::updateColorRamp( QgsVectorColorRampV2 *ramp, bool inverted )
{
  int i = 0;
  if ( ramp )
  {
    setSourceColorRamp( ramp );
    setInvertedColorRamp( inverted );
  }

  if ( mSourceColorRamp )
  {
    foreach ( QgsRendererRangeV2 range, mRanges )
    {
      QgsSymbolV2 *symbol = range.symbol() ? range.symbol()->clone() : 0;
      if( symbol )
      {
        double colorValue;
        if ( inverted )
          colorValue = ( mRanges.count() > 1 ? ( double )( mRanges.count() - i - 1 ) / ( mRanges.count() - 1 ) : 0 );
        else
          colorValue = ( mRanges.count() > 1 ? ( double ) i / ( mRanges.count() - 1 ) : 0 );
        symbol->setColor( mSourceColorRamp->color( colorValue ) );
      }
      updateRangeSymbol( i, symbol );
      ++i;
    }
  }

}

void QgsGraduatedSymbolRendererV2::updateSymbols( QgsSymbolV2 *sym )
{
  if( !sym )
    return;

  int i = 0;
  foreach ( QgsRendererRangeV2 range, mRanges )
  {
    QgsSymbolV2 *symbol = sym->clone();
    symbol->setColor( range.symbol()->color() );
    updateRangeSymbol( i, symbol );
    ++i;
  }
  setSourceSymbol( sym->clone() );
}

void QgsGraduatedSymbolRendererV2::setRotationField( QString fieldOrExpression )
{
  mRotation.reset( QgsSymbolLayerV2Utils::fieldOrExpressionToExpression( fieldOrExpression ) );
}

QString QgsGraduatedSymbolRendererV2::rotationField() const
{
  return mRotation.data() ? QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mRotation.data() ) : QString();
}

void QgsGraduatedSymbolRendererV2::setSizeScaleField( QString fieldOrExpression )
{
  mSizeScale.reset( QgsSymbolLayerV2Utils::fieldOrExpressionToExpression( fieldOrExpression ) );
}

QString QgsGraduatedSymbolRendererV2::sizeScaleField() const
{
  return mSizeScale.data() ? QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mSizeScale.data() ) : QString();
}

void QgsGraduatedSymbolRendererV2::setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mScaleMethod = scaleMethod;
  for ( QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it )
  {
    if( it->symbol() )
      setScaleMethodToSymbol( it->symbol(), scaleMethod );
  }
}

bool QgsGraduatedSymbolRendererV2::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsGraduatedSymbolRendererV2::legendSymbolItemChecked( QString key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mRanges.size() )
    return mRanges[ index ].renderState();
  else
    return true;
}

void QgsGraduatedSymbolRendererV2::checkLegendSymbolItem( QString key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateRangeRenderState( index, state );
}


void QgsGraduatedSymbolRendererV2::addClass( QgsSymbolV2* symbol )
{
  QgsSymbolV2* newSymbol = symbol->clone();
  QString label = "0.0 - 0.0";
  mRanges.insert( 0, QgsRendererRangeV2( 0.0, 0.0, newSymbol, label ) );
}

void QgsGraduatedSymbolRendererV2::addClass( double lower, double upper )
{
  QgsSymbolV2* newSymbol = mSourceSymbol->clone();
  QString label = mLabelFormat.labelForRange( lower, upper );
  mRanges.append( QgsRendererRangeV2( lower, upper, newSymbol, label ) );
}

void QgsGraduatedSymbolRendererV2::addClass( QgsRendererRangeV2 range )
{
  mRanges.append( range );
}

void QgsGraduatedSymbolRendererV2::deleteClass( int idx )
{
  mRanges.removeAt( idx );
}

void QgsGraduatedSymbolRendererV2::deleteAllClasses()
{
  mRanges.clear();
}

void QgsGraduatedSymbolRendererV2::setLabelFormat( const QgsRendererRangeV2LabelFormat &labelFormat, bool updateRanges )
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


void QgsGraduatedSymbolRendererV2::calculateLabelPrecision( bool updateRanges )
{
  // Find the minimum size of a class
  double minClassRange = 0.0;
  for ( QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it )
  {
    double range = it->upperValue() - it->lowerValue();
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

void QgsGraduatedSymbolRendererV2::moveClass( int from, int to )
{
  if ( from < 0 || from >= mRanges.size() || to < 0 || to >= mRanges.size() )
    return;
  mRanges.move( from, to );
}

bool valueLessThan( const QgsRendererRangeV2 &r1, const QgsRendererRangeV2 &r2 )
{
  return r1 < r2;
}

bool valueGreaterThan( const QgsRendererRangeV2 &r1, const QgsRendererRangeV2 &r2 )
{
  return !valueLessThan( r1, r2 );
}

void QgsGraduatedSymbolRendererV2::sortByValue( Qt::SortOrder order )
{
  QgsDebugMsg( "Entered" );
  if ( order == Qt::AscendingOrder )
  {
    qSort( mRanges.begin(), mRanges.end(), valueLessThan );
  }
  else
  {
    qSort( mRanges.begin(), mRanges.end(), valueGreaterThan );
  }
}

bool labelLessThan( const QgsRendererRangeV2 &r1, const QgsRendererRangeV2 &r2 )
{
  return QString::localeAwareCompare( r1.label(), r2.label() ) < 0;
}

bool labelGreaterThan( const QgsRendererRangeV2 &r1, const QgsRendererRangeV2 &r2 )
{
  return !labelLessThan( r1, r2 );
}

void QgsGraduatedSymbolRendererV2::sortByLabel( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    qSort( mRanges.begin(), mRanges.end(), labelLessThan );
  }
  else
  {
    qSort( mRanges.begin(), mRanges.end(), labelGreaterThan );
  }
}

QgsGraduatedSymbolRendererV2* QgsGraduatedSymbolRendererV2::convertFromRenderer( const QgsFeatureRendererV2 *renderer )
{
  if ( renderer->type() == "graduatedSymbol" )
  {
    return dynamic_cast<QgsGraduatedSymbolRendererV2*>( renderer->clone() );
  }
  if ( renderer->type() == "pointDisplacement" )
  {
    const QgsPointDisplacementRenderer* pointDisplacementRenderer = dynamic_cast<const QgsPointDisplacementRenderer*>( renderer );
    return convertFromRenderer( pointDisplacementRenderer->embeddedRenderer() );
  }
  if ( renderer->type() == "invertedPolygonRenderer" )
  {
    const QgsInvertedPolygonRenderer* invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer*>( renderer );
    return convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  // If not one of the specifically handled renderers, then just grab the symbol from the renderer
  // Could have applied this to specific renderer types (singleSymbol, graduatedSymbo)

  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( "", QgsRangeList() );
  QgsSymbolV2List symbols = const_cast<QgsFeatureRendererV2 *>( renderer )->symbols();
  if ( symbols.size() > 0 )
  {
    r->setSourceSymbol( symbols.at( 0 )->clone() );
  }

  return r;
}
