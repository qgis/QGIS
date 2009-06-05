#include "pallabeling.h"

#include <iostream>
#include <list>

#include <pal/pal.h>
#include <pal/layer.h>
#include <pal/palgeometry.h>
#include <pal/palexception.h>

#include <geos_c.h>

#include <cmath>

#include <QByteArray>
#include <QString>
#include <QFontMetrics>

#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmapcanvas.h>

using namespace pal;


class MyLabel : public PalGeometry
{
public:
  MyLabel(int id, QString text, GEOSGeometry* g): mG(g), mText(text), mId(id)
  {
    mStrId = QByteArray::number(id);
  }

  ~MyLabel()
  {
    if (mG) GEOSGeom_destroy(mG);
  }

  // getGeosGeometry + releaseGeosGeometry is called twice: once when adding, second time when labeling

  GEOSGeometry* getGeosGeometry()
  {
    return mG;
  }
  void releaseGeosGeometry(GEOSGeometry* /*geom*/)
  {
    // nothing here - we'll delete the geometry in destructor
  }

  const char* strId() { return mStrId.data(); }
  QString text() { return mText; }

protected:
  GEOSGeometry* mG;
  QString mText;
  QByteArray mStrId;
  int mId;
};



// -------------

PalLabeling::PalLabeling(QgsMapCanvas* mapCanvas)
  : mMapCanvas(mapCanvas)
{
  // find out engine defaults
  Pal p;
  mCandPoint = p.getPointP();
  mCandLine = p.getLineP();
  mCandPolygon = p.getPolyP();

  switch (p.getSearch())
  {
    case CHAIN: mSearch = Chain; break;
    case POPMUSIC_TABU: mSearch = Popmusic_Tabu; break;
    case POPMUSIC_CHAIN: mSearch = Popmusic_Chain; break;
    case POPMUSIC_TABU_CHAIN: mSearch = Popmusic_Tabu_Chain; break;
  }
}

void PalLabeling::addLayer(LayerSettings layerSettings)
{
  mLayers.append(layerSettings);
}

void PalLabeling::removeLayer(QString layerId)
{
  for (int i = 0; i < mLayers.count(); i++)
  {
    if (mLayers.at(i).layerId == layerId)
    {
      mLayers.removeAt(i);
      return;
    }
  }
}

PalLabeling::LayerSettings PalLabeling::layer(QString layerId)
{
  for (int i = 0; i < mLayers.count(); i++)
  {
    if (mLayers.at(i).layerId == layerId)
    {
      return mLayers.at(i);
    }
  }
  return LayerSettings();
}


int PalLabeling::prepareLayer(Pal& pal, const LayerSettings& lyr)
{
  if (!lyr.enabled)
    return 0;

  QgsVectorLayer* vlayer = (QgsVectorLayer*) QgsMapLayerRegistry::instance()->mapLayer(lyr.layerId);
  if (vlayer == NULL)
    return 0;

  QgsAttributeList attrs;

  int fldName = vlayer->dataProvider()->fieldNameIndex(lyr.fieldName);
  if (fldName == -1)
    return 0;
  attrs << fldName;
  vlayer->select(attrs, mMapCanvas->extent());


  // how to place the labels
  Arrangement arrangement;
  switch (lyr.placement)
  {
    case AroundPoint: arrangement = P_POINT; break;
    case OnLine:      arrangement = P_LINE; break;
    case AroundLine:  arrangement = P_LINE_AROUND; break;
    case Horizontal:  arrangement = P_HORIZ; break;
    case Free:        arrangement = P_FREE; break;
  }

  // create the pal layer
  Layer* l = pal.addLayer(lyr.layerId.toLocal8Bit().data(), -1, -1, arrangement, METER, 0, true, true, true);

  QFontMetrics fm(lyr.textFont);

  QgsFeature f;
  int feats = 0;
  const QgsMapToPixel* xform = mMapCanvas->mapRenderer()->coordinateTransform();
  QgsPoint ptZero = xform->toMapCoordinates( 0,0 );

  while (vlayer->nextFeature(f))
  {
    QString labelText = f.attributeMap()[fldName].toString();
    QRect labelRect = fm.boundingRect(labelText);
    //std::cout << "bound: " << labelRect.width() << "x" << labelRect.height() << std::endl;
    // 2px border...
    QgsPoint ptSize = xform->toMapCoordinates( labelRect.width()+2,labelRect.height()+2 );
    double labelX = fabs(ptSize.x()-ptZero.x());
    double labelY = fabs(ptSize.y()-ptZero.y());
    //std::cout << "L " << labelX << " " << labelY << std::endl;

    MyLabel* lbl = new MyLabel(f.id(), labelText, GEOSGeom_clone( f.geometry()->asGeos() ) );

    // TODO: owner of the id?
    l->registerFeature(lbl->strId(), lbl, labelX, labelY);
    feats++;
  }

  return feats;
}


