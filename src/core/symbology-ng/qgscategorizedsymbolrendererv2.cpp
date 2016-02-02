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
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsscaleexpression.h"
#include "qgsdatadefined.h"

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend

QgsRendererCategoryV2::QgsRendererCategoryV2()
    : mRender( true )
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2( const QVariant& value, QgsSymbolV2* symbol, const QString& label, bool render )
    : mValue( value )
    , mSymbol( symbol )
    , mLabel( label )
    , mRender( render )
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2( const QgsRendererCategoryV2& cat )
    : mValue( cat.mValue )
    , mSymbol( cat.mSymbol.data() ? cat.mSymbol->clone() : nullptr )
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
  return QString( "%1::%2::%3:%4\n" ).arg( mValue.toString(), mLabel, mSymbol->dump() ).arg( mRender );
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
  QString descrStr = QString( "%1 is '%2'" ).arg( attrName, mValue.toString() );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc = QString( "%1 = '%2'" )
                       .arg( attrName.replace( '\"', "\"\"" ),
                             mValue.toString().replace( '\'', "''" ) );
  QgsSymbolLayerV2Utils::createFunctionElement( doc, ruleElem, filterFunc );

  mSymbol->toSld( doc, ruleElem, props );
}

///////////////////

QgsCategorizedSymbolRendererV2::QgsCategorizedSymbolRendererV2( const QString& attrName, const QgsCategoryList& categories )
    : QgsFeatureRendererV2( "categorizedSymbol" )
    , mAttrName( attrName )
    , mInvertedColorRamp( false )
    , mScaleMethod( DEFAULT_SCALE_METHOD )
    , mAttrNum( -1 )
    , mCounting( false )
{
  //important - we need a deep copy of the categories list, not a shared copy. This is required because
  //QgsRendererCategoryV2::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mCategories BUT that same method CAN be used to modify a symbol in place
  Q_FOREACH ( const QgsRendererCategoryV2& cat, categories )
  {
    if ( cat.symbol() )
    {
      QgsDebugMsg( "invalid symbol in a category! ignoring..." );
    }
    mCategories << cat;
  }
}

QgsCategorizedSymbolRendererV2::~QgsCategorizedSymbolRendererV2()
{
}

void QgsCategorizedSymbolRendererV2::rebuildHash()
{
  mSymbolHash.clear();

  for ( int i = 0; i < mCategories.size(); ++i )
  {
    const QgsRendererCategoryV2& cat = mCategories.at( i );
    mSymbolHash.insert( cat.value().toString(), ( cat.renderState() || mCounting ) ? cat.symbol() : skipRender() );
  }
}

QgsSymbolV2*QgsCategorizedSymbolRendererV2::skipRender()
{
  static QgsMarkerSymbolV2* skipRender = nullptr;
  if ( !skipRender )
    skipRender = new QgsMarkerSymbolV2();

  return skipRender;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForValue( const QVariant& value )
{
  // TODO: special case for int, double
  QHash<QString, QgsSymbolV2*>::const_iterator it = mSymbolHash.constFind( value.isNull() ? "" : value.toString() );
  if ( it == mSymbolHash.constEnd() )
  {
    if ( mSymbolHash.isEmpty() )
    {
      QgsDebugMsg( "there are no hashed symbols!!!" );
    }
    else
    {
      QgsDebugMsgLevel( "attribute value not found: " + value.toString(), 3 );
    }
    return nullptr;
  }

  return *it;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  QgsSymbolV2* symbol = originalSymbolForFeature( feature, context );
  if ( !symbol )
    return nullptr;

  if ( !mRotation.data() && !mSizeScale.data() )
    return symbol; // no data-defined rotation/scaling - just return the symbol

  // find out rotation, size scale
  const double rotation = mRotation.data() ? mRotation->evaluate( &context.expressionContext() ).toDouble() : 0;
  const double sizeScale = mSizeScale.data() ? mSizeScale->evaluate( &context.expressionContext() ).toDouble() : 1.;

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


QVariant QgsCategorizedSymbolRendererV2::valueForFeature( QgsFeature& feature, QgsRenderContext &context ) const
{
  QgsAttributes attrs = feature.attributes();
  QVariant value;
  if ( mAttrNum == -1 )
  {
    Q_ASSERT( mExpression.data() );

    value = mExpression->evaluate( &context.expressionContext() );
  }
  else
  {
    value = attrs.value( mAttrNum );
  }

  return value;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::originalSymbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  QVariant value = valueForFeature( feature, context );

  // find the right symbol for the category
  QgsSymbolV2 *symbol = symbolForValue( value );
  if ( symbol == skipRender() )
    return nullptr;

  if ( !symbol )
  {
    // if no symbol found use default one
    return symbolForValue( QVariant( "" ) );
  }

  return symbol;
}


int QgsCategorizedSymbolRendererV2::categoryIndexForValue( const QVariant& val )
{
  for ( int i = 0; i < mCategories.count(); i++ )
  {
    if ( mCategories[i].value() == val )
      return i;
  }
  return -1;
}

int QgsCategorizedSymbolRendererV2::categoryIndexForLabel( const QString& val )
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

bool QgsCategorizedSymbolRendererV2::updateCategoryLabel( int catIndex, const QString& label )
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
    mExpression->prepare( &context.expressionContext() );
  }

  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    cat.symbol()->startRender( context, &fields );

    if ( mRotation.data() || mSizeScale.data() )
    {
      QgsSymbolV2* tempSymbol = cat.symbol()->clone();
      tempSymbol->setRenderHints(( mRotation.data() ? QgsSymbolV2::DataDefinedRotation : 0 ) |
                                 ( mSizeScale.data() ? QgsSymbolV2::DataDefinedSizeScale : 0 ) );
      tempSymbol->startRender( context, &fields );
      mTempSymbols[ cat.symbol()] = tempSymbol;
    }
  }
  return;
}

void QgsCategorizedSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    cat.symbol()->stopRender( context );
  }

  // cleanup mTempSymbols
  QHash<QgsSymbolV2*, QgsSymbolV2*>::const_iterator it2 = mTempSymbols.constBegin();
  for ( ; it2 != mTempSymbols.constEnd(); ++it2 )
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

QgsCategorizedSymbolRendererV2* QgsCategorizedSymbolRendererV2::clone() const
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
  r->setSizeScaleField( sizeScaleField() );

  copyRendererData( r );
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

QString QgsCategorizedSymbolRendererV2::filter( const QgsFields& fields )
{
  int attrNum = fields.fieldNameIndex( mAttrName );
  bool isExpression = ( attrNum == -1 );

  bool hasDefault = false;
  bool defaultActive = false;
  bool allActive = true;
  bool noneActive = true;

  //we need to build lists of both inactive and active values, as either list may be required
  //depending on whether the default category is active or not
  QString activeValues;
  QString inactiveValues;

  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    if ( cat.value() == "" )
    {
      hasDefault = true;
      defaultActive = cat.renderState();
    }

    noneActive = noneActive && !cat.renderState();
    allActive = allActive && cat.renderState();

    QVariant::Type valType = isExpression ? cat.value().type() : fields.at( attrNum ).type();
    QString value = QgsExpression::quotedValue( cat.value(), valType );

    if ( !cat.renderState() )
    {
      if ( cat.value() != "" )
      {
        if ( !inactiveValues.isEmpty() )
          inactiveValues.append( ',' );

        inactiveValues.append( value );
      }
    }
    else
    {
      if ( cat.value() != "" )
      {
        if ( !activeValues.isEmpty() )
          activeValues.append( ',' );

        activeValues.append( value );
      }
    }
  }

  QString attr = isExpression ? mAttrName : QString( "\"%1\"" ).arg( mAttrName );

  if ( allActive && hasDefault )
  {
    return QString();
  }
  else if ( noneActive )
  {
    return "FALSE";
  }
  else if ( defaultActive )
  {
    return QString( "(%1) NOT IN (%2) OR (%1) IS NULL" ).arg( attr, inactiveValues );
  }
  else
  {
    return QString( "(%1) IN (%2)" ).arg( attr, activeValues );
  }
}

