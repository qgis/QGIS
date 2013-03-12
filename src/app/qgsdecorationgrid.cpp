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
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance
#include "qgssymbolv2.h" //for symbology
#include "qgsmarkersymbollayerv2.h"
#include "qgsrendercontext.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"

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


#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter


QgsDecorationGrid::QgsDecorationGrid( QObject* parent )
    : QgsDecorationItem( parent )
{
  setName( "Grid" );

  mLineSymbol = 0;
  mMarkerSymbol = 0;
  projectRead();

  connect( QgisApp::instance()->mapCanvas()->mapRenderer(), SIGNAL( mapUnitsChanged() ),
           this, SLOT( checkMapUnitsChanged() ) );
}

QgsDecorationGrid::~QgsDecorationGrid()
{
  if ( mLineSymbol )
    delete mLineSymbol;
  if ( mMarkerSymbol )
    delete mMarkerSymbol;
}

void QgsDecorationGrid::setLineSymbol( QgsLineSymbolV2* symbol )
{
  if ( mLineSymbol )
    delete mLineSymbol;
  mLineSymbol = symbol;
}

void QgsDecorationGrid::setMarkerSymbol( QgsMarkerSymbolV2* symbol )
{
  if ( mMarkerSymbol )
    delete mMarkerSymbol;
  mMarkerSymbol = symbol;
}

void QgsDecorationGrid::projectRead()
{
  QgsDecorationItem::projectRead();

  mEnabled = QgsProject::instance()->readBoolEntry( mNameConfig, "/Enabled", false );
  mMapUnits = ( QGis::UnitType ) QgsProject::instance()->readNumEntry( mNameConfig, "/MapUnits",
              QGis::UnknownUnit );
  mGridStyle = ( GridStyle ) QgsProject::instance()->readNumEntry( mNameConfig, "/Style",
               QgsDecorationGrid::Line );
  mGridIntervalX = QgsProject::instance()->readDoubleEntry( mNameConfig, "/IntervalX", 10 );
  mGridIntervalY = QgsProject::instance()->readDoubleEntry( mNameConfig, "/IntervalY", 10 );
  mGridOffsetX = QgsProject::instance()->readDoubleEntry( mNameConfig, "/OffsetX", 0 );
  mGridOffsetY = QgsProject::instance()->readDoubleEntry( mNameConfig, "/OffsetY", 0 );
  // mCrossLength = QgsProject::instance()->readDoubleEntry( mNameConfig, "/CrossLength", 3 );
  mShowGridAnnotation = QgsProject::instance()->readBoolEntry( mNameConfig, "/ShowAnnotation", false );
  // mGridAnnotationPosition = ( GridAnnotationPosition ) QgsProject::instance()->readNumEntry( mNameConfig,
  //                           "/AnnotationPosition", 0 );
  mGridAnnotationPosition = InsideMapFrame; // don't allow outside frame, doesn't make sense
  mGridAnnotationDirection = ( GridAnnotationDirection ) QgsProject::instance()->readNumEntry( mNameConfig,
                             "/AnnotationDirection", 0 );
  QString fontStr = QgsProject::instance()->readEntry( mNameConfig, "/AnnotationFont", "" );
  if ( fontStr != "" )
  {
    mGridAnnotationFont.fromString( fontStr );
  }
  else
  {
    mGridAnnotationFont = QFont();
    // TODO fix font scaling problem - put a slightly large font for now
    mGridAnnotationFont.setPointSize( 16 );
  }
  mAnnotationFrameDistance = QgsProject::instance()->readDoubleEntry( mNameConfig, "/AnnotationFrameDistance", 0 );
  mGridAnnotationPrecision = QgsProject::instance()->readNumEntry( mNameConfig, "/AnnotationPrecision", 0 );

  // read symbol info from xml
  QDomDocument doc;
  QDomElement elem;
  QString xml;

  if ( mLineSymbol )
    setLineSymbol( 0 );
  xml = QgsProject::instance()->readEntry( mNameConfig, "/LineSymbol" );
  if ( xml != "" )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mLineSymbol = dynamic_cast<QgsLineSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( elem ) );
  }
  if ( ! mLineSymbol )
    mLineSymbol = new QgsLineSymbolV2();

  if ( mMarkerSymbol )
    setMarkerSymbol( 0 );
  xml = QgsProject::instance()->readEntry( mNameConfig, "/MarkerSymbol" );
  if ( xml != "" )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mMarkerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( elem ) );
  }
  if ( ! mMarkerSymbol )
  {
    // set default symbol : cross with width=20
    QgsSymbolLayerV2List symbolList;
    symbolList << new QgsSimpleMarkerSymbolLayerV2( "cross", DEFAULT_SIMPLEMARKER_COLOR,
        DEFAULT_SIMPLEMARKER_BORDERCOLOR, 20, 0 );
    mMarkerSymbol = new QgsMarkerSymbolV2( symbolList );
    // mMarkerSymbol = new QgsMarkerSymbolV2();
  }
}

