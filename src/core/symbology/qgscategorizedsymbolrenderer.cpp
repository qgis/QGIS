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
#include "qgscategorizedsymbolrenderer.h"

#include <algorithm>
#include <memory>

#include "qgsapplication.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsembeddedsymbolrenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsfeature.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgspainteffect.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsproperty.h"
#include "qgsrulebasedrenderer.h"
#include "qgssldexportcontext.h"
#include "qgsstyle.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>
#include <QSettings>
#include <QUuid>

QgsRendererCategory::QgsRendererCategory( const QVariant &value, QgsSymbol *symbol, const QString &label, bool render, const QString &uuid )
  : mValue( value )
  , mSymbol( symbol )
  , mLabel( label )
  , mRender( render )
{
  mUuid = !uuid.isEmpty() ? uuid : QUuid::createUuid().toString();
}

QgsRendererCategory::QgsRendererCategory( const QgsRendererCategory &cat )
  : mValue( cat.mValue )
  , mSymbol( cat.mSymbol ? cat.mSymbol->clone() : nullptr )
  , mLabel( cat.mLabel )
  , mRender( cat.mRender )
  , mUuid( cat.mUuid )
{
}

QgsRendererCategory &QgsRendererCategory::operator=( QgsRendererCategory cat )
{
  if ( &cat == this )
    return *this;

  mValue = cat.mValue;
  mSymbol.reset( cat.mSymbol ? cat.mSymbol->clone() : nullptr );
  mLabel = cat.mLabel;
  mRender = cat.mRender;
  mUuid = cat.mUuid;
  return *this;
}

QgsRendererCategory::~QgsRendererCategory() = default;

QString QgsRendererCategory::uuid() const
{
  return mUuid;
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
  return u"%1::%2::%3:%4\n"_s.arg( mValue.toString(), mLabel, mSymbol->dump() ).arg( mRender );
}

void QgsRendererCategory::toSld( QDomDocument &doc, QDomElement &element, QVariantMap props ) const
{
  if ( !mSymbol.get() || props.value( u"attribute"_s, QString() ).toString().isEmpty() )
    return;

  QString attrName = props[ u"attribute"_s].toString();

  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, attrName, context );
}

bool QgsRendererCategory::toSld( QDomDocument &doc, QDomElement &element, const QString &classAttribute, QgsSldExportContext &context ) const
{
  if ( !mSymbol.get() || classAttribute.isEmpty() )
    return false;

  QString attrName = classAttribute;

  // try to determine if attribute name is actually a field reference or expression.
  // If it's a field reference, we need to quote it.
  // Because we don't have access to the layer or fields here, we treat a parser error
  // as just an unquoted field name (eg a field name with spaces)
  const QgsExpression attrExpression = QgsExpression( attrName );
  if ( attrExpression.hasParserError() )
  {
    attrName = QgsExpression::quotedColumnRef( attrName );
  }
  else if ( attrExpression.isField() )
  {
    attrName = QgsExpression::quotedColumnRef(
                 qgis::down_cast<const QgsExpressionNodeColumnRef *>( attrExpression.rootNode() )->name()
               );
  }

  QDomElement ruleElem = doc.createElement( u"se:Rule"_s );

  QDomElement nameElem = doc.createElement( u"se:Name"_s );
  nameElem.appendChild( doc.createTextNode( mLabel ) );
  ruleElem.appendChild( nameElem );

  QDomElement descrElem = doc.createElement( u"se:Description"_s );
  QDomElement titleElem = doc.createElement( u"se:Title"_s );
  QString descrStr = u"%1 is '%2'"_s.arg( attrName, mValue.toString() );
  titleElem.appendChild( doc.createTextNode( !mLabel.isEmpty() ? mLabel : descrStr ) );
  descrElem.appendChild( titleElem );
  ruleElem.appendChild( descrElem );

  // create the ogc:Filter for the range
  QString filterFunc;
  if ( mValue.userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList list = mValue.toList();
    if ( list.size() == 1 )
    {
      filterFunc = u"%1 = %2"_s.arg( attrName, QgsExpression::quotedValue( list.at( 0 ) ) );
    }
    else
    {
      QStringList valuesList;
      valuesList.reserve( list.size() );
      for ( const QVariant &v : list )
      {
        valuesList << QgsExpression::quotedValue( v );
      }
      filterFunc = u"%1 IN (%2)"_s.arg( attrName,
                                        valuesList.join( ',' ) );
    }
  }
  else if ( QgsVariantUtils::isNull( mValue ) || mValue.toString().isEmpty() )
  {
    filterFunc = u"ELSE"_s;
  }
  else
  {
    filterFunc = u"%1 = %2"_s.arg( attrName, QgsExpression::quotedValue( mValue ) );
  }

  QgsSymbolLayerUtils::createFunctionElement( doc, ruleElem, filterFunc, context );

  // add the mix/max scale denoms if we got any from the callers
  const QVariantMap oldProps = context.extraProperties();
  QVariantMap props = oldProps;
  QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, props );
  context.setExtraProperties( props );
  mSymbol->toSld( doc, ruleElem, context );
  context.setExtraProperties( oldProps );
  if ( !QgsSymbolLayerUtils::hasSldSymbolizer( ruleElem ) )
  {
    // symbol could not be converted to SLD, or is an "empty" symbol. In this case we do not generate a rule, as
    // SLD spec requires a Symbolizer element to be present
    return false;
  }

  element.appendChild( ruleElem );
  return true;
}

