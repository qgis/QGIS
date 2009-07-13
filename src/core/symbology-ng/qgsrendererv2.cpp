#include "qgsrendererv2.h"
#include "qgssymbolv2.h"

#include "qgsrendercontext.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgslogger.h"

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
  : mType(type)
{
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
	
void QgsSingleSymbolRendererV2::startRender(QgsRenderContext& context)
{
	mSymbol->startRender(context);
}
	
void QgsSingleSymbolRendererV2::stopRender(QgsRenderContext& context)
{
	mSymbol->stopRender(context);
}

QList<int> QgsSingleSymbolRendererV2::usedAttributes()
{
	return QList<int>();
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


///////////////////

QgsRendererCategoryV2::QgsRendererCategoryV2(const QgsRendererCategoryV2& cat)
  : mValue(cat.mValue), mLabel(cat.mLabel)
{
  mSymbol = cat.mSymbol->clone();
}


QgsRendererCategoryV2::~QgsRendererCategoryV2()
{
  delete mSymbol;
}

void QgsRendererCategoryV2::setSymbol(QgsSymbolV2* s)
{
  delete mSymbol;
  mSymbol = s;
}

QString QgsRendererCategoryV2::dump()
{
  return QString("%1::%2::%3\n").arg(mValue.toString()).arg(mLabel).arg(mSymbol->dump());
}

///////////////////

QgsCategorizedSymbolRendererV2::QgsCategorizedSymbolRendererV2(int attrNum, QgsCategoryList categories)
  : QgsFeatureRendererV2(RendererCategorizedSymbol), mAttrNum(attrNum), mCategories(categories)
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
    QgsDebugMsg("attribute required by renderer not found: "+QString::number(mAttrNum));
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

void QgsCategorizedSymbolRendererV2::startRender(QgsRenderContext& context)
{
  // make sure that the hash table is up to date
  rebuildHash();

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

QList<int> QgsCategorizedSymbolRendererV2::usedAttributes()
{
  QList<int> lst;
  lst.append(mAttrNum);
  return lst;
}

QString QgsCategorizedSymbolRendererV2::dump()
{
  QString s = QString("CATEGORIZED: idx %1\n").arg(mAttrNum);
  for (int i=0; i<mCategories.count();i++)
    s += mCategories[i].dump();
  return s;
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

///////////


QgsGraduatedSymbolRendererV2::QgsGraduatedSymbolRendererV2(int attrNum, QgsRangeList ranges)
  : QgsFeatureRendererV2(RendererGraduatedSymbol), mAttrNum(attrNum), mRanges(ranges), mMode(Custom)
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
    QgsDebugMsg("attribute required by renderer not found: "+QString::number(mAttrNum));
    return NULL;
  }
  
  // find the right category
  return symbolForValue(ita->toDouble());

}

void QgsGraduatedSymbolRendererV2::startRender(QgsRenderContext& context)
{
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

QList<int> QgsGraduatedSymbolRendererV2::usedAttributes()
{
  QList<int> lst;
  lst.append(mAttrNum);
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
         int attrNum,
         int classes,
         Mode mode,
         QgsSymbolV2* symbol,
         QgsVectorColorRampV2* ramp)
{
  QgsVectorDataProvider* provider = vlayer->dataProvider();

  double minimum = provider->minimumValue( attrNum ).toDouble();
  double maximum = provider->maximumValue( attrNum ).toDouble();

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
  
  return new QgsGraduatedSymbolRendererV2( attrNum, ranges );
}
