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

#include <qgslogger.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaprenderer.h>


using namespace pal;


class MyLabel : public PalGeometry
{
  public:
    MyLabel( int id, QString text, GEOSGeometry* g ): mG( g ), mText( text ), mId( id ), mInfo( NULL )
    {
      mStrId = QByteArray::number( id );
    }

    ~MyLabel()
    {
      if ( mG ) GEOSGeom_destroy( mG );
      delete mInfo;
    }

    // getGeosGeometry + releaseGeosGeometry is called twice: once when adding, second time when labeling

    GEOSGeometry* getGeosGeometry()
    {
      return mG;
    }
    void releaseGeosGeometry( GEOSGeometry* /*geom*/ )
    {
      // nothing here - we'll delete the geometry in destructor
    }

    const char* strId() { return mStrId.data(); }
    QString text() { return mText; }

    pal::LabelInfo* info( QFontMetrics* fm, const QgsMapToPixel* xform )
    {
      if ( mInfo ) return mInfo;

      // create label info!
      QgsPoint ptZero = xform->toMapCoordinates( 0, 0 );
      QgsPoint ptSize = xform->toMapCoordinates( 0, -fm->height() );

      mInfo = new pal::LabelInfo( mText.count(), ptSize.y() - ptZero.y() );
      for ( int i = 0; i < mText.count(); i++ )
      {
        mInfo->char_info[i].chr = mText[i].unicode();
        ptSize = xform->toMapCoordinates( fm->width( mText[i] ), 0 );
        mInfo->char_info[i].width = ptSize.x() - ptZero.x();
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
    : palLayer( NULL ), fontMetrics( NULL ), ct( NULL )
{
  placement = AroundPoint;
  placementFlags = 0;
  //textFont = QFont();
  textColor = Qt::black;
  enabled = false;
  priority = 5;
  obstacle = true;
  dist = 0;
  scaleMin = 0;
  scaleMax = 0;
  bufferSize = 1;
  bufferColor = Qt::white;
  labelPerPart = false;
  mergeLines = false;
}

LayerSettings::LayerSettings( const LayerSettings& s )
{
  // copy only permanent stuff
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
  labelPerPart = s.labelPerPart;
  mergeLines = s.mergeLines;

  fontMetrics = NULL;
  ct = NULL;
}


LayerSettings::~LayerSettings()
{
  // pal layer is deleted internally in PAL

  delete fontMetrics;
  delete ct;
}

static QColor _readColor( QgsVectorLayer* layer, QString property )
{
  int r = layer->customProperty( property + "R" ).toInt();
  int g = layer->customProperty( property + "G" ).toInt();
  int b = layer->customProperty( property + "B" ).toInt();
  return QColor( r, g, b );
}

static void _writeColor( QgsVectorLayer* layer, QString property, QColor color )
{
  layer->setCustomProperty( property + "R", color.red() );
  layer->setCustomProperty( property + "G", color.green() );
  layer->setCustomProperty( property + "B", color.blue() );
}

void LayerSettings::readFromLayer( QgsVectorLayer* layer )
{
  if ( layer->customProperty( "labeling" ).toString() != QString( "pal" ) )
    return; // there's no information available

  fieldName = layer->customProperty( "labeling/fieldName" ).toString();
  placement = ( Placement ) layer->customProperty( "labeling/placement" ).toInt();
  placementFlags = layer->customProperty( "labeling/placementFlags" ).toUInt();
  QString fontFamily = layer->customProperty( "labeling/fontFamily" ).toString();
  int fontSize = layer->customProperty( "labeling/fontSize" ).toInt();
  int fontWeight = layer->customProperty( "labeling/fontWeight" ).toInt();
  bool fontItalic = layer->customProperty( "labeling/fontItalic" ).toBool();
  textFont = QFont( fontFamily, fontSize, fontWeight, fontItalic );
  textColor = _readColor( layer, "labeling/textColor" );
  enabled = layer->customProperty( "labeling/enabled" ).toBool();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle" ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  scaleMin = layer->customProperty( "labeling/scaleMin" ).toInt();
  scaleMax = layer->customProperty( "labeling/scaleMax" ).toInt();
  bufferSize = layer->customProperty( "labeling/bufferSize" ).toInt();
  bufferColor = _readColor( layer, "labeling/bufferColor" );
  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
}

void LayerSettings::writeToLayer( QgsVectorLayer* layer )
{
  // this is a mark that labeling information is present
  layer->setCustomProperty( "labeling", "pal" );

  layer->setCustomProperty( "labeling/fieldName", fieldName );
  layer->setCustomProperty( "labeling/placement", placement );
  layer->setCustomProperty( "labeling/placementFlags", ( unsigned int )placementFlags );

  layer->setCustomProperty( "labeling/fontFamily", textFont.family() );
  layer->setCustomProperty( "labeling/fontSize", textFont.pointSize() );
  layer->setCustomProperty( "labeling/fontWeight", textFont.weight() );
  layer->setCustomProperty( "labeling/fontItalic", textFont.italic() );

  _writeColor( layer, "labeling/textColor", textColor );
  layer->setCustomProperty( "labeling/enabled", enabled );
  layer->setCustomProperty( "labeling/priority", priority );
  layer->setCustomProperty( "labeling/obstacle", obstacle );
  layer->setCustomProperty( "labeling/dist", dist );
  layer->setCustomProperty( "labeling/scaleMin", scaleMin );
  layer->setCustomProperty( "labeling/scaleMax", scaleMax );
  layer->setCustomProperty( "labeling/bufferSize", bufferSize );
  _writeColor( layer, "labeling/bufferColor", bufferColor );
  layer->setCustomProperty( "labeling/labelPerPart", labelPerPart );
  layer->setCustomProperty( "labeling/mergeLines", mergeLines );
}

void LayerSettings::calculateLabelSize( QString text, double& labelX, double& labelY )
{
  //QFontMetrics fontMetrics(textFont);
  QRect labelRect = /*QRect(0,0,20,20);*/ fontMetrics->boundingRect( text );

  // 2px border...
  QgsPoint ptSize = xform->toMapCoordinates( labelRect.width() + 2, labelRect.height() + 2 );
  labelX = fabs( ptSize.x() - ptZero.x() );
  labelY = fabs( ptSize.y() - ptZero.y() );
}


void LayerSettings::registerFeature( QgsFeature& f )
{
  QString labelText = f.attributeMap()[fieldIndex].toString();
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelText, labelX, labelY );

  QgsGeometry* geom = f.geometry();
  if ( ct != NULL ) // reproject the geometry if necessary
    geom->transform( *ct );

  MyLabel* lbl = new MyLabel( f.id(), labelText, GEOSGeom_clone( geom->asGeos() ) );

  // record the created geometry - it will be deleted at the end.
  geometries.append( lbl );

  // register feature to the layer
  try
  {
    if ( !palLayer->registerFeature( lbl->strId(), lbl, labelX, labelY, labelText.toUtf8().constData() ) )
      return;
  }
  catch ( std::exception* e )
  {
    QgsDebugMsg( QString( "Ignoring feature %1 due PAL exception: " ).arg( f.id() ) + QString::fromLatin1( e->what() ) );
    return;
  }

  // TODO: only for placement which needs character info
  pal::Feature* feat = palLayer->getFeature( lbl->strId() );
  feat->setLabelInfo( lbl->info( fontMetrics, xform ) );

  // TODO: allow layer-wide feature dist in PAL...?
  if ( dist != 0 )
    feat->setDistLabel( fabs( ptOne.x() - ptZero.x() )* dist );
}


// -------------

PalLabeling::PalLabeling()
    : mMapRenderer( NULL ), mPal( NULL )
{

  // find out engine defaults
  Pal p;
  mCandPoint = p.getPointP();
  mCandLine = p.getLineP();
  mCandPolygon = p.getPolyP();

  switch ( p.getSearch() )
  {
    case CHAIN: mSearch = Chain; break;
    case POPMUSIC_TABU: mSearch = Popmusic_Tabu; break;
    case POPMUSIC_CHAIN: mSearch = Popmusic_Chain; break;
    case POPMUSIC_TABU_CHAIN: mSearch = Popmusic_Tabu_Chain; break;
    case FALP: mSearch = Falp; break;
  }

  mShowingCandidates = FALSE;
  mShowingAllLabels = FALSE;
}


PalLabeling::~PalLabeling()
{
  // make sure we've freed everything
  exit();
}


bool PalLabeling::willUseLayer( QgsVectorLayer* layer )
{
  LayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );
  return lyrTmp.enabled;
}


int PalLabeling::prepareLayer( QgsVectorLayer* layer, int& attrIndex, QgsRenderContext& ctx )
{
  Q_ASSERT( mMapRenderer != NULL );

  // start with a temporary settings class, find out labeling info
  LayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );

