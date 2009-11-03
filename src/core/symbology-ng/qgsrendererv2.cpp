#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgslogger.h"

#include <QDomElement>
#include <QDomDocument>
#include <QPolygonF>



static unsigned char* _getPoint(QPointF& pt, const QgsMapToPixel& mapToPixel, unsigned char* wkb)
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof(unsigned int);
  
  double x = *(( double * ) wkb ); wkb += sizeof(double);
  double y = *(( double * ) wkb ); wkb += sizeof(double);
  
  if ( wkbType == QGis::WKBPolygon25D )
    wkb += sizeof(double);
  
  mapToPixel.transformInPlace(x,y);
  // TODO: coordinate transform
  pt = QPointF(x,y);
  return wkb;
}

static unsigned char* _getLineString(QPolygonF& pts, const QgsMapToPixel& mapToPixel, unsigned char* wkb)
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof(unsigned int);
  unsigned int nPoints = *(( int* ) wkb );
  wkb += sizeof(unsigned int);
  
  bool hasZValue = ( wkbType == QGis::WKBLineString25D );
  double x,y;

  pts.resize(nPoints);

  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    x = *(( double * ) wkb );
    wkb += sizeof( double );
    y = *(( double * ) wkb );
    wkb += sizeof( double );

    if ( hasZValue ) // ignore Z value
      wkb += sizeof( double );
    
    mapToPixel.transformInPlace(x,y);
    
    pts[i] = QPointF(x,y);
    
    // TODO: coordinate transform
  }
  
  return wkb;
}

static unsigned char* _getPolygon(QPolygonF& pts, QList<QPolygonF>& holes, const QgsMapToPixel& mapToPixel, unsigned char* wkb)
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof(unsigned int); // jump over wkb type
  unsigned int numRings = *(( int* ) wkb );
  wkb += sizeof(unsigned int);
  
  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return wkb;

  bool hasZValue = ( wkbType == QGis::WKBPolygon25D );
  double x,y;
  holes.clear();

  for ( unsigned int idx = 0; idx < numRings; idx++ )
  {
    unsigned int nPoints = *(( int* )wkb );
    wkb += sizeof(unsigned int);

    QPolygonF poly(nPoints);

    // Extract the points from the WKB and store in a pair of vectors.
    for ( unsigned int jdx = 0; jdx < nPoints; jdx++ )
    {
      x = *(( double * ) wkb ); wkb += sizeof( double );
      y = *(( double * ) wkb ); wkb += sizeof( double );
      
      mapToPixel.transformInPlace(x,y);
      // TODO: coordinate transform

      poly[jdx] = QPointF(x,y);

      if ( hasZValue )
        wkb += sizeof( double );
    }
    
    if ( nPoints < 1 )
      continue;
    
    if (idx == 0)
      pts = poly;
    else
      holes.append(poly);
  }
  
  return wkb;
}


QgsFeatureRendererV2::QgsFeatureRendererV2(RendererType type)
  : mType(type), mUsingSymbolLevels(false)
{
}

QgsFeatureRendererV2* QgsFeatureRendererV2::defaultRenderer(QGis::GeometryType geomType)
{
  return new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol(geomType) );
}