///////////////////

QgsCategorizedSymbolRenderer::QgsCategorizedSymbolRenderer( const QString &attrName, const QgsCategoryList &categories )
  : QgsFeatureRenderer( u"categorizedSymbol"_s )
  , mAttrName( attrName )
{
  //important - we need a deep copy of the categories list, not a shared copy. This is required because
  //QgsRendererCategory::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mCategories BUT that same method CAN be used to modify a symbol in place
  for ( const QgsRendererCategory &cat : categories )
  {
    if ( !cat.symbol() )
    {
      QgsDebugError( u"invalid symbol in a category! ignoring..."_s );
    }
    mCategories << cat;
  }
}

Qgis::FeatureRendererFlags QgsCategorizedSymbolRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  QgsCategoryList::const_iterator catIt = mCategories.constBegin();
  for ( ; catIt != mCategories.constEnd(); ++catIt )
  {
    if ( QgsSymbol *catSymbol = catIt->symbol() )
    {
      if ( catSymbol->flags().testFlag( Qgis::SymbolFlag::AffectsLabeling ) )
        res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
    }
  }

  return res;
}

QgsCategorizedSymbolRenderer::~QgsCategorizedSymbolRenderer() = default;

void QgsCategorizedSymbolRenderer::rebuildHash()
{
  mSymbolHash.clear();

  for ( const QgsRendererCategory &cat : std::as_const( mCategories ) )
  {
    const QVariant val = cat.value();
    if ( val.userType() == QMetaType::Type::QVariantList )
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
  QHash<QString, QgsSymbol *>::const_iterator it = mSymbolHash.constFind( QgsVariantUtils::isNull( value ) ? QString() : value.toString() );
  if ( it == mSymbolHash.constEnd() )
  {
    if ( mSymbolHash.isEmpty() )
    {
      QgsDebugError( u"there are no hashed symbols!!!"_s );
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
    QgsDebugError( u"invalid symbol in a category! ignoring..."_s );
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
  return QString::localeAwareCompare( c1.label(), c2.label() ) > 0;
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
    mExpression = std::make_unique<QgsExpression>( mAttrName );
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
  QString s = u"CATEGORIZED: idx %1\n"_s.arg( mAttrName );
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
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsCategorizedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  const QVariantMap oldProps = context.extraProperties();
  QVariantMap newProps = oldProps;
  newProps[ u"attribute"_s] = mAttrName;
  context.setExtraProperties( newProps );

  // create a Rule for each range
  bool result = true;
  for ( QgsCategoryList::const_iterator it = mCategories.constBegin(); it != mCategories.constEnd(); ++it )
  {
    if ( !it->toSld( doc, element, mAttrName, context ) )
      result = false;
  }
  context.setExtraProperties( oldProps );
  return result;
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
    if ( cat.value() == "" || QgsVariantUtils::isNull( cat.value() ) )
    {
      hasDefault = true;
      defaultActive = cat.renderState();
    }

    noneActive = noneActive && !cat.renderState();
    allActive = allActive && cat.renderState();

    const bool isList = cat.value().userType() == QMetaType::Type::QVariantList;
    QString value = QgsExpression::quotedValue( cat.value(), static_cast<QMetaType::Type>( cat.value().userType() ) );

    if ( !cat.renderState() )
    {
      if ( value != "" )
      {
        if ( isList )
        {
          const QVariantList list = cat.value().toList();
          for ( const QVariant &v : list )
          {
            if ( !inactiveValues.isEmpty() )
              inactiveValues.append( ',' );

            inactiveValues.append( QgsExpression::quotedValue( v, isExpression ? static_cast<QMetaType::Type>( v.userType() ) : fields.at( attrNum ).type() ) );
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
      if ( value != "" )
      {
        if ( isList )
        {
          const QVariantList list = cat.value().toList();
          for ( const QVariant &v : list )
          {
            if ( !activeValues.isEmpty() )
              activeValues.append( ',' );

            activeValues.append( QgsExpression::quotedValue( v, isExpression ? static_cast<QMetaType::Type>( v.userType() ) : fields.at( attrNum ).type() ) );
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

  QString attr = isExpression ? mAttrName : u"\"%1\""_s.arg( mAttrName );

  if ( allActive && hasDefault )
  {
    return QString();
  }
  else if ( noneActive )
  {
    return u"FALSE"_s;
  }
  else if ( defaultActive )
  {
    return u"(%1) NOT IN (%2) OR (%1) IS NULL"_s.arg( attr, inactiveValues );
  }
  else
  {
    return u"(%1) IN (%2)"_s.arg( attr, activeValues );
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
  QDomElement symbolsElem = element.firstChildElement( u"symbols"_s );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement catsElem = element.firstChildElement( u"categories"_s );
  if ( catsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
  QgsCategoryList cats;

  // Value from string (long, ulong, double and string)
  const auto valueFromString = []( const QString & value, const QString & valueType ) -> QVariant
  {
    if ( valueType == "double"_L1 )
    {
      bool ok;
      const auto val { value.toDouble( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    else if ( valueType == "ulong"_L1 )
    {
      bool ok;
      const auto val { value.toULongLong( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    else if ( valueType == "long"_L1 )
    {
      bool ok;
      const auto val { value.toLongLong( &ok ) };
      if ( ok )
      {
        return val;
      }
    }
    else if ( valueType == "bool"_L1 )
    {
      if ( value.toLower() == "false"_L1 )
        return false;
      if ( value.toLower() == "true"_L1 )
        return true;
    }
    else if ( valueType == "NULL"_L1 )
    {
      // This is the default ("fallback") category
      return QVariant();
    }
    return value;
  };

  QDomElement catElem = catsElem.firstChildElement();
  int i = 0;
  QSet<QString> usedUuids;
  while ( !catElem.isNull() )
  {
    if ( catElem.tagName() == "category"_L1 )
    {
      QVariant value;
      if ( catElem.hasAttribute( u"value"_s ) )
      {
        value = valueFromString( catElem.attribute( u"value"_s ), catElem.attribute( u"type"_s, QString() ) ) ;
      }
      else
      {
        QVariantList values;
        QDomElement valElem = catElem.firstChildElement();
        while ( !valElem.isNull() )
        {
          if ( valElem.tagName() == "val"_L1 )
          {
            values << valueFromString( valElem.attribute( u"value"_s ), valElem.attribute( u"type"_s, QString() ) );
          }
          valElem = valElem.nextSiblingElement();
        }
        if ( !values.isEmpty() )
          value = values;
      }
      QString symbolName = catElem.attribute( u"symbol"_s );
      QString label = catElem.attribute( u"label"_s );
      bool render = catElem.attribute( u"render"_s ) != "false"_L1;
      QString uuid = catElem.attribute( u"uuid"_s, QString::number( i++ ) );
      while ( usedUuids.contains( uuid ) )
      {
        uuid = QUuid::createUuid().toString();
      }
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbol *symbol = symbolMap.take( symbolName );
        cats.append( QgsRendererCategory( value, symbol, label, render, uuid ) );
        usedUuids << uuid;
      }
    }
    catElem = catElem.nextSiblingElement();
  }

  QString attrName = element.attribute( u"attr"_s );

  QgsCategorizedSymbolRenderer *r = new QgsCategorizedSymbolRenderer( attrName, cats );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( u"source-symbol"_s );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolMap sourceSymbolMap = QgsSymbolLayerUtils::loadSymbols( sourceSymbolElem, context );
    if ( sourceSymbolMap.contains( u"0"_s ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( u"0"_s ) );
    }
    QgsSymbolLayerUtils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( u"colorramp"_s );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( u"name"_s ) == "[source]"_L1 )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ).release() );
  }

  QDomElement rotationElem = element.firstChildElement( u"rotation"_s );
  if ( !rotationElem.isNull() && !rotationElem.attribute( u"field"_s ).isEmpty() )
  {
    for ( const QgsRendererCategory &cat : r->mCategories )
    {
      convertSymbolRotation( cat.symbol(), rotationElem.attribute( u"field"_s ) );
    }
    if ( r->mSourceSymbol )
    {
      convertSymbolRotation( r->mSourceSymbol.get(), rotationElem.attribute( u"field"_s ) );
    }
  }

  QDomElement sizeScaleElem = element.firstChildElement( u"sizescale"_s );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( u"field"_s ).isEmpty() )
  {
    for ( const QgsRendererCategory &cat : r->mCategories )
    {
      convertSymbolSizeScale( cat.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( u"scalemethod"_s ) ),
                              sizeScaleElem.attribute( u"field"_s ) );
    }
    if ( r->mSourceSymbol && r->mSourceSymbol->type() == Qgis::SymbolType::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.get(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( u"scalemethod"_s ) ),
                              sizeScaleElem.attribute( u"field"_s ) );
    }
  }

  QDomElement ddsLegendSizeElem = element.firstChildElement( u"data-defined-size-legend"_s );
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
  rendererElem.setAttribute( u"type"_s, u"categorizedSymbol"_s );
  rendererElem.setAttribute( u"attr"_s, mAttrName );

  // String for type
  // We just need string, bool, and three numeric types: double, ulong and long for unsigned, signed and float/double
  const auto stringForType = []( const QMetaType::Type type ) -> QString
  {
    if ( type == QMetaType::Type::QChar || type == QMetaType::Type::Int || type == QMetaType::Type::LongLong )
    {
      return u"long"_s;
    }
    else if ( type == QMetaType::Type::UInt || type == QMetaType::Type::ULongLong )
    {
      return u"ulong"_s;
    }
    else if ( type == QMetaType::Type::Double )
    {
      return u"double"_s ;
    }
    else if ( type == QMetaType::Type::Bool )
    {
      return u"bool"_s;
    }
    else // Default: string
    {
      return u"string"_s;
    }
  };

  // categories
  if ( !mCategories.isEmpty() )
  {
    int i = 0;
    QgsSymbolMap symbols;
    QDomElement catsElem = doc.createElement( u"categories"_s );
    QgsCategoryList::const_iterator it = mCategories.constBegin();
    for ( ; it != mCategories.constEnd(); ++it )
    {
      const QgsRendererCategory &cat = *it;
      QString symbolName = QString::number( i );
      symbols.insert( symbolName, cat.symbol() );

      QDomElement catElem = doc.createElement( u"category"_s );
      if ( cat.value().userType() == QMetaType::Type::QVariantList )
      {
        const QVariantList list = cat.value().toList();
        for ( const QVariant &v : list )
        {
          QDomElement valueElem = doc.createElement( u"val"_s );
          valueElem.setAttribute( u"value"_s, v.toString() );
          valueElem.setAttribute( u"type"_s, stringForType( static_cast<QMetaType::Type>( v.userType() ) ) );
          catElem.appendChild( valueElem );
        }
      }
      else
      {
        if ( QgsVariantUtils::isNull( cat.value() ) )
        {
          // We need to save NULL value as specific kind, it is the default ("fallback") category
          catElem.setAttribute( u"value"_s, "NULL" );
          catElem.setAttribute( u"type"_s, "NULL" );
        }
        else
        {
          catElem.setAttribute( u"value"_s, cat.value().toString() );
          catElem.setAttribute( u"type"_s, stringForType( static_cast<QMetaType::Type>( cat.value().userType() ) ) );
        }
      }
      catElem.setAttribute( u"symbol"_s, symbolName );
      catElem.setAttribute( u"label"_s, cat.label() );
      catElem.setAttribute( u"render"_s, cat.renderState() ? "true" : "false" );
      catElem.setAttribute( u"uuid"_s, cat.uuid() );
      catsElem.appendChild( catElem );
      i++;
    }
    rendererElem.appendChild( catsElem );

    // save symbols
    QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, u"symbols"_s, doc, context );
    rendererElem.appendChild( symbolsElem );
  }

  // save source symbol
  if ( mSourceSymbol )
  {
    QgsSymbolMap sourceSymbols;
    sourceSymbols.insert( u"0"_s, mSourceSymbol.get() );
    QDomElement sourceSymbolElem = QgsSymbolLayerUtils::saveSymbols( sourceSymbols, u"source-symbol"_s, doc, context );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( u"[source]"_s, mSourceColorRamp.get(), doc );
    rendererElem.appendChild( colorRampElem );
  }

  QDomElement rotationElem = doc.createElement( u"rotation"_s );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( u"sizescale"_s );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( u"data-defined-size-legend"_s );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}


QgsLegendSymbolList QgsCategorizedSymbolRenderer::baseLegendSymbolItems() const
{
  QgsLegendSymbolList lst;
  for ( const QgsRendererCategory &cat : mCategories )
  {
    lst << QgsLegendSymbolItem( cat.symbol(), cat.label(), cat.uuid(), true );
  }
  return lst;
}

QString QgsCategorizedSymbolRenderer::displayString( const QVariant &v, int precision )
{
  return QgsVariantUtils::displayString( v, precision );
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

  for ( const QgsRendererCategory &cat : mCategories )
  {
    bool match = false;
    if ( cat.value().userType() == QMetaType::Type::QVariantList )
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
      // NULL cat value may be stored as an empty string or an invalid variant, depending on how
      // the renderer was constructed and which QGIS version was used
      if ( QgsVariantUtils::isNull( value ) )
      {
        match = cat.value().toString().isEmpty() || QgsVariantUtils::isNull( cat.value() );
      }
      else
      {
        match = value == cat.value();
      }
    }

    if ( match )
    {
      if ( cat.renderState() || mCounting )
        return QSet< QString >() << cat.uuid();
      else
        return QSet< QString >();
    }
  }

  return QSet< QString >();
}

QString QgsCategorizedSymbolRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const
{
  ok = false;
  int i = 0;
  for ( i = 0; i < mCategories.size(); i++ )
  {
    if ( mCategories[i].uuid() == key )
    {
      ok = true;
      break;
    }
  }

  if ( !ok )
  {
    ok = false;
    return QString();
  }

  const int fieldIndex = layer ? layer->fields().lookupField( mAttrName ) : -1;
  const bool isNumeric = layer && fieldIndex >= 0 ? layer->fields().at( fieldIndex ).isNumeric() : false;
  const QMetaType::Type fieldType = layer && fieldIndex >= 0 ? layer->fields().at( fieldIndex ).type() : QMetaType::Type::UnknownType;
  const QString attributeComponent = QgsExpression::quoteFieldExpression( mAttrName, layer );

  ok = true;
  const QgsRendererCategory &cat = mCategories[i];
  if ( cat.value().userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList list = cat.value().toList();
    QStringList parts;
    parts.reserve( list.size() );
    for ( const QVariant &v : list )
    {
      parts.append( QgsExpression::quotedValue( v ) );
    }

    return u"%1 IN (%2)"_s.arg( attributeComponent, parts.join( ", "_L1 ) );
  }
  else
  {
    // Numeric NULL cat value is stored as an empty string
    QVariant value = cat.value();
    if ( isNumeric && value.toString().isEmpty() )
    {
      value = QVariant();
    }

    if ( QgsVariantUtils::isNull( value ) )
      return u"%1 IS NULL"_s.arg( attributeComponent );
    else if ( fieldType == QMetaType::Type::UnknownType )
      return u"%1 = %2"_s.arg( attributeComponent, QgsExpression::quotedValue( value ) );
    else
      return u"%1 = %2"_s.arg( attributeComponent, QgsExpression::quotedValue( value, fieldType ) );
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
  for ( const QgsRendererCategory &category : std::as_const( mCategories ) )
  {
    if ( category.uuid() == key )
    {
      return category.renderState();
    }
  }

  return true;
}

void QgsCategorizedSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  bool ok = false;
  int i = 0;
  for ( i = 0; i < mCategories.size(); i++ )
  {
    if ( mCategories[i].uuid() == key )
    {
      ok = true;
      break;
    }
  }

  if ( ok )
    updateCategorySymbol( i, symbol );
  else
    delete symbol;
}

void QgsCategorizedSymbolRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  for ( int i = 0; i < mCategories.size(); i++ )
  {
    if ( mCategories[i].uuid() == key )
    {
      updateCategoryRenderState( i, state );
      break;
    }
  }
}

QgsCategorizedSymbolRenderer *QgsCategorizedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer, QgsVectorLayer *layer )
{
  std::unique_ptr< QgsCategorizedSymbolRenderer > r;
  if ( renderer->type() == "categorizedSymbol"_L1 )
  {
    r.reset( static_cast<QgsCategorizedSymbolRenderer *>( renderer->clone() ) );
  }
  else if ( renderer->type() == "graduatedSymbol"_L1 )
  {
    const QgsGraduatedSymbolRenderer *graduatedSymbolRenderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( renderer );
    if ( graduatedSymbolRenderer )
    {
      r = std::make_unique<QgsCategorizedSymbolRenderer>( QString(), QgsCategoryList() );
      if ( graduatedSymbolRenderer->sourceSymbol() )
        r->setSourceSymbol( graduatedSymbolRenderer->sourceSymbol()->clone() );
      if ( graduatedSymbolRenderer->sourceColorRamp() )
      {
        r->setSourceColorRamp( graduatedSymbolRenderer->sourceColorRamp()->clone() );
      }
      r->setClassAttribute( graduatedSymbolRenderer->classAttribute() );
    }
  }
  else if ( renderer->type() == "RuleRenderer"_L1 )
  {
    const QgsRuleBasedRenderer *ruleBasedSymbolRenderer = dynamic_cast<const QgsRuleBasedRenderer *>( renderer );
    if ( ruleBasedSymbolRenderer )
    {
      r = std::make_unique<QgsCategorizedSymbolRenderer>( QString(), QgsCategoryList() );

      const QList< QgsRuleBasedRenderer::Rule * > rules = const_cast< QgsRuleBasedRenderer * >( ruleBasedSymbolRenderer )->rootRule()->children();
      bool canConvert = true;

      bool isFirst = true;
      QString attribute;
      QVariant value;
      QgsCategoryList categories;

      for ( QgsRuleBasedRenderer::Rule *rule : rules )
      {
        if ( rule->isElse() || rule->minimumScale() != 0 || rule->maximumScale() != 0 || !rule->symbol() || !rule->children().isEmpty() )
        {
          canConvert = false;
          break;
        }

        QgsExpression e( rule->filterExpression() );

        if ( !e.rootNode() )
        {
          canConvert = false;
          break;
        }

        if ( const QgsExpressionNodeBinaryOperator *binOp = dynamic_cast<const QgsExpressionNodeBinaryOperator *>( e.rootNode() ) )
        {
          if ( binOp->op() == QgsExpressionNodeBinaryOperator::boEQ )
          {
            const QString left = binOp->opLeft()->dump();
            if ( !isFirst && left != attribute )
            {
              canConvert = false;
              break;
            }
            else if ( isFirst )
            {
              attribute = left;
            }

            const QgsExpressionNodeLiteral *literal = dynamic_cast<const QgsExpressionNodeLiteral *>( binOp->opRight() );
            if ( literal )
            {
              QgsRendererCategory cat;
              cat.setValue( literal->value() );
              cat.setSymbol( rule->symbol()->clone() );
              cat.setLabel( rule->label().isEmpty() ? literal->value().toString() : rule->label() );
              cat.setRenderState( rule->active() );
              categories.append( cat );
            }
            else
            {
              canConvert = false;
              break;
            }
          }
          else
          {
            canConvert = false;
          }
        }
        else
        {
          canConvert = false;
          break;
        }

        isFirst = false;
      }

      if ( canConvert )
      {
        r = std::make_unique< QgsCategorizedSymbolRenderer >( attribute, categories );
      }
      else
      {
        r.reset();
      }
    }
  }
  else if ( renderer->type() == "pointDisplacement"_L1 || renderer->type() == "pointCluster"_L1 )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r.reset( convertFromRenderer( pointDistanceRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == "invertedPolygonRenderer"_L1 )
  {
    const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer );
    if ( invertedPolygonRenderer )
      r.reset( convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == "embeddedSymbol"_L1 && layer )
  {
    const QgsEmbeddedSymbolRenderer *embeddedRenderer = dynamic_cast<const QgsEmbeddedSymbolRenderer *>( renderer );
    QgsCategoryList categories;
    QgsFeatureRequest req;
    req.setFlags( Qgis::FeatureRequestFlag::EmbeddedSymbols | Qgis::FeatureRequestFlag::NoGeometry );
    req.setNoAttributes();
    QgsFeatureIterator it = layer->getFeatures( req );
    QgsFeature feature;
    while ( it.nextFeature( feature ) && categories.size() < 2000 )
    {
      if ( feature.embeddedSymbol() )
        categories.append( QgsRendererCategory( feature.id(), feature.embeddedSymbol()->clone(), QString::number( feature.id() ) ) );
    }
    categories.append( QgsRendererCategory( QVariant(), embeddedRenderer->defaultSymbol()->clone(), QString() ) );
    r = std::make_unique<QgsCategorizedSymbolRenderer>( u"$id"_s, categories );
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
      QgsSymbol *newSymbol = symbols.at( 0 )->clone();
      QgsSymbolLayerUtils::resetSymbolLayerIds( newSymbol );
      QgsSymbolLayerUtils::clearSymbolLayerMasks( newSymbol );
      r->setSourceSymbol( newSymbol );
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
  const QSet< QString > allSymbolNames( unmatchedSymbols.begin(), unmatchedSymbols.end() );

  const thread_local QRegularExpression tolerantMatchRe( u"[^\\w\\d ]"_s, QRegularExpression::UseUnicodePropertiesOption );

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
      QgsSymbolLayerUtils::resetSymbolLayerIds( newSymbol );
      if ( !QgsVariantUtils::isNull( value ) )
      {
        const int fieldIdx = fields.lookupField( attributeName );
        QString categoryName = QgsVariantUtils::displayString( value );
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
  QgsSymbolLayerUtils::resetSymbolLayerIds( newSymbol );
  cats.append( QgsRendererCategory( QVariant(), newSymbol, QString(), true ) );

  return cats;
}
