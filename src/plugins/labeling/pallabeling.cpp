#include "pallabeling.h"

#include <iostream>
#include <list>

#include <pal/pal.h>
#include <pal/feature.h>
#include <pal/layer.h>
#include <pal/palgeometry.h>
#include <pal/palexception.h>
#include <pal/problem.h>
#include <pal/labelposition.h>

#include <geos_c.h>

#include <cmath>

#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>

#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaprenderer.h>

using namespace pal;


class MyLabel : public PalGeometry
{
public:
  MyLabel(int id, QString text, GEOSGeometry* g): mG(g), mText(text), mId(id), mInfo(NULL)
  {
    mStrId = QByteArray::number(id);
  }

  ~MyLabel()
  {
    if (mG) GEOSGeom_destroy(mG);
    delete mInfo;
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

  pal::LabelInfo* info(QFontMetrics* fm, const QgsMapToPixel* xform)
  {
    if (mInfo) return mInfo;

    // create label info!
    QgsPoint ptZero = xform->toMapCoordinates( 0,0 );
    QgsPoint ptSize = xform->toMapCoordinates( 0,-fm->height() );

    mInfo = new pal::LabelInfo( mText.count(), ptSize.y()-ptZero.y() );
    for (int i = 0; i < mText.count(); i++)
    {
      mInfo->char_info[i].chr = mText[i].unicode();
      ptSize = xform->toMapCoordinates( fm->width( mText[i] ), 0 );
      mInfo->char_info[i].width = ptSize.x()-ptZero.x();
    }
    return mInfo;
  }

protected:
  GEOSGeometry* mG;
  QString mText;
  QByteArray mStrId;
  int mId;
  LabelInfo* mInfo;
};

// -------------

LayerSettings::LayerSettings()
  : palLayer(NULL), fontMetrics(NULL), ct(NULL)
{
  bufferColor = Qt::white;
}

LayerSettings::LayerSettings(const LayerSettings& s)
{
  // copy only permanent stuff
  layerId = s.layerId;
  fieldName = s.fieldName;
  placement = s.placement;
  placementFlags = s.placementFlags;
  textFont = s.textFont;
  textColor = s.textColor;
  enabled = s.enabled;
  priority = s.priority;
  obstacle = s.obstacle;
  dist = s.dist;
  scaleMin = s.scaleMin;
  scaleMax = s.scaleMax;
  bufferSize = s.bufferSize;
  bufferColor = s.bufferColor;

  fontMetrics = NULL;
  ct = NULL;
}


LayerSettings::~LayerSettings()
{
  // pal layer is deleted internally in PAL

  delete fontMetrics;
  delete ct;
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

  QgsGeometry* geom = f.geometry();
  if (ct != NULL) // reproject the geometry if necessary
    geom->transform(*ct);

  MyLabel* lbl = new MyLabel(f.id(), labelText, GEOSGeom_clone( geom->asGeos() ) );

  // record the created geometry - it will be deleted at the end.
  geometries.append(lbl);

  // register feature to the layer
  if (!palLayer->registerFeature(lbl->strId(), lbl, labelX, labelY))
    return;

  // TODO: only for placement which needs character info
  palLayer->setFeatureLabelInfo( lbl->strId(), lbl->info( fontMetrics, xform ) );

  // TODO: allow layer-wide feature dist in PAL...?
  if (dist != 0)
    palLayer->setFeatureDistlabel(lbl->strId(), fabs(ptOne.x()-ptZero.x())* dist);
}


// -------------

PalLabeling::PalLabeling(QgsMapRenderer* mapRenderer)
  : mMapRenderer(mapRenderer), mPal(NULL)
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
    case FALP: mSearch = Falp; break;
  }

  mShowingCandidates = FALSE;

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

const LayerSettings& PalLabeling::layer(QString layerId)
{
  for (int i = 0; i < mLayers.count(); i++)
  {
    if (mLayers.at(i).layerId == layerId)
    {
      return mLayers.at(i);
    }
  }
  return mInvalidLayer;
}



int PalLabeling::prepareLayerHook(void* context, void* layerContext, int& attrIndex)
{
  PalLabeling* thisClass = (PalLabeling*) context;
  LayerSettings* lyr = (LayerSettings*) layerContext;

  if (!lyr->enabled)
    return 0;

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
    case LayerSettings::OverPoint:   arrangement = P_POINT_OVER; break;
    case LayerSettings::Line:        arrangement = P_LINE; break;
    case LayerSettings::Curved:      arrangement = P_CURVED; break;
    case LayerSettings::Horizontal:  arrangement = P_HORIZ; break;
    case LayerSettings::Free:        arrangement = P_FREE; break;
  }

  // create the pal layer
  double priority = 1 - lyr->priority/10.0; // convert 0..10 --> 1..0
  double min_scale = -1, max_scale = -1;
  if (lyr->scaleMin != 0 && lyr->scaleMax != 0)
  {
    min_scale = lyr->scaleMin;
    max_scale = lyr->scaleMax;
  }

  Layer* l = thisClass->mPal->addLayer(lyr->layerId.toLocal8Bit().data(), min_scale, max_scale, arrangement, METER, priority, lyr->obstacle, true, true);

  if ( lyr->placementFlags )
    l->setArrangementFlags( lyr->placementFlags );

  // save the pal layer to our layer context (with some additional info)
  lyr->palLayer = l;
  lyr->fieldIndex = fldIndex;
  lyr->fontMetrics = new QFontMetrics(lyr->textFont);
  lyr->fontBaseline = lyr->fontMetrics->boundingRect("X").bottom(); // dummy text to find out how many pixels of the text are below the baseline
  lyr->xform = thisClass->mMapRenderer->coordinateTransform();
  if (thisClass->mMapRenderer->hasCrsTransformEnabled())
    lyr->ct = new QgsCoordinateTransform( vlayer->srs(), thisClass->mMapRenderer->destinationSrs() );
  else
    lyr->ct = NULL;
  lyr->ptZero = lyr->xform->toMapCoordinates( 0,0 );
  lyr->ptOne = lyr->xform->toMapCoordinates( 1,0 );

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
    case Falp: s = FALP; break;
  }
  mPal->setSearch(s);

  // set number of candidates generated per feature
  mPal->setPointP(mCandPoint);
  mPal->setLineP(mCandLine);
  mPal->setPolyP(mCandPolygon);
}