void QgsFeatureRendererV2::renderFeature(QgsFeature& feature, QgsRenderContext& context, int layer)
{
  QgsSymbolV2* symbol = symbolForFeature(feature);
  if (symbol == NULL)
    return;

  QgsSymbolV2::SymbolType symbolType = symbol->type();
  
  QgsGeometry* geom = feature.geometry();
  switch (geom->wkbType())
  {
    case QGis::WKBPoint:
      {
        if (symbolType != QgsSymbolV2::Marker)
        {
          QgsDebugMsg("point can be drawn only with marker symbol!");
          break;
        }
        QPointF pt;
        _getPoint(pt, context.mapToPixel(), geom->asWkb());
        ((QgsMarkerSymbolV2*)symbol)->renderPoint(pt, context, layer);
      }
      break;
      
    case QGis::WKBLineString:
      {
        if (symbolType != QgsSymbolV2::Line)
        {
          QgsDebugMsg("linestring can be drawn only with line symbol!");
          break;
        }
        QPolygonF pts;
        _getLineString(pts, context.mapToPixel(), geom->asWkb());
        ((QgsLineSymbolV2*)symbol)->renderPolyline(pts, context, layer);
      }
      break;
	   
    case QGis::WKBPolygon:
      {
        if (symbolType != QgsSymbolV2::Fill)
        {
          QgsDebugMsg("polygon can be drawn only with fill symbol!");
          break;
        }
        QPolygonF pts;
        QList<QPolygonF> holes;
        _getPolygon(pts, holes, context.mapToPixel(), geom->asWkb());
        ((QgsFillSymbolV2*)symbol)->renderPolygon(pts, (holes.count() ? &holes : NULL), context, layer);
      }
      break;
      
    case QGis::WKBMultiPoint:
      {
        if (symbolType != QgsSymbolV2::Marker)
        {
          QgsDebugMsg("multi-point can be drawn only with marker symbol!");
          break;
        }
        
        unsigned char* wkb = geom->asWkb();
        unsigned int num = *(( int* )(wkb + 5) );
        unsigned char* ptr = wkb + 9;
        QPointF pt;
        
        for (unsigned int i = 0; i < num; ++i)
        {
          ptr = _getPoint(pt, context.mapToPixel(), ptr);
          ((QgsMarkerSymbolV2*)symbol)->renderPoint(pt, context, layer);
        }
      }
      break;
    
    case QGis::WKBMultiLineString:
      {
        if (symbolType != QgsSymbolV2::Line)
        {
          QgsDebugMsg("multi-linestring can be drawn only with line symbol!");
          break;
        }
        
        unsigned char* wkb = geom->asWkb();
        unsigned int num = *(( int* )(wkb + 5) );
        unsigned char* ptr = wkb + 9;
        QPolygonF pts;
        
        for (unsigned int i = 0; i < num; ++i)
        {
          ptr = _getLineString(pts, context.mapToPixel(), ptr);
          ((QgsLineSymbolV2*)symbol)->renderPolyline(pts, context, layer);
        }
      }
      break;
    
    case QGis::WKBMultiPolygon:
      {
        if (symbolType != QgsSymbolV2::Fill)
        {
          QgsDebugMsg("multi-polygon can be drawn only with fill symbol!");
          break;
        }
        
        unsigned char* wkb = geom->asWkb();
        unsigned int num = *(( int* )(wkb + 5) );
        unsigned char* ptr = wkb + 9;
        QPolygonF pts;
        QList<QPolygonF> holes;
        
        for (unsigned int i = 0; i < num; ++i)
        {
          ptr = _getPolygon(pts, holes, context.mapToPixel(), ptr);
          ((QgsFillSymbolV2*)symbol)->renderPolygon(pts, (holes.count() ? &holes : NULL), context, layer);
        }
      }
      break;
    
    default:
      QgsDebugMsg("unsupported wkb type for rendering");
  }
}

QString QgsFeatureRendererV2::dump()
{
  return "UNKNOWN RENDERER\n";
}


QgsFeatureRendererV2* QgsFeatureRendererV2::load(QDomElement& element)
{
  // <renderer-v2 type=""> ... </renderer-v2>

  if (element.isNull())
    return NULL;

  // load renderer
  QString rendererType = element.attribute("type");

  // TODO: use renderer registry
  if (rendererType == "singleSymbol")
  {
    return QgsSingleSymbolRendererV2::create(element);
  }
  else if (rendererType == "categorizedSymbol")
  {
    return QgsCategorizedSymbolRendererV2::create(element);
  }
  else if (rendererType == "graduatedSymbol")
  {
    return QgsGraduatedSymbolRendererV2::create(element);
  }

  // unknown renderer type
  return NULL;
}