QgsSymbolV2List QgsCategorizedSymbolRendererV2::symbols( QgsRenderContext &context )
{
  Q_UNUSED( context );
  QgsSymbolV2List lst;
  lst.reserve( mCategories.count() );
  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    lst.append( cat.symbol() );
  }
  return lst;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement catsElem = element.firstChildElement( "categories" );
  if ( catsElem.isNull() )
    return nullptr;

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
  if ( !rotationElem.isNull() && !rotationElem.attribute( "field" ).isEmpty() )
  {
    Q_FOREACH ( const QgsRendererCategoryV2& cat, r->mCategories )
    {
      convertSymbolRotation( cat.symbol(), rotationElem.attribute( "field" ) );
    }
    if ( r->mSourceSymbol.data() )
    {
      convertSymbolRotation( r->mSourceSymbol.data(), rotationElem.attribute( "field" ) );
    }
  }

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( "field" ).isEmpty() )
  {
    Q_FOREACH ( const QgsRendererCategoryV2& cat, r->mCategories )
    {
      convertSymbolSizeScale( cat.symbol(),
                              QgsSymbolLayerV2Utils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ),
                              sizeScaleElem.attribute( "field" ) );
    }
    if ( r->mSourceSymbol.data() && r->mSourceSymbol->type() == QgsSymbolV2::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.data(),
                              QgsSymbolLayerV2Utils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ),
                              sizeScaleElem.attribute( "field" ) );
    }
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "categorizedSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );
  rendererElem.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );
  rendererElem.setAttribute( "attr", mAttrName );

  // categories
  if ( !mCategories.isEmpty() )
  {
    int i = 0;
    QgsSymbolV2Map symbols;
    QDomElement catsElem = doc.createElement( "categories" );
    QgsCategoryList::const_iterator it = mCategories.constBegin();
    for ( ; it != mCategories.constEnd(); ++it )
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

  }

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

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( "orderby" );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( "enableorderby", ( mOrderByEnabled ? "1" : "0" ) );

  return rendererElem;
}

QgsLegendSymbologyList QgsCategorizedSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  int count = categories().count();
  lst.reserve( count );
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererCategoryV2& cat = categories()[i];
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( cat.symbol(), iconSize );
    lst << qMakePair( cat.label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsCategorizedSymbolRendererV2::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  Q_UNUSED( scaleDenominator );
  QgsLegendSymbolList lst;

  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    if ( rule.isEmpty() || cat.label() == rule )
    {
      lst << qMakePair( cat.label(), cat.symbol() );
    }
  }
  return lst;
}

QgsLegendSymbolListV2 QgsCategorizedSymbolRendererV2::legendSymbolItemsV2() const
{
  QgsLegendSymbolListV2 lst;
  if ( mSourceSymbol.data() && mSourceSymbol->type() == QgsSymbolV2::Marker )
  {
    // check that all symbols that have the same size expression
    QgsDataDefined ddSize;
    Q_FOREACH ( const QgsRendererCategoryV2& category, mCategories )
    {
      const QgsMarkerSymbolV2 * symbol = static_cast<const QgsMarkerSymbolV2 *>( category.symbol() );
      if ( !ddSize.hasDefaultValues() && symbol->dataDefinedSize() != ddSize )
      {
        // no common size expression
        return QgsFeatureRendererV2::legendSymbolItemsV2();
      }
      else
      {
        ddSize = symbol->dataDefinedSize();
      }
    }

    if ( !ddSize.isActive() || !ddSize.useExpression() )
    {
      return QgsFeatureRendererV2::legendSymbolItemsV2();
    }

    QgsScaleExpression exp( ddSize.expressionString() );
    if ( exp.type() != QgsScaleExpression::Unknown )
    {
      QgsLegendSymbolItemV2 title( nullptr, exp.baseExpression(), "" );
      lst << title;
      Q_FOREACH ( double v, QgsSymbolLayerV2Utils::prettyBreaks( exp.minValue(), exp.maxValue(), 4 ) )
      {
        QgsLegendSymbolItemV2 si( mSourceSymbol.data(), QString::number( v ), "" );
        QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( si.symbol() );
        s->setDataDefinedSize( QgsDataDefined() );
        s->setSize( exp.size( v ) );
        lst << si;
      }
      // now list the categorized symbols
      const QgsLegendSymbolListV2 list2 = QgsFeatureRendererV2::legendSymbolItemsV2() ;
      Q_FOREACH ( const QgsLegendSymbolItemV2& item, list2 )
        lst << item;
      return lst;
    }
  }

  return QgsFeatureRendererV2::legendSymbolItemsV2();
}