void QgsDecorationGrid::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, "/Enabled", mEnabled );
  QgsProject::instance()->writeEntry( mNameConfig, "/MapUnits", ( int ) mMapUnits );
  QgsProject::instance()->writeEntry( mNameConfig, "/Style", ( int ) mGridStyle );
  QgsProject::instance()->writeEntry( mNameConfig, "/IntervalX", mGridIntervalX );
  QgsProject::instance()->writeEntry( mNameConfig, "/IntervalY", mGridIntervalY );
  QgsProject::instance()->writeEntry( mNameConfig, "/OffsetX", mGridOffsetX );
  QgsProject::instance()->writeEntry( mNameConfig, "/OffsetX", mGridOffsetY );
  // QgsProject::instance()->writeEntry( mNameConfig, "/CrossLength", mCrossLength );
  // missing mGridPen, but should use styles anyway
  QgsProject::instance()->writeEntry( mNameConfig, "/ShowAnnotation", mShowGridAnnotation );
  // QgsProject::instance()->writeEntry( mNameConfig, "/AnnotationPosition", ( int ) mGridAnnotationPosition );
  QgsProject::instance()->writeEntry( mNameConfig, "/AnnotationDirection", ( int ) mGridAnnotationDirection );
  QgsProject::instance()->writeEntry( mNameConfig, "/AnnotationFont", mGridAnnotationFont.toString() );
  QgsProject::instance()->writeEntry( mNameConfig, "/AnnotationFrameDistance", mAnnotationFrameDistance );
  QgsProject::instance()->writeEntry( mNameConfig, "/AnnotationPrecision", mGridAnnotationPrecision );

  // write symbol info to xml
  QDomDocument doc;
  QDomElement elem;
  if ( mLineSymbol )
  {
    elem = QgsSymbolLayerV2Utils::saveSymbol( "line symbol", mLineSymbol, doc );
    doc.appendChild( elem );
    // FIXME this works, but XML will not be valid as < is replaced by &lt;
    QgsProject::instance()->writeEntry( mNameConfig, "/LineSymbol", doc.toString() );
  }
  if ( mMarkerSymbol )
  {
    doc.setContent( QString( "" ) );
    elem = QgsSymbolLayerV2Utils::saveSymbol( "marker symbol", mMarkerSymbol, doc );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mNameConfig, "/MarkerSymbol", doc.toString() );
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

void QgsDecorationGrid::render( QPainter * p )
{
  if ( ! mEnabled )
    return;

  // p->setPen( mGridPen );

  QList< QPair< double, QLineF > > verticalLines;
  yGridLines( verticalLines, p );
  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< double, QLineF > > horizontalLines;
  xGridLines( horizontalLines, p );
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  // QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  // p->setClipRect( thisPaintRect );

  //simpler approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsDecorationGrid::Line )
  {
    if ( ! mLineSymbol )
      return;

    QgsRenderContext context;
    context.setPainter( p );
    mLineSymbol->startRender( context, 0 );

    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      // p->drawLine( vIt->second );
      // need to convert QLineF to QPolygonF ...
      QVector<QPointF> poly;
      poly << vIt->second.p1() << vIt->second.p2();
      mLineSymbol->renderPolyline( QPolygonF( poly ), 0, context );
    }

    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      // p->drawLine( hIt->second );
      // need to convert QLineF to QPolygonF ...
      QVector<QPointF> poly;
      poly << hIt->second.p1() << hIt->second.p2();
      mLineSymbol->renderPolyline( QPolygonF( poly ), 0, context );
    }

    mLineSymbol->stopRender( context );
  }
