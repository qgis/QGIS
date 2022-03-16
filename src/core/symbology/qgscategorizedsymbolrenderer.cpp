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

#include "qgsdatadefinedsizelegend.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgssymbollayer.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsproperty.h"
#include "qgsstyle.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsembeddedsymbolrenderer.h"
#include "qgsmarkersymbol.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend
#include <QRegularExpression>

QgsRendererCategory::QgsRendererCategory( const QVariant &value, QgsSymbol *symbol, const QString &label, bool render )
  : mValue( value )
  , mSymbol( symbol )
  , mLabel( label )
  , mRender( render )
{
}

QgsRendererCategory::QgsRendererCategory( const QgsRendererCategory &cat )
  : mValue( cat.mValue )
  , mSymbol( cat.mSymbol ? cat.mSymbol->clone() : nullptr )
  , mLabel( cat.mLabel )
  , mRender( cat.mRender )
{
}

// copy+swap idion, the copy is done through the 'pass by value'
QgsRendererCategory &QgsRendererCategory::operator=( QgsRendererCategory cat )
{
  swap( cat );
  return *this;
}

QgsRendererCategory::~QgsRendererCategory() = default;

void QgsRendererCategory::swap( QgsRendererCategory &cat )
{
  std::swap( mValue, cat.mValue );
  std::swap( mSymbol, cat.mSymbol );
  std::swap( mLabel, cat.mLabel );
}

QVariant QgsRendererCategory::value() const
{
  return mValue;
}

QgsSymbol *QgsRendererCategory::symbol() const
{
  return mSymbol.get();
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

void QgsRendererCategory::setSymbol( QgsSymbol *s )
{
  if ( mSymbol.get() != s ) mSymbol.reset( s );
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
  return QStringLiteral( "%1::%2::%3:%4\n" ).arg( mValue.toString(), mLabel, mSymbol->dump() ).arg( mRender );
}

void QgsRendererCategory::toSld( QDomDocument &doc, QDomElement &element, QVariantMap props ) const
{
  if ( !mSymbol.get() || props.value( QStringLiteral( "attribute" ), QString() ).toString().isEmpty() )
    return;

  QString attrName = props[ QStringLiteral( "attribute" )].toString();

  QDomElement ruleElem = doc.createElement( QStringLiteral( "se:Rule" ) );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( QStringLiteral( "se:Name" ) );
  nameElem.appendChild( doc.createTextNode( mLabel ) );
  ruleElem.appendChild( nameElem );

  QDomElement descrElem = doc.createElement( QStringLiteral( "se:Description" ) );
  QDomElement titleElem = doc.createElement( QStringLiteral( "se:Title" ) );
  QString descrStr = QStringLiteral( "%1 is '%2'" ).arg( attrName, mValue.toString() );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc;
  if ( mValue.isNull() || mValue.toString().isEmpty() )
  {
    filterFunc = QStringLiteral( "%1 = '%2' or %1 is null" )
                 .arg( attrName.replace( '\"', QLatin1String( "\"\"" ) ),
                       mValue.toString().replace( '\'', QLatin1String( "''" ) ) );
  }
  else
  {
    filterFunc = QStringLiteral( "%1 = '%2'" )
                 .arg( attrName.replace( '\"', QLatin1String( "\"\"" ) ),
                       mValue.toString().replace( '\'', QLatin1String( "''" ) ) );
  }

  QgsSymbolLayerUtils::createFunctionElement( doc, ruleElem, filterFunc );

  // add the mix/max scale denoms if we got any from the callers
  QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, props );

  mSymbol->toSld( doc, ruleElem, props );
}

///////////////////

QgsCategorizedSymbolRenderer::QgsCategorizedSymbolRenderer( const QString &attrName, const QgsCategoryList &categories )
  : QgsFeatureRenderer( QStringLiteral( "categorizedSymbol" ) )
  , mAttrName( attrName )
{
  //important - we need a deep copy of the categories list, not a shared copy. This is required because
  //QgsRendererCategory::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mCategories BUT that same method CAN be used to modify a symbol in place
  for ( const QgsRendererCategory &cat : categories )
  {
    if ( !cat.symbol() )
    {
      QgsDebugMsg( QStringLiteral( "invalid symbol in a category! ignoring..." ) );
    }
    mCategories << cat;
  }
}

QgsCategorizedSymbolRenderer::~QgsCategorizedSymbolRenderer() = default;

