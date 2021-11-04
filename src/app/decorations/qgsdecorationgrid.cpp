/***************************************************************************
                         qgsdecorationgrid.h
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationgrid.h"
#include "qgsdecorationgriddialog.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgspathresolver.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgstextrenderer.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

#include <QPainter>
#include <QAction>
#include <QPen>
#include <QPolygon>
#include <QString>
#include <QFontMetrics>
#include <QFont>
#include <QColor>
#include <QMenu>
#include <QFile>
#include <QLocale>
#include <QDomDocument>
#include <QMessageBox>

//non qt includes
#include <cmath>


QgsDecorationGrid::QgsDecorationGrid( QObject *parent )
  : QgsDecorationItem( parent )
{
  setDisplayName( tr( "Grid" ) );
  mConfigurationName = QStringLiteral( "Grid" );

  projectRead();

  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::destinationCrsChanged,
           this, &QgsDecorationGrid::checkMapUnitsChanged );
}

QgsDecorationGrid::~QgsDecorationGrid() = default;

void QgsDecorationGrid::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

void QgsDecorationGrid::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mMarkerSymbol.reset( symbol );
}

void QgsDecorationGrid::projectRead()
{
  QgsDecorationItem::projectRead();

  mEnabled = QgsProject::instance()->readBoolEntry( mConfigurationName, QStringLiteral( "/Enabled" ), false );
  mMapUnits = static_cast< QgsUnitTypes::DistanceUnit >( QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MapUnits" ),
              QgsUnitTypes::DistanceUnknownUnit ) );
  mGridStyle = static_cast< GridStyle >( QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/Style" ),
                                         QgsDecorationGrid::Line ) );
  mGridIntervalX = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/IntervalX" ), 10 );
  mGridIntervalY = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/IntervalY" ), 10 );
  mGridOffsetX = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/OffsetX" ), 0 );
  mGridOffsetY = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/OffsetY" ), 0 );
  mShowGridAnnotation = QgsProject::instance()->readBoolEntry( mConfigurationName, QStringLiteral( "/ShowAnnotation" ), false );
  mGridAnnotationDirection = static_cast< GridAnnotationDirection >( QgsProject::instance()->readNumEntry( mConfigurationName,
                             QStringLiteral( "/AnnotationDirection" ), 0 ) );

  QDomDocument doc;
  QDomElement elem;
  const QString textXml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Font" ) );
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    elem = doc.documentElement();
    QgsReadWriteContext rwContext;
    rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
    mTextFormat.readXml( elem, rwContext );
  }

  mAnnotationFrameDistance = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/AnnotationFrameDistance" ), 0 );
  mGridAnnotationPrecision = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/AnnotationPrecision" ), 0 );

  // read symbol info from xml
  QString xml;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );

  if ( mLineSymbol )
    setLineSymbol( nullptr );
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, rwContext ) );
  }
  if ( ! mLineSymbol )
    mLineSymbol = std::make_unique< QgsLineSymbol >();

  if ( mMarkerSymbol )
    setMarkerSymbol( nullptr );
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( elem, rwContext ) );
  }
  if ( ! mMarkerSymbol )
  {
    // set default symbol : cross with width=3
    QgsSymbolLayerList symbolList;
    symbolList << new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross, 3, 0 );
    mMarkerSymbol = std::make_unique< QgsMarkerSymbol >( symbolList );
  }
}

void QgsDecorationGrid::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Enabled" ), mEnabled );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MapUnits" ), static_cast< int >( mMapUnits ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Style" ), static_cast< int >( mGridStyle ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/IntervalX" ), mGridIntervalX );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/IntervalY" ), mGridIntervalY );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/OffsetX" ), mGridOffsetX );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/OffsetY" ), mGridOffsetY );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ShowAnnotation" ), mShowGridAnnotation );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/AnnotationDirection" ), static_cast< int >( mGridAnnotationDirection ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/AnnotationFont" ), mGridAnnotationFont.toString() );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/AnnotationFrameDistance" ), mAnnotationFrameDistance );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/AnnotationPrecision" ), mGridAnnotationPrecision );

  QDomDocument textDoc;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  const QDomElement textElem = mTextFormat.writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Font" ), textDoc.toString() );

  // write symbol info to xml
  QDomDocument doc;
  QDomElement elem;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( mLineSymbol )
  {
    elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "line symbol" ), mLineSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    // FIXME this works, but XML will not be valid as < is replaced by &lt;
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ), doc.toString() );
  }
  if ( mMarkerSymbol )
  {
    doc.setContent( QString() );
    elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ), doc.toString() );
  }

}


void QgsDecorationGrid::run()
{
  QgsDecorationGridDialog dlg( *this );

  if ( dlg.exec() )
  {
    update();
  }
}

void QgsDecorationGrid::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( ! mEnabled )
    return;

  QList< QPair< qreal, QLineF > > verticalLines;
  yGridLines( mapSettings, verticalLines );
  QList< QPair< qreal, QLineF > > horizontalLines;
  xGridLines( mapSettings, horizontalLines );

  QList< QPair< qreal, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< qreal, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  switch ( mGridStyle )
  {
    case Line:
    {
      if ( ! mLineSymbol )
        return;

      mLineSymbol->startRender( context );

      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        // context.painter()->drawLine( vIt->second );
        // need to convert QLineF to QPolygonF ...
        QVector<QPointF> poly;
        poly << vIt->second.p1() << vIt->second.p2();
        mLineSymbol->renderPolyline( QPolygonF( poly ), nullptr, context );
      }

      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        // context.painter()->drawLine( hIt->second );
        // need to convert QLineF to QPolygonF ...
        QVector<QPointF> poly;
        poly << hIt->second.p1() << hIt->second.p2();
        mLineSymbol->renderPolyline( QPolygonF( poly ), nullptr, context );
      }

      mLineSymbol->stopRender( context );
      break;
    }

    case Marker:
    {
      if ( ! mMarkerSymbol )
        return;

      mMarkerSymbol->startRender( context );

      QPointF intersectionPoint;
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        //test for intersection with every horizontal line
        hIt = horizontalLines.constBegin();
        for ( ; hIt != horizontalLines.constEnd(); ++hIt )
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
          if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
          if ( hIt->second.intersects( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
          {
            mMarkerSymbol->renderPoint( intersectionPoint, nullptr, context );
          }
        }
      }
      mMarkerSymbol->stopRender( context );
      break;
    }
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( context, horizontalLines, verticalLines );
  }
}

void QgsDecorationGrid::drawCoordinateAnnotations( QgsRenderContext &context, const QList< QPair< qreal, QLineF > > &hLines, const QList< QPair< qreal, QLineF > > &vLines )
{
  QString currentAnnotationString;
  QList< QPair< qreal, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( context, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( context, it->second.p2(), currentAnnotationString );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( context, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( context, it->second.p2(), currentAnnotationString );
  }
}

void QgsDecorationGrid::drawCoordinateAnnotation( QgsRenderContext &context, QPointF pos, const QString &annotationString )
{
  const Border frameBorder = borderForLineCoord( pos, context.painter() );
  const QStringList annotationStringList = QStringList() << annotationString;

  const QFontMetricsF textMetrics = QgsTextRenderer::fontMetrics( context, mTextFormat );
  const double textDescent = textMetrics.descent();
  const double textWidth = QgsTextRenderer::textWidth( context, mTextFormat, annotationStringList );
  const double textHeight = QgsTextRenderer::textHeight( context, mTextFormat, annotationStringList, QgsTextRenderer::Point );

  double xpos = pos.x();
  double ypos = pos.y();
  double rotation = 0;

  switch ( frameBorder )
  {
    case Left:
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos += mAnnotationFrameDistance + textDescent;
        ypos -= textWidth / 2;
        rotation = 4.71239;
      }
      else //Horizontal
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0 - textDescent;
      }
      break;

    case Right:
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textHeight - textDescent + mAnnotationFrameDistance;
        ypos -= textWidth / 2;
        rotation = 4.71239;
      }
      else //Horizontal
      {
        xpos -= textWidth + mAnnotationFrameDistance;
        ypos += textHeight / 2.0 - textDescent;
      }
      break;

    case Bottom:
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        ypos -= mAnnotationFrameDistance + textDescent;
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos -= textDescent;
        ypos -= textWidth + mAnnotationFrameDistance;
        rotation = 4.71239;
      }
      break;

    case Top:
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos += textHeight - textDescent + mAnnotationFrameDistance;
      }
      else //Vertical
      {
        xpos -= textDescent;
        ypos += mAnnotationFrameDistance;
        rotation = 4.71239;
      }
  }

  QgsTextRenderer::drawText( QPointF( xpos, ypos ), rotation, QgsTextRenderer::AlignLeft, annotationStringList, context, mTextFormat );
}

static bool clipByRect( QLineF &line, const QPolygonF &rect )
{
  QVector<QLineF> borderLines;
  borderLines << QLineF( rect.at( 0 ), rect.at( 1 ) );
  borderLines << QLineF( rect.at( 1 ), rect.at( 2 ) );
  borderLines << QLineF( rect.at( 2 ), rect.at( 3 ) );
  borderLines << QLineF( rect.at( 3 ), rect.at( 0 ) );

  QVector<QPointF> intersectionList;
  QVector<QLineF>::const_iterator it = borderLines.constBegin();
  for ( ; it != borderLines.constEnd(); ++it )
  {
    QPointF intersectionPoint;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if ( it->intersect( line, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
    if ( it->intersects( line, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
    {
      intersectionList.push_back( intersectionPoint );
      if ( intersectionList.size() >= 2 )
      {
        break; //we already have two intersections, skip further tests
      }
    }
  }
  if ( intersectionList.size() < 2 ) return false; // no intersection

  line = QLineF( intersectionList.at( 0 ), intersectionList.at( 1 ) );
  return true;
}

static QPolygonF canvasExtent( const QgsMapSettings &mapSettings )
{
  QPolygonF poly;
  const QgsRectangle extent = mapSettings.visibleExtent();
  poly << QPointF( extent.xMinimum(), extent.yMaximum() );
  poly << QPointF( extent.xMaximum(), extent.yMaximum() );
  poly << QPointF( extent.xMaximum(), extent.yMinimum() );
  poly << QPointF( extent.xMinimum(), extent.yMinimum() );
  return poly;
}

int QgsDecorationGrid::xGridLines( const QgsMapSettings &mapSettings, QList< QPair< qreal, QLineF > > &lines ) const
{
  // prepare horizontal lines
  lines.clear();
  if ( mGridIntervalY <= 0.0 )
  {
    return 1;
  }

  const QgsMapToPixel &m2p = mapSettings.mapToPixel();

  // draw nothing if the distance between grid lines would be less than 1px
  // otherwise the grid lines would completely cover the whole map
  //if ( mapBoundingRect.height() / mGridIntervalY >= p->device()->height() )
  if ( mGridIntervalY / mapSettings.mapUnitsPerPixel() < 1 )
    return 1;

  const QPolygonF canvasPoly = mapSettings.visiblePolygon();
  const QPolygonF mapPolygon = canvasExtent( mapSettings );
  const QRectF mapBoundingRect = mapPolygon.boundingRect();
  const QLineF lineEast( mapPolygon[2], mapPolygon[1] );
  const QLineF lineWest( mapPolygon[3], mapPolygon[0] );

  const double len = lineEast.length();
  Q_ASSERT( std::fabs( len - lineWest.length() ) < 1e-6 ); // no shear

  const double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double dist = static_cast< int >( ( mapBoundingRect.top() - mGridOffsetY ) / mGridIntervalY + roundCorrection ) * mGridIntervalY + mGridOffsetY;
  dist = dist - mapBoundingRect.top();
  while ( dist < len )
  {
    const double t = dist / len;
    const QPointF p0 = lineWest.pointAt( t );
    const QPointF p1 = lineEast.pointAt( t );
    QLineF line( p0, p1 );
    clipByRect( line, canvasPoly );
    line = QLineF( m2p.transform( QgsPointXY( line.pointAt( 0 ) ) ).toQPointF(), m2p.transform( QgsPointXY( line.pointAt( 1 ) ) ).toQPointF() );
    lines.push_back( qMakePair( p0.y(), line ) );
    dist += mGridIntervalY;
  }

  return 0;
}

int QgsDecorationGrid::yGridLines( const QgsMapSettings &mapSettings, QList< QPair< qreal, QLineF > > &lines ) const
{
  // prepare vertical lines

  lines.clear();
  if ( mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  const QgsMapToPixel &m2p = mapSettings.mapToPixel();

  // draw nothing if the distance between grid lines would be less than 1px
  // otherwise the grid lines would completely cover the whole map
  //if ( mapBoundingRect.height() / mGridIntervalY >= p->device()->height() )
  if ( mGridIntervalX / mapSettings.mapUnitsPerPixel() < 1 )
    return 1;

  const QPolygonF canvasPoly = mapSettings.visiblePolygon();
  const QPolygonF mapPolygon = canvasExtent( mapSettings );
  const QLineF lineSouth( mapPolygon[3], mapPolygon[2] );
  const QLineF lineNorth( mapPolygon[0], mapPolygon[1] );

  const double len = lineSouth.length();
  Q_ASSERT( std::fabs( len - lineNorth.length() ) < 1e-6 ); // no shear

  const QRectF mapBoundingRect = mapPolygon.boundingRect();
  const double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double dist = static_cast< int >( ( mapBoundingRect.left() - mGridOffsetX ) / mGridIntervalX + roundCorrection ) * mGridIntervalX + mGridOffsetX;
  dist = dist - mapBoundingRect.left();
  while ( dist < len )
  {
    const double t = dist / len;
    const QPointF p0( lineNorth.pointAt( t ) );
    const QPointF p1( lineSouth.pointAt( t ) );
    QLineF line( p0, p1 );
    clipByRect( line, canvasPoly );
    line = QLineF( m2p.transform( QgsPointXY( line.pointAt( 0 ) ) ).toQPointF(), m2p.transform( QgsPointXY( line.pointAt( 1 ) ) ).toQPointF() );
    lines.push_back( qMakePair( p0.x(), line ) );
    dist += mGridIntervalX;
  }

  return 0;
}

QgsDecorationGrid::Border QgsDecorationGrid::borderForLineCoord( QPointF point, const QPainter *p ) const
{
  if ( point.x() <= mGridPen.widthF() )
  {
    return Left;
  }
  else if ( point.x() >= ( p->device()->width() - mGridPen.widthF() ) )
  {
    return Right;
  }
  else if ( point.y() <= mGridPen.widthF() )
  {
    return Top;
  }
  else
  {
    return Bottom;
  }
}

void QgsDecorationGrid::checkMapUnitsChanged()
{
  // disable grid if units changed and grid is enabled, and update the canvas
  // this is to avoid problems when CRS changes to/from geographic and projected
  // a better solution would be to change the grid interval, but this is a little tricky
  // note: we could be less picky (e.g. from degrees to DMS)
  const QgsUnitTypes::DistanceUnit mapUnits = QgisApp::instance()->mapCanvas()->mapSettings().mapUnits();
  if ( mEnabled && ( mMapUnits != mapUnits ) )
  {
    mEnabled = false;
    mMapUnits = QgsUnitTypes::DistanceUnknownUnit; // make sure isDirty() returns true
    if ( ! QgisApp::instance()->mapCanvas()->isFrozen() )
    {
      update();
    }
  }
}

bool QgsDecorationGrid::isDirty()
{
  // checks if stored map units is undefined or different from canvas map units
  // or if interval is 0
  return mMapUnits == QgsUnitTypes::DistanceUnknownUnit ||
         mMapUnits != QgisApp::instance()->mapCanvas()->mapSettings().mapUnits() ||
         qgsDoubleNear( mGridIntervalX, 0.0 ) || qgsDoubleNear( mGridIntervalY, 0.0 );
}

void QgsDecorationGrid::setDirty( bool dirty )
{
  if ( dirty )
  {
    mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
  }
  else
  {
    mMapUnits = QgisApp::instance()->mapCanvas()->mapSettings().mapUnits();
  }
}

bool QgsDecorationGrid::getIntervalFromExtent( double *values, bool useXAxis ) const
{
  // get default interval from current extents
  // calculate a default interval that is approx (extent width)/5, adjusted so that it is a rounded number
  // e.g. 12.7 -> 10  66556 -> 70000
  double interval = 0;
  const QgsRectangle extent = QgisApp::instance()->mapCanvas()->extent();
  if ( useXAxis )
    interval = ( extent.xMaximum() - extent.xMinimum() ) / 5;
  else
    interval = ( extent.yMaximum() - extent.yMinimum() ) / 5;
  QgsDebugMsg( QStringLiteral( "interval: %1" ).arg( interval ) );
  if ( !qgsDoubleNear( interval, 0.0 ) )
  {
    double interval2 = 0;
    const int factor = std::pow( 10, std::floor( std::log10( interval ) ) );
    if ( factor != 0 )
    {
      interval2 = std::round( interval / factor ) * factor;
      QgsDebugMsg( QStringLiteral( "interval2: %1" ).arg( interval2 ) );
      if ( !qgsDoubleNear( interval2, 0.0 ) )
        interval = interval2;
    }
  }
  values[0] = values[1] = interval;
  values[2] = values[3] = 0;
  return true;
}

bool QgsDecorationGrid::getIntervalFromCurrentLayer( double *values ) const
{
  // get current layer and make sure it is a raster layer and CRSs match
  QgsMapLayer *layer = QgisApp::instance()->mapCanvas()->currentLayer();
  if ( ! layer )
  {
    QMessageBox::warning( nullptr, tr( "Get Interval from Layer" ), tr( "No active layer" ) );
    return false;
  }
  if ( layer->type() != QgsMapLayerType::RasterLayer )
  {
    QMessageBox::warning( nullptr, tr( "Get Interval from Layer" ), tr( "Please select a raster layer." ) );
    return false;
  }
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rlayer || rlayer->width() == 0 || rlayer->height() == 0 )
  {
    QMessageBox::warning( nullptr, tr( "Get Interval from Layer" ), tr( "Invalid raster layer" ) );
    return false;
  }
  const QgsCoordinateReferenceSystem layerCRS = layer->crs();
  const QgsCoordinateReferenceSystem mapCRS =
    QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs();
  // is this the best way to compare CRS? should we also make sure map has OTF enabled?
  // TODO calculate transformed values if necessary
  if ( layerCRS != mapCRS )
  {
    QMessageBox::warning( nullptr, tr( "Get Interval from Layer" ), tr( "Layer CRS must be equal to project CRS." ) );
    return false;
  }

  // calculate interval
  // TODO add a function in QgsRasterLayer to get x/y resolution from provider,
  // because this might not be 100% accurate
  const QgsRectangle extent = rlayer->extent();
  values[0] = std::fabs( extent.xMaximum() - extent.xMinimum() ) / rlayer->width();
  values[1] = std::fabs( extent.yMaximum() - extent.yMinimum() ) / rlayer->height();

  // calculate offset - when using very high resolution rasters in geographic CRS
  // there seems to be a small shift, but this may be due to rendering issues and depends on zoom
  double ratio = extent.xMinimum() / values[0];
  values[2] = ( ratio - std::floor( ratio ) ) * values[0];
  ratio = extent.yMinimum() / values[1];
  values[3] = ( ratio - std::floor( ratio ) ) * values[1];

  QgsDebugMsg( QStringLiteral( "xmax: %1 xmin: %2 width: %3 xInterval: %4 xOffset: %5" ).arg(
                 extent.xMaximum() ).arg( extent.xMinimum() ).arg( rlayer->width() ).arg( values[0] ).arg( values[2] ) );
  QgsDebugMsg( QStringLiteral( "ymax: %1 ymin: %2 height: %3 yInterval: %4 yOffset: %5" ).arg(
                 extent.yMaximum() ).arg( extent.yMinimum() ).arg( rlayer->height() ).arg( values[1] ).arg( values[3] ) );

  return true;
}