QSet<QString> QgsCategorizedSymbolRendererV2::legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  QString value = valueForFeature( feature, context ).toString();
  int i = 0;

  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    if ( value == cat.value() )
    {
      if ( cat.renderState() )
        return QSet< QString >() << QString::number( i );
      else
        return QSet< QString >();
    }
    i++;
  }

  return QSet< QString >();
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

void QgsCategorizedSymbolRendererV2::updateColorRamp( QgsVectorColorRampV2* ramp, bool inverted )
{
  setSourceColorRamp( ramp );
  setInvertedColorRamp( inverted );
  double num = mCategories.count() - 1;
  double count = 0;

  QgsRandomColorsV2* randomRamp = dynamic_cast<QgsRandomColorsV2*>( ramp );
  if ( randomRamp )
  {
    //ramp is a random colors ramp, so inform it of the total number of required colors
    //this allows the ramp to pregenerate a set of visually distinctive colors
    randomRamp->setTotalColorCount( mCategories.count() );
  }

  Q_FOREACH ( const QgsRendererCategoryV2 &cat, mCategories )
  {
    double value = count / num;
    if ( mInvertedColorRamp ) value = 1.0 - value;
    cat.symbol()->setColor( mSourceColorRamp->color( value ) );
    count += 1;
  }
}

void QgsCategorizedSymbolRendererV2::setRotationField( const QString& fieldOrExpression )
{
  if ( mSourceSymbol && mSourceSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( mSourceSymbol.data() );
    s->setDataDefinedAngle( QgsDataDefined( fieldOrExpression ) );
  }
}

QString QgsCategorizedSymbolRendererV2::rotationField() const
{
  if ( mSourceSymbol && mSourceSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( mSourceSymbol.data() );
    QgsDataDefined ddAngle = s->dataDefinedAngle();
    return ddAngle.useExpression() ? ddAngle.expressionString() : ddAngle.field();
  }

  return QString();
}

void QgsCategorizedSymbolRendererV2::setSizeScaleField( const QString& fieldOrExpression )
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
  Q_FOREACH ( const QgsRendererCategoryV2& cat, mCategories )
  {
    QgsSymbolV2* symbol = sym->clone();
    symbol->setColor( cat.symbol()->color() );
    updateCategorySymbol( i, symbol );
    ++i;
  }
  setSourceSymbol( sym->clone() );
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

bool QgsCategorizedSymbolRendererV2::legendSymbolItemChecked( const QString& key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mCategories.size() )
    return mCategories.at( index ).renderState();
  else
    return true;
}

void QgsCategorizedSymbolRendererV2::setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategorySymbol( index, symbol );
  else
    delete symbol;
}

void QgsCategorizedSymbolRendererV2::checkLegendSymbolItem( const QString& key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategoryRenderState( index, state );
}

QgsCategorizedSymbolRendererV2* QgsCategorizedSymbolRendererV2::convertFromRenderer( const QgsFeatureRendererV2 *renderer )
{
  QgsCategorizedSymbolRendererV2* r = nullptr;
  if ( renderer->type() == "categorizedSymbol" )
  {
    r = dynamic_cast<QgsCategorizedSymbolRendererV2*>( renderer->clone() );
  }
  else if ( renderer->type() == "pointDisplacement" )
  {
    const QgsPointDisplacementRenderer* pointDisplacementRenderer = dynamic_cast<const QgsPointDisplacementRenderer*>( renderer );
    if ( pointDisplacementRenderer )
      r = convertFromRenderer( pointDisplacementRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == "invertedPolygonRenderer" )
  {
    const QgsInvertedPolygonRenderer* invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer*>( renderer );
    if ( invertedPolygonRenderer )
      r = convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  // If not one of the specifically handled renderers, then just grab the symbol from the renderer
  // Could have applied this to specific renderer types (singleSymbol, graduatedSymbo)

  if ( !r )
  {
    r = new QgsCategorizedSymbolRendererV2( "", QgsCategoryList() );
    QgsRenderContext context;
    QgsSymbolV2List symbols = const_cast<QgsFeatureRendererV2 *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r->setSourceSymbol( symbols.at( 0 )->clone() );
    }
  }

  r->setOrderBy( renderer->orderBy() );
  r->setOrderByEnabled( renderer->orderByEnabled() );

  return r;
}