QDomElement QgsFeatureRendererV2::save(QDomDocument& doc)
{
  // create empty renderer element
  return doc.createElement(RENDERER_TAG_NAME);
}

int QgsFeatureRendererV2::fieldNameIndex( const QgsFieldMap& fields, const QString& fieldName )
{
  for ( QgsFieldMap::const_iterator it = fields.constBegin(); it != fields.constEnd(); ++it )
  {
    if ( it->name() == fieldName )
      return it.key();
  }
  return -1;
}

///////////////////

QgsSingleSymbolRendererV2::QgsSingleSymbolRendererV2(QgsSymbolV2* symbol)
  : QgsFeatureRendererV2(RendererSingleSymbol)
{
	mSymbol = symbol;
}
	
QgsSingleSymbolRendererV2::~QgsSingleSymbolRendererV2()
{
	delete mSymbol;
}
	
QgsSymbolV2* QgsSingleSymbolRendererV2::symbolForFeature(QgsFeature& feature)
{
	return mSymbol;
}
	
void QgsSingleSymbolRendererV2::startRender(QgsRenderContext& context, const QgsFieldMap& fields)
{
	mSymbol->startRender(context);
}
	
void QgsSingleSymbolRendererV2::stopRender(QgsRenderContext& context)
{
	mSymbol->stopRender(context);
}