void QgsCategorizedSymbolRenderer::rebuildHash()
{
  mSymbolHash.clear();

  for ( const QgsRendererCategory &cat : std::as_const( mCategories ) )
  {
    const QVariant val = cat.value();
    if ( val.type() == QVariant::List )
    {
      const QVariantList list = val.toList();
      for ( const QVariant &v : list )
      {
        mSymbolHash.insert( v.toString(), ( cat.renderState() || mCounting ) ? cat.symbol() : nullptr );
      }
    }
    else
    {
      mSymbolHash.insert( val.toString(), ( cat.renderState() || mCounting ) ? cat.symbol() : nullptr );
    }
  }
}

QgsSymbol *QgsCategorizedSymbolRenderer::skipRender()
{
  return nullptr;
}

QgsSymbol *QgsCategorizedSymbolRenderer::symbolForValue( const QVariant &value ) const
{
  bool found = false;
  return symbolForValue( value, found );
}

QgsSymbol *QgsCategorizedSymbolRenderer::symbolForValue( const QVariant &value, bool &foundMatchingSymbol ) const
{
  foundMatchingSymbol = false;

  // TODO: special case for int, double
  QHash<QString, QgsSymbol *>::const_iterator it = mSymbolHash.constFind( value.isNull() ? QString() : value.toString() );
  if ( it == mSymbolHash.constEnd() )
  {
    if ( mSymbolHash.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "there are no hashed symbols!!!" ) );
    }
    else
    {
      QgsDebugMsgLevel( "attribute value not found: " + value.toString(), 3 );
    }
    return nullptr;
  }

  foundMatchingSymbol = true;

  return *it;
}

QgsSymbol *QgsCategorizedSymbolRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return originalSymbolForFeature( feature, context );
}

QVariant QgsCategorizedSymbolRenderer::valueForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QgsAttributes attrs = feature.attributes();
  QVariant value;
  if ( mAttrNum == -1 )
  {
    Q_ASSERT( mExpression );

    value = mExpression->evaluate( &context.expressionContext() );
  }
  else
  {
    value = attrs.value( mAttrNum );
  }

  return value;
}

QgsSymbol *QgsCategorizedSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QVariant value = valueForFeature( feature, context );

  bool foundCategory = false;
  // find the right symbol for the category
  QgsSymbol *symbol = symbolForValue( value, foundCategory );

  if ( !foundCategory )
  {
    // if no symbol found, use default symbol
    return symbolForValue( QVariant( "" ), foundCategory );
  }

  return symbol;
}


int QgsCategorizedSymbolRenderer::categoryIndexForValue( const QVariant &val )
{
  for ( int i = 0; i < mCategories.count(); i++ )
  {
    if ( mCategories[i].value() == val )
      return i;
  }
  return -1;
}

int QgsCategorizedSymbolRenderer::categoryIndexForLabel( const QString &val )
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

bool QgsCategorizedSymbolRenderer::updateCategorySymbol( int catIndex, QgsSymbol *symbol )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
    return false;
  mCategories[catIndex].setSymbol( symbol );
  return true;
}

bool QgsCategorizedSymbolRenderer::updateCategoryLabel( int catIndex, const QString &label )
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
    QgsDebugMsg( QStringLiteral( "invalid symbol in a category! ignoring..." ) );
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
    std::sort( mCategories.begin(), mCategories.end(), valueLessThan );
  }
  else
  {
    std::sort( mCategories.begin(), mCategories.end(), valueGreaterThan );
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
    std::sort( mCategories.begin(), mCategories.end(), labelLessThan );
  }
  else
  {
    std::sort( mCategories.begin(), mCategories.end(), labelGreaterThan );
  }
}

void QgsCategorizedSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

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

  for ( const QgsRendererCategory &cat : std::as_const( mCategories ) )
  {
    cat.symbol()->startRender( context, fields );
  }
}

void QgsCategorizedSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  for ( const QgsRendererCategory &cat : std::as_const( mCategories ) )
  {
    cat.symbol()->stopRender( context );
  }
  mExpression.reset();
}

QSet<QString> QgsCategorizedSymbolRenderer::usedAttributes( const QgsRenderContext &context ) const
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
    QgsSymbol *catSymbol = catIt->symbol();
    if ( catSymbol )
    {
      attributes.unite( catSymbol->usedAttributes( context ) );
    }
  }
  return attributes;
}

