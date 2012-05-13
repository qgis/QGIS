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
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance

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

//non qt includes
#include <cmath>


#ifdef _MSC_VER
#define round(x)  ((x) >= 0 ? floor((x)+0.5) : floor((x)-0.5))
#endif

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter


QgsDecorationGrid::QgsDecorationGrid( QObject* parent )
    : QObject( parent )
{
  projectRead();
}

QgsDecorationGrid::~QgsDecorationGrid()
{

}

void QgsDecorationGrid::update()
{
  QgisApp::instance()->mapCanvas()->refresh();
}

void QgsDecorationGrid::projectRead()
{
  mGridEnabled = QgsProject::instance()->readBoolEntry( "Grid", "/Enabled", false );
  mGridStyle = ( GridStyle ) QgsProject::instance()->readNumEntry( "Grid", "/Style", 0 );
  mGridIntervalX = QgsProject::instance()->readDoubleEntry( "Grid", "/IntervalX", 0 );
  mGridIntervalY = QgsProject::instance()->readDoubleEntry( "Grid", "/IntervalY", 0 );
  mGridOffsetX = QgsProject::instance()->readDoubleEntry( "Grid", "/OffsetX", 0 );
  mGridOffsetY = QgsProject::instance()->readDoubleEntry( "Grid", "/OffsetY", 0 );
  mCrossLength = QgsProject::instance()->readDoubleEntry( "Grid", "/CrossLength", 0 );
  // missing mGridPen, mGridAnnotationFont
  mGridAnnotationPrecision = ( GridAnnotationPosition ) QgsProject::instance()->readNumEntry( "Grid", "/AnnotationPrecision", 3 );
  mShowGridAnnotation = QgsProject::instance()->readBoolEntry( "Grid", "/ShowAnnotation", false );
  // mGridAnnotationPosition = (GridAnnotationPosition) QgsProject::instance()->readNumEntry( "Grid", "/AnnotationPosition", 0 );
  mAnnotationFrameDistance = QgsProject::instance()->readDoubleEntry( "Grid", "/AnnotationFrameDistance", 0 );
  mGridAnnotationDirection = ( GridAnnotationDirection ) QgsProject::instance()->readNumEntry( "Grid", "/AnnotationDirection", 0 );
}

void QgsDecorationGrid::saveToProject()
{
  QgsProject::instance()->writeEntry( "Grid", "/Enabled", mGridEnabled );
  QgsProject::instance()->writeEntry( "Grid", "/IntervalX", mGridIntervalX );
  QgsProject::instance()->writeEntry( "Grid", "/IntervalY", mGridIntervalY );
  QgsProject::instance()->writeEntry( "Grid", "/OffsetX", mGridOffsetX );
  QgsProject::instance()->writeEntry( "Grid", "/OffsetX", mGridOffsetY );
  QgsProject::instance()->writeEntry( "Grid", "/CrossLength", mCrossLength );
  // missing mGridPen, mGridAnnotationFont
  QgsProject::instance()->writeEntry( "Grid", "/AnnotationPrecision", mGridAnnotationPrecision );
  QgsProject::instance()->writeEntry( "Grid", "/ShowAnnotation", mShowGridAnnotation );
  // QgsProject::instance()->writeEntry( "Grid", "/AnnotationPosition", mGridAnnotationPosition );
  QgsProject::instance()->writeEntry( "Grid", "/AnnotationFrameDistance", mAnnotationFrameDistance );
  QgsProject::instance()->writeEntry( "Grid", "/AnnotationDirection", mGridAnnotationDirection );
}


void QgsDecorationGrid::run()
{
  QgsDecorationGridDialog dlg( *this );

  if ( dlg.exec() )
  {
    saveToProject();
    QgisApp::instance()->mapCanvas()->refresh();
  }
}



void QgsDecorationGrid::renderGrid( QPainter * p )
{
  if ( ! mGridEnabled )
    return;

  p->setPen( mGridPen );

  QList< QPair< double, QLineF > > verticalLines;
  yGridLines( verticalLines, p );
  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< double, QLineF > > horizontalLines;
  xGridLines( horizontalLines, p );
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  // QRectF thisPaintRect = QRectF( 0, 0, QGraphicsRectItem::rect().width(), QGraphicsRectItem::rect().height() );
  // p->setClipRect( thisPaintRect );

  //simpler approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsDecorationGrid::Solid )
  {
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      p->drawLine( vIt->second );
    }

    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      p->drawLine( hIt->second );
    }
  }
  else //cross
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

void QgsDecorationGrid::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignement, Qt::AlignmentFlag valignment ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignement | valignment | Qt::TextWordWrap, text );
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
  return ( pointSize * 0.3527 );
}

QFont QgsDecorationGrid::scaledFontPixelSize( const QFont& font ) const
{
  QFont scaledFont = font;
  double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