  if ( !lyrTmp.enabled )
    return 0;

  // find out which field will be needed
  int fldIndex = layer->fieldNameIndex( lyrTmp.fieldName );
  if ( fldIndex == -1 )
    return 0;
  attrIndex = fldIndex;

  // add layer settings to the pallabeling hashtable: <QgsVectorLayer*, LayerSettings>
  mActiveLayers.insert( layer, lyrTmp );
  // start using the reference to the layer in hashtable instead of local instance
  LayerSettings& lyr = mActiveLayers[layer];

  // how to place the labels
  Arrangement arrangement;
  switch ( lyr.placement )
  {
    case LayerSettings::AroundPoint: arrangement = P_POINT; break;
    case LayerSettings::OverPoint:   arrangement = P_POINT_OVER; break;
    case LayerSettings::Line:        arrangement = P_LINE; break;
    case LayerSettings::Curved:      arrangement = P_CURVED; break;
    case LayerSettings::Horizontal:  arrangement = P_HORIZ; break;
    case LayerSettings::Free:        arrangement = P_FREE; break;
    default: Q_ASSERT( "unsupported placement" && 0 ); return 0; break;
  }

  // create the pal layer
  double priority = 1 - lyr.priority / 10.0; // convert 0..10 --> 1..0
  double min_scale = -1, max_scale = -1;
  if ( lyr.scaleMin != 0 && lyr.scaleMax != 0 )
  {
    min_scale = lyr.scaleMin;
    max_scale = lyr.scaleMax;
  }