bool QgsCategorizedSymbolRenderer::filterNeedsGeometry() const
{
  QgsExpression testExpr( mAttrName );
  if ( !testExpr.hasParserError() )
  {
    QgsExpressionContext context;
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) ); // unfortunately no layer access available!
    testExpr.prepare( &context );
    return testExpr.needsGeometry();
  }
  return false;
}

QString QgsCategorizedSymbolRenderer::dump() const
{
  QString s = QStringLiteral( "CATEGORIZED: idx %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mCategories.count(); i++ )
    s += mCategories[i].dump();
  return s;
}

QgsCategorizedSymbolRenderer *QgsCategorizedSymbolRenderer::clone() const
{
  QgsCategorizedSymbolRenderer *r = new QgsCategorizedSymbolRenderer( mAttrName, mCategories );
  if ( mSourceSymbol )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp )
  {
    r->setSourceColorRamp( mSourceColorRamp->clone() );
  }
  r->setDataDefinedSizeLegend( mDataDefinedSizeLegend ? new QgsDataDefinedSizeLegend( *mDataDefinedSizeLegend ) : nullptr );

  copyRendererData( r );
  return r;
}

void QgsCategorizedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QVariantMap newProps = props;
  newProps[ QStringLiteral( "attribute" )] = mAttrName;

  // create a Rule for each range
  for ( QgsCategoryList::const_iterator it = mCategories.constBegin(); it != mCategories.constEnd(); ++it )
  {
    it->toSld( doc, element, newProps );
  }
}

QString QgsCategorizedSymbolRenderer::filter( const QgsFields &fields )
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

  for ( const QgsRendererCategory &cat : std::as_const( mCategories ) )
  {
    if ( cat.value() == "" || cat.value().isNull() )
    {
      hasDefault = true;
      defaultActive = cat.renderState();
    }

    noneActive = noneActive && !cat.renderState();
    allActive = allActive && cat.renderState();

    QVariant::Type valType = isExpression ? cat.value().type() : fields.at( attrNum ).type();
    const bool isList = cat.value().type() == QVariant::List;
    QString value = QgsExpression::quotedValue( cat.value(), valType );

    if ( !cat.renderState() )
    {
      if ( cat.value() != "" )
      {
        if ( isList )
        {
          const QVariantList list = cat.value().toList();
          for ( const QVariant &v : list )
          {
            if ( !inactiveValues.isEmpty() )
              inactiveValues.append( ',' );

            inactiveValues.append( QgsExpression::quotedValue( v, isExpression ? v.type() : fields.at( attrNum ).type() ) );
          }
        }
        else
        {
          if ( !inactiveValues.isEmpty() )
            inactiveValues.append( ',' );

          inactiveValues.append( value );
        }
      }
    }
    else
    {
      if ( cat.value() != "" )
      {
        if ( isList )
        {
          const QVariantList list = cat.value().toList();
          for ( const QVariant &v : list )
          {
            if ( !activeValues.isEmpty() )
              activeValues.append( ',' );

            activeValues.append( QgsExpression::quotedValue( v, isExpression ? v.type() : fields.at( attrNum ).type() ) );
          }
        }
        else
        {
          if ( !activeValues.isEmpty() )
            activeValues.append( ',' );

          activeValues.append( value );
        }
      }
    }
  }

  QString attr = isExpression ? mAttrName : QStringLiteral( "\"%1\"" ).arg( mAttrName );

  if ( allActive && hasDefault )
  {
    return QString();
  }
  else if ( noneActive )
  {
    return QStringLiteral( "FALSE" );
  }
  else if ( defaultActive )
  {
    return QStringLiteral( "(%1) NOT IN (%2) OR (%1) IS NULL" ).arg( attr, inactiveValues );
  }
  else
  {
    return QStringLiteral( "(%1) IN (%2)" ).arg( attr, activeValues );
  }
}

QgsSymbolList QgsCategorizedSymbolRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  QgsSymbolList lst;
  lst.reserve( mCategories.count() );
  for ( const QgsRendererCategory &cat : mCategories )
  {
    lst.append( cat.symbol() );
  }
  return lst;
}

bool QgsCategorizedSymbolRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  for ( const QgsRendererCategory &cat : mCategories )
  {
    QgsStyleSymbolEntity entity( cat.symbol() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, cat.value().toString(), cat.label() ) ) )
      return false;
  }

  if ( mSourceColorRamp )
  {
    QgsStyleColorRampEntity entity( mSourceColorRamp.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }

  return true;
}

