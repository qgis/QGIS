
#include "qgscategorizedsymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"

#include "qgsfeature.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings> // for legend

QgsRendererCategoryV2::QgsRendererCategoryV2( QVariant value, QgsSymbolV2* symbol, QString label )
    : mValue( value ), mSymbol( symbol ), mLabel( label )
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2( const QgsRendererCategoryV2& cat )
    : mValue( cat.mValue ), mLabel( cat.mLabel )
{
  mSymbol = cat.mSymbol->clone();
}


QgsRendererCategoryV2::~QgsRendererCategoryV2()
{
  delete mSymbol;
}

QVariant QgsRendererCategoryV2::value() const
{
  return mValue;
}

QgsSymbolV2* QgsRendererCategoryV2::symbol() const
{
  return mSymbol;
}

QString QgsRendererCategoryV2::label() const
{
  return mLabel;
}

void QgsRendererCategoryV2::setSymbol( QgsSymbolV2* s )
{
  if ( mSymbol == s )
    return;
  delete mSymbol;
  mSymbol = s;
}

void QgsRendererCategoryV2::setLabel( QString label )
{
  mLabel = label;
}

QString QgsRendererCategoryV2::dump()
{
  return QString( "%1::%2::%3\n" ).arg( mValue.toString() ).arg( mLabel ).arg( mSymbol->dump() );
}

///////////////////

QgsCategorizedSymbolRendererV2::QgsCategorizedSymbolRendererV2( QString attrName, QgsCategoryList categories )
    : QgsFeatureRendererV2( "categorizedSymbol" ),
    mAttrName( attrName ),
    mCategories( categories ),
    mSourceSymbol( NULL ),
    mSourceColorRamp( NULL )
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
  mCategories.clear(); // this should also call destructors of symbols
  delete mSourceSymbol;
  delete mSourceColorRamp;
}

void QgsCategorizedSymbolRendererV2::rebuildHash()
{
  mSymbolHash.clear();

  for ( int i = 0; i < mCategories.count(); ++i )
  {
    QgsRendererCategoryV2& cat = mCategories[i];
    mSymbolHash.insert( cat.value().toString(), cat.symbol() );
  }
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForValue( QVariant value )
{
  // TODO: special case for int, double

  QHash<QString, QgsSymbolV2*>::iterator it = mSymbolHash.find( value.toString() );
  if ( it == mSymbolHash.end() )
  {
    if ( mSymbolHash.count() == 0 )
      QgsDebugMsg( "there are no hashed symbols!!!" );
    else
      QgsDebugMsg( "attribute value not found: " + value.toString() );
    return NULL;
  }
  else
    return *it;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  const QgsAttributeMap& attrMap = feature.attributeMap();
  QgsAttributeMap::const_iterator ita = attrMap.find( mAttrNum );
  if ( ita == attrMap.end() )
  {
    QgsDebugMsg( "attribute '" + mAttrName + "' (index " + QString::number( mAttrNum ) + ") required by renderer not found" );
    return NULL;
  }

  // find the right category
  return symbolForValue( *ita );
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

void QgsCategorizedSymbolRendererV2::startRender( QgsRenderContext& context, const QgsFieldMap& fields )
{
  // make sure that the hash table is up to date
  rebuildHash();

  // find out classification attribute index from name
  mAttrNum = fieldNameIndex( fields, mAttrName );

  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it )
    it->symbol()->startRender( context );
}

void QgsCategorizedSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it )
    it->symbol()->stopRender( context );
}

QList<QString> QgsCategorizedSymbolRendererV2::usedAttributes()
{
  QList<QString> lst;
  lst.append( mAttrName );
  return lst;
}

QString QgsCategorizedSymbolRendererV2::dump()
{
  QString s = QString( "CATEGORIZED: idx %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mCategories.count(); i++ )
    s += mCategories[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::clone()
{
  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2( mAttrName, mCategories );
  if ( mSourceSymbol )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp )
    r->setSourceColorRamp( mSourceColorRamp->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  return r;
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
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbolV2* symbol = symbolMap.take( symbolName );
        cats.append( QgsRendererCategoryV2( value, symbol, label ) );
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
  }

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "categorizedSymbol" );
  rendererElem.setAttribute( "attr", mAttrName );

  // categories
  int i = 0;
  QgsSymbolV2Map symbols;
  QDomElement catsElem = doc.createElement( "categories" );
  QgsCategoryList::const_iterator it = mCategories.constBegin();
  for ( ; it != mCategories.end(); it++ )
  {
    const QgsRendererCategoryV2& cat = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, cat.symbol() );

    QDomElement catElem = doc.createElement( "category" );
    catElem.setAttribute( "value", cat.value().toString() );
    catElem.setAttribute( "symbol", symbolName );
    catElem.setAttribute( "label", cat.label() );
    catsElem.appendChild( catElem );
    i++;
  }

  rendererElem.appendChild( catsElem );

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

  return rendererElem;
}

QgsLegendSymbologyList QgsCategorizedSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();

  QgsLegendSymbologyList lst;
  if ( showClassifiers )
  {
    lst << qMakePair( classAttribute(), QPixmap() );
  }

  int count = categories().count();
  for ( int i = 0; i < count; i++ )
  {
    const QgsRendererCategoryV2& cat = categories()[i];
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( cat.symbol(), iconSize );
    lst << qMakePair( cat.label(), pix );
  }
  return lst;
}


QgsSymbolV2* QgsCategorizedSymbolRendererV2::sourceSymbol()
{
  return mSourceSymbol;
}
void QgsCategorizedSymbolRendererV2::setSourceSymbol( QgsSymbolV2* sym )
{
  delete mSourceSymbol;
  mSourceSymbol = sym;
}

QgsVectorColorRampV2* QgsCategorizedSymbolRendererV2::sourceColorRamp()
{
  return mSourceColorRamp;
}
void QgsCategorizedSymbolRendererV2::setSourceColorRamp( QgsVectorColorRampV2* ramp )
{
  delete mSourceColorRamp;
  mSourceColorRamp = ramp;
}