#if 0
  else if ( mGridStyle == QgsDecorationGrid::Cross )
  {
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //start mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p1(), vIt->second.p2(), mCrossLength );
      p->drawLine( vIt->second.p1(), crossEnd1 );

      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength );
          crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength );
          p->drawLine( crossEnd1, crossEnd2 );
        }
      }
      //end mark
      QPointF crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p2(), vIt->second.p1(), mCrossLength );
      p->drawLine( vIt->second.p2(), crossEnd2 );
    }

    hIt = horizontalLines.constBegin();
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      //start mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p1(), hIt->second.p2(), mCrossLength );
      p->drawLine( hIt->second.p1(), crossEnd1 );

      vIt = verticalLines.constBegin();
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        if ( vIt->second.intersect( hIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p1(), mCrossLength );
          crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p2(), mCrossLength );
          p->drawLine( crossEnd1, crossEnd2 );
        }
      }
      //end mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p2(), hIt->second.p1(), mCrossLength );
      p->drawLine( hIt->second.p2(), crossEnd1 );
    }
  }
#endif
  else //marker
  {
    if ( ! mMarkerSymbol )
      return;

    QgsRenderContext context;
    context.setPainter( p );
    mMarkerSymbol->startRender( context, 0 );

    QPointF intersectionPoint;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          mMarkerSymbol->renderPoint( intersectionPoint, 0, context );
        }
      }
    }
    mMarkerSymbol->stopRender( context );
  }

  // p->setClipRect( thisPaintRect , Qt::NoClip );

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines );
  }
}

void QgsDecorationGrid::drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines )
{
  if ( !p )
  {
    return;
  }

  QString currentAnnotationString;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString = QString::number( it->first, 'f', mGridAnnotationPrecision );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }
}

void QgsDecorationGrid::drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString )
{
  Border frameBorder = borderForLineCoord( pos, p );
  double textWidth = textWidthMillimeters( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  if ( frameBorder == Left )
  {

    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else //Outside map frame
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos -= textWidth + mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }

  }
  else if ( frameBorder == Right )
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos -= textWidth + mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Vertical || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos += mAnnotationFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
  }
  else if ( frameBorder == Bottom )
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        ypos -= mAnnotationFrameDistance;
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= mAnnotationFrameDistance;
        rotation = 270;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        ypos += mAnnotationFrameDistance + textHeight;
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += textWidth + mAnnotationFrameDistance;
        rotation = 270;
      }
    }
  }
  else //Top
  {
    if ( mGridAnnotationPosition == InsideMapFrame )
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos += textHeight + mAnnotationFrameDistance;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += textWidth + mAnnotationFrameDistance;
        rotation = 270;
      }
    }
    else //OutsideMapFrame
    {
      if ( mGridAnnotationDirection == Horizontal || mGridAnnotationDirection == BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= mAnnotationFrameDistance;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= mAnnotationFrameDistance;
        rotation = 270;
      }
    }
  }

  drawAnnotation( p, QPointF( xpos, ypos ), rotation, annotationString );
}

void QgsDecorationGrid::drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText )
{
  p->save();
  p->translate( pos );
  p->rotate( rotation );
  p->setPen( QColor( 0, 0, 0 ) );
  drawText( p, 0, 0, annotationText, mGridAnnotationFont );
  p->restore();
}

QPolygonF canvasExtent()
{
  QPolygonF poly;
  QgsRectangle extent = QgisApp::instance()->mapCanvas()->extent();
  poly << QPointF( extent.xMinimum(), extent.yMaximum() );
  poly << QPointF( extent.xMaximum(), extent.yMaximum() );
  poly << QPointF( extent.xMaximum(), extent.yMinimum() );
  poly << QPointF( extent.xMinimum(), extent.yMinimum() );
  return poly;
}

