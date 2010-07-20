
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
    mSourceColorRamp( NULL )
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
    it->symbol()->startRender( context );

    if ( mRotationFieldIdx != -1 || mSizeScaleFieldIdx != -1 )
    {
      QgsSymbolV2* tempSymbol = it->symbol()->clone();
      tempSymbol->setRenderHints(( mRotationFieldIdx != -1 ? QgsSymbolV2::DataDefinedRotation : 0 ) |
                                 ( mSizeScaleFieldIdx != -1 ? QgsSymbolV2::DataDefinedSizeScale : 0 ) );
      tempSymbol->startRender( context );
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
  QList<QString> lst;
  lst.append( mAttrName );
  if ( !mRotationField.isEmpty() )
    lst.append( mRotationField );
  if ( !mSizeScaleField.isEmpty() )
    lst.append( mSizeScaleField );
  return lst;
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
  double step = ( maximum - minimum ) / classes;

  QList<double> breaks;
  double value = minimum;
  for ( int i = 0; i < classes; i++ )
  {
    value += step;
    breaks.append( value );
  }
  return breaks;
}

static QList<double> _calcQuantileBreaks( QList<double> values, int classes )
{
  // sort the values first
  qSort( values );

  QList<double> breaks;

  // q-th quantile of a data set:
  // value where q fraction of data is below and (1-q) fraction is above this value
  // Xq = (1 - r) * X_NI1 + r * X_NI2
  //   NI1 = (int) (q * (n+1))
  //   NI2 = NI1 + 1
  //   r = q * (n+1) - (int) (q * (n+1))
  // (indices of X: 1...n)

  int n = values.count();
  double q, a, aa, r, Xq;
  for ( int i = 0; i < ( classes - 1 ); i++ )
  {
    q = ( i + 1 ) / ( double ) classes;
    a = q * n;
    aa = ( int )( q * n );

    r = a - aa;
    Xq = ( 1 - r ) * values[aa] + r * values[aa+1];

    breaks.append( Xq );
  }

  breaks.append( values[ n-1 ] );

  return breaks;
}

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
  QgsVectorDataProvider* provider = vlayer->dataProvider();

  int attrNum = vlayer->fieldNameIndex( attrName );

  double minimum = provider->minimumValue( attrNum ).toDouble();
  double maximum = provider->maximumValue( attrNum ).toDouble();
  QgsDebugMsg( QString( "min %1 // max %2" ).arg( minimum ).arg( maximum ) );

  QList<double> breaks;
  if ( mode == EqualInterval )
  {
    breaks = _calcEqualIntervalBreaks( minimum, maximum, classes );
  }
  else if ( mode == Quantile )
  {
    // get values from layer
    QList<double> values;
    QgsFeature f;
    QgsAttributeList lst;
    lst.append( attrNum );
    provider->select( lst, QgsRectangle(), false );
    while ( provider->nextFeature( f ) )
      values.append( f.attributeMap()[attrNum].toDouble() );
    // calculate the breaks
    breaks = _calcQuantileBreaks( values, classes );
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
    label = QString::number( lower, 'f', 4 ) + " - " + QString::number( upper, 'f', 4 );

    QgsSymbolV2* newSymbol = symbol->clone();
    newSymbol->setColor( ramp->color(( double ) i / ( classes - 1 ) ) ); // color from (0 / cl-1) to (cl-1 / cl-1)

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
