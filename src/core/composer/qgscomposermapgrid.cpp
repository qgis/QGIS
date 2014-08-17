/***************************************************************************
                         qgscomposermapgrid.cpp
                         ----------------------
    begin                : December 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermapgrid.h"
#include "qgscomposerutils.h"
#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgsmaprenderer.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include <QPainter>
#include <QPen>

#define MAX_GRID_LINES 1000 //maximum number of horizontal or vertical grid lines to draw

QgsComposerMapGrid::QgsComposerMapGrid( const QString& name, QgsComposerMap* map ):
    mComposerMap( map ),
    mName( name ),
    mUuid( QUuid::createUuid().toString() ),
    mGridEnabled( true ),
    mGridStyle( QgsComposerMap::Solid ),
    mGridIntervalX( 0.0 ),
    mGridIntervalY( 0.0 ),
    mGridOffsetX( 0.0 ),
    mGridOffsetY( 0.0 ),
    mGridAnnotationFontColor( Qt::black ),
    mGridAnnotationPrecision( 3 ),
    mShowGridAnnotation( false ),
    mLeftGridAnnotationPosition( QgsComposerMap::OutsideMapFrame ),
    mRightGridAnnotationPosition( QgsComposerMap::OutsideMapFrame ),
    mTopGridAnnotationPosition( QgsComposerMap::OutsideMapFrame ),
    mBottomGridAnnotationPosition( QgsComposerMap::OutsideMapFrame ),
    mAnnotationFrameDistance( 1.0 ),
    mLeftGridAnnotationDirection( QgsComposerMap::Horizontal ),
    mRightGridAnnotationDirection( QgsComposerMap::Horizontal ),
    mTopGridAnnotationDirection( QgsComposerMap::Horizontal ),
    mBottomGridAnnotationDirection( QgsComposerMap::Horizontal ),
    mGridAnnotationFormat( QgsComposerMap::Decimal ),
    mGridFrameStyle( QgsComposerMap::NoGridFrame ),
    mGridFrameWidth( 2.0 ),
    mGridFramePenThickness( 0.5 ),
    mGridFramePenColor( QColor( 0, 0, 0 ) ),
    mGridFrameFillColor1( Qt::white ),
    mGridFrameFillColor2( Qt::black ),
    mCrossLength( 3 ),
    mGridLineSymbol( 0 ),
    mGridMarkerSymbol( 0 ),
    mGridUnit( MapUnit ),
    mBlendMode( QPainter::CompositionMode_SourceOver )
{
  //get default composer font from settings
  QSettings settings;
  QString defaultFontString = settings.value( "/Composer/defaultFont" ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mGridAnnotationFont.setFamily( defaultFontString );
  }

  createDefaultGridLineSymbol();
  createDefaultGridMarkerSymbol();
}

QgsComposerMapGrid::QgsComposerMapGrid(): mComposerMap( 0 )
{
}

QgsComposerMapGrid::~QgsComposerMapGrid()
{
  delete mGridLineSymbol;
  delete mGridMarkerSymbol;
}

void QgsComposerMapGrid::createDefaultGridLineSymbol()
{
  delete mGridLineSymbol;
  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "width", "0.3" );
  properties.insert( "capstyle", "flat" );
  mGridLineSymbol = QgsLineSymbolV2::createSimple( properties );
}

void QgsComposerMapGrid::createDefaultGridMarkerSymbol()
{
  delete mGridMarkerSymbol;
  QgsStringMap properties;
  properties.insert( "name", "circle" );
  properties.insert( "size", "2.0" );
  properties.insert( "color", "0,0,0,255" );
  mGridMarkerSymbol = QgsMarkerSymbolV2::createSimple( properties );
}

void QgsComposerMapGrid::setComposerMap( QgsComposerMap* map )
{
  mComposerMap = map;
}

void QgsComposerMapGrid::setGridPenWidth( double w )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setWidth( w );
  }
}

void QgsComposerMapGrid::setGridPenColor( const QColor& c )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setColor( c );
  }
}

void QgsComposerMapGrid::setGridPen( const QPen& p )
{
  setGridPenWidth( p.widthF() );
  setGridPenColor( p.color() );
}

QPen QgsComposerMapGrid::gridPen() const
{
  QPen p;
  if ( mGridLineSymbol )
  {
    p.setWidthF( mGridLineSymbol->width() );
    p.setColor( mGridLineSymbol->color() );
    p.setCapStyle( Qt::FlatCap );
  }
  return p;
}

bool QgsComposerMapGrid::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement mapGridElem = doc.createElement( "ComposerMapGrid" );
  mapGridElem.setAttribute( "name", mName );
  mapGridElem.setAttribute( "uuid", mUuid );
  mapGridElem.setAttribute( "show", mGridEnabled );
  mapGridElem.setAttribute( "gridStyle", mGridStyle );
  mapGridElem.setAttribute( "intervalX", qgsDoubleToString( mGridIntervalX ) );
  mapGridElem.setAttribute( "intervalY", qgsDoubleToString( mGridIntervalY ) );
  mapGridElem.setAttribute( "offsetX", qgsDoubleToString( mGridOffsetX ) );
  mapGridElem.setAttribute( "offsetY", qgsDoubleToString( mGridOffsetY ) );
  mapGridElem.setAttribute( "crossLength",  qgsDoubleToString( mCrossLength ) );

  QDomElement lineStyleElem = doc.createElement( "lineStyle" );
  QDomElement gridLineStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mGridLineSymbol, doc );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( "markerStyle" );
  QDomElement gridMarkerStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mGridMarkerSymbol, doc );
  markerStyleElem.appendChild( gridMarkerStyleElem );
  mapGridElem.appendChild( markerStyleElem );

  mapGridElem.setAttribute( "gridFrameStyle", mGridFrameStyle );
  mapGridElem.setAttribute( "gridFrameWidth", qgsDoubleToString( mGridFrameWidth ) );
  mapGridElem.setAttribute( "gridFramePenThickness", qgsDoubleToString( mGridFramePenThickness ) );
  mapGridElem.setAttribute( "gridFramePenColor", QgsSymbolLayerV2Utils::encodeColor( mGridFramePenColor ) );
  mapGridElem.setAttribute( "frameFillColor1", QgsSymbolLayerV2Utils::encodeColor( mGridFrameFillColor1 ) );
  mapGridElem.setAttribute( "frameFillColor2", QgsSymbolLayerV2Utils::encodeColor( mGridFrameFillColor2 ) );
  if ( mCRS.isValid() )
  {
    mCRS.writeXML( mapGridElem, doc );
  }

  mapGridElem.setAttribute( "annotationFormat", mGridAnnotationFormat );
  mapGridElem.setAttribute( "showAnnotation", mShowGridAnnotation );
  mapGridElem.setAttribute( "leftAnnotationPosition", mLeftGridAnnotationPosition );
  mapGridElem.setAttribute( "rightAnnotationPosition", mRightGridAnnotationPosition );
  mapGridElem.setAttribute( "topAnnotationPosition", mTopGridAnnotationPosition );
  mapGridElem.setAttribute( "bottomAnnotationPosition", mBottomGridAnnotationPosition );
  mapGridElem.setAttribute( "leftAnnotationDirection", mLeftGridAnnotationDirection );
  mapGridElem.setAttribute( "rightAnnotationDirection", mRightGridAnnotationDirection );
  mapGridElem.setAttribute( "topAnnotationDirection", mTopGridAnnotationDirection );
  mapGridElem.setAttribute( "bottomAnnotationDirection", mBottomGridAnnotationDirection );
  mapGridElem.setAttribute( "frameAnnotationDistance",  QString::number( mAnnotationFrameDistance ) );
  mapGridElem.setAttribute( "annotationFont", mGridAnnotationFont.toString() );
  mapGridElem.setAttribute( "annotationFontColor", QgsSymbolLayerV2Utils::encodeColor( mGridAnnotationFontColor ) );
  mapGridElem.setAttribute( "annotationPrecision", mGridAnnotationPrecision );
  mapGridElem.setAttribute( "unit", mGridUnit );
  mapGridElem.setAttribute( "blendMode", mBlendMode );

  elem.appendChild( mapGridElem );
  return true;
}

bool QgsComposerMapGrid::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  mName = itemElem.attribute( "name" );
  mUuid = itemElem.attribute( "uuid" );

  //grid
  mGridEnabled = ( itemElem.attribute( "show", "0" ) != "0" );
  mGridStyle = QgsComposerMap::GridStyle( itemElem.attribute( "gridStyle", "0" ).toInt() );
  mGridIntervalX = itemElem.attribute( "intervalX", "0" ).toDouble();
  mGridIntervalY = itemElem.attribute( "intervalY", "0" ).toDouble();
  mGridOffsetX = itemElem.attribute( "offsetX", "0" ).toDouble();
  mGridOffsetY = itemElem.attribute( "offsetY", "0" ).toDouble();
  mCrossLength = itemElem.attribute( "crossLength", "3" ).toDouble();
  mGridFrameStyle = ( QgsComposerMap::GridFrameStyle )itemElem.attribute( "gridFrameStyle", "0" ).toInt();
  mGridFrameWidth = itemElem.attribute( "gridFrameWidth", "2.0" ).toDouble();
  mGridFramePenThickness = itemElem.attribute( "gridFramePenThickness", "0.5" ).toDouble();
  mGridFramePenColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "gridFramePenColor", "0,0,0" ) );
  mGridFrameFillColor1 = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "frameFillColor1", "255,255,255,255" ) );
  mGridFrameFillColor2 = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "frameFillColor2", "0,0,0,255" ) );

  QDomElement lineStyleElem = itemElem.firstChildElement( "lineStyle" );
  if ( !lineStyleElem.isNull() )
  {
    QDomElement symbolElem = lineStyleElem.firstChildElement( "symbol" );
    if ( !symbolElem.isNull( ) )
    {
      delete mGridLineSymbol;
      mGridLineSymbol = dynamic_cast<QgsLineSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( symbolElem ) );
    }
  }
  else
  {
    //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
    mGridLineSymbol = QgsLineSymbolV2::createSimple( QgsStringMap() );
    mGridLineSymbol->setWidth( itemElem.attribute( "penWidth", "0" ).toDouble() );
    mGridLineSymbol->setColor( QColor( itemElem.attribute( "penColorRed", "0" ).toInt(),
                                       itemElem.attribute( "penColorGreen", "0" ).toInt(),
                                       itemElem.attribute( "penColorBlue", "0" ).toInt() ) );
  }

  QDomElement markerStyleElem = itemElem.firstChildElement( "markerStyle" );
  if ( !markerStyleElem.isNull() )
  {
    QDomElement symbolElem = markerStyleElem.firstChildElement( "symbol" );
    if ( !symbolElem.isNull( ) )
    {
      delete mGridMarkerSymbol;
      mGridMarkerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( symbolElem ) );
    }
  }


  QDomElement crsElem = itemElem.firstChildElement( "spatialrefsys" );
  if ( !crsElem.isNull() )
  {
    mCRS.readXML( const_cast<QDomElement&>( itemElem ) ); //better would be to change argument in QgsCoordinateReferenceSystem::readXML to const
  }
  else
  {
    mCRS = QgsCoordinateReferenceSystem();
  }
  mBlendMode = ( QPainter::CompositionMode )( itemElem.attribute( "blendMode", "0" ).toUInt() );

  //annotation
  mShowGridAnnotation = ( itemElem.attribute( "showAnnotation", "0" ) != "0" );
  mGridAnnotationFormat = QgsComposerMap::GridAnnotationFormat( itemElem.attribute( "annotationFormat", "0" ).toInt() );
  mLeftGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( itemElem.attribute( "leftAnnotationPosition", "0" ).toInt() );
  mRightGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( itemElem.attribute( "rightAnnotationPosition", "0" ).toInt() );
  mTopGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( itemElem.attribute( "topAnnotationPosition", "0" ).toInt() );
  mBottomGridAnnotationPosition = QgsComposerMap::GridAnnotationPosition( itemElem.attribute( "bottomAnnotationPosition", "0" ).toInt() );
  mLeftGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( itemElem.attribute( "leftAnnotationDirection", "0" ).toInt() );
  mRightGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( itemElem.attribute( "rightAnnotationDirection", "0" ).toInt() );
  mTopGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( itemElem.attribute( "topAnnotationDirection", "0" ).toInt() );
  mBottomGridAnnotationDirection = QgsComposerMap::GridAnnotationDirection( itemElem.attribute( "bottomAnnotationDirection", "0" ).toInt() );
  mAnnotationFrameDistance = itemElem.attribute( "frameAnnotationDistance", "0" ).toDouble();
  mGridAnnotationFont.fromString( itemElem.attribute( "annotationFont", "" ) );
  mGridAnnotationFontColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "annotationFontColor", "0,0,0,255" ) );
  mGridAnnotationPrecision = itemElem.attribute( "annotationPrecision", "3" ).toInt();
  int gridUnitInt =  itemElem.attribute( "unit", QString::number( MapUnit ) ).toInt();
  mGridUnit = ( gridUnitInt <= ( int )CM ) ? ( GridUnit )gridUnitInt : MapUnit;
  return true;
}

QPolygonF QgsComposerMapGrid::scalePolygon( const QPolygonF &polygon, const double scale ) const
{
  QTransform t = QTransform::fromScale( scale, scale );
  return t.map( polygon );
}

void QgsComposerMapGrid::drawGridCRSTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines ) const
{
  if ( !mComposerMap || !mGridEnabled )
  {
    return;
  }

  QgsRectangle crsBoundingRect;
  QgsCoordinateTransform inverseTr;
  if ( crsGridParams( crsBoundingRect, inverseTr ) != 0 )
  {
    return;
  }

  //x grid lines
  QList< QPair< double, QPolygonF > > xGridLines;
  xGridLinesCRSTransform( crsBoundingRect, inverseTr, xGridLines );
  QList< QPair< double, QPolygonF > >::const_iterator xGridIt = xGridLines.constBegin();
  for ( ; xGridIt != xGridLines.constEnd(); ++xGridIt )
  {
    drawGridLine( scalePolygon( xGridIt->second, dotsPerMM ), context );
  }

  //y grid lines
  QList< QPair< double, QPolygonF > > yGridLines;
  yGridLinesCRSTransform( crsBoundingRect, inverseTr, yGridLines );
  QList< QPair< double, QPolygonF > >::const_iterator yGridIt = yGridLines.constBegin();
  for ( ; yGridIt != yGridLines.constEnd(); ++yGridIt )
  {
    drawGridLine( scalePolygon( yGridIt->second, dotsPerMM ), context );
  }

  //convert QPolygonF to QLineF to draw grid frames and annotations
  QList< QPair< double, QPolygonF > >::const_iterator yGridLineIt = yGridLines.constBegin();
  for ( ; yGridLineIt != yGridLines.constEnd(); ++yGridLineIt )
  {
    horizontalLines.push_back( qMakePair( yGridLineIt->first, QLineF( yGridLineIt->second.first(), yGridLineIt->second.last() ) ) );
  }
  QList< QPair< double, QPolygonF > >::const_iterator xGridLineIt = xGridLines.constBegin();
  for ( ; xGridLineIt != xGridLines.constEnd(); ++xGridLineIt )
  {
    verticalLines.push_back( qMakePair( xGridLineIt->first, QLineF( xGridLineIt->second.first(), xGridLineIt->second.last() ) ) );
  }

}

void QgsComposerMapGrid::drawGrid( QPainter* p ) const
{
  if ( !mComposerMap || !mGridEnabled )
  {
    return;
  }
  QPaintDevice* thePaintDevice = p->device();
  if ( !thePaintDevice )
  {
    return;
  }

  p->save();
  p->setCompositionMode( mBlendMode );
  p->setRenderHint( QPainter::Antialiasing );

  QRectF thisPaintRect = QRectF( 0, 0, mComposerMap->rect().width(), mComposerMap->rect().height() );
  p->setClipRect( thisPaintRect );

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = thePaintDevice->logicalDpiX() / 25.4;
  p->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsMapSettings ms = mComposerMap->composition()->mapSettings();
  //context units should be in dots
  ms.setOutputSize( QSizeF( mComposerMap->rect().width() * dotsPerMM, mComposerMap->rect().height() * dotsPerMM ).toSize() );
  ms.setExtent( *mComposerMap->currentMapExtent() );
  ms.setOutputDpi( p->device()->logicalDpiX() );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setForceVectorOutput( true );
  context.setPainter( p );

  QList< QPair< double, QLineF > > verticalLines;
  QList< QPair< double, QLineF > > horizontalLines;

  //is grid in a different crs than map?
  if ( mGridUnit == MapUnit && mCRS.isValid() && mCRS != ms.destinationCrs() )
  {
    drawGridCRSTransform( context, dotsPerMM, horizontalLines, verticalLines );
  }
  else
  {
    drawGridNoTransform( context, dotsPerMM, horizontalLines, verticalLines );
  }

  p->restore();
  p->setClipping( false );

  if ( mGridFrameStyle != QgsComposerMap::NoGridFrame )
  {
    drawGridFrame( p, horizontalLines, verticalLines );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines );
  }
}

void QgsComposerMapGrid::drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines ) const
{
  //get line positions
  yGridLines( verticalLines );
  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  xGridLines( horizontalLines );
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  //simple approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsComposerMap::Solid )
  {
    //we need to scale line coordinates to dots, rather than mm, since the painter has already been scaled to dots
    //this is done by multiplying each line coordinate by dotsPerMM
    QLineF line;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      line = QLineF( vIt->second.p1() * dotsPerMM, vIt->second.p2() * dotsPerMM );
      drawGridLine( line, context );
    }

    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      line = QLineF( hIt->second.p1() * dotsPerMM, hIt->second.p2() * dotsPerMM );
      drawGridLine( line, context );
    }
  }
  else //cross or markers
  {
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //start mark
      if ( mGridStyle == QgsComposerMap::Cross )
      {
        crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p1(), vIt->second.p2(), mCrossLength );
        drawGridLine( QLineF( vIt->second.p1() * dotsPerMM, crossEnd1 * dotsPerMM ), context );
      }

      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          if ( mGridStyle == QgsComposerMap::Cross )
          {
            //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
            crossEnd1 = (( intersectionPoint - vIt->second.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength ) : intersectionPoint;
            crossEnd2 = (( intersectionPoint - vIt->second.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength ) : intersectionPoint;
            //draw line using coordinates scaled to dots
            drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
          }
          else if ( mGridStyle == QgsComposerMap::Markers )
          {
            drawGridMarker( intersectionPoint * dotsPerMM , context );
          }
        }
      }
      //end mark
      if ( mGridStyle == QgsComposerMap::Cross )
      {
        QPointF crossEnd2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( vIt->second.p2(), vIt->second.p1(), mCrossLength );
        drawGridLine( QLineF( vIt->second.p2() * dotsPerMM, crossEnd2 * dotsPerMM ), context );
      }
    }
    if ( mGridStyle == QgsComposerMap::Markers )
    {
      //markers mode, so we have no need to process horizontal lines (we've already
      //drawn markers on the intersections between horizontal and vertical lines)
      return;
    }

    hIt = horizontalLines.constBegin();
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
      //start mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p1(), hIt->second.p2(), mCrossLength );
      drawGridLine( QLineF( hIt->second.p1() * dotsPerMM, crossEnd1 * dotsPerMM ), context );

      vIt = verticalLines.constBegin();
      for ( ; vIt != verticalLines.constEnd(); ++vIt )
      {
        if ( vIt->second.intersect( hIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
          crossEnd1 = (( intersectionPoint - hIt->second.p1() ).manhattanLength() > 0.01 ) ?
                      QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p1(), mCrossLength ) : intersectionPoint;
          crossEnd2 = (( intersectionPoint - hIt->second.p2() ).manhattanLength() > 0.01 )  ?
                      QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, hIt->second.p2(), mCrossLength ) : intersectionPoint;
          //draw line using coordinates scaled to dots
          drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
        }
      }
      //end mark
      crossEnd1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( hIt->second.p2(), hIt->second.p1(), mCrossLength );
      drawGridLine( QLineF( hIt->second.p2() * dotsPerMM, crossEnd1 * dotsPerMM ), context );
    }
  }
}

void QgsComposerMapGrid::drawGridFrame( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const
{
  p->save();
  p->setRenderHint( QPainter::Antialiasing );

  //Sort the coordinate positions for each side
  QMap< double, double > leftGridFrame;
  QMap< double, double > rightGridFrame;
  QMap< double, double > topGridFrame;
  QMap< double, double > bottomGridFrame;

  sortGridLinesOnBorders( hLines, vLines, leftGridFrame, rightGridFrame, topGridFrame, bottomGridFrame );

  drawGridFrameBorder( p, leftGridFrame, QgsComposerMap::Left );
  drawGridFrameBorder( p, rightGridFrame, QgsComposerMap::Right );
  drawGridFrameBorder( p, topGridFrame, QgsComposerMap::Top );
  drawGridFrameBorder( p, bottomGridFrame, QgsComposerMap::Bottom );

  p->restore();
}

void QgsComposerMapGrid::drawGridLine( const QLineF& line, QgsRenderContext& context ) const
{
  QPolygonF poly;
  poly << line.p1() << line.p2();
  drawGridLine( poly, context );
}

void QgsComposerMapGrid::drawGridLine( const QPolygonF& line, QgsRenderContext& context ) const
{
  if ( !mComposerMap || !mComposerMap->composition() || !mGridLineSymbol )
  {
    return;
  }

  mGridLineSymbol->startRender( context );
  mGridLineSymbol->renderPolyline( line, 0, context );
  mGridLineSymbol->stopRender( context );
}

void QgsComposerMapGrid::drawGridMarker( const QPointF& point, QgsRenderContext& context ) const
{
  if ( !mComposerMap || !mComposerMap->composition() || !mGridMarkerSymbol )
  {
    return;
  }

  mGridMarkerSymbol->startRender( context );
  mGridMarkerSymbol->renderPoint( point, 0, context );
  mGridMarkerSymbol->stopRender( context );
}

void QgsComposerMapGrid::drawGridFrameBorder( QPainter* p, const QMap< double, double >& borderPos, QgsComposerMap::Border border ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  double currentCoord = - mGridFrameWidth;
  bool color1 = true;
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  QMap< double, double > pos = borderPos;
  pos.insert( 0, 0 );
  if ( border == QgsComposerMap::Left || border == QgsComposerMap::Right )
  {
    pos.insert( mComposerMap->rect().height(), mComposerMap->rect().height() );
    pos.insert( mComposerMap->rect().height() + mGridFrameWidth, mComposerMap->rect().height() + mGridFrameWidth );
  }
  else //top or bottom
  {
    pos.insert( mComposerMap->rect().width(), mComposerMap->rect().width() );
    pos.insert( mComposerMap->rect().width() + mGridFrameWidth, mComposerMap->rect().width() + mGridFrameWidth );
  }

  //set pen to current frame pen
  QPen framePen = QPen( mGridFramePenColor );
  framePen.setWidthF( mGridFramePenThickness );
  framePen.setJoinStyle( Qt::MiterJoin );
  p->setPen( framePen );

  QMap< double, double >::const_iterator posIt = pos.constBegin();
  for ( ; posIt != pos.constEnd(); ++posIt )
  {
    p->setBrush( QBrush( color1 ? mGridFrameFillColor1 : mGridFrameFillColor2 ) );
    if ( border == QgsComposerMap::Left || border == QgsComposerMap::Right )
    {
      height = posIt.key() - currentCoord;
      width = mGridFrameWidth;
      x = ( border == QgsComposerMap::Left ) ? -mGridFrameWidth : mComposerMap->rect().width();
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == QgsComposerMap::Top ) ? -mGridFrameWidth : mComposerMap->rect().height();
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    color1 = !color1;
  }
}

void QgsComposerMapGrid::drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const
{
  if ( !p )
  {
    return;
  }


  QString currentAnnotationString;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, QgsComposerMap::Longitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString =  gridAnnotationString( it->first, QgsComposerMap::Latitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString );
  }
}

void QgsComposerMapGrid::drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString ) const
{
  if ( !mComposerMap )
  {
    return;
  }
  QgsComposerMap::Border frameBorder = borderForLineCoord( pos );
  double textWidth = QgsComposerUtils::textWidthMM( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = QgsComposerUtils::fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  double gridFrameDistance = ( mGridFrameStyle == QgsComposerMap::NoGridFrame ) ? 0 : mGridFrameWidth;

  if ( frameBorder == QgsComposerMap::Left )
  {

    if ( mLeftGridAnnotationPosition == QgsComposerMap::InsideMapFrame )
    {
      if ( mLeftGridAnnotationDirection == QgsComposerMap::Vertical || mLeftGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
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
    else if ( mLeftGridAnnotationPosition == QgsComposerMap::OutsideMapFrame ) //Outside map frame
    {
      if ( mLeftGridAnnotationDirection == QgsComposerMap::Vertical || mLeftGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
      {
        xpos -= ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else
      {
        xpos -= ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
      }
    }
    else
    {
      return;
    }

  }
  else if ( frameBorder == QgsComposerMap::Right )
  {
    if ( mRightGridAnnotationPosition == QgsComposerMap::InsideMapFrame )
    {
      if ( mRightGridAnnotationDirection == QgsComposerMap::Vertical || mRightGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
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
    else if ( mRightGridAnnotationPosition == QgsComposerMap::OutsideMapFrame )//OutsideMapFrame
    {
      if ( mRightGridAnnotationDirection == QgsComposerMap::Vertical || mRightGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
      {
        xpos += ( textHeight + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else //Horizontal
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
      }
    }
    else
    {
      return;
    }
  }
  else if ( frameBorder == QgsComposerMap::Bottom )
  {
    if ( mBottomGridAnnotationPosition == QgsComposerMap::InsideMapFrame )
    {
      if ( mBottomGridAnnotationDirection == QgsComposerMap::Horizontal || mBottomGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
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
    else if ( mBottomGridAnnotationPosition == QgsComposerMap::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mBottomGridAnnotationDirection == QgsComposerMap::Horizontal || mBottomGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
      {
        ypos += ( mAnnotationFrameDistance + textHeight + gridFrameDistance );
        xpos -= textWidth / 2.0;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
      }
    }
    else
    {
      return;
    }
  }
  else //Top
  {
    if ( mTopGridAnnotationPosition == QgsComposerMap::InsideMapFrame )
    {
      if ( mTopGridAnnotationDirection == QgsComposerMap::Horizontal || mTopGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
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
    else if ( mTopGridAnnotationPosition == QgsComposerMap::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mTopGridAnnotationDirection == QgsComposerMap::Horizontal || mTopGridAnnotationDirection == QgsComposerMap::BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
      }
    }
    else
    {
      return;
    }
  }

  drawAnnotation( p, QPointF( xpos, ypos ), rotation, annotationString );
}

void QgsComposerMapGrid::drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  p->save();
  p->translate( pos );
  p->rotate( rotation );
  QgsComposerUtils::drawText( p, QPointF( 0, 0 ), annotationText, mGridAnnotationFont, mGridAnnotationFontColor );
  p->restore();
}

QString QgsComposerMapGrid::gridAnnotationString( double value, QgsComposerMap::AnnotationCoordinate coord ) const
{
  if ( mGridAnnotationFormat == QgsComposerMap::Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }

  QgsPoint p;
  p.setX( coord == QgsComposerMap::Longitude ? value : 0 );
  p.setY( coord == QgsComposerMap::Longitude ? 0 : value );

  QString annotationString;
  if ( mGridAnnotationFormat == QgsComposerMap::DegreeMinute )
  {
    annotationString = p.toDegreesMinutes( mGridAnnotationPrecision );
  }
  else //DegreeMinuteSecond
  {
    annotationString = p.toDegreesMinutesSeconds( mGridAnnotationPrecision );
  }

  QStringList split = annotationString.split( "," );
  if ( coord == QgsComposerMap::Longitude )
  {
    return split.at( 0 );
  }
  else
  {
    if ( split.size() < 2 )
    {
      return "";
    }
    return split.at( 1 );
  }
}

int QgsComposerMapGrid::xGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalY <= 0.0 )
  {
    return 1;
  }


  QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalY = mGridIntervalY;
  double gridOffsetY = mGridOffsetY;
  double annotationScale = 1.0;
  if ( mGridUnit != MapUnit )
  {
    mapBoundingRect = mComposerMap->rect();
    mapPolygon = QPolygonF( mComposerMap->rect() );
    if ( mGridUnit == CM )
    {
      annotationScale = 0.1;
      gridIntervalY *= 10; gridOffsetY *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.top() - gridOffsetY ) / gridIntervalY + roundCorrection ) * gridIntervalY + gridOffsetY;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mComposerMap->mapRotation(), 0.0 ) || mGridUnit != MapUnit )
  {
    //no rotation. Do it 'the easy way'

    double yCanvasCoord;
    while ( currentLevel <= mapBoundingRect.bottom() && gridLineCount < MAX_GRID_LINES )
    {
      yCanvasCoord = mComposerMap->rect().height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
      lines.push_back( qMakePair( currentLevel * annotationScale, QLineF( 0, yCanvasCoord, mComposerMap->rect().width(), yCanvasCoord ) ) );
      currentLevel += gridIntervalY;
      gridLineCount++;
    }
    return 0;
  }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.bottom() && gridLineCount < MAX_GRID_LINES )
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
      lines.push_back( qMakePair( currentLevel, QLineF( mComposerMap->mapToItemCoords( intersectionList.at( 0 ) ), mComposerMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalY;
  }


  return 0;
}

int QgsComposerMapGrid::yGridLines( QList< QPair< double, QLineF > >& lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalX = mGridIntervalX;
  double gridOffsetX = mGridOffsetX;
  double annotationScale = 1.0;
  if ( mGridUnit != MapUnit )
  {
    mapBoundingRect = mComposerMap->rect();
    mapPolygon = QPolygonF( mComposerMap->rect() );
    if ( mGridUnit == CM )
    {
      annotationScale = 0.1;
      gridIntervalX *= 10; gridOffsetX *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( mapBoundingRect.left() - gridOffsetX ) / gridIntervalX + roundCorrection ) * gridIntervalX + gridOffsetX;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mComposerMap->mapRotation(), 0.0 ) || mGridUnit != MapUnit )
  {
    //no rotation. Do it 'the easy way'
    double xCanvasCoord;
    while ( currentLevel <= mapBoundingRect.right() && gridLineCount < MAX_GRID_LINES )
    {
      xCanvasCoord = mComposerMap->rect().width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();
      lines.push_back( qMakePair( currentLevel * annotationScale, QLineF( xCanvasCoord, 0, xCanvasCoord, mComposerMap->rect().height() ) ) );
      currentLevel += gridIntervalX;
      gridLineCount++;
    }
    return 0;
  }

  //the four border lines
  QVector<QLineF> borderLines;
  borderLines << QLineF( mapPolygon.at( 0 ), mapPolygon.at( 1 ) );
  borderLines << QLineF( mapPolygon.at( 1 ), mapPolygon.at( 2 ) );
  borderLines << QLineF( mapPolygon.at( 2 ), mapPolygon.at( 3 ) );
  borderLines << QLineF( mapPolygon.at( 3 ), mapPolygon.at( 0 ) );

  QList<QPointF> intersectionList; //intersects between border lines and grid lines

  while ( currentLevel <= mapBoundingRect.right() && gridLineCount < MAX_GRID_LINES )
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
      lines.push_back( qMakePair( currentLevel, QLineF( mComposerMap->mapToItemCoords( intersectionList.at( 0 ) ), mComposerMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalX;
  }

  return 0;
}

int QgsComposerMapGrid::xGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalY <= 0.0 )
  {
    return 1;
  }

  double roundCorrection = bbox.yMaximum() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( bbox.yMaximum() - mGridOffsetY ) / mGridIntervalY + roundCorrection ) * mGridIntervalY + mGridOffsetY;

  double minX = bbox.xMinimum();
  double maxX = bbox.xMaximum();
  double step = ( maxX - minX ) / 20;

  int gridLineCount = 0;
  while ( currentLevel >= bbox.yMinimum() && gridLineCount < MAX_GRID_LINES )
  {
    QPolygonF gridLine;
    double currentX = minX;
    bool cont = true;
    while ( cont )
    {
      if ( currentX > maxX )
      {
        cont = false;
      }

      QgsPoint mapPoint = t.transform( currentX, currentLevel ); //transform back to map crs
      gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) ); //transform back to composer coords
      currentX += step;
    }

    gridLine = trimLineToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
    if ( gridLine.size() > 0 )
    {
      lines.append( qMakePair( currentLevel, gridLine ) );
      gridLineCount++;
    }
    currentLevel -= mGridIntervalY;
  }

  return 0;
}

int QgsComposerMapGrid::yGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  double roundCorrection = bbox.xMinimum() > 0 ? 1.0 : 0.0;
  double currentLevel = ( int )(( bbox.xMinimum() - mGridOffsetX ) / mGridIntervalX + roundCorrection ) * mGridIntervalX + mGridOffsetX;

  double minY = bbox.yMinimum();
  double maxY = bbox.yMaximum();
  double step = ( maxY - minY ) / 20;

  int gridLineCount = 0;
  while ( currentLevel <= bbox.xMaximum() && gridLineCount < MAX_GRID_LINES )
  {
    QPolygonF gridLine;
    double currentY = minY;
    bool cont = true;
    while ( cont )
    {
      if ( currentY > maxY )
      {
        cont = false;
      }
      //transform back to map crs
      QgsPoint mapPoint = t.transform( currentLevel, currentY );
      //transform back to composer coords
      gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) );
      currentY += step;
    }
    //clip grid line to map polygon
    gridLine = trimLineToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
    if ( gridLine.size() > 0 )
    {
      lines.append( qMakePair( currentLevel, gridLine ) );
      gridLineCount++;
    }
    currentLevel += mGridIntervalX;
  }

  return 0;
}

void QgsComposerMapGrid::sortGridLinesOnBorders( const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines,  QMap< double, double >& leftFrameEntries,
    QMap< double, double >& rightFrameEntries, QMap< double, double >& topFrameEntries, QMap< double, double >& bottomFrameEntries ) const
{
  QList< QPair< double, QPointF > > borderPositions;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    borderPositions << qMakePair( it->first, it->second.p1() );
    borderPositions << qMakePair( it->first, it->second.p2() );
  }
  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    borderPositions << qMakePair( it->first, it->second.p1() );
    borderPositions << qMakePair( it->first, it->second.p2() );
  }

  QList< QPair< double, QPointF > >::const_iterator bIt = borderPositions.constBegin();
  for ( ; bIt != borderPositions.constEnd(); ++bIt )
  {
    QgsComposerMap::Border frameBorder = borderForLineCoord( bIt->second );
    if ( frameBorder == QgsComposerMap::Left )
    {
      leftFrameEntries.insert( bIt->second.y(), bIt->first );
    }
    else if ( frameBorder == QgsComposerMap::Right )
    {
      rightFrameEntries.insert( bIt->second.y(), bIt->first );
    }
    else if ( frameBorder == QgsComposerMap::Top )
    {
      topFrameEntries.insert( bIt->second.x(), bIt->first );
    }
    else //Bottom
    {
      bottomFrameEntries.insert( bIt->second.x(), bIt->first );
    }
  }
}

QgsComposerMap::Border QgsComposerMapGrid::borderForLineCoord( const QPointF& p ) const
{
  if ( !mComposerMap )
  {
    return QgsComposerMap::Left;
  }

  double framePenWidth = mComposerMap->hasFrame() ? mComposerMap->pen().widthF() : 0;
  if ( p.y() <= framePenWidth )
  {
    return QgsComposerMap::Top;
  }
  else if ( p.x() <= framePenWidth )
  {
    return QgsComposerMap::Left;
  }
  else if ( p.x() >= ( mComposerMap->rect().width() - framePenWidth ) )
  {
    return QgsComposerMap::Right;
  }
  else
  {
    return QgsComposerMap::Bottom;
  }
}

void QgsComposerMapGrid::setGridLineSymbol( QgsLineSymbolV2* symbol )
{
  delete mGridLineSymbol;
  mGridLineSymbol = symbol;
}

void QgsComposerMapGrid::setGridMarkerSymbol( QgsMarkerSymbolV2 *symbol )
{
  delete mGridMarkerSymbol;
  mGridMarkerSymbol = symbol;
}

double QgsComposerMapGrid::maxExtension() const
{
  if ( !mComposerMap )
  {
    return 0;
  }

  if ( !mGridEnabled || ( mGridFrameStyle == QgsComposerMap::NoGridFrame && ( !mShowGridAnnotation || ( mLeftGridAnnotationPosition != QgsComposerMap::OutsideMapFrame && mRightGridAnnotationPosition != QgsComposerMap::OutsideMapFrame
                          && mTopGridAnnotationPosition != QgsComposerMap::OutsideMapFrame && mBottomGridAnnotationPosition != QgsComposerMap::OutsideMapFrame ) ) ) )
  {
    return 0;
  }

  const QgsMapSettings& ms = mComposerMap->composition()->mapSettings();
  QStringList coordStrings;
  if ( mCRS.isValid() && mCRS != ms.destinationCrs() )
  {
    QList< QPair< double, QPolygonF > > xGridLines;
    QList< QPair< double, QPolygonF > > yGridLines;
    QgsRectangle crsRect;
    QgsCoordinateTransform inverseTransform;
    if ( crsGridParams( crsRect, inverseTransform ) != 0 )
    {
      return 0;
    }

    int xGridReturn = xGridLinesCRSTransform( crsRect, inverseTransform, xGridLines );
    int yGridReturn = yGridLinesCRSTransform( crsRect, inverseTransform, yGridLines );
    if ( xGridReturn != 0 || yGridReturn != 0 )
    {
      return 0;
    }

    QList< QPair< double, QPolygonF > >::const_iterator it = xGridLines.constBegin();
    for ( ; it != xGridLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMap::Latitude ) );
    }
    it = yGridLines.constBegin();
    for ( ; it != yGridLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMap::Longitude ) );
    }
  }
  else
  {
    QList< QPair< double, QLineF > > xLines;
    QList< QPair< double, QLineF > > yLines;
    int xGridReturn = xGridLines( xLines );
    int yGridReturn = yGridLines( yLines );
    if ( xGridReturn != 0 && yGridReturn != 0 )
    {
      return 0;
    }

    QList< QPair< double, QLineF > >::const_iterator it = xLines.constBegin();
    for ( ; it != xLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMap::Latitude ) );
    }

    it = yLines.constBegin();
    for ( ; it != yLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMap::Longitude ) );
    }
  }

  double maxExtension = 0;
  double currentExtension = 0;

  QStringList::const_iterator coordIt = coordStrings.constBegin();
  for ( ; coordIt != coordStrings.constEnd(); ++coordIt )
  {
    currentExtension = qMax( QgsComposerUtils::textWidthMM( mGridAnnotationFont, *coordIt ), QgsComposerUtils::fontAscentMM( mGridAnnotationFont ) );
    maxExtension = qMax( maxExtension, currentExtension );
  }

  //grid frame
  double gridFrameDist = ( mGridFrameStyle == QgsComposerMap::NoGridFrame ) ? 0 : mGridFrameWidth + ( mGridFramePenThickness / 2.0 );
  return maxExtension + mAnnotationFrameDistance + gridFrameDist;
}

void QgsComposerMapGrid::setGridAnnotationDirection( QgsComposerMap::GridAnnotationDirection d, QgsComposerMap::Border border )
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      mLeftGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Right:
      mRightGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Top:
      mTopGridAnnotationDirection = d;
      break;
    case QgsComposerMap::Bottom:
      mBottomGridAnnotationDirection = d;
      break;
    default:
      return;
      break;
  }

  if ( mComposerMap )
  {
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
  }
}

QgsComposerMap::GridAnnotationDirection QgsComposerMapGrid::gridAnnotationDirection() const
{
  return mLeftGridAnnotationDirection;
}

void QgsComposerMapGrid::setGridAnnotationDirection( QgsComposerMap::GridAnnotationDirection d )
{
  mLeftGridAnnotationDirection = d;
  mRightGridAnnotationDirection = d;
  mTopGridAnnotationDirection = d;
  mBottomGridAnnotationDirection = d;
}

void QgsComposerMapGrid::setGridAnnotationPosition( QgsComposerMap::GridAnnotationPosition p, QgsComposerMap::Border border )
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      mLeftGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Right:
      mRightGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Top:
      mTopGridAnnotationPosition = p;
      break;
    case QgsComposerMap::Bottom:
      mBottomGridAnnotationPosition = p;
      break;
    default:
      return;
  }

  if ( mComposerMap )
  {
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
  }
}

QgsComposerMap::GridAnnotationPosition QgsComposerMapGrid::gridAnnotationPosition( QgsComposerMap::Border border ) const
{
  switch ( border )
  {
    case QgsComposerMap::Left:
      return mLeftGridAnnotationPosition;
      break;
    case QgsComposerMap::Right:
      return mRightGridAnnotationPosition;
      break;
    case QgsComposerMap::Top:
      return mTopGridAnnotationPosition;
      break;
    case QgsComposerMap::Bottom:
    default:
      return mBottomGridAnnotationPosition;
      break;
  }
}

QgsComposerMap::GridAnnotationDirection QgsComposerMapGrid::gridAnnotationDirection( QgsComposerMap::Border border ) const
{
  if ( !mComposerMap )
  {
    return mLeftGridAnnotationDirection;
  }

  switch ( border )
  {
    case QgsComposerMap::Left:
      return mLeftGridAnnotationDirection;
      break;
    case QgsComposerMap::Right:
      return mRightGridAnnotationDirection;
      break;
    case QgsComposerMap::Top:
      return mTopGridAnnotationDirection;
      break;
    case QgsComposerMap::Bottom:
    default:
      return mBottomGridAnnotationDirection;
      break;
  }
}

int QgsComposerMapGrid::crsGridParams( QgsRectangle& crsRect, QgsCoordinateTransform& inverseTransform ) const
{
  if ( !mComposerMap )
  {
    return 1;
  }

  QgsCoordinateTransform tr( mComposerMap->composition()->mapSettings().destinationCrs(), mCRS );
  QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
  QRectF mbr = mapPolygon.boundingRect();
  QgsRectangle mapBoundingRect( mbr.left(), mbr.bottom(), mbr.right(), mbr.top() );
  crsRect = tr.transformBoundingBox( mapBoundingRect );
  inverseTransform.setSourceCrs( mCRS );
  inverseTransform.setDestCRS( mComposerMap->composition()->mapSettings().destinationCrs() );
  return 0;
}

QPolygonF QgsComposerMapGrid::trimLineToMap( const QPolygonF& line, const QgsRectangle& rect )
{
  QgsPolyline polyLine;
  QPolygonF::const_iterator lineIt = line.constBegin();
  for ( ; lineIt != line.constEnd(); ++lineIt )
  {
    polyLine.append( QgsPoint( lineIt->x(), lineIt->y() ) );
  }

  QgsGeometry* geom = QgsGeometry::fromPolyline( polyLine );

  QPolygonF clippedLine;
  QgsClipper::clippedLineWKB( geom->asWkb(), rect, clippedLine );
  delete geom;
  return clippedLine;
}