int QgsDecorationGrid::xGridLines( QList< QPair< double, QLineF > >& lines, QPainter * p ) const
{
  lines.clear();
  if ( mGridIntervalY <= 0.0 )
  {
    return 1;
  }

  // QPolygonF mapPolygon = transformedMapPolygon();
  QPolygonF mapPolygon = canvasExtent();
  QRectF mapBoundingRect = mapPolygon.boundingRect();

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.top() - mGridOffsetY ) / mGridIntervalY + roundCorrection ) * mGridIntervalY + mGridOffsetY;

  // if ( doubleNear( mRotation, 0.0 ) )
  // {
  //no rotation. Do it 'the easy way'

  double yCanvasCoord;

  while ( currentLevel <= mapBoundingRect.bottom() )
  {
    yCanvasCoord = p->device()->height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
    lines.push_back( qMakePair( currentLevel, QLineF( 0, yCanvasCoord, p->device()->width(), yCanvasCoord ) ) );
    currentLevel += mGridIntervalY;
  }
  // }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.bottom() )
  {
    intersectionList.clear();
    QLineF gridLine( mapBoundingRect.left(), currentLevel, mapBoundingRect.right(), currentLevel );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
      {
        intersectionList.push_back( intersectionPoint );
        if ( intersectionList.size() >= 2 )
        {
          break; //we already have two intersections, skip further tests
        }
      }
    }

    if ( intersectionList.size() >= 2 )
    {
      lines.push_back( qMakePair( currentLevel, QLineF( intersectionList.at( 0 ), intersectionList.at( 1 ) ) ) );
    }
    currentLevel += mGridIntervalY;
  }


  return 0;
}

int QgsDecorationGrid::yGridLines( QList< QPair< double, QLineF > >& lines, QPainter * p ) const
{
  lines.clear();
  if ( mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  // QPolygonF mapPolygon = transformedMapPolygon();
  QPolygonF mapPolygon = canvasExtent();
  QRectF mapBoundingRect = mapPolygon.boundingRect();

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.left() - mGridOffsetX ) / mGridIntervalX + roundCorrection ) * mGridIntervalX + mGridOffsetX;

  // if ( doubleNear( mRotation, 0.0 ) )
  // {
  //   //no rotation. Do it 'the easy way'
  double xCanvasCoord;

  while ( currentLevel <= mapBoundingRect.right() )
  {
    xCanvasCoord = p->device()->width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();
    lines.push_back( qMakePair( currentLevel, QLineF( xCanvasCoord, 0, xCanvasCoord, p->device()->height() ) ) );
    currentLevel += mGridIntervalX;
  }
  // }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.right() )
  {
    intersectionList.clear();
    QLineF gridLine( currentLevel, mapBoundingRect.bottom(), currentLevel, mapBoundingRect.top() );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
      {
        intersectionList.push_back( intersectionPoint );
        if ( intersectionList.size() >= 2 )
        {
          break; //we already have two intersections, skip further tests
        }
      }
    }

    if ( intersectionList.size() >= 2 )
    {
      lines.push_back( qMakePair( currentLevel, QLineF( intersectionList.at( 0 ), intersectionList.at( 1 ) ) ) );
    }
    currentLevel += mGridIntervalX;
  }

  return 0;
}

QgsDecorationGrid::Border QgsDecorationGrid::borderForLineCoord( const QPointF& point, QPainter* p ) const
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

void QgsDecorationGrid::drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  p->setPen( QColor( 0, 0, 0 ) ); //draw text always in black
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
  p->restore();
}

void QgsDecorationGrid::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignment | valignment | Qt::TextWordWrap, text );
  p->restore();
}

double QgsDecorationGrid::textWidthMillimeters( const QFont& font, const QString& text ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetrics fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsDecorationGrid::fontHeightCharacterMM( const QFont& font, const QChar& c ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( c ).height() / FONT_WORKAROUND_SCALE );
}

double QgsDecorationGrid::fontAscentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsDecorationGrid::pixelFontSize( double pointSize ) const
{
  // return ( pointSize * 0.3527 );
  // TODO fix font scaling problem - this seems to help, but text seems still a bit too small (about 5/6)
  return pointSize;
}