  Layer* l = mPal->addLayer( layer->getLayerID().toLocal8Bit().data(),
                             min_scale, max_scale, arrangement,
                             METER, priority, lyr.obstacle, true, true );

  if ( lyr.placementFlags )
    l->setArrangementFlags( lyr.placementFlags );

  // set label mode (label per feature is the default)
  l->setLabelMode( lyr.labelPerPart ? Layer::LabelPerFeaturePart : Layer::LabelPerFeature );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( lyr.mergeLines );

  // set font size from points to output size
  double size = 0.3527 * lyr.textFont.pointSizeF() * ctx.scaleFactor(); //* ctx.rasterScaleFactor();
  lyr.textFont.setPixelSize((int)size);

  // save the pal layer to our layer context (with some additional info)
  lyr.palLayer = l;
  lyr.fieldIndex = fldIndex;
  lyr.fontMetrics = new QFontMetrics( lyr.textFont );
  lyr.fontBaseline = lyr.fontMetrics->boundingRect( "X" ).bottom(); // dummy text to find out how many pixels of the text are below the baseline
  lyr.xform = mMapRenderer->coordinateTransform();
  if ( mMapRenderer->hasCrsTransformEnabled() )
    lyr.ct = new QgsCoordinateTransform( layer->srs(), mMapRenderer->destinationSrs() );
  else
    lyr.ct = NULL;
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  return 1; // init successful
}


void PalLabeling::registerFeature( QgsVectorLayer* layer, QgsFeature& f )
{
  mActiveLayers[layer].registerFeature( f );
}


void PalLabeling::init( QgsMapRenderer* mr )
{
  mMapRenderer = mr;

  // delete if exists already
  if ( mPal )
    delete mPal;

  mPal = new Pal;

  SearchMethod s;
  switch ( mSearch )
  {
    default:
    case Chain: s = CHAIN; break;
    case Popmusic_Tabu: s = POPMUSIC_TABU; break;
    case Popmusic_Chain: s = POPMUSIC_CHAIN; break;
    case Popmusic_Tabu_Chain: s = POPMUSIC_TABU_CHAIN; break;
    case Falp: s = FALP; break;
  }
  mPal->setSearch( s );

  // set number of candidates generated per feature
  mPal->setPointP( mCandPoint );
  mPal->setLineP( mCandLine );
  mPal->setPolyP( mCandPolygon );
}

void PalLabeling::exit()
{
  delete mPal;
  mPal = NULL;
  mMapRenderer = NULL;
}

LayerSettings& PalLabeling::layer( const char* layerName )
{
  QHash<QgsVectorLayer*, LayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    LayerSettings& lyr = lit.value();
    if ( lyr.palLayer->getName() == layerName )
      return lyr;
  }
  return mInvalidLayerSettings;
}


