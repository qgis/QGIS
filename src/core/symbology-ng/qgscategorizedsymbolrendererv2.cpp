/***************************************************************************
    qgscategorizedsymbolrendererv2.cpp
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
#include <algorithm>

#include "qgscategorizedsymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend

QgsRendererCategoryV2::QgsRendererCategoryV2()
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2( QVariant value, QgsSymbolV2* symbol, QString label, bool render )
    : mValue( value )
    , mSymbol( symbol )
    , mLabel( label )
    , mRender( render )
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2( const QgsRendererCategoryV2& cat )
    : mValue( cat.mValue )
    , mSymbol( cat.mSymbol.data() ? cat.mSymbol->clone() : NULL )
    , mLabel( cat.mLabel )
    , mRender( cat.mRender )
{
}

// copy+swap idion, the copy is done through the 'pass by value'
QgsRendererCategoryV2& QgsRendererCategoryV2::operator=( QgsRendererCategoryV2 cat )
{
  swap( cat );
  return *this;
}

void QgsRendererCategoryV2::swap( QgsRendererCategoryV2 & cat )
{
  qSwap( mValue, cat.mValue );
  qSwap( mSymbol, cat.mSymbol );
  qSwap( mLabel, cat.mLabel );
}

QVariant QgsRendererCategoryV2::value() const
{
  return mValue;
}

QgsSymbolV2* QgsRendererCategoryV2::symbol() const
{
  return mSymbol.data();
}

QString QgsRendererCategoryV2::label() const
{
  return mLabel;
}

bool QgsRendererCategoryV2::renderState() const
{
  return mRender;
}

void QgsRendererCategoryV2::setValue( const QVariant &value )
{
  mValue = value;
}

void QgsRendererCategoryV2::setSymbol( QgsSymbolV2* s )
{
  if ( mSymbol.data() != s ) mSymbol.reset( s );
}

void QgsRendererCategoryV2::setLabel( const QString &label )
{
  mLabel = label;
}

void QgsRendererCategoryV2::setRenderState( bool render )
{
  mRender = render;
}

QString QgsRendererCategoryV2::dump() const
{
  return QString( "%1::%2::%3:%4\n" ).arg( mValue.toString() ).arg( mLabel ).arg( mSymbol->dump() ).arg( mRender );
}

void QgsRendererCategoryV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
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
  QString descrStr = QString( "%1 is '%2'" ).arg( attrName ).arg( mValue.toString() );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc = QString( "%1 = '%2'" )
                       .arg( attrName.replace( "\"", "\"\"" ) )
                       .arg( mValue.toString().replace( "'", "''" ) );
  QgsSymbolLayerV2Utils::createFunctionElement( doc, ruleElem, filterFunc );

  mSymbol->toSld( doc, ruleElem, props );
}

///////////////////

QgsCategorizedSymbolRendererV2::QgsCategorizedSymbolRendererV2( QString attrName, QgsCategoryList categories )
    : QgsFeatureRendererV2( "categorizedSymbol" )
    , mAttrName( attrName )
    , mCategories( categories )
    , mInvertedColorRamp( false )
    , mScaleMethod( DEFAULT_SCALE_METHOD )
{
  for ( int i = 0; i < mCategories.count(); ++i )
  {
    QgsRendererCategoryV2& cat = mCategories[i];
    if ( cat.symbol() == NULL )
    {
      QgsDebugMsg( "invalid symbol in a category! ignoring..." );
      mCategories.removeAt( i-- );
    }
    //mCategories.insert(cat.value().toString(), cat);
  }
}

QgsCategorizedSymbolRendererV2::~QgsCategorizedSymbolRendererV2()
{
}

void QgsCategorizedSymbolRendererV2::rebuildHash()
{
  mSymbolHash.clear();

  for ( int i = 0; i < mCategories.count(); ++i )
  {
    QgsRendererCategoryV2& cat = mCategories[i];
    mSymbolHash.insert( cat.value().toString(), ( cat.renderState() || mCounting ) ? cat.symbol() : &sSkipRender );
  }
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForValue( QVariant value )
{
  // TODO: special case for int, double
  QHash<QString, QgsSymbolV2*>::iterator it = mSymbolHash.find( value.toString() );
  if ( it == mSymbolHash.end() )
  {
    if ( mSymbolHash.count() == 0 )
    {
      QgsDebugMsg( "there are no hashed symbols!!!" );
    }
    else
    {
      QgsDebugMsgLevel( "attribute value not found: " + value.toString(), 3 );
    }
    return NULL;
  }

  return *it;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  const QgsAttributes& attrs = feature.attributes();
  QVariant value;
  if ( mAttrNum == -1 )
  {
    Q_ASSERT( mExpression.data() );
    value = mExpression->evaluate( &feature );
  }
  else
  {
    value = attrs.value( mAttrNum );
  }

  // find the right symbol for the category
  QgsSymbolV2 *symbol = symbolForValue( value );
  if ( symbol == &sSkipRender )
    return 0;

  if ( !symbol )
  {
    // if no symbol found use default one
    return symbolForValue( QVariant( "" ) );
  }

  if ( !mRotation.data() && !mSizeScale.data() )
    return symbol; // no data-defined rotation/scaling - just return the symbol

  // find out rotation, size scale
  const double rotation = mRotation.data() ? mRotation->evaluate( feature ).toDouble() : 0;
  const double sizeScale = mSizeScale.data() ? mSizeScale->evaluate( feature ).toDouble() : 1.;

  // take a temporary symbol (or create it if doesn't exist)
  QgsSymbolV2* tempSymbol = mTempSymbols[value.toString()];

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

int QgsCategorizedSymbolRendererV2::categoryIndexForValue( QVariant val )
{
  for ( int i = 0; i < mCategories.count(); i++ )
  {
    if ( mCategories[i].value() == val )
      return i;
  }
  return -1;
}

int QgsCategorizedSymbolRendererV2::categoryIndexForLabel( QString val )
{
  int idx = -1;
  for ( int i = 0; i < mCategories.count(); i++ )
  {
    if ( mCategories[i].label() == val )
    {
      if ( idx != -1 )
        return -1;
      else
        idx = i;
    }
  }
  return idx;
}

bool QgsCategorizedSymbolRendererV2::updateCategoryValue( int catIndex, const QVariant &value )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setValue( value );
  return true;
}

bool QgsCategorizedSymbolRendererV2::updateCategorySymbol( int catIndex, QgsSymbolV2* symbol )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setSymbol( symbol );
  return true;
}

bool QgsCategorizedSymbolRendererV2::updateCategoryLabel( int catIndex, QString label )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setLabel( label );
  return true;
}

bool QgsCategorizedSymbolRendererV2::updateCategoryRenderState( int catIndex, bool render )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setRenderState( render );
  return true;
}

void QgsCategorizedSymbolRendererV2::addCategory( const QgsRendererCategoryV2 &cat )
{
  if ( !cat.symbol() )
  {
    QgsDebugMsg( "invalid symbol in a category! ignoring..." );
    return;
  }

  mCategories.append( cat );
}

bool QgsCategorizedSymbolRendererV2::deleteCategory( int catIndex )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;

  mCategories.removeAt( catIndex );
  return true;
}

void QgsCategorizedSymbolRendererV2::deleteAllCategories()
{
  mCategories.clear();
}

void QgsCategorizedSymbolRendererV2::moveCategory( int from, int to )
{
  if ( from < 0 || from >= mCategories.size() || to < 0 || to >= mCategories.size() ) return;
  mCategories.move( from, to );
}

bool valueLessThan( const QgsRendererCategoryV2 &c1, const QgsRendererCategoryV2 &c2 )
{
  return qgsVariantLessThan( c1.value(), c2.value() );
}
bool valueGreaterThan( const QgsRendererCategoryV2 &c1, const QgsRendererCategoryV2 &c2 )
{
  return qgsVariantGreaterThan( c1.value(), c2.value() );
}

void QgsCategorizedSymbolRendererV2::sortByValue( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    qSort( mCategories.begin(), mCategories.end(), valueLessThan );
  }
  else
  {
    qSort( mCategories.begin(), mCategories.end(), valueGreaterThan );
  }
}

bool labelLessThan( const QgsRendererCategoryV2 &c1, const QgsRendererCategoryV2 &c2 )
{
  return QString::localeAwareCompare( c1.label(), c2.label() ) < 0;
}

bool labelGreaterThan( const QgsRendererCategoryV2 &c1, const QgsRendererCategoryV2 &c2 )
{
  return !labelLessThan( c1, c2 );
}

void QgsCategorizedSymbolRendererV2::sortByLabel( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    qSort( mCategories.begin(), mCategories.end(), labelLessThan );
  }
  else
  {
    qSort( mCategories.begin(), mCategories.end(), labelGreaterThan );
  }
}

void QgsCategorizedSymbolRendererV2::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  mCounting = context.rendererScale() == 0.0;

  // make sure that the hash table is up to date
  rebuildHash();

  // find out classification attribute index from name
  mAttrNum = fields.fieldNameIndex( mAttrName );
  if ( mAttrNum == -1 )
  {
    mExpression.reset( new QgsExpression( mAttrName ) );
    mExpression->prepare( fields );
  }

  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it )
  {
    it->symbol()->startRender( context, &fields );

    if ( mRotation.data() || mSizeScale.data() )
    {
      QgsSymbolV2* tempSymbol = it->symbol()->clone();
      tempSymbol->setRenderHints(( mRotation.data() ? QgsSymbolV2::DataDefinedRotation : 0 ) |
                                 ( mSizeScale.data() ? QgsSymbolV2::DataDefinedSizeScale : 0 ) );
      tempSymbol->startRender( context, &fields );
      mTempSymbols[ it->value().toString()] = tempSymbol;
    }
  }
}

void QgsCategorizedSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it )
    it->symbol()->stopRender( context );

  // cleanup mTempSymbols
  QHash<QString, QgsSymbolV2*>::iterator it2 = mTempSymbols.begin();
  for ( ; it2 != mTempSymbols.end(); ++it2 )
  {
    it2.value()->stopRender( context );
    delete it2.value();
  }
  mTempSymbols.clear();
  mExpression.reset();
}

QList<QString> QgsCategorizedSymbolRendererV2::usedAttributes()
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

  QgsCategoryList::const_iterator catIt = mCategories.constBegin();
  for ( ; catIt != mCategories.constEnd(); ++catIt )
  {
    QgsSymbolV2* catSymbol = catIt->symbol();
    if ( catSymbol )
    {
      attributes.unite( catSymbol->usedAttributes() );
    }
  }
  return attributes.toList();
}

QString QgsCategorizedSymbolRendererV2::dump() const
{
  QString s = QString( "CATEGORIZED: idx %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mCategories.count(); i++ )
    s += mCategories[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::clone()
{
  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2( mAttrName, mCategories );
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
  return r;
}

void QgsCategorizedSymbolRendererV2::toSld( QDomDocument &doc, QDomElement &element ) const
{
  QgsStringMap props;
  props[ "attribute" ] = mAttrName;
  if ( mRotation.data() )
    props[ "angle" ] = mRotation->expression();
  if ( mSizeScale.data() )
    props[ "scale" ] = mSizeScale->expression();

  // create a Rule for each range
  for ( QgsCategoryList::const_iterator it = mCategories.constBegin(); it != mCategories.constEnd(); ++it )
  {
    QgsStringMap catProps( props );
    it->toSld( doc, element, catProps );
  }
}

QgsSymbolV2List QgsCategorizedSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  for ( int i = 0; i < mCategories.count(); i++ )
    lst.append( mCategories[i].symbol() );
  return lst;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QDomElement catsElem = element.firstChildElement( "categories" );
  if ( catsElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );
  QgsCategoryList cats;

  QDomElement catElem = catsElem.firstChildElement();
  while ( !catElem.isNull() )
  {
    if ( catElem.tagName() == "category" )
    {
      QVariant value = QVariant( catElem.attribute( "value" ) );
      QString symbolName = catElem.attribute( "symbol" );
      QString label = catElem.attribute( "label" );
      bool render = catElem.attribute( "render" ) != "false";
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbolV2* symbol = symbolMap.take( symbolName );
        cats.append( QgsRendererCategoryV2( value, symbol, label, render ) );
      }
    }
    catElem = catElem.nextSiblingElement();
  }

  QString attrName = element.attribute( "attr" );

  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2( attrName, cats );

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

  QDomElement rotationElem = element.firstChildElement( "rotation" );
  if ( !rotationElem.isNull() )
    r->setRotationField( rotationElem.attribute( "field" ) );

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() )
  {
    r->setSizeScaleField( sizeScaleElem.attribute( "field" ) );
    r->setScaleMethod( QgsSymbolLayerV2Utils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ) );
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "categorizedSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );
  rendererElem.setAttribute( "attr", mAttrName );

  // categories
  int i = 0;
  QgsSymbolV2Map symbols;
  QDomElement catsElem = doc.createElement( "categories" );
  QgsCategoryList::const_iterator it = mCategories.constBegin();
  for ( ; it != mCategories.end(); ++it )
  {
    const QgsRendererCategoryV2& cat = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, cat.symbol() );

    QDomElement catElem = doc.createElement( "category" );
    catElem.setAttribute( "value", cat.value().toString() );
    catElem.setAttribute( "symbol", symbolName );
    catElem.setAttribute( "label", cat.label() );
    catElem.setAttribute( "render", cat.renderState() ? "true" : "false" );
    catsElem.appendChild( catElem );
    i++;
  }

  rendererElem.appendChild( catsElem );

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

  QDomElement rotationElem = doc.createElement( "rotation" );
  if ( mRotation.data() )
    rotationElem.setAttribute( "field", QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mRotation.data() ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  if ( mSizeScale.data() )
    sizeScaleElem.setAttribute( "field", QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mSizeScale.data() ) );
  sizeScaleElem.setAttribute( "scalemethod", QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod ) );
  rendererElem.appendChild( sizeScaleElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsCategorizedSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  int count = categories().count();
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererCategoryV2& cat = categories()[i];
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( cat.symbol(), iconSize );
    lst << qMakePair( cat.label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsCategorizedSymbolRendererV2::legendSymbolItems( double scaleDenominator, QString rule )
{
  Q_UNUSED( scaleDenominator );
  QgsLegendSymbolList lst;

  foreach ( const QgsRendererCategoryV2& cat, mCategories )
  {
    if ( rule.isEmpty() || cat.label() == rule )
    {
      lst << qMakePair( cat.label(), cat.symbol() );
    }
  }
  return lst;
}


QgsSymbolV2* QgsCategorizedSymbolRendererV2::sourceSymbol()
{
  return mSourceSymbol.data();
}
void QgsCategorizedSymbolRendererV2::setSourceSymbol( QgsSymbolV2* sym )
{
  mSourceSymbol.reset( sym );
}

QgsVectorColorRampV2* QgsCategorizedSymbolRendererV2::sourceColorRamp()
{
  return mSourceColorRamp.data();
}
void QgsCategorizedSymbolRendererV2::setSourceColorRamp( QgsVectorColorRampV2* ramp )
{
  mSourceColorRamp.reset( ramp );
}

void QgsCategorizedSymbolRendererV2::setRotationField( QString fieldOrExpression )
{
  mRotation.reset( QgsSymbolLayerV2Utils::fieldOrExpressionToExpression( fieldOrExpression ) );
}

QString QgsCategorizedSymbolRendererV2::rotationField() const
{
  return mRotation.data() ? QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mRotation.data() ) : QString();
}

void QgsCategorizedSymbolRendererV2::setSizeScaleField( QString fieldOrExpression )
{
  mSizeScale.reset( QgsSymbolLayerV2Utils::fieldOrExpressionToExpression( fieldOrExpression ) );
}

QString QgsCategorizedSymbolRendererV2::sizeScaleField() const
{
  return mSizeScale.data() ? QgsSymbolLayerV2Utils::fieldOrExpressionFromExpression( mSizeScale.data() ) : QString();
}

void QgsCategorizedSymbolRendererV2::updateSymbols( QgsSymbolV2 * sym )
{
  int i = 0;
  foreach ( QgsRendererCategoryV2 cat, mCategories )
  {
    QgsSymbolV2* symbol = sym->clone();
    symbol->setColor( cat.symbol()->color() );
    updateCategorySymbol( i, symbol );
    ++i;
  }
}

void QgsCategorizedSymbolRendererV2::setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mScaleMethod = scaleMethod;
  QgsCategoryList::const_iterator catIt = mCategories.constBegin();
  for ( ; catIt != mCategories.constEnd(); ++catIt )
  {
    setScaleMethodToSymbol( catIt->symbol(), scaleMethod );
  }
}

bool QgsCategorizedSymbolRendererV2::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsCategorizedSymbolRendererV2::legendSymbolItemChecked( QString key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mCategories.size() )
    return mCategories[ index ].renderState();
  else
    return true;
}

void QgsCategorizedSymbolRendererV2::checkLegendSymbolItem( QString key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategoryRenderState( index, state );
}

QgsMarkerSymbolV2 QgsCategorizedSymbolRendererV2::sSkipRender;