QFont QgsDecorationGrid::scaledFontPixelSize( const QFont& font ) const
{
  QFont scaledFont = font;
  double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

void QgsDecorationGrid::checkMapUnitsChanged()
{
  // disable grid if units changed and grid is enabled, and update the canvas
  // this is to avoid problems when CRS changes to/from geographic and projected
  // a better solution would be to change the grid interval, but this is a little tricky
  // note: we could be less picky (e.g. from degrees to DMS)
  QGis::UnitType mapUnits = QgisApp::instance()->mapCanvas()->mapRenderer()->mapUnits();
  if ( mEnabled && ( mMapUnits != mapUnits ) )
  {
    mEnabled = false;
    mMapUnits = QGis::UnknownUnit; // make sure isDirty() returns true
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
  if ( mMapUnits == QGis::UnknownUnit ||
       mMapUnits != QgisApp::instance()->mapCanvas()->mapRenderer()->mapUnits() ||
       mGridIntervalX == 0 || mGridIntervalY == 0 )
    return true;
  return false;
}

void QgsDecorationGrid::setDirty( bool dirty )
{
  if ( dirty )
  {
    mMapUnits = QGis::UnknownUnit;
  }
  else
  {
    mMapUnits = QgisApp::instance()->mapCanvas()->mapRenderer()->mapUnits();
  }
}

bool QgsDecorationGrid::getIntervalFromExtent( double* values, bool useXAxis )
{
  // get default interval from current extents
  // calculate a default interval that is approx (extent width)/5 , adjusted so that it is a rounded number
  // e.g. 12.7 -> 10  66556 -> 70000
  double interval = 0;
  QgsRectangle extent = QgisApp::instance()->mapCanvas()->extent();
  if ( useXAxis )
    interval = ( extent.xMaximum() - extent.xMinimum() ) / 5;
  else
    interval = ( extent.yMaximum() - extent.yMinimum() ) / 5;
  QgsDebugMsg( QString( "interval: %1" ).arg( interval ) );
  if ( interval != 0 )
  {
    double interval2 = 0;
    int factor =  pow( 10, floor( log10( interval ) ) );
    if ( factor != 0 )
    {
      interval2 = qRound( interval / factor ) * factor;
      QgsDebugMsg( QString( "interval2: %1" ).arg( interval2 ) );
      if ( interval2 != 0 )
        interval = interval2;
    }
  }
  values[0] = values[1] = interval;
  values[2] = values[3] = 0;
  return true;
}

bool QgsDecorationGrid::getIntervalFromCurrentLayer( double* values )
{
  // get current layer and make sure it is a raster layer and CRSs match
  QgsMapLayer* layer = QgisApp::instance()->mapCanvas()->currentLayer();
  if ( ! layer )
  {
    QMessageBox::warning( 0, tr( "Error" ), tr( "No active layer" ) );
    return false;
  }
  if ( layer->type() != QgsMapLayer::RasterLayer )
  {
    QMessageBox::warning( 0, tr( "Error" ), tr( "Please select a raster layer" ) );
    return false;
  }
  QgsRasterLayer* rlayer = dynamic_cast<QgsRasterLayer*>( layer );
  if ( !rlayer )
  {
    QMessageBox::warning( 0, tr( "Error" ), tr( "Invalid raster layer" ) );
    return false;
  }
  const QgsCoordinateReferenceSystem& layerCRS = layer->crs();
  const QgsCoordinateReferenceSystem& mapCRS =
    QgisApp::instance()->mapCanvas()->mapRenderer()->destinationCrs();
  // is this the best way to compare CRS? should we also make sure map has OTF enabled?
  // TODO calculate transformed values if necessary
  if ( layerCRS != mapCRS )
  {
    QMessageBox::warning( 0, tr( "Error" ), tr( "Layer CRS must be equal to project CRS" ) );
    return false;
  }

  // calculate interval
  // TODO add a function in QgsRasterLayer to get x/y resolution from provider,
  // because this might not be 100% accurate
  QgsRectangle extent = rlayer->extent();
  values[0] = fabs( extent.xMaximum() - extent.xMinimum() ) / rlayer->width();
  values[1] = fabs( extent.yMaximum() - extent.yMinimum() ) / rlayer->height();

  // calculate offset - when using very high resolution rasters in geographic CRS
  // there seems to be a small shift, but this may be due to rendering issues and depends on zoom
  double ratio = extent.xMinimum() / values[0];
  values[2] = ( ratio - floor( ratio ) ) * values[0];
  ratio = extent.yMinimum() / values[1];
  values[3] = ( ratio - floor( ratio ) ) * values[1];

  QgsDebugMsg( QString( "xmax: %1 xmin: %2 width: %3 xInterval: %4 xOffset: %5" ).arg(
                 extent.xMaximum() ).arg( extent.xMinimum() ).arg( rlayer->width() ).arg( values[0] ).arg( values[2] ) );
  QgsDebugMsg( QString( "ymax: %1 ymin: %2 height: %3 yInterval: %4 yOffset: %5" ).arg(
                 extent.yMaximum() ).arg( extent.yMinimum() ).arg( rlayer->height() ).arg( values[1] ).arg( values[3] ) );

  return true;
}