QgsFeatureRenderer *QgsCategorizedSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement catsElem = element.firstChildElement( QStringLiteral( "categories" ) );
  if ( catsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
  QgsCategoryList cats;

  // Value from string (long, ulong, double and string)
  const auto valueFromString = []( const QString & value, const QString & valueType ) -> QVariant
  {
    if ( valueType == QLatin1String( "double" ) )
    {
      bool ok;
      const auto val { value.toDouble( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    else if ( valueType == QLatin1String( "ulong" ) )
    {
      bool ok;
      const auto val { value.toULongLong( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    else if ( valueType == QLatin1String( "long" ) )
    {
      bool ok;
      const auto val { value.toLongLong( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    return value;
  };

  QDomElement catElem = catsElem.firstChildElement();
  while ( !catElem.isNull() )
  {
    if ( catElem.tagName() == QLatin1String( "category" ) )
    {
      QVariant value;
      if ( catElem.hasAttribute( QStringLiteral( "value" ) ) )
      {
        value = valueFromString( catElem.attribute( QStringLiteral( "value" ) ), catElem.attribute( QStringLiteral( "type" ), QString() ) ) ;

      }
      else
      {
        QVariantList values;
        QDomElement valElem = catElem.firstChildElement();
        while ( !valElem.isNull() )
        {
          if ( valElem.tagName() == QLatin1String( "val" ) )
          {
            values << valueFromString( valElem.attribute( QStringLiteral( "value" ) ), valElem.attribute( QStringLiteral( "type" ), QString() ) );
          }
          valElem = valElem.nextSiblingElement();
        }
        if ( !values.isEmpty() )
          value = values;
      }
      QString symbolName = catElem.attribute( QStringLiteral( "symbol" ) );
      QString label = catElem.attribute( QStringLiteral( "label" ) );
      bool render = catElem.attribute( QStringLiteral( "render" ) ) != QLatin1String( "false" );
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbol *symbol = symbolMap.take( symbolName );
        cats.append( QgsRendererCategory( value, symbol, label, render ) );
      }
    }
    catElem = catElem.nextSiblingElement();
  }

  QString attrName = element.attribute( QStringLiteral( "attr" ) );

  QgsCategorizedSymbolRenderer *r = new QgsCategorizedSymbolRenderer( attrName, cats );

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

  QDomElement rotationElem = element.firstChildElement( QStringLiteral( "rotation" ) );
  if ( !rotationElem.isNull() && !rotationElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    for ( const QgsRendererCategory &cat : r->mCategories )
    {
      convertSymbolRotation( cat.symbol(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol )
    {
      convertSymbolRotation( r->mSourceSymbol.get(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
  }

  QDomElement sizeScaleElem = element.firstChildElement( QStringLiteral( "sizescale" ) );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    for ( const QgsRendererCategory &cat : r->mCategories )
    {
      convertSymbolSizeScale( cat.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol && r->mSourceSymbol->type() == Qgis::SymbolType::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.get(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
  }

  QDomElement ddsLegendSizeElem = element.firstChildElement( QStringLiteral( "data-defined-size-legend" ) );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  // clazy:skip
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "categorizedSymbol" ) );
  rendererElem.setAttribute( QStringLiteral( "attr" ), mAttrName );

  // String for type
  // We just need string and three numeric types: double, ulong and long for unsigned, signed and float/double
  const auto stringForType = []( const QVariant::Type type ) -> QString
  {
    if ( type == QVariant::Char || type == QVariant::Int || type == QVariant::LongLong )
    {
      return QStringLiteral( "long" );
    }
    else if ( type == QVariant::UInt || type == QVariant::ULongLong )
    {
      return QStringLiteral( "ulong" );
    }
    else if ( type == QVariant::Double )
    {
      return QStringLiteral( "double" ) ;
    }
    else // Default: string
    {
      return QStringLiteral( "string" );
    }
  };

  // categories
  if ( !mCategories.isEmpty() )
  {
    int i = 0;
    QgsSymbolMap symbols;
    QDomElement catsElem = doc.createElement( QStringLiteral( "categories" ) );
    QgsCategoryList::const_iterator it = mCategories.constBegin();
    for ( ; it != mCategories.constEnd(); ++it )
    {
      const QgsRendererCategory &cat = *it;
      QString symbolName = QString::number( i );
      symbols.insert( symbolName, cat.symbol() );

      QDomElement catElem = doc.createElement( QStringLiteral( "category" ) );
      if ( cat.value().type() == QVariant::List )
      {
        const QVariantList list = cat.value().toList();
        for ( const QVariant &v : list )
        {
          QDomElement valueElem = doc.createElement( QStringLiteral( "val" ) );
          valueElem.setAttribute( QStringLiteral( "value" ), v.toString() );
          valueElem.setAttribute( QStringLiteral( "type" ), stringForType( v.type() ) );
          catElem.appendChild( valueElem );
        }
      }
      else
      {
        catElem.setAttribute( QStringLiteral( "value" ), cat.value().toString() );
        catElem.setAttribute( QStringLiteral( "type" ), stringForType( cat.value().type() ) );
      }
      catElem.setAttribute( QStringLiteral( "symbol" ), symbolName );
      catElem.setAttribute( QStringLiteral( "label" ), cat.label() );
      catElem.setAttribute( QStringLiteral( "render" ), cat.renderState() ? "true" : "false" );
      catsElem.appendChild( catElem );
      i++;
    }
    rendererElem.appendChild( catsElem );

    // save symbols
    QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
    rendererElem.appendChild( symbolsElem );
  }

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

  QDomElement rotationElem = doc.createElement( QStringLiteral( "rotation" ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( QStringLiteral( "sizescale" ) );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( QStringLiteral( "data-defined-size-legend" ) );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}


QgsLegendSymbolList QgsCategorizedSymbolRenderer::baseLegendSymbolItems() const
{
  QgsLegendSymbolList lst;
  int i = 0;
  for ( const QgsRendererCategory &cat : mCategories )
  {
    lst << QgsLegendSymbolItem( cat.symbol(), cat.label(), QString::number( i++ ), true );
  }
  return lst;
}

QString QgsCategorizedSymbolRenderer::displayString( const QVariant &v, int precision )
{

  auto _displayString = [ ]( const QVariant & v, int precision ) -> QString
  {

    if ( v.isNull() )
    {
      return QgsApplication::nullRepresentation();
    }

    const bool isNumeric {v.type() == QVariant::Double || v.type() == QVariant::Int || v.type() == QVariant::UInt || v.type() == QVariant::LongLong || v.type() == QVariant::ULongLong};

    // Special treatment for numeric types if group separator is set or decimalPoint is not a dot
    if ( v.type() == QVariant::Double )
    {
      // if value doesn't contain a double (a default value expression for instance),
      // apply no transformation
      bool ok;
      v.toDouble( &ok );
      if ( !ok )
        return v.toString();

      // Locales with decimal point != '.' or that require group separator: use QLocale
      if ( QLocale().decimalPoint() != '.' ||
           !( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator ) )
      {
        if ( precision > 0 )
        {
          if ( -1 < v.toDouble() && v.toDouble() < 1 )
          {
            return QLocale().toString( v.toDouble(), 'g', precision );
          }
          else
          {
            return QLocale().toString( v.toDouble(), 'f', precision );
          }
        }
        else
        {
          // Precision is not set, let's guess it from the
          // standard conversion to string
          const QString s( v.toString() );
          const int dotPosition( s.indexOf( '.' ) );
          int precision;
          if ( dotPosition < 0 && s.indexOf( 'e' ) < 0 )
          {
            precision = 0;
            return QLocale().toString( v.toDouble(), 'f', precision );
          }
          else
          {
            if ( dotPosition < 0 ) precision = 0;
            else precision = s.length() - dotPosition - 1;

            if ( -1 < v.toDouble() && v.toDouble() < 1 )
            {
              return QLocale().toString( v.toDouble(), 'g', precision );
            }
            else
            {
              return QLocale().toString( v.toDouble(), 'f', precision );
            }
          }
        }
      }
      // Default for doubles with precision
      else if ( precision > 0 )
      {
        if ( -1 < v.toDouble() && v.toDouble() < 1 )
        {
          return QString::number( v.toDouble(), 'g', precision );
        }
        else
        {
          return QString::number( v.toDouble(), 'f', precision );
        }
      }
    }
    // Other numeric types than doubles
    else if ( isNumeric &&
              !( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator ) )
    {
      bool ok;
      const qlonglong converted( v.toLongLong( &ok ) );
      if ( ok )
        return QLocale().toString( converted );
    }
    else if ( v.type() == QVariant::ByteArray )
    {
      return QObject::tr( "BLOB" );
    }

    // Fallback if special rules do not apply
    return v.toString();
  };

  if ( v.type() == QVariant::StringList || v.type() == QVariant::List )
  {
    // Note that this code is never hit because the joining of lists (merged categories) happens
    // in data(); I'm leaving this here anyway because it is tested and it may be useful for
    // other purposes in the future.
    QString result;
    const QVariantList list = v.toList();
    for ( const QVariant &var : list )
    {
      if ( !result.isEmpty() )
      {
        result.append( ';' );
      }
      result.append( _displayString( var, precision ) );
    }
    return result;
  }
  else
  {
    return _displayString( v, precision );
  }
}

QgsLegendSymbolList QgsCategorizedSymbolRenderer::legendSymbolItems() const
{
  if ( mDataDefinedSizeLegend && mSourceSymbol && mSourceSymbol->type() == Qgis::SymbolType::Marker )
  {
    // check that all symbols that have the same size expression
    QgsProperty ddSize;
    for ( const QgsRendererCategory &category : mCategories )
    {
      const QgsMarkerSymbol *symbol = static_cast<const QgsMarkerSymbol *>( category.symbol() );
      if ( ddSize )
      {
        QgsProperty sSize( symbol->dataDefinedSize() );
        if ( sSize != ddSize )
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

QSet<QString> QgsCategorizedSymbolRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  const QVariant value = valueForFeature( feature, context );
  int i = 0;

  for ( const QgsRendererCategory &cat : mCategories )
  {
    bool match = false;
    if ( cat.value().type() == QVariant::List )
    {
      const QVariantList list = cat.value().toList();
      for ( const QVariant &v : list )
      {
        if ( value == v )
        {
          match = true;
          break;
        }
      }
    }
    else
    {
      // Numeric NULL cat value is stored as an empty string
      if ( value.isNull() && ( value.type() == QVariant::Double || value.type() == QVariant::Int ||
                               value.type() == QVariant::UInt || value.type() == QVariant::LongLong ||
                               value.type() == QVariant::ULongLong ) )
      {
        match = cat.value().toString().isEmpty();
      }
      else
      {
        match = value == cat.value();
      }
    }

    if ( match )
    {
      if ( cat.renderState() || mCounting )
        return QSet< QString >() << QString::number( i );
      else
        return QSet< QString >();
    }
    i++;
  }

  return QSet< QString >();
}

QString QgsCategorizedSymbolRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const
{
  ok = false;
  int ruleIndex = key.toInt( &ok );
  if ( !ok || ruleIndex < 0 || ruleIndex >= mCategories.size() )
  {
    ok = false;
    return QString();
  }

  const int fieldIndex = layer ? layer->fields().lookupField( mAttrName ) : -1;
  const bool isNumeric = layer && fieldIndex >= 0 ? layer->fields().at( fieldIndex ).isNumeric() : false;
  const QVariant::Type fieldType = layer && fieldIndex >= 0 ? layer->fields().at( fieldIndex ).type() : QVariant::Invalid;
  const QString attributeComponent = QgsExpression::quoteFieldExpression( mAttrName, layer );

  ok = true;
  const QgsRendererCategory &cat = mCategories[ ruleIndex ];
  if ( cat.value().type() == QVariant::List )
  {
    const QVariantList list = cat.value().toList();
    QStringList parts;
    parts.reserve( list.size() );
    for ( const QVariant &v : list )
    {
      parts.append( QgsExpression::quotedValue( v ) );
    }

    return QStringLiteral( "%1 IN (%2)" ).arg( attributeComponent, parts.join( QStringLiteral( ", " ) ) );
  }
  else
  {
    // Numeric NULL cat value is stored as an empty string
    QVariant value = cat.value();
    if ( isNumeric && value.toString().isEmpty() )
    {
      value = QVariant();
    }

    if ( value.isNull() )
      return QStringLiteral( "%1 IS NULL" ).arg( attributeComponent );
    else if ( fieldType == QVariant::Type::Invalid )
      return QStringLiteral( "%1 = %2" ).arg( attributeComponent, QgsExpression::quotedValue( value ) );
    else
      return QStringLiteral( "%1 = %2" ).arg( attributeComponent, QgsExpression::quotedValue( value, fieldType ) );
  }
}

QgsSymbol *QgsCategorizedSymbolRenderer::sourceSymbol()
{
  return mSourceSymbol.get();
}

const QgsSymbol *QgsCategorizedSymbolRenderer::sourceSymbol() const
{
  return mSourceSymbol.get();
}

void QgsCategorizedSymbolRenderer::setSourceSymbol( QgsSymbol *sym )
{
  mSourceSymbol.reset( sym );
}

QgsColorRamp *QgsCategorizedSymbolRenderer::sourceColorRamp()
{
  return mSourceColorRamp.get();
}

const QgsColorRamp *QgsCategorizedSymbolRenderer::sourceColorRamp() const
{
  return mSourceColorRamp.get();
}

void QgsCategorizedSymbolRenderer::setSourceColorRamp( QgsColorRamp *ramp )
{
  mSourceColorRamp.reset( ramp );
}

void QgsCategorizedSymbolRenderer::updateColorRamp( QgsColorRamp *ramp )
{
  setSourceColorRamp( ramp );
  double num = mCategories.count() - 1;
  double count = 0;

  QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp );
  if ( randomRamp )
  {
    //ramp is a random colors ramp, so inform it of the total number of required colors
    //this allows the ramp to pregenerate a set of visually distinctive colors
    randomRamp->setTotalColorCount( mCategories.count() );
  }

  for ( const QgsRendererCategory &cat : mCategories )
  {
    double value = count / num;
    cat.symbol()->setColor( mSourceColorRamp->color( value ) );
    count += 1;
  }
}

void QgsCategorizedSymbolRenderer::updateSymbols( QgsSymbol *sym )
{
  int i = 0;
  for ( const QgsRendererCategory &cat : mCategories )
  {
    QgsSymbol *symbol = sym->clone();
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

bool QgsCategorizedSymbolRenderer::legendSymbolItemChecked( const QString &key )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mCategories.size() )
    return mCategories.at( index ).renderState();
  else
    return true;
}

void QgsCategorizedSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategorySymbol( index, symbol );
  else
    delete symbol;
}

void QgsCategorizedSymbolRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateCategoryRenderState( index, state );
}

QgsCategorizedSymbolRenderer *QgsCategorizedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer, QgsVectorLayer *layer )
{
  std::unique_ptr< QgsCategorizedSymbolRenderer > r;
  if ( renderer->type() == QLatin1String( "categorizedSymbol" ) )
  {
    r.reset( static_cast<QgsCategorizedSymbolRenderer *>( renderer->clone() ) );
  }
  else if ( renderer->type() == QLatin1String( "graduatedSymbol" ) )
  {
    const QgsGraduatedSymbolRenderer *graduatedSymbolRenderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( renderer );
    if ( graduatedSymbolRenderer )
    {
      r.reset( new QgsCategorizedSymbolRenderer( QString(), QgsCategoryList() ) );
      if ( graduatedSymbolRenderer->sourceSymbol() )
        r->setSourceSymbol( graduatedSymbolRenderer->sourceSymbol()->clone() );
      if ( graduatedSymbolRenderer->sourceColorRamp() )
      {
        r->setSourceColorRamp( graduatedSymbolRenderer->sourceColorRamp()->clone() );
      }
      r->setClassAttribute( graduatedSymbolRenderer->classAttribute() );
    }
  }
  else if ( renderer->type() == QLatin1String( "pointDisplacement" ) || renderer->type() == QLatin1String( "pointCluster" ) )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r.reset( convertFromRenderer( pointDistanceRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer );
    if ( invertedPolygonRenderer )
      r.reset( convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == QLatin1String( "embeddedSymbol" ) && layer )
  {
    const QgsEmbeddedSymbolRenderer *embeddedRenderer = dynamic_cast<const QgsEmbeddedSymbolRenderer *>( renderer );
    QgsCategoryList categories;
    QgsFeatureRequest req;
    req.setFlags( QgsFeatureRequest::EmbeddedSymbols | QgsFeatureRequest::NoGeometry );
    req.setNoAttributes();
    QgsFeatureIterator it = layer->getFeatures( req );
    QgsFeature feature;
    while ( it.nextFeature( feature ) && categories.size() < 2000 )
    {
      if ( feature.embeddedSymbol() )
        categories.append( QgsRendererCategory( feature.id(), feature.embeddedSymbol()->clone(), QString::number( feature.id() ) ) );
    }
    categories.append( QgsRendererCategory( QVariant(), embeddedRenderer->defaultSymbol()->clone(), QString() ) );
    r.reset( new QgsCategorizedSymbolRenderer( QStringLiteral( "$id" ), categories ) );
  }

  // If not one of the specifically handled renderers, then just grab the symbol from the renderer
  // Could have applied this to specific renderer types (singleSymbol, graduatedSymbol)

  if ( !r )
  {
    r = std::make_unique< QgsCategorizedSymbolRenderer >( QString(), QgsCategoryList() );
    QgsRenderContext context;
    QgsSymbolList symbols = const_cast<QgsFeatureRenderer *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r->setSourceSymbol( symbols.at( 0 )->clone() );
    }
  }

  renderer->copyRendererData( r.get() );

  return r.release();
}

void QgsCategorizedSymbolRenderer::setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings )
{
  mDataDefinedSizeLegend.reset( settings );
}

QgsDataDefinedSizeLegend *QgsCategorizedSymbolRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend.get();
}

int QgsCategorizedSymbolRenderer::matchToSymbols( QgsStyle *style, Qgis::SymbolType type, QVariantList &unmatchedCategories, QStringList &unmatchedSymbols, const bool caseSensitive, const bool useTolerantMatch )
{
  if ( !style )
    return 0;

  int matched = 0;
  unmatchedSymbols = style->symbolNames();
  const QSet< QString > allSymbolNames = qgis::listToSet( unmatchedSymbols );

  const QRegularExpression tolerantMatchRe( QStringLiteral( "[^\\w\\d ]" ), QRegularExpression::UseUnicodePropertiesOption );

  for ( int catIdx = 0; catIdx < mCategories.count(); ++catIdx )
  {
    const QVariant value = mCategories.at( catIdx ).value();
    const QString val = value.toString().trimmed();
    std::unique_ptr< QgsSymbol > symbol( style->symbol( val ) );
    // case-sensitive match
    if ( symbol && symbol->type() == type )
    {
      matched++;
      unmatchedSymbols.removeAll( val );
      updateCategorySymbol( catIdx, symbol.release() );
      continue;
    }

    if ( !caseSensitive || useTolerantMatch )
    {
      QString testVal = val;
      if ( useTolerantMatch )
        testVal.replace( tolerantMatchRe, QString() );

      bool foundMatch = false;
      for ( const QString &name : allSymbolNames )
      {
        QString testName = name.trimmed();
        if ( useTolerantMatch )
          testName.replace( tolerantMatchRe, QString() );

        if ( testName == testVal || ( !caseSensitive && testName.trimmed().compare( testVal, Qt::CaseInsensitive ) == 0 ) )
        {
          // found a case-insensitive match
          std::unique_ptr< QgsSymbol > symbol( style->symbol( name ) );
          if ( symbol && symbol->type() == type )
          {
            matched++;
            unmatchedSymbols.removeAll( name );
            updateCategorySymbol( catIdx, symbol.release() );
            foundMatch = true;
            break;
          }
        }
      }
      if ( foundMatch )
        continue;
    }

    unmatchedCategories << value;
  }

  return matched;
}

QgsCategoryList QgsCategorizedSymbolRenderer::createCategories( const QList<QVariant> &values, const QgsSymbol *symbol, QgsVectorLayer *layer, const QString &attributeName )
{
  QgsCategoryList cats;
  QVariantList vals = values;
  // sort the categories first
  QgsSymbolLayerUtils::sortVariantList( vals, Qt::AscendingOrder );

  if ( layer && !attributeName.isNull() )
  {
    const QgsFields fields = layer->fields();
    for ( const QVariant &value : vals )
    {
      QgsSymbol *newSymbol = symbol->clone();
      if ( !value.isNull() )
      {
        const int fieldIdx = fields.lookupField( attributeName );
        QString categoryName = displayString( value );
        if ( fieldIdx != -1 )
        {
          const QgsField field = fields.at( fieldIdx );
          const QgsEditorWidgetSetup setup = field.editorWidgetSetup();
          const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
          categoryName = formatter->representValue( layer, fieldIdx, setup.config(), QVariant(), value );
        }
        cats.append( QgsRendererCategory( value, newSymbol,  categoryName, true ) );
      }
    }
  }

  // add null (default) value
  QgsSymbol *newSymbol = symbol->clone();
  cats.append( QgsRendererCategory( QVariant(), newSymbol, QString(), true ) );

  return cats;
}

