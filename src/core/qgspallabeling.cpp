/***************************************************************************
  qgspallabeling.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspallabeling.h"

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


class QgsPalGeometry : public PalGeometry
{
  public:
    QgsPalGeometry( int id, QString text, GEOSGeometry* g ): mG( g ), mText( text ), mId( id ), mInfo( NULL )
    {
      mStrId = QByteArray::number( id );
    }

    ~QgsPalGeometry()
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

    pal::LabelInfo* info( QFontMetrics* fm, const QgsMapToPixel* xform, double fontScale )
    {
      if ( mInfo ) return mInfo;

      // create label info!
      QgsPoint ptZero = xform->toMapCoordinates( 0, 0 );
      QgsPoint ptSize = xform->toMapCoordinates( 0, ( int )( -fm->height() / fontScale ) );

      mInfo = new pal::LabelInfo( mText.count(), ptSize.y() - ptZero.y() );
      for ( int i = 0; i < mText.count(); i++ )
      {
        mInfo->char_info[i].chr = mText[i].unicode();
        ptSize = xform->toMapCoordinates(( int )( fm->width( mText[i] ) / fontScale ) , 0 );
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

QgsPalLayerSettings::QgsPalLayerSettings()
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
  minFeatureSize = 0.0;
  vectorScaleFactor = 1.0;
  rasterCompressFactor = 1.0;
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings& s )
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
  minFeatureSize = s.minFeatureSize;
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;

  fontMetrics = NULL;
  ct = NULL;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
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

void QgsPalLayerSettings::readFromLayer( QgsVectorLayer* layer )
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
  bufferSize = layer->customProperty( "labeling/bufferSize" ).toDouble();
  bufferColor = _readColor( layer, "labeling/bufferColor" );
  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
  minFeatureSize = layer->customProperty( "labeling/minFeatureSize" ).toDouble();
}

void QgsPalLayerSettings::writeToLayer( QgsVectorLayer* layer )
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
  layer->setCustomProperty( "labeling/minFeatureSize", minFeatureSize );
}

bool QgsPalLayerSettings::checkMinimumSizeMM( const QgsRenderContext& ct, QgsGeometry* geom, double minSize ) const
{
  if ( minSize <= 0 )
  {
    return true;
  }

  if ( !geom )
  {
    return false;
  }

  QGis::GeometryType featureType = geom->type();
  if ( featureType == QGis::Point ) //minimum size does not apply to point features
  {
    return true;
  }

  double mapUnitsPerMM = ct.mapToPixel().mapUnitsPerPixel() * ct.scaleFactor();
  if ( featureType == QGis::Line )
  {
    double length = geom->length();
    if ( length >= 0.0 )
    {
      return ( length >= ( minSize * mapUnitsPerMM ) );
    }
  }
  else if ( featureType == QGis::Polygon )
  {
    double area = geom->area();
    if ( area >= 0.0 )
    {
      return ( sqrt( area ) >= ( minSize * mapUnitsPerMM ) );
    }
  }
  return true; //should never be reached. Return true in this case to label such geometries anyway.
}

void QgsPalLayerSettings::calculateLabelSize( QString text, double& labelX, double& labelY )
{
  QRectF labelRect = fontMetrics->boundingRect( text );
  double w = labelRect.width() / rasterCompressFactor;
  double h = labelRect.height() / rasterCompressFactor;

  QgsPoint ptSize = xform->toMapCoordinates( w, h );

  labelX = fabs( ptSize.x() - ptZero.x() );
  labelY = fabs( ptSize.y() - ptZero.y() );
}


void QgsPalLayerSettings::registerFeature( QgsFeature& f, const QgsRenderContext& context )
{
  QString labelText = f.attributeMap()[fieldIndex].toString();
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelText, labelX, labelY );

  QgsGeometry* geom = f.geometry();
  if ( ct != NULL ) // reproject the geometry if necessary
    geom->transform( *ct );

  if ( !checkMinimumSizeMM( context, geom, minFeatureSize ) )
  {
    return;
  }

  QgsPalGeometry* lbl = new QgsPalGeometry( f.id(), labelText, GEOSGeom_clone( geom->asGeos() ) );

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
  feat->setLabelInfo( lbl->info( fontMetrics, xform, rasterCompressFactor ) );

  // TODO: allow layer-wide feature dist in PAL...?
  if ( dist != 0 )
    feat->setDistLabel( fabs( ptOne.x() - ptZero.x() )* dist * vectorScaleFactor );
}


// -------------

QgsPalLabeling::QgsPalLabeling()
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

  mShowingCandidates = false;
  mShowingAllLabels = false;
}


QgsPalLabeling::~QgsPalLabeling()
{
  // make sure we've freed everything
  exit();
}


bool QgsPalLabeling::willUseLayer( QgsVectorLayer* layer )
{
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );
  return lyrTmp.enabled;
}


int QgsPalLabeling::prepareLayer( QgsVectorLayer* layer, int& attrIndex, QgsRenderContext& ctx )
{
  Q_ASSERT( mMapRenderer != NULL );

  // start with a temporary settings class, find out labeling info
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );

  if ( !lyrTmp.enabled )
    return 0;

  // find out which field will be needed
  int fldIndex = layer->fieldNameIndex( lyrTmp.fieldName );
  if ( fldIndex == -1 )
    return 0;
  attrIndex = fldIndex;

  // add layer settings to the pallabeling hashtable: <QgsVectorLayer*, QgsPalLayerSettings>
  mActiveLayers.insert( layer, lyrTmp );
  // start using the reference to the layer in hashtable instead of local instance
  QgsPalLayerSettings& lyr = mActiveLayers[layer];

  // how to place the labels
  Arrangement arrangement;
  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint: arrangement = P_POINT; break;
    case QgsPalLayerSettings::OverPoint:   arrangement = P_POINT_OVER; break;
    case QgsPalLayerSettings::Line:        arrangement = P_LINE; break;
    case QgsPalLayerSettings::Curved:      arrangement = P_CURVED; break;
    case QgsPalLayerSettings::Horizontal:  arrangement = P_HORIZ; break;
    case QgsPalLayerSettings::Free:        arrangement = P_FREE; break;
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
  double size = 0.3527 * lyr.textFont.pointSizeF() * ctx.scaleFactor();
  // request larger font and then scale down painter (to avoid Qt font scale bug)
  lyr.textFont.setPixelSize(( int )( size*ctx.rasterScaleFactor() + 0.5 ) );

  //raster and vector scale factors
  lyr.vectorScaleFactor = ctx.scaleFactor();
  lyr.rasterCompressFactor = ctx.rasterScaleFactor();

  // save the pal layer to our layer context (with some additional info)
  lyr.palLayer = l;
  lyr.fieldIndex = fldIndex;
  lyr.fontMetrics = new QFontMetrics( lyr.textFont );

  lyr.xform = mMapRenderer->coordinateTransform();
  if ( mMapRenderer->hasCrsTransformEnabled() )
    lyr.ct = new QgsCoordinateTransform( layer->srs(), mMapRenderer->destinationSrs() );
  else
    lyr.ct = NULL;
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  return 1; // init successful
}


void QgsPalLabeling::registerFeature( QgsVectorLayer* layer, QgsFeature& f, const QgsRenderContext& context )
{
  QgsPalLayerSettings& lyr = mActiveLayers[layer];
  lyr.registerFeature( f, context );
}


void QgsPalLabeling::init( QgsMapRenderer* mr )
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

void QgsPalLabeling::exit()
{
  delete mPal;
  mPal = NULL;
  mMapRenderer = NULL;
}

QgsPalLayerSettings& QgsPalLabeling::layer( const char* layerName )
{
  QHash<QgsVectorLayer*, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    QgsPalLayerSettings& lyr = lit.value();
    if ( lyr.palLayer->getName() == layerName )
      return lyr;
  }
  return mInvalidLayerSettings;
}


void QgsPalLabeling::drawLabeling( QgsRenderContext& context )
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
    const QgsPalLayerSettings& lyr = layer(( *it )->getLayerName() );

    if ( lyr.bufferSize != 0 )
      drawLabel( *it, painter, xform, true );

    drawLabel( *it, painter, xform );
  }

  QgsDebugMsg( QString( "LABELING draw:  %1 ms" ).arg( t.elapsed() ) );

  delete problem;
  delete labels;

  // delete all allocated geometries for features
  QHash<QgsVectorLayer*, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    QgsPalLayerSettings& lyr = lit.value();
    for ( QList<QgsPalGeometry*>::iterator git = lyr.geometries.begin(); git != lyr.geometries.end(); ++git )
      delete *git;
    lyr.geometries.clear();
  }
  // labeling is done: clear the active layers hashtable
  mActiveLayers.clear();

}

void QgsPalLabeling::numCandidatePositions( int& candPoint, int& candLine, int& candPolygon )
{
  candPoint = mCandPoint;
  candLine = mCandLine;
  candPolygon = mCandPolygon;
}

void QgsPalLabeling::setNumCandidatePositions( int candPoint, int candLine, int candPolygon )
{
  mCandPoint = candPoint;
  mCandLine = candLine;
  mCandPolygon = candPolygon;
}

void QgsPalLabeling::setSearchMethod( QgsPalLabeling::Search s )
{
  mSearch = s;
}

QgsPalLabeling::Search QgsPalLabeling::searchMethod() const
{
  return mSearch;
}

void QgsPalLabeling::drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform )
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
  mCandidates.append( QgsLabelCandidate( rect, lp->getCost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->getNextPart() )
    drawLabelCandidateRect( lp->getNextPart(), painter, xform );
}

#include "qgslogger.h"

void QgsPalLabeling::drawLabel( pal::LabelPosition* label, QPainter* painter, const QgsMapToPixel* xform, bool drawBuffer )
{
  QgsPoint outPt = xform->transform( label->getX(), label->getY() );

  // TODO: optimize access :)
  const QgsPalLayerSettings& lyr = layer( label->getLayerName() );

  QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();
  QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );

  //QgsDebugMsg( "drawLabel " + QString::number( drawBuffer ) + " " + txt );

  painter->save();
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -label->getAlpha() * 180 / M_PI );

  // scale down painter: the font size has been multiplied by raster scale factor
  // to workaround a Qt font scaling bug with small font sizes
  painter->scale( 1.0 / lyr.rasterCompressFactor, 1.0 / lyr.rasterCompressFactor );

  painter->translate( QPointF( 0, - lyr.fontMetrics->descent() ) );

  if ( drawBuffer )
  {
    // we're drawing buffer
    drawLabelBuffer( painter, txt, lyr.textFont, lyr.bufferSize * lyr.vectorScaleFactor * lyr.rasterCompressFactor , lyr.bufferColor );
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


void QgsPalLabeling::drawLabelBuffer( QPainter* p, QString text, const QFont& font, double size, QColor color )
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
  QPen pen( color );
  pen.setWidthF( size );
  p->setPen( pen );
  p->setBrush( color );
  p->drawPath( path );
}

QgsLabelingEngineInterface* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  return lbl;
}
