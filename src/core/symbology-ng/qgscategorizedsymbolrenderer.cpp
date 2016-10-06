/***************************************************************************
    qgscategorizedsymbolrenderer.cpp
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

#include "qgscategorizedsymbolrenderer.h"

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsscaleexpression.h"
#include "qgsdatadefined.h"
#include "qgssymbollayer.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend

QgsRendererCategory::QgsRendererCategory()
    : mRender( true )
{
}

QgsRendererCategory::QgsRendererCategory( const QVariant& value, QgsSymbol* symbol, const QString& label, bool render )
    : mValue( value )
    , mSymbol( symbol )
    , mLabel( label )
    , mRender( render )
{
}

QgsRendererCategory::QgsRendererCategory( const QgsRendererCategory& cat )
    : mValue( cat.mValue )
    , mSymbol( cat.mSymbol.data() ? cat.mSymbol->clone() : nullptr )
    , mLabel( cat.mLabel )
    , mRender( cat.mRender )
{
}

// copy+swap idion, the copy is done through the 'pass by value'
QgsRendererCategory& QgsRendererCategory::operator=( QgsRendererCategory cat )
{
  swap( cat );
  return *this;
}

void QgsRendererCategory::swap( QgsRendererCategory & cat )
{
  qSwap( mValue, cat.mValue );
  qSwap( mSymbol, cat.mSymbol );
  qSwap( mLabel, cat.mLabel );
}

QVariant QgsRendererCategory::value() const
{
  return mValue;
}

QgsSymbol* QgsRendererCategory::symbol() const
{
  return mSymbol.data();
}

QString QgsRendererCategory::label() const
{
  return mLabel;
}

bool QgsRendererCategory::renderState() const
{
  return mRender;
}

void QgsRendererCategory::setValue( const QVariant &value )
{
  mValue = value;
}

void QgsRendererCategory::setSymbol( QgsSymbol* s )
{
  if ( mSymbol.data() != s ) mSymbol.reset( s );
}

void QgsRendererCategory::setLabel( const QString &label )
{
  mLabel = label;
}

void QgsRendererCategory::setRenderState( bool render )
{
  mRender = render;
}

QString QgsRendererCategory::dump() const
{
  return QString( "%1::%2::%3:%4\n" ).arg( mValue.toString(), mLabel, mSymbol->dump() ).arg( mRender );
}

void QgsRendererCategory::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
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
  QgsSymbolLayerUtils::createFunctionElement( doc, ruleElem, filterFunc );

  // add the mix/max scale denoms if we got any from the callers
  QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, props );

  mSymbol->toSld( doc, ruleElem, props );
}

///////////////////

QgsCategorizedSymbolRenderer::QgsCategorizedSymbolRenderer( const QString& attrName, const QgsCategoryList& categories )
    : QgsFeatureRenderer( "categorizedSymbol" )
    , mAttrName( attrName )
    , mInvertedColorRamp( false )
    , mAttrNum( -1 )
    , mCounting( false )
{
  //important - we need a deep copy of the categories list, not a shared copy. This is required because
  //QgsRendererCategory::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mCategories BUT that same method CAN be used to modify a symbol in place
  Q_FOREACH ( const QgsRendererCategory& cat, categories )
  {
    if ( !cat.symbol() )
    {
      QgsDebugMsg( "invalid symbol in a category! ignoring..." );
    }
    mCategories << cat;
  }
}

QgsCategorizedSymbolRenderer::~QgsCategorizedSymbolRenderer()
{
}

void QgsCategorizedSymbolRenderer::rebuildHash()
{
  mSymbolHash.clear();

  for ( int i = 0; i < mCategories.size(); ++i )
  {
    const QgsRendererCategory& cat = mCategories.at( i );
    mSymbolHash.insert( cat.value().toString(), ( cat.renderState() || mCounting ) ? cat.symbol() : skipRender() );
  }
}

QgsSymbol*QgsCategorizedSymbolRenderer::skipRender()
{
  static QgsMarkerSymbol* skipRender = nullptr;
  if ( !skipRender )
    skipRender = new QgsMarkerSymbol();

  return skipRender;
}

QgsSymbol* QgsCategorizedSymbolRenderer::symbolForValue( const QVariant& value )
{
  // TODO: special case for int, double
  QHash<QString, QgsSymbol*>::const_iterator it = mSymbolHash.constFind( value.isNull() ? "" : value.toString() );
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

QgsSymbol* QgsCategorizedSymbolRenderer::symbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  return originalSymbolForFeature( feature, context );
}

QVariant QgsCategorizedSymbolRenderer::valueForFeature( QgsFeature& feature, QgsRenderContext &context ) const
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

QgsSymbol* QgsCategorizedSymbolRenderer::originalSymbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  QVariant value = valueForFeature( feature, context );

  // find the right symbol for the category
  QgsSymbol *symbol = symbolForValue( value );
  if ( symbol == skipRender() )
    return nullptr;

  if ( !symbol )
  {
    // if no symbol found use default one
    return symbolForValue( QVariant( "" ) );
  }

  return symbol;
}


int QgsCategorizedSymbolRenderer::categoryIndexForValue( const QVariant& val )
{
  for ( int i = 0; i < mCategories.count(); i++ )
  {
    if ( mCategories[i].value() == val )
      return i;
  }
  return -1;
}

int QgsCategorizedSymbolRenderer::categoryIndexForLabel( const QString& val )
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

bool QgsCategorizedSymbolRenderer::updateCategoryValue( int catIndex, const QVariant &value )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setValue( value );
  return true;
}

bool QgsCategorizedSymbolRenderer::updateCategorySymbol( int catIndex, QgsSymbol* symbol )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setSymbol( symbol );
  return true;
}

bool QgsCategorizedSymbolRenderer::updateCategoryLabel( int catIndex, const QString& label )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setLabel( label );
  return true;
}

bool QgsCategorizedSymbolRenderer::updateCategoryRenderState( int catIndex, bool render )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setRenderState( render );
  return true;
}

void QgsCategorizedSymbolRenderer::addCategory( const QgsRendererCategory &cat )
{
  if ( !cat.symbol() )
  {
    QgsDebugMsg( "invalid symbol in a category! ignoring..." );
    return;
  }

  mCategories.append( cat );
}

bool QgsCategorizedSymbolRenderer::deleteCategory( int catIndex )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;

  mCategories.removeAt( catIndex );
  return true;
}

void QgsCategorizedSymbolRenderer::deleteAllCategories()
{
  mCategories.clear();
}

void QgsCategorizedSymbolRenderer::moveCategory( int from, int to )
{
  if ( from < 0 || from >= mCategories.size() || to < 0 || to >= mCategories.size() ) return;
  mCategories.move( from, to );
}

bool valueLessThan( const QgsRendererCategory &c1, const QgsRendererCategory &c2 )
{
  return qgsVariantLessThan( c1.value(), c2.value() );
}
bool valueGreaterThan( const QgsRendererCategory &c1, const QgsRendererCategory &c2 )
{
  return qgsVariantGreaterThan( c1.value(), c2.value() );
}

void QgsCategorizedSymbolRenderer::sortByValue( Qt::SortOrder order )
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

bool labelLessThan( const QgsRendererCategory &c1, const QgsRendererCategory &c2 )
{
  return QString::localeAwareCompare( c1.label(), c2.label() ) < 0;
}

bool labelGreaterThan( const QgsRendererCategory &c1, const QgsRendererCategory &c2 )
{
  return !labelLessThan( c1, c2 );
}

void QgsCategorizedSymbolRenderer::sortByLabel( Qt::SortOrder order )
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

void QgsCategorizedSymbolRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  mCounting = context.rendererScale() == 0.0;

  // make sure that the hash table is up to date
  rebuildHash();

  // find out classification attribute index from name
  mAttrNum = fields.lookupField( mAttrName );
  if ( mAttrNum == -1 )
  {
    mExpression.reset( new QgsExpression( mAttrName ) );
    mExpression->prepare( &context.expressionContext() );
  }

  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
  {
    cat.symbol()->startRender( context, fields );
  }

  Q_FOREACH ( QgsSymbol *sym, mSymbolHash.values() )
  {
    sym->startRender( context, fields );
  }

  return;
}

void QgsCategorizedSymbolRenderer::stopRender( QgsRenderContext& context )
{
  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
  {
    cat.symbol()->stopRender( context );
  }

  Q_FOREACH ( QgsSymbol *sym, mSymbolHash.values() )
  {
    sym->stopRender( context );
  }

  mExpression.reset();
}

QSet<QString> QgsCategorizedSymbolRenderer::usedAttributes() const
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

  QgsCategoryList::const_iterator catIt = mCategories.constBegin();
  for ( ; catIt != mCategories.constEnd(); ++catIt )
  {
    QgsSymbol* catSymbol = catIt->symbol();
    if ( catSymbol )
    {
      attributes.unite( catSymbol->usedAttributes() );
    }
  }
  return attributes;
}

QString QgsCategorizedSymbolRenderer::dump() const
{
  QString s = QString( "CATEGORIZED: idx %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mCategories.count(); i++ )
    s += mCategories[i].dump();
  return s;
}

QgsCategorizedSymbolRenderer* QgsCategorizedSymbolRenderer::clone() const
{
  QgsCategorizedSymbolRenderer* r = new QgsCategorizedSymbolRenderer( mAttrName, mCategories );
  if ( mSourceSymbol.data() )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp.data() )
  {
    r->setSourceColorRamp( mSourceColorRamp->clone() );
    r->setInvertedColorRamp( mInvertedColorRamp );
  }
  r->setUsingSymbolLevels( usingSymbolLevels() );

  copyRendererData( r );
  return r;
}

void QgsCategorizedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  props[ "attribute" ] = mAttrName;

  // create a Rule for each range
  for ( QgsCategoryList::const_iterator it = mCategories.constBegin(); it != mCategories.constEnd(); ++it )
  {
    QgsStringMap catProps( props );
    it->toSld( doc, element, catProps );
  }
}

QString QgsCategorizedSymbolRenderer::filter( const QgsFields& fields )
{
  int attrNum = fields.lookupField( mAttrName );
  bool isExpression = ( attrNum == -1 );

  bool hasDefault = false;
  bool defaultActive = false;
  bool allActive = true;
  bool noneActive = true;

  //we need to build lists of both inactive and active values, as either list may be required
  //depending on whether the default category is active or not
  QString activeValues;
  QString inactiveValues;

  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
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

QgsSymbolList QgsCategorizedSymbolRenderer::symbols( QgsRenderContext &context )
{
  Q_UNUSED( context );
  QgsSymbolList lst;
  lst.reserve( mCategories.count() );
  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
  {
    lst.append( cat.symbol() );
  }
  return lst;
}

QgsFeatureRenderer* QgsCategorizedSymbolRenderer::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement catsElem = element.firstChildElement( "categories" );
  if ( catsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem );
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
        QgsSymbol* symbol = symbolMap.take( symbolName );
        cats.append( QgsRendererCategory( value, symbol, label, render ) );
      }
    }
    catElem = catElem.nextSiblingElement();
  }

  QString attrName = element.attribute( "attr" );

  QgsCategorizedSymbolRenderer* r = new QgsCategorizedSymbolRenderer( attrName, cats );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( "source-symbol" );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolMap sourceSymbolMap = QgsSymbolLayerUtils::loadSymbols( sourceSymbolElem );
    if ( sourceSymbolMap.contains( "0" ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( "0" ) );
    }
    QgsSymbolLayerUtils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( "colorramp" );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( "name" ) == "[source]" )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
    QDomElement invertedColorRampElem = element.firstChildElement( "invertedcolorramp" );
    if ( !invertedColorRampElem.isNull() )
      r->setInvertedColorRamp( invertedColorRampElem.attribute( "value" ) == "1" );
  }

  QDomElement rotationElem = element.firstChildElement( "rotation" );
  if ( !rotationElem.isNull() && !rotationElem.attribute( "field" ).isEmpty() )
  {
    Q_FOREACH ( const QgsRendererCategory& cat, r->mCategories )
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
    Q_FOREACH ( const QgsRendererCategory& cat, r->mCategories )
    {
      convertSymbolSizeScale( cat.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ),
                              sizeScaleElem.attribute( "field" ) );
    }
    if ( r->mSourceSymbol.data() && r->mSourceSymbol->type() == QgsSymbol::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.data(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ),
                              sizeScaleElem.attribute( "field" ) );
    }
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRenderer::save( QDomDocument& doc )
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
    QgsSymbolMap symbols;
    QDomElement catsElem = doc.createElement( "categories" );
    QgsCategoryList::const_iterator it = mCategories.constBegin();
    for ( ; it != mCategories.constEnd(); ++it )
    {
      const QgsRendererCategory& cat = *it;
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
    QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, "symbols", doc );
    rendererElem.appendChild( symbolsElem );

  }

  // save source symbol
  if ( mSourceSymbol.data() )
  {
    QgsSymbolMap sourceSymbols;
    sourceSymbols.insert( "0", mSourceSymbol.data() );
    QDomElement sourceSymbolElem = QgsSymbolLayerUtils::saveSymbols( sourceSymbols, "source-symbol", doc );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp.data() )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( "[source]", mSourceColorRamp.data(), doc );
    rendererElem.appendChild( colorRampElem );
    QDomElement invertedElem = doc.createElement( "invertedcolorramp" );
    invertedElem.setAttribute( "value", mInvertedColorRamp );
    rendererElem.appendChild( invertedElem );
  }

  QDomElement rotationElem = doc.createElement( "rotation" );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
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

QgsLegendSymbologyList QgsCategorizedSymbolRenderer::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  int count = categories().count();
  lst.reserve( count );
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererCategory& cat = categories()[i];
    QPixmap pix = QgsSymbolLayerUtils::symbolPreviewPixmap( cat.symbol(), iconSize );
    lst << qMakePair( cat.label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsCategorizedSymbolRenderer::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  Q_UNUSED( scaleDenominator );
  QgsLegendSymbolList lst;

  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
  {
    if ( rule.isEmpty() || cat.label() == rule )
    {
      lst << qMakePair( cat.label(), cat.symbol() );
    }
  }
  return lst;
}

QgsLegendSymbolListV2 QgsCategorizedSymbolRenderer::legendSymbolItemsV2() const
{
  QgsLegendSymbolListV2 lst;
  if ( mSourceSymbol.data() && mSourceSymbol->type() == QgsSymbol::Marker )
  {
    // check that all symbols that have the same size expression
    QgsDataDefined ddSize;
    Q_FOREACH ( const QgsRendererCategory& category, mCategories )
    {
      const QgsMarkerSymbol * symbol = static_cast<const QgsMarkerSymbol *>( category.symbol() );
      if ( !ddSize.hasDefaultValues() && symbol->dataDefinedSize() != ddSize )
      {
        // no common size expression
        return QgsFeatureRenderer::legendSymbolItemsV2();
      }
      else
      {
        ddSize = symbol->dataDefinedSize();
      }
    }

    if ( !ddSize.isActive() || !ddSize.useExpression() )
    {
      return QgsFeatureRenderer::legendSymbolItemsV2();
    }

    QgsScaleExpression exp( ddSize.expressionString() );
    if ( exp.type() != QgsScaleExpression::Unknown )
    {
      QgsLegendSymbolItem title( nullptr, exp.baseExpression(), "" );
      lst << title;
      Q_FOREACH ( double v, QgsSymbolLayerUtils::prettyBreaks( exp.minValue(), exp.maxValue(), 4 ) )
      {
        QgsLegendSymbolItem si( mSourceSymbol.data(), QString::number( v ), "" );
        QgsMarkerSymbol * s = static_cast<QgsMarkerSymbol *>( si.symbol() );
        s->setDataDefinedSize( QgsDataDefined() );
        s->setSize( exp.size( v ) );
        lst << si;
      }
      // now list the categorized symbols
      const QgsLegendSymbolListV2 list2 = QgsFeatureRenderer::legendSymbolItemsV2() ;
      Q_FOREACH ( const QgsLegendSymbolItem& item, list2 )
        lst << item;
      return lst;
    }
  }

  return QgsFeatureRenderer::legendSymbolItemsV2();
}

QSet<QString> QgsCategorizedSymbolRenderer::legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  QString value = valueForFeature( feature, context ).toString();
  int i = 0;

  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
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

QgsSymbol* QgsCategorizedSymbolRenderer::sourceSymbol()
{
  return mSourceSymbol.data();
}
void QgsCategorizedSymbolRenderer::setSourceSymbol( QgsSymbol* sym )
{
  mSourceSymbol.reset( sym );
}

QgsColorRamp* QgsCategorizedSymbolRenderer::sourceColorRamp()
{
  return mSourceColorRamp.data();
}

void QgsCategorizedSymbolRenderer::setSourceColorRamp( QgsColorRamp* ramp )
{
  mSourceColorRamp.reset( ramp );
}

void QgsCategorizedSymbolRenderer::updateColorRamp( QgsColorRamp* ramp, bool inverted )
{
  setSourceColorRamp( ramp );
  setInvertedColorRamp( inverted );
  double num = mCategories.count() - 1;
  double count = 0;

  QgsRandomColorRamp* randomRamp = dynamic_cast<QgsRandomColorRamp*>( ramp );
  if ( randomRamp )
  {
    //ramp is a random colors ramp, so inform it of the total number of required colors
    //this allows the ramp to pregenerate a set of visually distinctive colors
    randomRamp->setTotalColorCount( mCategories.count() );
  }

  Q_FOREACH ( const QgsRendererCategory &cat, mCategories )
  {
    double value = count / num;
    if ( mInvertedColorRamp ) value = 1.0 - value;
    cat.symbol()->setColor( mSourceColorRamp->color( value ) );
    count += 1;
  }
}

void QgsCategorizedSymbolRenderer::updateSymbols( QgsSymbol * sym )
{
  int i = 0;
  Q_FOREACH ( const QgsRendererCategory& cat, mCategories )
  {
    QgsSymbol* symbol = sym->clone();
    symbol->setColor( cat.symbol()->color() );
    updateCategorySymbol( i, symbol );
    ++i;
  }
  setSourceSymbol( sym->clone() );
}

bool QgsCategorizedSymbolRenderer::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsCategorizedSymbolRenderer::legendSymbolItemChecked( const QString& key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mCategories.size() )
    return mCategories.at( index ).renderState();
  else
    return true;
}

void QgsCategorizedSymbolRenderer::setLegendSymbolItem( const QString& key, QgsSymbol* symbol )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategorySymbol( index, symbol );
  else
    delete symbol;
}

void QgsCategorizedSymbolRenderer::checkLegendSymbolItem( const QString& key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategoryRenderState( index, state );
}

QgsCategorizedSymbolRenderer* QgsCategorizedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  QgsCategorizedSymbolRenderer* r = nullptr;
  if ( renderer->type() == "categorizedSymbol" )
  {
    r = dynamic_cast<QgsCategorizedSymbolRenderer*>( renderer->clone() );
  }
  else if ( renderer->type() == "pointDisplacement" || renderer->type() == "pointCluster" )
  {
    const QgsPointDistanceRenderer* pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer*>( renderer );
    if ( pointDistanceRenderer )
      r = convertFromRenderer( pointDistanceRenderer->embeddedRenderer() );
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
    r = new QgsCategorizedSymbolRenderer( "", QgsCategoryList() );
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
