/***************************************************************************

qgsgraduatedsymbolrendererv2.cpp - Graduated Symbol Renderer Version 2

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

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend
#include <limits> // for jenks classification
#include <cmath> // for pretty classification

QgsRendererRangeV2::QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label )
    : mLowerValue( lowerValue ), mUpperValue( upperValue ), mSymbol( symbol ), mLabel( label )
{
}

QgsRendererRangeV2::QgsRendererRangeV2( const QgsRendererRangeV2& range )
    : mLowerValue( range.mLowerValue ), mUpperValue( range.mUpperValue ), mLabel( range.mLabel )
{
  mSymbol = range.mSymbol->clone();
}

QgsRendererRangeV2::~QgsRendererRangeV2()
{
  delete mSymbol;
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
  return mSymbol;
}

QString QgsRendererRangeV2::label() const
{
  return mLabel;
}

void QgsRendererRangeV2::setSymbol( QgsSymbolV2* s )
{
  if ( mSymbol == s )
    return;
  delete mSymbol;
  mSymbol = s;
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

QString QgsRendererRangeV2::dump()
{
  return QString( "%1 - %2::%3::%4\n" ).arg( mLowerValue ).arg( mUpperValue ).arg( mLabel ).arg( mSymbol->dump() );
}

///////////


QgsGraduatedSymbolRendererV2::QgsGraduatedSymbolRendererV2( QString attrName, QgsRangeList ranges )
    : QgsFeatureRendererV2( "graduatedSymbol" ),
    mAttrName( attrName ),
    mRanges( ranges ),
    mMode( Custom ),
    mSourceSymbol( NULL ),
    mSourceColorRamp( NULL ),
    mRotationFieldIdx( -1 ),
    mSizeScaleFieldIdx( -1 )
{
  // TODO: check ranges for sanity (NULL symbols, invalid ranges)
}

QgsGraduatedSymbolRendererV2::~QgsGraduatedSymbolRendererV2()
{
  mRanges.clear(); // should delete all the symbols
  delete mSourceSymbol;
  delete mSourceColorRamp;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForValue( double value )
{
  for ( QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it )
  {
    if ( it->lowerValue() <= value && it->upperValue() >= value )
      return it->symbol();
  }
  // the value is out of the range: return NULL instead of symbol
  return NULL;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  const QgsAttributeMap& attrMap = feature.attributeMap();
  QgsAttributeMap::const_iterator ita = attrMap.find( mAttrNum );
  if ( ita == attrMap.end() )
  {
    QgsDebugMsg( "attribute required by renderer not found: " + mAttrName + "(index " + QString::number( mAttrNum ) + ")" );
    return NULL;
  }

  // find the right category
  QgsSymbolV2* symbol = symbolForValue( ita->toDouble() );
  if ( symbol == NULL )
    return NULL;

  if ( mRotationFieldIdx == -1 && mSizeScaleFieldIdx == -1 )
    return symbol; // no data-defined rotation/scaling - just return the symbol

  // find out rotation, size scale
  double rotation = 0;
  double sizeScale = 1;
  if ( mRotationFieldIdx != -1 )
    rotation = attrMap[mRotationFieldIdx].toDouble();
  if ( mSizeScaleFieldIdx != -1 )
    sizeScale = attrMap[mSizeScaleFieldIdx].toDouble();

  // take a temporary symbol (or create it if doesn't exist)
  QgsSymbolV2* tempSymbol = mTempSymbols[symbol];

  // modify the temporary symbol and return it
  if ( tempSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( tempSymbol );
    if ( mRotationFieldIdx != -1 )
      markerSymbol->setAngle( rotation );
    if ( mSizeScaleFieldIdx != -1 )
      markerSymbol->setSize( sizeScale * static_cast<QgsMarkerSymbolV2*>( symbol )->size() );
  }
  else if ( tempSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( tempSymbol );
    if ( mSizeScaleFieldIdx != -1 )
      lineSymbol->setWidth( sizeScale * static_cast<QgsLineSymbolV2*>( symbol )->width() );
  }
  return tempSymbol;
}

void QgsGraduatedSymbolRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  // find out classification attribute index from name
  mAttrNum = vlayer ? vlayer->fieldNameIndex( mAttrName ) : -1;

  mRotationFieldIdx  = ( mRotationField.isEmpty()  ? -1 : vlayer->fieldNameIndex( mRotationField ) );
  mSizeScaleFieldIdx = ( mSizeScaleField.isEmpty() ? -1 : vlayer->fieldNameIndex( mSizeScaleField ) );

  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it )
  {
    it->symbol()->startRender( context, vlayer );

    if ( mRotationFieldIdx != -1 || mSizeScaleFieldIdx != -1 )
    {
      QgsSymbolV2* tempSymbol = it->symbol()->clone();
      tempSymbol->setRenderHints(( mRotationFieldIdx != -1 ? QgsSymbolV2::DataDefinedRotation : 0 ) |
                                 ( mSizeScaleFieldIdx != -1 ? QgsSymbolV2::DataDefinedSizeScale : 0 ) );
      tempSymbol->startRender( context, vlayer );
      mTempSymbols[ it->symbol()] = tempSymbol;
    }
  }
}

void QgsGraduatedSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it )
    it->symbol()->startRender( context );

  // cleanup mTempSymbols
#if QT_VERSION < 0x40600
  QMap<QgsSymbolV2*, QgsSymbolV2*>::iterator it2 = mTempSymbols.begin();
#else
  QHash<QgsSymbolV2*, QgsSymbolV2*>::iterator it2 = mTempSymbols.begin();
#endif
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
  attributes.insert( mAttrName );
  if ( !mRotationField.isEmpty() )
  {
    attributes.insert( mRotationField );
  }
  if ( !mSizeScaleField.isEmpty() )
  {
    attributes.insert( mSizeScaleField );
  }

  QgsSymbolV2* symbol = 0;
  QgsRangeList::const_iterator range_it = mRanges.constBegin();
  for ( ; range_it != mRanges.constEnd(); ++range_it )
  {
    symbol = range_it->symbol();
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
  mRanges[rangeIndex].setUpperValue( value );
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeLowerValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setLowerValue( value );
  return true;
}

QString QgsGraduatedSymbolRendererV2::dump()
{
  QString s = QString( "GRADUATED: attr %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mRanges.count(); i++ )
    s += mRanges[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2::clone()
{
  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( mAttrName, mRanges );
  r->setMode( mMode );
  if ( mSourceSymbol )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp )
    r->setSourceColorRamp( mSourceColorRamp->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  r->setRotationField( rotationField() );
  r->setSizeScaleField( sizeScaleField() );
  return r;
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
    divisions = minimumCount;
  }
  else
  {
    divisions = k;
  }
  double minimumBreak = start * unit;
  double maximumBreak = end * unit;
  int count = ceil( maximumBreak - minimumBreak ) / unit;

  for ( int i = 1; i < count + 1; i++ )
  {
    breaks.append( minimumBreak + i * unit );
  }

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


static QList<double> _calcStdDevBreaks( QList<double> values, int classes, QList<int> &labels )
{

  // C++ implementation of the standard deviation class interval algorithm
  // as implemented in the 'classInt' package available for the R statistical
  // prgramming language.

  // Returns breaks based on '_calcPrettyBreaks' of the centred and scaled
  // values of 'values', and may have a number of classes different from 'classes'.

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
    labels.append(( int ) breaks[i] );
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

  QList<double> breaks;
  if ( classes <= 1 )
  {
    breaks.append( maximum );
    return breaks;
  }

  int n = values.count();
  if ( classes >= n )
  {
    return values;
  }

  QList<double> sample;

  // if we have lots of values, we need to take a random sample
  if ( n > maximumSize )
  {
    // for now, sample at least maximumSize values or a 10% sample, whichever
    // is larger. This will produce a more representative sample for very large
    // layers, but could end up being computationally intensive...
    n = qMax( maximumSize, ( int )(( double ) n * 0.10 ) );
    QgsDebugMsg( QString( "natural breaks (jenks) sample size: %1" ).arg( n ) );
    sample.append( minimum );
    sample.append( maximum );
    for ( int i = 0; i < n - 2; i++ )
    {
      // pick a random integer from 0 to n
      int c = ( int )(( double ) rand() / (( double ) RAND_MAX + 1 ) * n - 1 );
      sample.append( values[i+c] );
    }
  }
  else
  {
    sample = values;
  }
  // sort the values
  qSort( sample );

  QList< QList<double> > matrixOne;
  QList< QList<double> > matrixTwo;

  double v, s1, s2, w, i3, i4, val;

  for ( int i = 0; i < n + 1; i++ )
  {
    QList<double > tempOne;
    QList<double > tempTwo;
    for ( int j = 0; j < classes + 1; j++ )
    {
      tempOne.append( 0.0 );
      tempTwo.append( 0.0 );
    }
    matrixOne.append( tempOne );
    matrixTwo.append( tempTwo );
  }

  for ( int i = 1; i < classes + 1; i++ )
  {
    matrixOne[1][i] = 1.0;
    matrixTwo[1][i] = 0.0;
    for ( int j = 2; j < n + 1; j++ )
    {
      matrixTwo[j][i] = std::numeric_limits<qreal>::max();
    }
  }

  v = 0.0;
  for ( int l = 2; l < n + 1; l++ )
  {
    s1 = 0.0;
    s2 = 0.0;
    w = 0.0;
    for ( int m = 1; m < l + 1; m++ )
    {
      i3 = l - m + 1;

      val = sample[ i3 - 1 ];

      s2 += val * val;
      s1 += val;

      w += 1.0;
      v = s2 - ( s1 * s1 ) / w;
      i4 = i3 - 1;

      if ( i4 != 0.0 )
      {
        for ( int j = 2; j < classes + 1; j++ )
        {
          if ( matrixTwo[l][j] >= v + matrixTwo[i4][j - 1] )
          {
            matrixOne[l][j] = i3;
            matrixTwo[l][j] = v + matrixTwo[i4][j - 1];
          }
        }
      }
    }
    matrixOne[l][1] = 1.0;
    matrixTwo[l][1] = v;
  }

  for ( int i = 0; i < classes; i++ )
  {
    breaks.append( 0.0 );
  }

  breaks[classes - 1] = sample[sample.size() - 1];
//  breaks[0] = values[0];

  int k = n;
  int count = classes;
  while ( count >= 2 )
  {
    int id = matrixOne[k][count] - 2;
    breaks[count - 2] = sample[id];
    k = matrixOne[k][count] - 1;
    count -= 1;
  }

  return breaks;
} //_calcJenksBreaks

#include "qgsvectordataprovider.h"
#include "qgsvectorcolorrampv2.h"

QgsGraduatedSymbolRendererV2* QgsGraduatedSymbolRendererV2::createRenderer(
  QgsVectorLayer* vlayer,
  QString attrName,
  int classes,
  Mode mode,
  QgsSymbolV2* symbol,
  QgsVectorColorRampV2* ramp )
{
  if ( classes < 1 )
    return NULL;

  int attrNum = vlayer->fieldNameIndex( attrName );

  double minimum = vlayer->minimumValue( attrNum ).toDouble();
  double maximum = vlayer->maximumValue( attrNum ).toDouble();
  QgsDebugMsg( QString( "min %1 // max %2" ).arg( minimum ).arg( maximum ) );

  QList<double> breaks;
  QList<int> labels;
  if ( mode == EqualInterval )
  {
    breaks = _calcEqualIntervalBreaks( minimum, maximum, classes );
  }
  else if ( mode == Pretty )
  {
    breaks = _calcPrettyBreaks( minimum, maximum, classes );
  }
  else if ( mode == Quantile || mode == Jenks || mode == StdDev )
  {
    // get values from layer
    QList<double> values;
    QgsFeature f;
    QgsAttributeList lst;
    lst.append( attrNum );
    vlayer->select( lst, QgsRectangle(), false );
    while ( vlayer->nextFeature( f ) )
      values.append( f.attributeMap()[attrNum].toDouble() );
    // calculate the breaks
    if ( mode == Quantile )
    {
      breaks = _calcQuantileBreaks( values, classes );
    }
    else if ( mode == Jenks )
    {
      breaks = _calcJenksBreaks( values, classes, minimum, maximum );
    }
    else if ( mode == StdDev )
    {
      breaks = _calcStdDevBreaks( values, classes, labels );
    }
  }
  else
  {
    Q_ASSERT( false );
  }

  QgsRangeList ranges;
  double lower, upper = minimum;
  QString label;

  // "breaks" list contains all values at class breaks plus maximum as last break
  int i = 0;
  for ( QList<double>::iterator it = breaks.begin(); it != breaks.end(); ++it, ++i )
  {
    lower = upper; // upper border from last interval
    upper = *it;
    if ( mode == StdDev )
    {
      if ( i == 0 )
      {
        label = "< " + QString::number( labels[i], 'i', 0 ) + " Std Dev";
      }
      else if ( i == labels.count() - 1 )
      {
        label = ">= " + QString::number( labels[i-1], 'i', 0 ) + " Std Dev";
      }
      else
      {
        label = QString::number( labels[i-1], 'i', 0 ) + " Std Dev" + " - " + QString::number( labels[i], 'i', 0 ) + " Std Dev";
      }
    }
    else
    {
      label = QString::number( lower, 'f', 4 ) + " - " + QString::number( upper, 'f', 4 );
    }

    QgsSymbolV2* newSymbol = symbol->clone();
    double colorValue = ( breaks.count() > 1 ? ( double ) i / ( breaks.count() - 1 ) : 0 );
    newSymbol->setColor( ramp->color( colorValue ) ); // color from (0 / cl-1) to (cl-1 / cl-1)

    ranges.append( QgsRendererRangeV2( lower, upper, newSymbol, label ) );
  }

  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( attrName, ranges );
  r->setSourceSymbol( symbol->clone() );
  r->setSourceColorRamp( ramp->clone() );
  r->setMode( mode );
  return r;
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
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbolV2* symbol = symbolMap.take( symbolName );
        ranges.append( QgsRendererRangeV2( lowerValue, upperValue, symbol, label ) );
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
  for ( ; it != mRanges.end(); it++ )
  {
    const QgsRendererRangeV2& range = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, range.symbol() );

    QDomElement rangeElem = doc.createElement( "range" );
    rangeElem.setAttribute( "lower", range.lowerValue() );
    rangeElem.setAttribute( "upper", range.upperValue() );
    rangeElem.setAttribute( "symbol", symbolName );
    rangeElem.setAttribute( "label", range.label() );
    rangesElem.appendChild( rangeElem );
    i++;
  }

  rendererElem.appendChild( rangesElem );

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  // save source symbol
  if ( mSourceSymbol )
  {
    QgsSymbolV2Map sourceSymbols;
    sourceSymbols.insert( "0", mSourceSymbol );
    QDomElement sourceSymbolElem = QgsSymbolLayerV2Utils::saveSymbols( sourceSymbols, "source-symbol", doc );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerV2Utils::saveColorRamp( "[source]", mSourceColorRamp, doc );
    rendererElem.appendChild( colorRampElem );
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
  rotationElem.setAttribute( "field", mRotationField );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  sizeScaleElem.setAttribute( "field", mSizeScaleField );
  rendererElem.appendChild( sizeScaleElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsGraduatedSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();

  QgsLegendSymbologyList lst;
  if ( showClassifiers )
  {
    lst << qMakePair( classAttribute(), QPixmap() );
  }

  int count = ranges().count();
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererRangeV2& range = ranges()[i];
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( range.symbol(), iconSize );
    lst << qMakePair( range.label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsGraduatedSymbolRendererV2::legendSymbolItems()
{
  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();

  QgsLegendSymbolList lst;
  if ( showClassifiers )
  {
    lst << qMakePair( classAttribute(), ( QgsSymbolV2* )0 );
  }

  QgsRangeList::const_iterator rangeIt = mRanges.constBegin();
  for ( ; rangeIt != mRanges.constEnd(); ++rangeIt )
  {
    lst << qMakePair( rangeIt->label(), rangeIt->symbol() );
  }
  return lst;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::sourceSymbol()
{
  return mSourceSymbol;
}
void QgsGraduatedSymbolRendererV2::setSourceSymbol( QgsSymbolV2* sym )
{
  delete mSourceSymbol;
  mSourceSymbol = sym;
}

QgsVectorColorRampV2* QgsGraduatedSymbolRendererV2::sourceColorRamp()
{
  return mSourceColorRamp;
}
void QgsGraduatedSymbolRendererV2::setSourceColorRamp( QgsVectorColorRampV2* ramp )
{
  delete mSourceColorRamp;
  mSourceColorRamp = ramp;
}

void QgsGraduatedSymbolRendererV2::addClass( QgsSymbolV2* symbol )
{
  QgsSymbolV2* newSymbol = symbol->clone();
  QString label = "0.0 - 0.0";
  mRanges.insert( 0, QgsRendererRangeV2( 0.0, 0.0, newSymbol, label ) );

}

void QgsGraduatedSymbolRendererV2::deleteClass( int idx )
{
  mRanges.removeAt( idx );
}