QList<QString> QgsSingleSymbolRendererV2::usedAttributes()
{
  return QList<QString>();
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbol() const
{
  return mSymbol;
}

void QgsSingleSymbolRendererV2::setSymbol(QgsSymbolV2* s)
{
  delete mSymbol;
  mSymbol = s;
}

QString QgsSingleSymbolRendererV2::dump()
{
  return QString("SINGLE: %1").arg(mSymbol->dump());
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::clone()
{
  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( mSymbol->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

QgsSymbolV2List QgsSingleSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  lst.append(mSymbol);
  return lst;
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::create(QDomElement& element)
{
  QDomElement symbolsElem = element.firstChildElement("symbols");
  if (symbolsElem.isNull())
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols(symbolsElem);

  if (!symbolMap.contains("0"))
    return NULL;

  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( symbolMap.take("0") );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap(symbolMap);

  // TODO: symbol levels
  return r;
}

QDomElement QgsSingleSymbolRendererV2::save(QDomDocument& doc)
{
  QDomElement rendererElem = doc.createElement(RENDERER_TAG_NAME);
  rendererElem.setAttribute("type", "singleSymbol");

  QgsSymbolV2Map symbols;
  symbols["0"] = mSymbol;
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols(symbols, doc);
  rendererElem.appendChild(symbolsElem);

  return rendererElem;
}

///////////////////

QgsRendererCategoryV2::QgsRendererCategoryV2(QVariant value, QgsSymbolV2* symbol, QString label)
  : mValue(value), mSymbol(symbol), mLabel(label)
{
}

QgsRendererCategoryV2::QgsRendererCategoryV2(const QgsRendererCategoryV2& cat)
  : mValue(cat.mValue), mLabel(cat.mLabel)
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

void QgsRendererCategoryV2::setSymbol(QgsSymbolV2* s)
{
  if (mSymbol == s)
    return;
  delete mSymbol;
  mSymbol = s;
}

void QgsRendererCategoryV2::setLabel(QString label)
{
  mLabel = label;
}

QString QgsRendererCategoryV2::dump()
{
  return QString("%1::%2::%3\n").arg(mValue.toString()).arg(mLabel).arg(mSymbol->dump());
}

///////////////////

QgsCategorizedSymbolRendererV2::QgsCategorizedSymbolRendererV2(QString attrName, QgsCategoryList categories)
  : QgsFeatureRendererV2(RendererCategorizedSymbol), mAttrName(attrName), mCategories(categories)
{
  for (int i = 0; i < mCategories.count(); ++i)
  {
    QgsRendererCategoryV2& cat = mCategories[i];
    if (cat.symbol() == NULL)
    {
      QgsDebugMsg("invalid symbol in a category! ignoring...");
      mCategories.removeAt(i--);
    }
    //mCategories.insert(cat.value().toString(), cat);
  }
}

QgsCategorizedSymbolRendererV2::~QgsCategorizedSymbolRendererV2()
{
  mCategories.clear(); // this should also call destructors of symbols
}

void QgsCategorizedSymbolRendererV2::rebuildHash()
{
  mSymbolHash.clear();

  for (int i = 0; i < mCategories.count(); ++i)
  {
    QgsRendererCategoryV2& cat = mCategories[i];
    mSymbolHash.insert(cat.value().toString(), cat.symbol());
  }
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForValue(QVariant value)
{
  // TODO: special case for int, double
  
  QHash<QString, QgsSymbolV2*>::iterator it = mSymbolHash.find(value.toString());
  if (it == mSymbolHash.end())
  {
    if (mSymbolHash.count() == 0)
      QgsDebugMsg("there are no hashed symbols!!!");
    else
      QgsDebugMsg("attribute value not found: " + value.toString());
    return NULL;
  }
  else
    return *it;
}

QgsSymbolV2* QgsCategorizedSymbolRendererV2::symbolForFeature(QgsFeature& feature)
{
  const QgsAttributeMap& attrMap = feature.attributeMap();
  QgsAttributeMap::const_iterator ita = attrMap.find(mAttrNum);
  if (ita == attrMap.end())
  {
    QgsDebugMsg("attribute '"+mAttrName+"' (index "+QString::number(mAttrNum)+") required by renderer not found");
    return NULL;
  }
  
  // find the right category
  return symbolForValue(*ita);
}

int QgsCategorizedSymbolRendererV2::categoryIndexForValue(QVariant val)
{
  for (int i = 0; i < mCategories.count(); i++)
  {
    if (mCategories[i].value() == val)
      return i;
  }
  return -1;
}

bool QgsCategorizedSymbolRendererV2::updateCategorySymbol(int catIndex, QgsSymbolV2* symbol)
{
  if (catIndex < 0 || catIndex >= mCategories.size())
    return false;
  mCategories[catIndex].setSymbol(symbol);
  return true;
}

bool QgsCategorizedSymbolRendererV2::updateCategoryLabel(int catIndex, QString label)
{
  if (catIndex < 0 || catIndex >= mCategories.size())
    return false;
  mCategories[catIndex].setLabel(label);
  return true;
}

bool QgsCategorizedSymbolRendererV2::deleteCategory(int catIndex)
{
  if (catIndex < 0 || catIndex >= mCategories.size())
    return false;
  
  mCategories.removeAt(catIndex);
  return true;
}

void QgsCategorizedSymbolRendererV2::deleteAllCategories()
{
  mCategories.clear();
}

void QgsCategorizedSymbolRendererV2::startRender(QgsRenderContext& context, const QgsFieldMap& fields)
{
  // make sure that the hash table is up to date
  rebuildHash();

  // find out classification attribute index from name
  mAttrNum = fieldNameIndex(fields, mAttrName);

  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it)
    it->symbol()->startRender(context);
}

void QgsCategorizedSymbolRendererV2::stopRender(QgsRenderContext& context)
{
  QgsCategoryList::iterator it = mCategories.begin();
  for ( ; it != mCategories.end(); ++it)
    it->symbol()->stopRender(context);
}

QList<QString> QgsCategorizedSymbolRendererV2::usedAttributes()
{
  QList<QString> lst;
  lst.append(mAttrName);
  return lst;
}

QString QgsCategorizedSymbolRendererV2::dump()
{
  QString s = QString("CATEGORIZED: idx %1\n").arg(mAttrName);
  for (int i=0; i<mCategories.count();i++)
    s += mCategories[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::clone()
{
  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2( mAttrName, mCategories );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

QgsSymbolV2List QgsCategorizedSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  for (int i = 0; i < mCategories.count(); i++)
    lst.append(mCategories[i].symbol());
  return lst;
}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2::create(QDomElement& element)
{
  QDomElement symbolsElem = element.firstChildElement("symbols");
  if (symbolsElem.isNull())
    return NULL;

  QDomElement catsElem = element.firstChildElement("categories");
  if (catsElem.isNull())
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols(symbolsElem);
  QgsCategoryList cats;

  QDomElement catElem = catsElem.firstChildElement();
  while (!catElem.isNull())
  {
    if (catElem.tagName() == "category")
    {
      QVariant value = QVariant(catElem.attribute("value"));
      QString symbolName = catElem.attribute("symbol");
      QString label = catElem.attribute("label");
      if (symbolMap.contains(symbolName))
      {
        QgsSymbolV2* symbol = symbolMap.take(symbolName);
        cats.append( QgsRendererCategoryV2(value, symbol, label) );
      }
    }
    catElem = catElem.nextSiblingElement();
  }

  QString attrName = element.attribute("attr");

  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2(attrName, cats);

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap(symbolMap);

  // TODO: symbol levels
  return r;
}

QDomElement QgsCategorizedSymbolRendererV2::save(QDomDocument& doc)
{
  QDomElement rendererElem = doc.createElement(RENDERER_TAG_NAME);
  rendererElem.setAttribute("type", "categorizedSymbol");
  rendererElem.setAttribute("attr", mAttrName);

  // categories
  int i = 0;
  QgsSymbolV2Map symbols;
  QDomElement catsElem = doc.createElement("categories");
  QgsCategoryList::const_iterator it = mCategories.constBegin();
  for ( ; it != mCategories.end(); it++)
  {
    const QgsRendererCategoryV2& cat = *it;
    QString symbolName = QString::number(i);
    symbols.insert(symbolName, cat.symbol());

    QDomElement catElem = doc.createElement("category");
    catElem.setAttribute("value", cat.value().toString());
    catElem.setAttribute("symbol", symbolName);
    catElem.setAttribute("label", cat.label());
    catsElem.appendChild(catElem);
    i++;
  }

  rendererElem.appendChild(catsElem);

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols(symbols, doc);
  rendererElem.appendChild(symbolsElem);

  return rendererElem;
}

/////////////////////////
// graduated


QgsRendererRangeV2::QgsRendererRangeV2(double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label)
  : mLowerValue(lowerValue), mUpperValue(upperValue), mSymbol(symbol), mLabel(label)
{
}

QgsRendererRangeV2::QgsRendererRangeV2(const QgsRendererRangeV2& range)
  : mLowerValue(range.mLowerValue), mUpperValue(range.mUpperValue), mLabel(range.mLabel)
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

void QgsRendererRangeV2::setSymbol(QgsSymbolV2* s)
{
  if (mSymbol == s)
    return;
  delete mSymbol;
  mSymbol = s;
}

void QgsRendererRangeV2::setLabel(QString label)
{
  mLabel = label;
}

QString QgsRendererRangeV2::dump()
{
  return QString("%1 - %2::%3::%4\n").arg(mLowerValue).arg(mUpperValue).arg(mLabel).arg(mSymbol->dump());
}

///////////


QgsGraduatedSymbolRendererV2::QgsGraduatedSymbolRendererV2(QString attrName, QgsRangeList ranges)
  : QgsFeatureRendererV2(RendererGraduatedSymbol), mAttrName(attrName), mRanges(ranges), mMode(Custom)
{
  // TODO: check ranges for sanity (NULL symbols, invalid ranges)
}

QgsGraduatedSymbolRendererV2::~QgsGraduatedSymbolRendererV2()
{
  mRanges.clear(); // should delete all the symbols
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForValue(double value)
{
  for (QgsRangeList::iterator it = mRanges.begin(); it != mRanges.end(); ++it)
  {
    if (it->lowerValue() <= value && it->upperValue() >= value)
      return it->symbol();
  }
  // the value is out of the range: return NULL instead of symbol
  return NULL;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2::symbolForFeature(QgsFeature& feature)
{
  const QgsAttributeMap& attrMap = feature.attributeMap();
  QgsAttributeMap::const_iterator ita = attrMap.find(mAttrNum);
  if (ita == attrMap.end())
  {
    QgsDebugMsg("attribute required by renderer not found: "+mAttrName+"(index "+QString::number(mAttrNum)+")");
    return NULL;
  }
  
  // find the right category
  return symbolForValue(ita->toDouble());

}

void QgsGraduatedSymbolRendererV2::startRender(QgsRenderContext& context, const QgsFieldMap& fields)
{
  // find out classification attribute index from name
  mAttrNum = fieldNameIndex(fields, mAttrName);

  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it)
    it->symbol()->startRender(context);
}

void QgsGraduatedSymbolRendererV2::stopRender(QgsRenderContext& context)
{
  QgsRangeList::iterator it = mRanges.begin();
  for ( ; it != mRanges.end(); ++it)
    it->symbol()->startRender(context);
}

QList<QString> QgsGraduatedSymbolRendererV2::usedAttributes()
{
  QList<QString> lst;
  lst.append( mAttrName );
  return lst;
}

bool QgsGraduatedSymbolRendererV2::updateRangeSymbol(int rangeIndex, QgsSymbolV2* symbol)
{
  if (rangeIndex < 0 || rangeIndex >= mRanges.size())
    return false;
  mRanges[rangeIndex].setSymbol(symbol);
  return true;
}

bool QgsGraduatedSymbolRendererV2::updateRangeLabel(int rangeIndex, QString label)
{
  if (rangeIndex < 0 || rangeIndex >= mRanges.size())
    return false;
  mRanges[rangeIndex].setLabel(label);
  return true;
}

QString QgsGraduatedSymbolRendererV2::dump()
{
  QString s = QString("GRADUATED: attr %1\n").arg(mAttrName);
  for (int i=0; i<mRanges.count();i++)
    s += mRanges[i].dump();
  return s;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2::clone()
{
  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( mAttrName, mRanges );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

QgsSymbolV2List QgsGraduatedSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  for (int i = 0; i < mRanges.count(); i++)
    lst.append(mRanges[i].symbol());
  return lst;
}

static QList<double> _calcEqualIntervalBreaks(double minimum, double maximum, int classes)
{
  double step = (maximum - minimum) / classes;
  
  QList<double> breaks;
  double value = minimum;
  for (int i = 0; i < classes; i++)
  {
    value += step;
    breaks.append( value );
  }
  return breaks;
}

static QList<double> _calcQuantileBreaks(QList<double> values, int classes)
{
  // sort the values first
  qSort(values);
  
  QList<double> breaks;
  
  // q-th quantile of a data set:
  // value where q fraction of data is below and (1-q) fraction is above this value
  // Xq = (1 - r) * X_NI1 + r * X_NI2
  //   NI1 = (int) (q * (n+1))
  //   NI2 = NI1 + 1
  //   r = q * (n+1) - (int) (q * (n+1))
  // (indices of X: 1...n)
  
  int n = values.count();
  double q,a,aa,r,Xq;
  for (int i = 0; i < (classes-1); i++)
  {
    q = (i+1) / (double) classes;
    a = q * n;
    aa = (int) (q * n);
    
    r = a - aa;
    Xq = (1-r)* values[aa] + r * values[aa+1];
    
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
         QgsVectorColorRampV2* ramp)
{
  QgsVectorDataProvider* provider = vlayer->dataProvider();

  int attrNum = fieldNameIndex(vlayer->pendingFields(), attrName);

  double minimum = provider->minimumValue( attrNum ).toDouble();
  double maximum = provider->maximumValue( attrNum ).toDouble();
  QgsDebugMsg(QString("min %1 // max %2").arg(minimum).arg(maximum));

  QList<double> breaks;
  if (mode == EqualInterval)
  {
    breaks = _calcEqualIntervalBreaks(minimum, maximum, classes);
  }
  else if (mode == Quantile)
  {
    // get values from layer
    QList<double> values;
    QgsFeature f;
    QgsAttributeList lst;
    lst.append(attrNum);
    provider->select(lst, QgsRectangle(), false);
    while (provider->nextFeature(f))
      values.append(f.attributeMap()[attrNum].toDouble());
    // calculate the breaks
    breaks = _calcQuantileBreaks(values, classes);
  }
  else
  {
    Q_ASSERT(false);
  }
  
  QgsRangeList ranges;
  double lower, upper = minimum;
  QString label;
  
  // "breaks" list contains all values at class breaks plus maximum as last break
  int i = 0;
  for (QList<double>::iterator it = breaks.begin(); it != breaks.end(); ++it, ++i)
  {
    lower = upper; // upper border from last interval
    upper = *it;
    label = QString::number(lower,'f',4) + " - " + QString::number(upper,'f',4);
    
    QgsSymbolV2* newSymbol = symbol->clone();
    newSymbol->setColor( ramp->color( (double) i / (classes-1) ) ); // color from (0 / cl-1) to (cl-1 / cl-1)
    
    ranges.append( QgsRendererRangeV2(lower, upper, newSymbol, label) );
  }
  
  return new QgsGraduatedSymbolRendererV2( attrName, ranges );
}



QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2::create(QDomElement& element)
{
  QDomElement symbolsElem = element.firstChildElement("symbols");
  if (symbolsElem.isNull())
    return NULL;

  QDomElement rangesElem = element.firstChildElement("ranges");
  if (rangesElem.isNull())
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols(symbolsElem);
  QgsRangeList ranges;

  QDomElement rangeElem = rangesElem.firstChildElement();
  while (!rangeElem.isNull())
  {
    if (rangeElem.tagName() == "range")
    {
      double lowerValue = rangeElem.attribute("lower").toDouble();
      double upperValue = rangeElem.attribute("upper").toDouble();
      QString symbolName = rangeElem.attribute("symbol");
      QString label = rangeElem.attribute("label");
      if (symbolMap.contains(symbolName))
      {
        QgsSymbolV2* symbol = symbolMap.take(symbolName);
        ranges.append( QgsRendererRangeV2(lowerValue, upperValue, symbol, label) );
      }
    }
    rangeElem = rangeElem.nextSiblingElement();
  }

  QString attrName = element.attribute("attr");

  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2(attrName, ranges);

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap(symbolMap);

  // TODO: symbol levels
  return r;
}

QDomElement QgsGraduatedSymbolRendererV2::save(QDomDocument& doc)
{
  QDomElement rendererElem = doc.createElement(RENDERER_TAG_NAME);
  rendererElem.setAttribute("type", "graduatedSymbol");
  rendererElem.setAttribute("attr", mAttrName);

  // ranges
  int i = 0;
  QgsSymbolV2Map symbols;
  QDomElement rangesElem = doc.createElement("ranges");
  QgsRangeList::const_iterator it = mRanges.constBegin();
  for ( ; it != mRanges.end(); it++)
  {
    const QgsRendererRangeV2& range = *it;
    QString symbolName = QString::number(i);
    symbols.insert(symbolName, range.symbol());

    QDomElement rangeElem = doc.createElement("range");
    rangeElem.setAttribute("lower", range.lowerValue());
    rangeElem.setAttribute("upper", range.upperValue());
    rangeElem.setAttribute("symbol", symbolName);
    rangeElem.setAttribute("label", range.label());
    rangesElem.appendChild(rangeElem);
    i++;
  }

  rendererElem.appendChild(rangesElem);

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols(symbols, doc);
  rendererElem.appendChild(symbolsElem);

  return rendererElem;
}