void PalLabeling::doLabeling(QPainter* painter, QgsRectangle extent)
{

  QTime t;
  t.start();

  // do the labeling itself
  double scale = mMapRenderer->scale(); // scale denominator
  QgsRectangle r = extent;
  double bbox[] = { r.xMinimum(), r.yMinimum(), r.xMaximum(), r.yMaximum() };

  std::list<LabelPosition*>* labels;
  pal::Problem* problem;
  try
  {
    problem = mPal->extractProblem(scale, bbox);
  }
  catch ( std::exception& e )
  {
    std::cerr << "PAL EXCEPTION :-( " << e.what() << std::endl;
    return;
  }

  const QgsMapToPixel* xform = mMapRenderer->coordinateTransform();

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  mCandidates.clear();
  if (mShowingCandidates && problem)
  {
    painter->setPen(QColor(0,0,0,64));
    painter->setBrush(Qt::NoBrush);
    for (int i = 0; i < problem->getNumFeatures(); i++)
    {
      for (int j = 0; j < problem->getFeatureCandidateCount(i); j++)
      {
        pal::LabelPosition* lp = problem->getFeatureCandidate(i, j);

        drawLabelCandidateRect(lp, painter, xform);
      }
    }
  }

  // find the solution
  labels = mPal->solveProblem( problem );

  std::cout << "LABELING work:   " << t.elapsed() << "ms  ... labels# " << labels->size() << std::endl;
  t.restart();

  // draw the labels
  std::list<LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it)
  {
    drawLabel( *it, painter, xform );
  }

  std::cout << "LABELING draw:   " << t.elapsed() << "ms" << std::endl;

  delete problem;
  delete labels;

  // delete all allocated geometries for features
  for (int i = 0; i < mLayers.count(); i++)
  {
    LayerSettings& lyr = mLayers[i];
    for (QList<MyLabel*>::iterator git = lyr.geometries.begin(); git != lyr.geometries.end(); ++git)
      delete *git;
    lyr.geometries.clear();
  }

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

void PalLabeling::drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform )
{
  QgsPoint outPt = xform->transform(lp->getX(), lp->getY());
  QgsPoint outPt2 = xform->transform(lp->getX()+lp->getWidth(), lp->getY()+lp->getHeight());

  painter->save();
  painter->translate( QPointF(outPt.x(), outPt.y()) );
  painter->rotate(-lp->getAlpha() * 180 / M_PI );
  QRectF rect(0,0, outPt2.x()-outPt.x(), outPt2.y()-outPt.y());
  painter->drawRect(rect);
  painter->restore();

  // save the rect
  rect.moveTo(outPt.x(),outPt.y());
  mCandidates.append( LabelCandidate(rect, lp->getCost() * 1000) );

  // show all parts of the multipart label
  if (lp->getNextPart())
    drawLabelCandidateRect(lp->getNextPart(), painter, xform);
}


void PalLabeling::drawLabel( pal::LabelPosition* label, QPainter* painter, const QgsMapToPixel* xform)
{
  QgsPoint outPt = xform->transform(label->getX(), label->getY());

  // TODO: optimize access :)
  const LayerSettings& lyr = layer(label->getLayerName());

  QString text = ((MyLabel*)label->getFeature()->getUserGeometry())->text();

  // shift by one as we have 2px border
  painter->save();
  painter->translate( QPointF(outPt.x()+1, outPt.y()-1-lyr.fontBaseline) );
  painter->rotate(-label->getAlpha() * 180 / M_PI );
  painter->setFont( lyr.textFont );

  if (lyr.bufferSize != 0)
    drawLabelBuffer(painter, text, lyr.bufferSize, lyr.bufferColor);

  painter->setPen( lyr.textColor );
  if (label->getPartId() == -1)
    painter->drawText(0,0, text);
  else
    painter->drawText(0,0, QString( text[label->getPartId()] ) );
  painter->restore();

  if (label->getNextPart())
    drawLabel( label->getNextPart(), painter, xform );
}


void PalLabeling::drawLabelBuffer(QPainter* p, QString text, int size, QColor color)
{
  p->save();
  p->setPen(color);
  for (int x = -size; x <= size; x++)
    for (int y = -size; y <= size; y++)
      p->drawText(x,y, text);
  p->restore();
}