void PalLabeling::doLabeling(QPainter* painter)
{
  Pal p;

  SearchMethod s;
  switch (mSearch)
  {
    case Chain: s = CHAIN; break;
    case Popmusic_Tabu: s = POPMUSIC_TABU; break;
    case Popmusic_Chain: s = POPMUSIC_CHAIN; break;
    case Popmusic_Tabu_Chain: s = POPMUSIC_TABU_CHAIN; break;
  }
  p.setSearch(s);

  // set number of candidates generated per feature
  p.setPointP(mCandPoint);
  p.setLineP(mCandLine);
  p.setPolyP(mCandPolygon);

  //p.setSearch(POPMUSIC_TABU_CHAIN);// this is really slow! // default is CHAIN (worst, fastest)
  // TODO: API 0.2 - no mention about changing map units!
  // pal map units = METER by default ... change setMapUnit
  //p.setMapUnit(METER);
  // pal label units ... to be chosen
  // pal dist label - pixels?

  QTime t;
  t.start();

  int feats = 0;
  for (int i = 0; i < mLayers.count(); i++)
  {
    feats += prepareLayer(p, mLayers.at(i));
  }

  std::cout << "LABELING prepare: " << t.elapsed() << "ms" << std::endl;
  t.restart();

  // do the labeling itself
  double scale = 1; // scale denominator
  QgsRectangle r = mMapCanvas->extent();
  double bbox[] = { r.xMinimum(), r.yMinimum(), r.xMaximum(), r.yMaximum() };

  std::list<Label*>* labels;
  try
  {
     labels = p.labeller(scale, bbox, NULL, false);
  }
  catch ( std::exception e )
  {
    std::cerr << "PAL EXCEPTION :-( " << e.what() << std::endl;
    return;
  }

  std::cout << "LABELING work:   " << t.elapsed() << "ms" << std::endl;
  std::cout << "-->> " << labels->size() << "/" << feats << std::endl;
  t.restart();

  QFontMetrics fm = painter->fontMetrics();
  QRect labelRect = fm.boundingRect("X"); // dummy text to find out height
  int baseline = labelRect.bottom(); // how many pixels of the text are below the baseline

  // draw the labels
  const QgsMapToPixel* xform = mMapCanvas->mapRenderer()->coordinateTransform();
  std::list<Label*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it)
  {
    Label* label = *it;

    QgsPoint outPt = xform->transform(label->getOrigX(), label->getOrigY());

    // TODO: optimize access :)
    const LayerSettings& lyr = layer(label->getLayerName());

    // shift by one as we have 2px border
    painter->save();
    painter->setPen( lyr.textColor );
    painter->setFont( lyr.textFont );
    painter->translate( QPointF(outPt.x()+1, outPt.y()-1-baseline) );
    painter->rotate(-label->getRotation() * 180 / M_PI );
    painter->drawText(0,0, ((MyLabel*)label->getGeometry())->text());
    painter->restore();

    delete label->getGeometry();
    delete label;
  }

  std::cout << "LABELING draw:   " << t.elapsed() << "ms" << std::endl;

  delete labels;
}

void PalLabeling::numCandidatePositions(int& candPoint, int& candLine, int& candPolygon)
{
  candPoint = mCandPoint;
  candLine = mCandLine;
  candPolygon = mCandPolygon;
}

void PalLabeling::setNumCandidatePositions(int candPoint, int candLine, int candPolygon)
{
  mCandPoint = candPoint;
  mCandLine = candLine;
  mCandPolygon = candPolygon;
}

void PalLabeling::setSearchMethod(PalLabeling::Search s)
{
  mSearch = s;
}

PalLabeling::Search PalLabeling::searchMethod() const
{
  return mSearch;
}