void PalLabeling::drawLabeling( QgsRenderContext& context )
{
  Q_ASSERT( mMapRenderer != NULL );
  QPainter* painter = context.painter();
  QgsRectangle extent = context.extent();

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
    problem = mPal->extractProblem( scale, bbox );
  }
  catch ( std::exception& e )
  {
    QgsDebugMsg( "PAL EXCEPTION :-( " + QString::fromLatin1( e.what() ) );
    mActiveLayers.clear(); // clean up
    return;
  }

  const QgsMapToPixel* xform = mMapRenderer->coordinateTransform();

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  mCandidates.clear();
  if ( mShowingCandidates && problem )
  {
    painter->setPen( QColor( 0, 0, 0, 64 ) );
    painter->setBrush( Qt::NoBrush );
    for ( int i = 0; i < problem->getNumFeatures(); i++ )
    {
      for ( int j = 0; j < problem->getFeatureCandidateCount( i ); j++ )
      {
        pal::LabelPosition* lp = problem->getFeatureCandidate( i, j );

        drawLabelCandidateRect( lp, painter, xform );
      }
    }
  }

  // find the solution
  labels = mPal->solveProblem( problem, mShowingAllLabels );

  QgsDebugMsg( QString( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( labels->size() ) );
  t.restart();

  painter->setRenderHint( QPainter::Antialiasing );

  // draw the labels
  std::list<LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it )
  {
    const LayerSettings& lyr = layer(( *it )->getLayerName() );

    if ( lyr.bufferSize != 0 )
      drawLabel( *it, painter, xform, true );

    drawLabel( *it, painter, xform );
  }

  QgsDebugMsg( QString( "LABELING draw:  %1 ms" ).arg( t.elapsed() ) );

  delete problem;
  delete labels;

  // delete all allocated geometries for features
  QHash<QgsVectorLayer*, LayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    LayerSettings& lyr = lit.value();
    for ( QList<MyLabel*>::iterator git = lyr.geometries.begin(); git != lyr.geometries.end(); ++git )
      delete *git;
    lyr.geometries.clear();
  }
  // labeling is done: clear the active layers hashtable
  mActiveLayers.clear();

}

void PalLabeling::numCandidatePositions( int& candPoint, int& candLine, int& candPolygon )
{
  candPoint = mCandPoint;
  candLine = mCandLine;
  candPolygon = mCandPolygon;
}

void PalLabeling::setNumCandidatePositions( int candPoint, int candLine, int candPolygon )
{
  mCandPoint = candPoint;
  mCandLine = candLine;
  mCandPolygon = candPolygon;
}

void PalLabeling::setSearchMethod( PalLabeling::Search s )
{
  mSearch = s;
}

PalLabeling::Search PalLabeling::searchMethod() const
{
  return mSearch;
}

void PalLabeling::drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform )
{
  QgsPoint outPt = xform->transform( lp->getX(), lp->getY() );
  QgsPoint outPt2 = xform->transform( lp->getX() + lp->getWidth(), lp->getY() + lp->getHeight() );

  painter->save();
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -lp->getAlpha() * 180 / M_PI );
  QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
  painter->drawRect( rect );
  painter->restore();

  // save the rect
  rect.moveTo( outPt.x(), outPt.y() );
  mCandidates.append( LabelCandidate( rect, lp->getCost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->getNextPart() )
    drawLabelCandidateRect( lp->getNextPart(), painter, xform );
}

#include "qgslogger.h"

void PalLabeling::drawLabel( pal::LabelPosition* label, QPainter* painter, const QgsMapToPixel* xform, bool drawBuffer )
{
  QgsPoint outPt = xform->transform( label->getX(), label->getY() );

  // TODO: optimize access :)
  const LayerSettings& lyr = layer( label->getLayerName() );

  QString text = (( MyLabel* )label->getFeaturePart()->getUserGeometry() )->text();
  QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );

  //QgsDebugMsg( "drawLabel " + QString::number( drawBuffer ) + " " + txt );

  // shift by one as we have 2px border
  painter->save();
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -label->getAlpha() * 180 / M_PI );
  painter->translate( QPointF( 1, -1 - lyr.fontBaseline ) );

  if ( drawBuffer )
  {
    // we're drawing buffer
    drawLabelBuffer( painter, txt, lyr.textFont, lyr.bufferSize, lyr.bufferColor );
  }
  else
  {
    // we're drawing real label
    /*painter->setFont( lyr.textFont );
    painter->setPen( lyr.textColor );
    painter->drawText((0,0, txt);*/

    QPainterPath path;
    path.addText( 0, 0, lyr.textFont, txt );
    painter->setPen( Qt::NoPen );
    painter->setBrush( lyr.textColor );
    painter->drawPath( path );
  }
  painter->restore();

  if ( label->getNextPart() )
    drawLabel( label->getNextPart(), painter, xform, drawBuffer );
}


void PalLabeling::drawLabelBuffer( QPainter* p, QString text, const QFont& font, int size, QColor color )
{
  /*
  p->setFont( font );
  p->setPen( color );
  for (int x = -size; x <= size; x++)
    for (int y = -size; y <= size; y++)
      p->drawText(x,y, text);
  */

  QPainterPath path;
  path.addText( 0, 0, font, text );
  p->setPen( QPen( color, size ) );
  p->setBrush( color );
  p->drawPath( path );
}

QgsLabelingEngineInterface* PalLabeling::clone()
{
  PalLabeling* lbl = new PalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  return lbl;
}
