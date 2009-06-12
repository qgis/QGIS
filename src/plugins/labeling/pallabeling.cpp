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

LayerSettings::LayerSettings()
  : palLayer(NULL), fontMetrics(NULL)
{
}

LayerSettings::~LayerSettings()
{
  // pal layer is deleted internally in PAL
  delete fontMetrics;
}

void LayerSettings::calculateLabelSize(QString text, double& labelX, double& labelY)
{
  //QFontMetrics fontMetrics(textFont);
  QRect labelRect = /*QRect(0,0,20,20);*/ fontMetrics->boundingRect(text);

  // 2px border...
  QgsPoint ptSize = xform->toMapCoordinates( labelRect.width()+2,labelRect.height()+2 );
  labelX = fabs(ptSize.x()-ptZero.x());
  labelY = fabs(ptSize.y()-ptZero.y());
}

void LayerSettings::registerFeature(QgsFeature& f)
{
  QString labelText = f.attributeMap()[fieldIndex].toString();
  double labelX, labelY; // will receive label size
  calculateLabelSize(labelText, labelX, labelY);

  //std::cout << labelX << " " << labelY << std::endl;
  MyLabel* lbl = new MyLabel(f.id(), labelText, GEOSGeom_clone( f.geometry()->asGeos() ) );

  // register feature to the layer
  palLayer->registerFeature(lbl->strId(), lbl, labelX, labelY);
}


// -------------

PalLabeling::PalLabeling(QgsMapCanvas* mapCanvas)
  : mMapCanvas(mapCanvas), mPal(NULL)
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

  initPal();
}


PalLabeling::~PalLabeling()
{
  delete mPal;

  // make sure to remove hooks from all layers
  while (mLayers.count())
  {
    removeLayer(mLayers[0].layerId);
  }
}


void PalLabeling::addLayer(LayerSettings layerSettings)
{
  mLayers.append(layerSettings);

  QgsVectorLayer* vlayer = (QgsVectorLayer*) QgsMapLayerRegistry::instance()->mapLayer(layerSettings.layerId);

  LayerSettings& lyr = mLayers[ mLayers.count()-1 ]; // make sure we have the right pointer
  vlayer->setLabelingHooks(PalLabeling::prepareLayerHook, PalLabeling::registerFeatureHook, this, &lyr);
}

void PalLabeling::removeLayer(QString layerId)
{
  for (int i = 0; i < mLayers.count(); i++)
  {
    if (mLayers.at(i).layerId == layerId)
    {
      QgsVectorLayer* vlayer = (QgsVectorLayer*) QgsMapLayerRegistry::instance()->mapLayer(mLayers.at(i).layerId);
      if (vlayer) { vlayer->setLabelingHooks(NULL, NULL, NULL, NULL); }

      mLayers.removeAt(i);
      return;
    }
  }
}

LayerSettings PalLabeling::layer(QString layerId)
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



int PalLabeling::prepareLayerHook(void* context, void* layerContext, int& attrIndex)
{
  PalLabeling* thisClass = (PalLabeling*) context;
  LayerSettings* lyr = (LayerSettings*) layerContext;

  QgsVectorLayer* vlayer = (QgsVectorLayer*) QgsMapLayerRegistry::instance()->mapLayer(lyr->layerId);
  if (vlayer == NULL)
    return 0;

  // find out which field will be needed
  int fldIndex = vlayer->dataProvider()->fieldNameIndex(lyr->fieldName);
  if (fldIndex == -1)
    return 0;
  attrIndex = fldIndex;


  // how to place the labels
  Arrangement arrangement;
  switch (lyr->placement)
  {
    case LayerSettings::AroundPoint: arrangement = P_POINT; break;
    case LayerSettings::OnLine:      arrangement = P_LINE; break;
    case LayerSettings::AroundLine:  arrangement = P_LINE_AROUND; break;
    case LayerSettings::Horizontal:  arrangement = P_HORIZ; break;
    case LayerSettings::Free:        arrangement = P_FREE; break;
  }

  // create the pal layer
  double priority = 1 - lyr->priority/10.0; // convert 0..10 --> 1..0
  Layer* l = thisClass->mPal->addLayer(lyr->layerId.toLocal8Bit().data(), -1, -1, arrangement, METER, priority, lyr->obstacle, true, true);

  // save the pal layer to our layer context (with some additional info)
  lyr->palLayer = l;
  lyr->fieldIndex = fldIndex;
  lyr->fontMetrics = new QFontMetrics(lyr->textFont);
  lyr->fontBaseline = lyr->fontMetrics->boundingRect("X").bottom(); // dummy text to find out how many pixels of the text are below the baseline
  lyr->xform = thisClass->mMapCanvas->mapRenderer()->coordinateTransform();
  lyr->ptZero = lyr->xform->toMapCoordinates( 0,0 );

  return 1; // init successful
}

void PalLabeling::registerFeatureHook(QgsFeature& f, void* layerContext)
{
  LayerSettings* lyr = (LayerSettings*) layerContext;
  lyr->registerFeature(f);
}


void PalLabeling::initPal()
{
  // delete if exists already
  if (mPal)
    delete mPal;
  
  mPal = new Pal;

  SearchMethod s;
  switch (mSearch)
  {
    case Chain: s = CHAIN; break;
    case Popmusic_Tabu: s = POPMUSIC_TABU; break;
    case Popmusic_Chain: s = POPMUSIC_CHAIN; break;
    case Popmusic_Tabu_Chain: s = POPMUSIC_TABU_CHAIN; break;
  }
  mPal->setSearch(s);

  // set number of candidates generated per feature
  mPal->setPointP(mCandPoint);
  mPal->setLineP(mCandLine);
  mPal->setPolyP(mCandPolygon);
}



void PalLabeling::doLabeling(QPainter* painter)
{

  QTime t;
  t.start();

  // make sure to delete fontmetrics otherwise it crashes inside Qt when drawing... :-(
  // probably gets invalid when setting fonts in the label drawing loop
  for (int i = 0; i < mLayers.count(); i++)
  {
    LayerSettings& lyr = mLayers[i];
    delete lyr.fontMetrics;
    lyr.fontMetrics = NULL;
  }

  // do the labeling itself
  double scale = 1; // scale denominator
  QgsRectangle r = mMapCanvas->extent();
  double bbox[] = { r.xMinimum(), r.yMinimum(), r.xMaximum(), r.yMaximum() };

  std::list<Label*>* labels;
  try
  {
     labels = mPal->labeller(scale, bbox, NULL, false);
  }
  catch ( std::exception e )
  {
    std::cerr << "PAL EXCEPTION :-( " << e.what() << std::endl;
    return;
  }

  std::cout << "LABELING work:   " << t.elapsed() << "ms  ... labels# " << labels->size() << std::endl;
  t.restart();

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
    painter->translate( QPointF(outPt.x()+1, outPt.y()-1-lyr.fontBaseline) );
    painter->rotate(-label->getRotation() * 180 / M_PI );
    painter->drawText(0,0, ((MyLabel*)label->getGeometry())->text());
    painter->restore();

    delete label->getGeometry();
    delete label;
  }

  std::cout << "LABELING draw:   " << t.elapsed() << "ms" << std::endl;

  delete labels;

  // re-create PAL
  initPal();
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
