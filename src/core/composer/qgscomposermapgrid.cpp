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
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"

#include <QPainter>
#include <QPen>

#define MAX_GRID_LINES 1000 //maximum number of horizontal or vertical grid lines to draw

QgsComposerMapGridStack::QgsComposerMapGridStack( QgsComposerMap* map )
    : QgsComposerMapItemStack( map )
{

}

QgsComposerMapGridStack::~QgsComposerMapGridStack()
{
}

void QgsComposerMapGridStack::addGrid( QgsComposerMapGrid* grid )
{
  QgsComposerMapItemStack::addItem( grid );
}

void QgsComposerMapGridStack::removeGrid( const QString& gridId )
{
  QgsComposerMapItemStack::removeItem( gridId );
}

void QgsComposerMapGridStack::moveGridUp( const QString& gridId )
{
  QgsComposerMapItemStack::moveItemUp( gridId );
}

void QgsComposerMapGridStack::moveGridDown( const QString& gridId )
{
  QgsComposerMapItemStack::moveItemDown( gridId );
}

const QgsComposerMapGrid* QgsComposerMapGridStack::constGrid( const QString& gridId ) const
{
  const QgsComposerMapItem* item = QgsComposerMapItemStack::constItem( gridId );
  return dynamic_cast<const QgsComposerMapGrid*>( item );
}

QgsComposerMapGrid* QgsComposerMapGridStack::grid( const QString& gridId ) const
{
  QgsComposerMapItem* item = QgsComposerMapItemStack::item( gridId );
  return dynamic_cast<QgsComposerMapGrid*>( item );
}

QgsComposerMapGrid *QgsComposerMapGridStack::grid( const int index ) const
{
  QgsComposerMapItem* item = QgsComposerMapItemStack::item( index );
  return dynamic_cast<QgsComposerMapGrid*>( item );
}

QList<QgsComposerMapGrid *> QgsComposerMapGridStack::asList() const
{
  QList< QgsComposerMapGrid* > list;
  QList< QgsComposerMapItem* >::const_iterator it = mItems.begin();
  for ( ; it != mItems.end(); ++it )
  {
    QgsComposerMapGrid* grid = dynamic_cast<QgsComposerMapGrid*>( *it );
    if ( grid )
    {
      list.append( grid );
    }
  }
  return list;
}

QgsComposerMapGrid &QgsComposerMapGridStack::operator[]( int idx )
{
  QgsComposerMapItem* item = mItems[idx];
  QgsComposerMapGrid* grid = dynamic_cast<QgsComposerMapGrid*>( item );
  return *grid;
}

bool QgsComposerMapGridStack::readXML( const QDomElement &elem, const QDomDocument &doc )
{
  removeItems();

  //read grid stack
  QDomNodeList mapGridNodeList = elem.elementsByTagName( "ComposerMapGrid" );
  for ( int i = 0; i < mapGridNodeList.size(); ++i )
  {
    QDomElement mapGridElem = mapGridNodeList.at( i ).toElement();
    QgsComposerMapGrid* mapGrid = new QgsComposerMapGrid( mapGridElem.attribute( "name" ), mComposerMap );
    mapGrid->readXML( mapGridElem, doc );
    mItems.append( mapGrid );
  }

  return true;
}

double QgsComposerMapGridStack::maxGridExtension() const
{
  double maxGridExtension = 0;

  QList< QgsComposerMapItem* >::const_iterator it = mItems.constBegin();
  for ( ; it != mItems.constEnd(); ++it )
  {
    QgsComposerMapGrid* grid = dynamic_cast<QgsComposerMapGrid*>( *it );
    if ( grid )
    {
      maxGridExtension = qMax( maxGridExtension, grid->maxExtension() );
    }
  }

  return maxGridExtension;
}


//
// QgsComposerMapGrid
//


QgsComposerMapGrid::QgsComposerMapGrid( const QString& name, QgsComposerMap* map )
    : QgsComposerMapItem( name, map )
{
  init();
}

QgsComposerMapGrid::QgsComposerMapGrid()
    : QgsComposerMapItem( QString(), 0 )
{
  init();
}

void QgsComposerMapGrid::init()
{
  mTransformDirty = true;
  mGridStyle = QgsComposerMapGrid::Solid;
  mGridIntervalX = 0.0;
  mGridIntervalY = 0.0;
  mGridOffsetX = 0.0;
  mGridOffsetY = 0.0;
  mGridAnnotationFontColor = Qt::black;
  mGridAnnotationPrecision = 3;
  mShowGridAnnotation = false;
  mLeftGridAnnotationDisplay = QgsComposerMapGrid::ShowAll;
  mRightGridAnnotationDisplay = QgsComposerMapGrid::ShowAll;
  mTopGridAnnotationDisplay = QgsComposerMapGrid::ShowAll;
  mBottomGridAnnotationDisplay = QgsComposerMapGrid::ShowAll;
  mLeftGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  mRightGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  mTopGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  mBottomGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  mAnnotationFrameDistance = 1.0;
  mLeftGridAnnotationDirection = QgsComposerMapGrid::Horizontal;
  mRightGridAnnotationDirection = QgsComposerMapGrid::Horizontal;
  mTopGridAnnotationDirection = QgsComposerMapGrid::Horizontal;
  mBottomGridAnnotationDirection = QgsComposerMapGrid::Horizontal;
  mGridAnnotationFormat = QgsComposerMapGrid::Decimal;
  mGridFrameStyle = QgsComposerMapGrid::NoFrame;
  mGridFrameSides = QgsComposerMapGrid::FrameLeft | QgsComposerMapGrid::FrameRight |
                    QgsComposerMapGrid::FrameTop | QgsComposerMapGrid::FrameBottom;
  mGridFrameWidth = 2.0;
  mGridFramePenThickness = 0.3;
  mGridFramePenColor = QColor( 0, 0, 0 );
  mGridFrameFillColor1 = Qt::white;
  mGridFrameFillColor2 = Qt::black;
  mCrossLength = 3;
  mLeftFrameDivisions = QgsComposerMapGrid::ShowAll;
  mRightFrameDivisions = QgsComposerMapGrid::ShowAll;
  mTopFrameDivisions = QgsComposerMapGrid::ShowAll;
  mBottomFrameDivisions = QgsComposerMapGrid::ShowAll;
  mGridLineSymbol = 0;
  mGridMarkerSymbol = 0;
  mGridUnit = MapUnit;
  mBlendMode = QPainter::CompositionMode_SourceOver;

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

void QgsComposerMapGrid::setGridLineWidth( const double width )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setWidth( width );
  }
}

void QgsComposerMapGrid::setGridLineColor( const QColor& c )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setColor( c );
  }
}

bool QgsComposerMapGrid::writeXML( QDomElement& elem, QDomDocument& doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement mapGridElem = doc.createElement( "ComposerMapGrid" );
  mapGridElem.setAttribute( "gridStyle", mGridStyle );
  mapGridElem.setAttribute( "intervalX", qgsDoubleToString( mGridIntervalX ) );
  mapGridElem.setAttribute( "intervalY", qgsDoubleToString( mGridIntervalY ) );
  mapGridElem.setAttribute( "offsetX", qgsDoubleToString( mGridOffsetX ) );
  mapGridElem.setAttribute( "offsetY", qgsDoubleToString( mGridOffsetY ) );
  mapGridElem.setAttribute( "crossLength", qgsDoubleToString( mCrossLength ) );

  QDomElement lineStyleElem = doc.createElement( "lineStyle" );
  QDomElement gridLineStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mGridLineSymbol, doc );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( "markerStyle" );
  QDomElement gridMarkerStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mGridMarkerSymbol, doc );
  markerStyleElem.appendChild( gridMarkerStyleElem );
  mapGridElem.appendChild( markerStyleElem );

  mapGridElem.setAttribute( "gridFrameStyle", mGridFrameStyle );
  mapGridElem.setAttribute( "gridFrameSideFlags", mGridFrameSides );
  mapGridElem.setAttribute( "gridFrameWidth", qgsDoubleToString( mGridFrameWidth ) );
  mapGridElem.setAttribute( "gridFramePenThickness", qgsDoubleToString( mGridFramePenThickness ) );
  mapGridElem.setAttribute( "gridFramePenColor", QgsSymbolLayerV2Utils::encodeColor( mGridFramePenColor ) );
  mapGridElem.setAttribute( "frameFillColor1", QgsSymbolLayerV2Utils::encodeColor( mGridFrameFillColor1 ) );
  mapGridElem.setAttribute( "frameFillColor2", QgsSymbolLayerV2Utils::encodeColor( mGridFrameFillColor2 ) );
  mapGridElem.setAttribute( "leftFrameDivisions", mLeftFrameDivisions );
  mapGridElem.setAttribute( "rightFrameDivisions", mRightFrameDivisions );
  mapGridElem.setAttribute( "topFrameDivisions", mTopFrameDivisions );
  mapGridElem.setAttribute( "bottomFrameDivisions", mBottomFrameDivisions );
  if ( mCRS.isValid() )
  {
    mCRS.writeXML( mapGridElem, doc );
  }

  mapGridElem.setAttribute( "annotationFormat", mGridAnnotationFormat );
  mapGridElem.setAttribute( "showAnnotation", mShowGridAnnotation );
  mapGridElem.setAttribute( "leftAnnotationDisplay", mLeftGridAnnotationDisplay );
  mapGridElem.setAttribute( "rightAnnotationDisplay", mRightGridAnnotationDisplay );
  mapGridElem.setAttribute( "topAnnotationDisplay", mTopGridAnnotationDisplay );
  mapGridElem.setAttribute( "bottomAnnotationDisplay", mBottomGridAnnotationDisplay );
  mapGridElem.setAttribute( "leftAnnotationPosition", mLeftGridAnnotationPosition );
  mapGridElem.setAttribute( "rightAnnotationPosition", mRightGridAnnotationPosition );
  mapGridElem.setAttribute( "topAnnotationPosition", mTopGridAnnotationPosition );
  mapGridElem.setAttribute( "bottomAnnotationPosition", mBottomGridAnnotationPosition );
  mapGridElem.setAttribute( "leftAnnotationDirection", mLeftGridAnnotationDirection );
  mapGridElem.setAttribute( "rightAnnotationDirection", mRightGridAnnotationDirection );
  mapGridElem.setAttribute( "topAnnotationDirection", mTopGridAnnotationDirection );
  mapGridElem.setAttribute( "bottomAnnotationDirection", mBottomGridAnnotationDirection );
  mapGridElem.setAttribute( "frameAnnotationDistance", QString::number( mAnnotationFrameDistance ) );
  mapGridElem.setAttribute( "annotationFont", mGridAnnotationFont.toString() );
  mapGridElem.setAttribute( "annotationFontColor", QgsSymbolLayerV2Utils::encodeColor( mGridAnnotationFontColor ) );
  mapGridElem.setAttribute( "annotationPrecision", mGridAnnotationPrecision );
  mapGridElem.setAttribute( "unit", mGridUnit );
  mapGridElem.setAttribute( "blendMode", mBlendMode );

  bool ok = QgsComposerMapItem::writeXML( mapGridElem, doc );
  elem.appendChild( mapGridElem );
  return ok;
}

bool QgsComposerMapGrid::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  bool ok = QgsComposerMapItem::readXML( itemElem, doc );

  //grid
  mGridStyle = QgsComposerMapGrid::GridStyle( itemElem.attribute( "gridStyle", "0" ).toInt() );
  mGridIntervalX = itemElem.attribute( "intervalX", "0" ).toDouble();
  mGridIntervalY = itemElem.attribute( "intervalY", "0" ).toDouble();
  mGridOffsetX = itemElem.attribute( "offsetX", "0" ).toDouble();
  mGridOffsetY = itemElem.attribute( "offsetY", "0" ).toDouble();
  mCrossLength = itemElem.attribute( "crossLength", "3" ).toDouble();
  mGridFrameStyle = ( QgsComposerMapGrid::FrameStyle )itemElem.attribute( "gridFrameStyle", "0" ).toInt();
  mGridFrameSides = ( QgsComposerMapGrid::FrameSideFlags )itemElem.attribute( "gridFrameSideFlags", "15" ).toInt();
  mGridFrameWidth = itemElem.attribute( "gridFrameWidth", "2.0" ).toDouble();
  mGridFramePenThickness = itemElem.attribute( "gridFramePenThickness", "0.3" ).toDouble();
  mGridFramePenColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "gridFramePenColor", "0,0,0" ) );
  mGridFrameFillColor1 = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "frameFillColor1", "255,255,255,255" ) );
  mGridFrameFillColor2 = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "frameFillColor2", "0,0,0,255" ) );
  mLeftFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "leftFrameDivisions", "0" ).toInt() );
  mRightFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "rightFrameDivisions", "0" ).toInt() );
  mTopFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "topFrameDivisions", "0" ).toInt() );
  mBottomFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "bottomFrameDivisions", "0" ).toInt() );

  QDomElement lineStyleElem = itemElem.firstChildElement( "lineStyle" );
  if ( !lineStyleElem.isNull() )
  {
    QDomElement symbolElem = lineStyleElem.firstChildElement( "symbol" );
    if ( !symbolElem.isNull() )
    {
      delete mGridLineSymbol;
      mGridLineSymbol = QgsSymbolLayerV2Utils::loadSymbol<QgsLineSymbolV2>( symbolElem );
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
    if ( !symbolElem.isNull() )
    {
      delete mGridMarkerSymbol;
      mGridMarkerSymbol = QgsSymbolLayerV2Utils::loadSymbol<QgsMarkerSymbolV2>( symbolElem );
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
  mGridAnnotationFormat = QgsComposerMapGrid::AnnotationFormat( itemElem.attribute( "annotationFormat", "0" ).toInt() );
  mLeftGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( "leftAnnotationPosition", "0" ).toInt() );
  mRightGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( "rightAnnotationPosition", "0" ).toInt() );
  mTopGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( "topAnnotationPosition", "0" ).toInt() );
  mBottomGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( "bottomAnnotationPosition", "0" ).toInt() );
  mLeftGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "leftAnnotationDisplay", "0" ).toInt() );
  mRightGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "rightAnnotationDisplay", "0" ).toInt() );
  mTopGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "topAnnotationDisplay", "0" ).toInt() );
  mBottomGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( "bottomAnnotationDisplay", "0" ).toInt() );
  //upgrade pre-2.7 projects
  if ( mLeftGridAnnotationPosition == QgsComposerMapGrid::Disabled )
  {
    mLeftGridAnnotationDisplay = QgsComposerMapGrid::HideAll;
    mLeftGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  }
  if ( mRightGridAnnotationPosition == QgsComposerMapGrid::Disabled )
  {
    mRightGridAnnotationDisplay = QgsComposerMapGrid::HideAll;
    mRightGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  }
  if ( mTopGridAnnotationPosition == QgsComposerMapGrid::Disabled )
  {
    mTopGridAnnotationDisplay = QgsComposerMapGrid::HideAll;
    mTopGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  }
  if ( mBottomGridAnnotationPosition == QgsComposerMapGrid::Disabled )
  {
    mBottomGridAnnotationDisplay = QgsComposerMapGrid::HideAll;
    mBottomGridAnnotationPosition = QgsComposerMapGrid::OutsideMapFrame;
  }

  mLeftGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( "leftAnnotationDirection", "0" ).toInt() );
  mRightGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( "rightAnnotationDirection", "0" ).toInt() );
  mTopGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( "topAnnotationDirection", "0" ).toInt() );
  mBottomGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( "bottomAnnotationDirection", "0" ).toInt() );
  mAnnotationFrameDistance = itemElem.attribute( "frameAnnotationDistance", "0" ).toDouble();
  mGridAnnotationFont.fromString( itemElem.attribute( "annotationFont", "" ) );
  mGridAnnotationFontColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "annotationFontColor", "0,0,0,255" ) );
  mGridAnnotationPrecision = itemElem.attribute( "annotationPrecision", "3" ).toInt();
  int gridUnitInt =  itemElem.attribute( "unit", QString::number( MapUnit ) ).toInt();
  mGridUnit = ( gridUnitInt <= ( int )CM ) ? ( GridUnit )gridUnitInt : MapUnit;
  return ok;
}

void QgsComposerMapGrid::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCRS = crs;
  mTransformDirty = true;
}

bool QgsComposerMapGrid::usesAdvancedEffects() const
{
  return mBlendMode == QPainter::CompositionMode_SourceOver;
}

QPolygonF QgsComposerMapGrid::scalePolygon( const QPolygonF &polygon, const double scale ) const
{
  QTransform t = QTransform::fromScale( scale, scale );
  return t.map( polygon );
}

void QgsComposerMapGrid::drawGridCRSTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines )
{
  if ( !mComposerMap || !mEnabled )
  {
    return;
  }

  //has map extent/scale changed?
  QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
  if ( mapPolygon != mPrevMapPolygon )
  {
    mTransformDirty = true;
    mPrevMapPolygon = mapPolygon;
  }

  if ( mTransformDirty )
  {
    calculateCRSTransformLines();
  }

  //draw lines
  if ( mGridStyle == QgsComposerMapGrid::Solid )
  {
    QList< QPair< double, QPolygonF > >::const_iterator xGridIt = mTransformedXLines.constBegin();
    for ( ; xGridIt != mTransformedXLines.constEnd(); ++xGridIt )
    {
      drawGridLine( scalePolygon( xGridIt->second, dotsPerMM ), context );
    }

    QList< QPair< double, QPolygonF > >::const_iterator yGridIt = mTransformedYLines.constBegin();
    for ( ; yGridIt != mTransformedYLines.constEnd(); ++yGridIt )
    {
      drawGridLine( scalePolygon( yGridIt->second, dotsPerMM ), context );
    }
  }
  else if ( mGridStyle == QgsComposerMapGrid::Cross || mGridStyle == QgsComposerMapGrid::Markers )
  {
    double maxX = mComposerMap->rect().width();
    double maxY = mComposerMap->rect().height();

    QList< QgsPoint >::const_iterator intersectionIt = mTransformedIntersections.constBegin();
    for ( ; intersectionIt != mTransformedIntersections.constEnd(); ++intersectionIt )
    {
      double x = intersectionIt->x();
      double y = intersectionIt->y();
      if ( mGridStyle == QgsComposerMapGrid::Cross )
      {
        //ensure that crosses don't overshoot the map item bounds
        QLineF line1 = QLineF( x - mCrossLength, y, x + mCrossLength, y );
        line1.p1().rx() = line1.p1().x() < 0 ? 0 : line1.p1().x();
        line1.p2().rx() = line1.p2().x() > maxX ? maxX : line1.p2().x();
        QLineF line2 = QLineF( x, y - mCrossLength, x, y + mCrossLength );
        line2.p1().ry() = line2.p1().y() < 0 ? 0 : line2.p1().y();
        line2.p2().ry() = line2.p2().y() > maxY ? maxY : line2.p2().y();

        //draw line using coordinates scaled to dots
        drawGridLine( QLineF( line1.p1() * dotsPerMM, line1.p2() * dotsPerMM ), context );
        drawGridLine( QLineF( line2.p1() * dotsPerMM, line2.p2() * dotsPerMM ), context );
      }
      else if ( mGridStyle == QgsComposerMapGrid::Markers )
      {
        drawGridMarker( QPointF( x, y ) * dotsPerMM, context );
      }
    }
  }

  //convert QPolygonF to QLineF to draw grid frames and annotations
  QList< QPair< double, QPolygonF > >::const_iterator yGridLineIt = mTransformedYLines.constBegin();
  for ( ; yGridLineIt != mTransformedYLines.constEnd(); ++yGridLineIt )
  {
    verticalLines.push_back( qMakePair( yGridLineIt->first, QLineF( yGridLineIt->second.first(), yGridLineIt->second.last() ) ) );
  }
  QList< QPair< double, QPolygonF > >::const_iterator xGridLineIt = mTransformedXLines.constBegin();
  for ( ; xGridLineIt != mTransformedXLines.constEnd(); ++xGridLineIt )
  {
    horizontalLines.push_back( qMakePair( xGridLineIt->first, QLineF( xGridLineIt->second.first(), xGridLineIt->second.last() ) ) );
  }
}

void QgsComposerMapGrid::calculateCRSTransformLines()
{
  QgsRectangle crsBoundingRect;
  QgsCoordinateTransform inverseTr;
  if ( crsGridParams( crsBoundingRect, inverseTr ) != 0 )
  {
    return;
  }

  //calculate x grid lines
  mTransformedXLines.clear();
  xGridLinesCRSTransform( crsBoundingRect, inverseTr, mTransformedXLines );

  //calculate y grid lines
  mTransformedYLines.clear();
  yGridLinesCRSTransform( crsBoundingRect, inverseTr, mTransformedYLines );

  if ( mGridStyle == QgsComposerMapGrid::Cross || mGridStyle == QgsComposerMapGrid::Markers )
  {
    //cross or markers style - we also need to calculate intersections of lines

    //first convert lines to QgsGeometry
    QList< QgsGeometry* > yLines;
    QList< QPair< double, QPolygonF > >::const_iterator yGridIt = mTransformedYLines.constBegin();
    for ( ; yGridIt != mTransformedYLines.constEnd(); ++yGridIt )
    {
      QgsPolyline yLine;
      for ( int i = 0; i < ( *yGridIt ).second.size(); ++i )
      {
        yLine.append( QgsPoint(( *yGridIt ).second.at( i ).x(), ( *yGridIt ).second.at( i ).y() ) );
      }
      yLines << QgsGeometry::fromPolyline( yLine );
    }
    QList< QgsGeometry* > xLines;
    QList< QPair< double, QPolygonF > >::const_iterator xGridIt = mTransformedXLines.constBegin();
    for ( ; xGridIt != mTransformedXLines.constEnd(); ++xGridIt )
    {
      QgsPolyline xLine;
      for ( int i = 0; i < ( *xGridIt ).second.size(); ++i )
      {
        xLine.append( QgsPoint(( *xGridIt ).second.at( i ).x(), ( *xGridIt ).second.at( i ).y() ) );
      }
      xLines << QgsGeometry::fromPolyline( xLine );
    }

    //now, loop through geometries and calculate intersection points
    mTransformedIntersections.clear();
    QList< QgsGeometry* >::const_iterator yLineIt = yLines.constBegin();
    for ( ; yLineIt != yLines.constEnd(); ++yLineIt )
    {
      QList< QgsGeometry* >::const_iterator xLineIt = xLines.constBegin();
      for ( ; xLineIt != xLines.constEnd(); ++xLineIt )
      {
        //look for intersections between lines
        QgsGeometry* intersects = ( *yLineIt )->intersection(( *xLineIt ) );

        //go through all intersections and draw grid markers/crosses
        int i = 0;
        QgsPoint vertex = intersects->vertexAt( i );
        while ( vertex != QgsPoint( 0, 0 ) )
        {
          mTransformedIntersections << vertex;
          i = i + 1;
          vertex = intersects->vertexAt( i );
        }
      }
    }
    //clean up
    qDeleteAll( yLines );
    yLines.clear();
    qDeleteAll( xLines );
    xLines.clear();
  }

  mTransformDirty = false;
}

void QgsComposerMapGrid::draw( QPainter* p )
{
  if ( !mComposerMap || !mEnabled )
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
  if ( thisPaintRect != mPrevPaintRect )
  {
    //rect has changed, so need to recalculate transform
    mTransformDirty = true;
    mPrevPaintRect = thisPaintRect;
  }

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

  if ( mGridFrameStyle != QgsComposerMapGrid::NoFrame )
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
  if ( mGridStyle == QgsComposerMapGrid::Solid )
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
  else if ( mGridStyle != QgsComposerMapGrid::FrameAnnotationsOnly ) //cross or markers
  {
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != verticalLines.constEnd(); ++vIt )
    {
      //test for intersection with every horizontal line
      hIt = horizontalLines.constBegin();
      for ( ; hIt != horizontalLines.constEnd(); ++hIt )
      {
        if ( hIt->second.intersect( vIt->second, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          if ( mGridStyle == QgsComposerMapGrid::Cross )
          {
            //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
            crossEnd1 = (( intersectionPoint - vIt->second.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength ) : intersectionPoint;
            crossEnd2 = (( intersectionPoint - vIt->second.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerV2Utils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength ) : intersectionPoint;
            //draw line using coordinates scaled to dots
            drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
          }
          else if ( mGridStyle == QgsComposerMapGrid::Markers )
          {
            drawGridMarker( intersectionPoint * dotsPerMM, context );
          }
        }
      }
    }
    if ( mGridStyle == QgsComposerMapGrid::Markers )
    {
      //markers mode, so we have no need to process horizontal lines (we've already
      //drawn markers on the intersections between horizontal and vertical lines)
      return;
    }

    hIt = horizontalLines.constBegin();
    for ( ; hIt != horizontalLines.constEnd(); ++hIt )
    {
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

  if ( testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) )
  {
    drawGridFrameBorder( p, leftGridFrame, QgsComposerMapGrid::Left );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameRight ) )
  {
    drawGridFrameBorder( p, rightGridFrame, QgsComposerMapGrid::Right );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameTop ) )
  {
    drawGridFrameBorder( p, topGridFrame, QgsComposerMapGrid::Top );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) )
  {
    drawGridFrameBorder( p, bottomGridFrame, QgsComposerMapGrid::Bottom );
  }
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

void QgsComposerMapGrid::drawGridFrameBorder( QPainter* p, const QMap< double, double >& borderPos, QgsComposerMapGrid::BorderSide border ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  switch ( mGridFrameStyle )
  {
    case QgsComposerMapGrid::Zebra:
      drawGridFrameZebraBorder( p, borderPos, border );
      break;
    case QgsComposerMapGrid::InteriorTicks:
    case QgsComposerMapGrid::ExteriorTicks:
    case QgsComposerMapGrid::InteriorExteriorTicks:
      drawGridFrameTicks( p, borderPos, border );
      break;

    case QgsComposerMapGrid::LineBorder:
      drawGridFrameLineBorder( p, border );
      break;

    case QgsComposerMapGrid::NoFrame:
      break;
  }

}

void QgsComposerMapGrid::drawGridFrameZebraBorder( QPainter* p, const QMap< double, double >& borderPos, QgsComposerMapGrid::BorderSide border ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  QMap< double, double > pos = borderPos;

  double currentCoord = 0;
  if (( border == QgsComposerMapGrid::Left || border == QgsComposerMapGrid::Right ) && testFrameSideFlag( QgsComposerMapGrid::FrameTop ) )
  {
    currentCoord = - mGridFrameWidth;
    pos.insert( 0, 0 );
  }
  else if (( border == QgsComposerMapGrid::Top || border == QgsComposerMapGrid::Bottom ) && testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) )
  {
    currentCoord = - mGridFrameWidth;
    pos.insert( 0, 0 );
  }
  bool color1 = true;
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  if ( border == QgsComposerMapGrid::Left || border == QgsComposerMapGrid::Right )
  {
    pos.insert( mComposerMap->rect().height(), mComposerMap->rect().height() );
    if ( testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) )
    {
      pos.insert( mComposerMap->rect().height() + mGridFrameWidth, mComposerMap->rect().height() + mGridFrameWidth );
    }
  }
  else if ( border == QgsComposerMapGrid::Top || border == QgsComposerMapGrid::Bottom )
  {
    pos.insert( mComposerMap->rect().width(), mComposerMap->rect().width() );
    if ( testFrameSideFlag( QgsComposerMapGrid::FrameRight ) )
    {
      pos.insert( mComposerMap->rect().width() + mGridFrameWidth, mComposerMap->rect().width() + mGridFrameWidth );
    }
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
    if ( border == QgsComposerMapGrid::Left || border == QgsComposerMapGrid::Right )
    {
      height = posIt.key() - currentCoord;
      width = mGridFrameWidth;
      x = ( border == QgsComposerMapGrid::Left ) ? -mGridFrameWidth : mComposerMap->rect().width();
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == QgsComposerMapGrid::Top ) ? -mGridFrameWidth : mComposerMap->rect().height();
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    color1 = !color1;
  }
}

void QgsComposerMapGrid::drawGridFrameTicks( QPainter* p, const QMap< double, double >& borderPos, QgsComposerMapGrid::BorderSide border ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  //set pen to current frame pen
  QPen framePen = QPen( mGridFramePenColor );
  framePen.setWidthF( mGridFramePenThickness );
  framePen.setCapStyle( Qt::FlatCap );
  p->setBrush( Qt::NoBrush );
  p->setPen( framePen );

  QMap< double, double >::const_iterator posIt = borderPos.constBegin();
  for ( ; posIt != borderPos.constEnd(); ++posIt )
  {
    if ( border == QgsComposerMapGrid::Left || border == QgsComposerMapGrid::Right )
    {
      y = posIt.key();
      height = 0;
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        width = mGridFrameWidth;
        x = ( border == QgsComposerMapGrid::Left ) ? 0 : mComposerMap->rect().width() - mGridFrameWidth;
      }
      else if ( mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        width = mGridFrameWidth;
        x = ( border == QgsComposerMapGrid::Left ) ? - mGridFrameWidth : mComposerMap->rect().width();
      }
      else if ( mGridFrameStyle == QgsComposerMapGrid::InteriorExteriorTicks )
      {
        width = mGridFrameWidth * 2;
        x = ( border == QgsComposerMapGrid::Left ) ? - mGridFrameWidth : mComposerMap->rect().width() - mGridFrameWidth;
      }
    }
    else //top or bottom
    {
      x = posIt.key();
      width = 0;
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        height = mGridFrameWidth;
        y = ( border == QgsComposerMapGrid::Top ) ? 0 : mComposerMap->rect().height() - mGridFrameWidth;
      }
      else if ( mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        height = mGridFrameWidth;
        y = ( border == QgsComposerMapGrid::Top ) ? -mGridFrameWidth : mComposerMap->rect().height();
      }
      else if ( mGridFrameStyle == QgsComposerMapGrid::InteriorExteriorTicks )
      {
        height = mGridFrameWidth * 2;
        y = ( border == QgsComposerMapGrid::Top ) ? -mGridFrameWidth : mComposerMap->rect().height() - mGridFrameWidth;
      }
    }
    p->drawLine( QLineF( x, y, x + width, y + height ) );
  }
}

void QgsComposerMapGrid::drawGridFrameLineBorder( QPainter* p, QgsComposerMapGrid::BorderSide border ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  //set pen to current frame pen
  QPen framePen = QPen( mGridFramePenColor );
  framePen.setWidthF( mGridFramePenThickness );
  framePen.setCapStyle( Qt::SquareCap );
  p->setBrush( Qt::NoBrush );
  p->setPen( framePen );

  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      p->drawLine( QLineF( 0, 0, 0, mComposerMap->rect().height() ) );
      break;
    case QgsComposerMapGrid::Right:
      p->drawLine( QLineF( mComposerMap->rect().width(), 0, mComposerMap->rect().width(), mComposerMap->rect().height() ) );
      break;
    case QgsComposerMapGrid::Top:
      p->drawLine( QLineF( 0, 0, mComposerMap->rect().width(), 0 ) );
      break;
    case QgsComposerMapGrid::Bottom:
      p->drawLine( QLineF( 0, mComposerMap->rect().height(), mComposerMap->rect().width(), mComposerMap->rect().height() ) );
      break;
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
    currentAnnotationString = gridAnnotationString( it->first, QgsComposerMapGrid::Latitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsComposerMapGrid::Latitude );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsComposerMapGrid::Latitude );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString =  gridAnnotationString( it->first, QgsComposerMapGrid::Longitude );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsComposerMapGrid::Longitude );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsComposerMapGrid::Longitude );
  }
}

void QgsComposerMapGrid::drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString, const AnnotationCoordinate coordinateType ) const
{
  if ( !mComposerMap )
  {
    return;
  }
  QgsComposerMapGrid::BorderSide frameBorder = borderForLineCoord( pos, coordinateType );
  double textWidth = QgsComposerUtils::textWidthMM( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = QgsComposerUtils::fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  double gridFrameDistance = 0;
  if ( mGridFrameStyle != QgsComposerMapGrid::NoFrame && mGridFrameStyle != QgsComposerMapGrid::LineBorder )
  {
    gridFrameDistance = mGridFrameWidth;
  }
  if ( mGridFrameStyle == QgsComposerMapGrid::Zebra || mGridFrameStyle == QgsComposerMapGrid::LineBorder )
  {
    gridFrameDistance += ( mGridFramePenThickness / 2.0 );
  }

  if ( frameBorder == QgsComposerMapGrid::Left )
  {
    if ( mLeftGridAnnotationDisplay == QgsComposerMapGrid::HideAll ||
         ( coordinateType == Longitude && mLeftGridAnnotationDisplay == QgsComposerMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mLeftGridAnnotationDisplay == QgsComposerMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) )
    {
      gridFrameDistance = 0;
    }

    if ( mLeftGridAnnotationPosition == QgsComposerMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::Zebra || mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mLeftGridAnnotationDirection == QgsComposerMapGrid::Vertical || mLeftGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance + gridFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mLeftGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos -= textWidth / 2.0;
        rotation = 90;
      }
      else
      {
        xpos += mAnnotationFrameDistance + gridFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else if ( mLeftGridAnnotationPosition == QgsComposerMapGrid::OutsideMapFrame ) //Outside map frame
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mLeftGridAnnotationDirection == QgsComposerMapGrid::Vertical || mLeftGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos -= ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mLeftGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos -= textHeight + mAnnotationFrameDistance + gridFrameDistance;
        ypos -= textWidth / 2.0;
        rotation = 90;
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
  else if ( frameBorder == QgsComposerMapGrid::Right )
  {
    if ( mRightGridAnnotationDisplay == QgsComposerMapGrid::HideAll ||
         ( coordinateType == Longitude && mRightGridAnnotationDisplay == QgsComposerMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mRightGridAnnotationDisplay == QgsComposerMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsComposerMapGrid::FrameRight ) )
    {
      gridFrameDistance = 0;
    }

    if ( mRightGridAnnotationPosition == QgsComposerMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::Zebra || mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mRightGridAnnotationDirection == QgsComposerMapGrid::Vertical )
      {
        xpos -= mAnnotationFrameDistance + gridFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mRightGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending || mRightGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos -= textHeight + mAnnotationFrameDistance + gridFrameDistance;
        ypos -= textWidth / 2.0;
        rotation = 90;
      }
      else
      {
        xpos -= textWidth + mAnnotationFrameDistance + gridFrameDistance;
        ypos += textHeight / 2.0;
      }
    }
    else if ( mRightGridAnnotationPosition == QgsComposerMapGrid::OutsideMapFrame )//OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mRightGridAnnotationDirection == QgsComposerMapGrid::Vertical )
      {
        xpos += ( textHeight + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mRightGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending || mRightGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos -= textWidth / 2.0;
        rotation = 90;
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
  else if ( frameBorder == QgsComposerMapGrid::Bottom )
  {
    if ( mBottomGridAnnotationDisplay == QgsComposerMapGrid::HideAll ||
         ( coordinateType == Longitude && mBottomGridAnnotationDisplay == QgsComposerMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mBottomGridAnnotationDisplay == QgsComposerMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) )
    {
      gridFrameDistance = 0;
    }

    if ( mBottomGridAnnotationPosition == QgsComposerMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::Zebra || mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mBottomGridAnnotationDirection == QgsComposerMapGrid::Horizontal || mBottomGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        ypos -= mAnnotationFrameDistance + gridFrameDistance;
        xpos -= textWidth / 2.0;
      }
      else if ( mBottomGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos -= textWidth + mAnnotationFrameDistance + gridFrameDistance;
        rotation = 90;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= mAnnotationFrameDistance + gridFrameDistance;
        rotation = 270;
      }
    }
    else if ( mBottomGridAnnotationPosition == QgsComposerMapGrid::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mBottomGridAnnotationDirection == QgsComposerMapGrid::Horizontal || mBottomGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        ypos += ( mAnnotationFrameDistance + textHeight + gridFrameDistance );
        xpos -= textWidth / 2.0;
      }
      else if ( mBottomGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos += gridFrameDistance + mAnnotationFrameDistance;
        rotation = 90;
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
  else //top
  {
    if ( mTopGridAnnotationDisplay == QgsComposerMapGrid::HideAll ||
         ( coordinateType == Longitude && mTopGridAnnotationDisplay == QgsComposerMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mTopGridAnnotationDisplay == QgsComposerMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsComposerMapGrid::FrameTop ) )
    {
      gridFrameDistance = 0;
    }

    if ( mTopGridAnnotationPosition == QgsComposerMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::Zebra || mGridFrameStyle == QgsComposerMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mTopGridAnnotationDirection == QgsComposerMapGrid::Horizontal || mTopGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos += textHeight + mAnnotationFrameDistance + gridFrameDistance;
      }
      else if ( mTopGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos += mAnnotationFrameDistance + gridFrameDistance;
        rotation = 90;
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += textWidth + mAnnotationFrameDistance + gridFrameDistance;
        rotation = 270;
      }
    }
    else if ( mTopGridAnnotationPosition == QgsComposerMapGrid::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsComposerMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mTopGridAnnotationDirection == QgsComposerMapGrid::Horizontal || mTopGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
      }
      else if ( mTopGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos -= textWidth + mAnnotationFrameDistance + gridFrameDistance;
        rotation = 90;
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

QString QgsComposerMapGrid::gridAnnotationString( double value, QgsComposerMapGrid::AnnotationCoordinate coord ) const
{
  if ( mGridAnnotationFormat == QgsComposerMapGrid::Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DecimalWithSuffix )
  {
    QString hemisphere;

    //check if we are using degrees (ie, geographic crs)
    bool geographic = false;
    if ( mCRS.isValid() && mCRS.geographicFlag() )
    {
      geographic = true;
    }
    else if ( mComposerMap && mComposerMap->composition() )
    {
      geographic = mComposerMap->composition()->mapSettings().destinationCrs().geographicFlag();
    }

    double coordRounded = qRound( value * pow( 10.0, mGridAnnotationPrecision ) ) / pow( 10.0, mGridAnnotationPrecision );
    if ( coord == QgsComposerMapGrid::Longitude )
    {
      //don't use E/W suffixes if ambiguous (eg 180 degrees)
      if ( !geographic || ( coordRounded != 180.0 && coordRounded != 0.0 ) )
      {
        hemisphere = value < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
      }
    }
    else
    {
      //don't use N/S suffixes if ambiguous (eg 0 degrees)
      if ( !geographic || coordRounded != 0.0 )
      {
        hemisphere = value < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
      }
    }
    if ( geographic )
    {
      //insert degree symbol for geographic coordinates
      return QString::number( qAbs( value ), 'f', mGridAnnotationPrecision ) + QChar( 176 ) + hemisphere;
    }
    else
    {
      return QString::number( qAbs( value ), 'f', mGridAnnotationPrecision ) + hemisphere;
    }
  }

  QgsPoint p;
  p.setX( coord == QgsComposerMapGrid::Longitude ? value : 0 );
  p.setY( coord == QgsComposerMapGrid::Longitude ? 0 : value );

  QString annotationString;
  if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinute )
  {
    annotationString = p.toDegreesMinutes( mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinuteNoSuffix )
  {
    annotationString = p.toDegreesMinutes( mGridAnnotationPrecision, false );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinutePadded )
  {
    annotationString = p.toDegreesMinutes( mGridAnnotationPrecision, true, true );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinuteSecond )
  {
    annotationString = p.toDegreesMinutesSeconds( mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinuteSecondNoSuffix )
  {
    annotationString = p.toDegreesMinutesSeconds( mGridAnnotationPrecision, false );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DegreeMinuteSecondPadded )
  {
    annotationString = p.toDegreesMinutesSeconds( mGridAnnotationPrecision, true, true );
  }

  QStringList split = annotationString.split( "," );
  if ( coord == QgsComposerMapGrid::Longitude )
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

  bool crosses180 = false;
  bool crossed180 = false;
  if ( mCRS.geographicFlag() && ( minX > maxX ) )
  {
    //handle 180 degree longitude crossover
    crosses180 = true;
    step = ( maxX + 360.0 - minX ) / 20;
  }

  int gridLineCount = 0;
  while ( currentLevel >= bbox.yMinimum() && gridLineCount < MAX_GRID_LINES )
  {
    QPolygonF gridLine;
    double currentX = minX;
    bool cont = true;
    while ( cont )
    {
      if (( !crosses180 || crossed180 ) && ( currentX > maxX ) )
      {
        cont = false;
      }

      try
      {
        QgsPoint mapPoint = t.transform( currentX, currentLevel ); //transform back to map crs
        gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) ); //transform back to composer coords
      }
      catch ( QgsCsException & cse )
      {
        QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentX += step;
      if ( crosses180 && currentX > 180.0 )
      {
        currentX -= 360.0;
        crossed180 = true;
      }
    }
    crossed180 = false;

    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if (( *lineIt ).size() > 0 )
      {
        lines.append( qMakePair( currentLevel, *lineIt ) );
        gridLineCount++;
      }
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

  bool crosses180 = false;
  bool crossed180 = false;
  if ( mCRS.geographicFlag() && ( bbox.xMinimum() > bbox.xMaximum() ) )
  {
    //handle 180 degree longitude crossover
    crosses180 = true;
  }

  int gridLineCount = 0;
  while (( currentLevel <= bbox.xMaximum() || ( crosses180 && !crossed180 ) ) && gridLineCount < MAX_GRID_LINES )
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
      try
      {
        //transform back to map crs
        QgsPoint mapPoint = t.transform( currentLevel, currentY );
        //transform back to composer coords
        gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) );
      }
      catch ( QgsCsException & cse )
      {
        QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentY += step;
    }
    //clip grid line to map polygon
    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if (( *lineIt ).size() > 0 )
      {
        lines.append( qMakePair( currentLevel, *lineIt ) );
        gridLineCount++;
      }
    }
    currentLevel += mGridIntervalX;
    if ( crosses180 && currentLevel > 180.0 )
    {
      currentLevel -= 360.0;
      crossed180 = true;
    }
  }

  return 0;
}

void QgsComposerMapGrid::sortGridLinesOnBorders( const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines, QMap< double, double >& leftFrameEntries,
    QMap< double, double >& rightFrameEntries, QMap< double, double >& topFrameEntries, QMap< double, double >& bottomFrameEntries ) const
{
  QList< QgsMapAnnotation > borderPositions;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    QgsMapAnnotation p1;
    p1.coordinate = it->first;
    p1.itemPosition = it->second.p1();
    p1.coordinateType = QgsComposerMapGrid::Latitude;
    borderPositions << p1;

    QgsMapAnnotation p2;
    p2.coordinate = it->first;
    p2.itemPosition = it->second.p2();
    p2.coordinateType = QgsComposerMapGrid::Latitude;
    borderPositions << p2;
  }
  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    QgsMapAnnotation p1;
    p1.coordinate = it->first;
    p1.itemPosition = it->second.p1();
    p1.coordinateType = QgsComposerMapGrid::Longitude;
    borderPositions << p1;

    QgsMapAnnotation p2;
    p2.coordinate = it->first;
    p2.itemPosition = it->second.p2();
    p2.coordinateType = QgsComposerMapGrid::Longitude;
    borderPositions << p2;
  }

  QList< QgsMapAnnotation >::const_iterator bIt = borderPositions.constBegin();
  for ( ; bIt != borderPositions.constEnd(); ++bIt )
  {
    QgsComposerMapGrid::BorderSide frameBorder = borderForLineCoord( bIt->itemPosition, bIt->coordinateType );
    if ( frameBorder == QgsComposerMapGrid::Left && shouldShowDivisionForSide( bIt->coordinateType, QgsComposerMapGrid::Left ) )
    {
      leftFrameEntries.insert( bIt->itemPosition.y(), bIt->coordinate );
    }
    else if ( frameBorder == QgsComposerMapGrid::Right && shouldShowDivisionForSide( bIt->coordinateType, QgsComposerMapGrid::Right ) )
    {
      rightFrameEntries.insert( bIt->itemPosition.y(), bIt->coordinate );
    }
    else if ( frameBorder == QgsComposerMapGrid::Top && shouldShowDivisionForSide( bIt->coordinateType, QgsComposerMapGrid::Top ) )
    {
      topFrameEntries.insert( bIt->itemPosition.x(), bIt->coordinate );
    }
    else if ( frameBorder == QgsComposerMapGrid::Bottom && shouldShowDivisionForSide( bIt->coordinateType, QgsComposerMapGrid::Bottom ) )
    {
      bottomFrameEntries.insert( bIt->itemPosition.x(), bIt->coordinate );
    }
  }
}

bool QgsComposerMapGrid::shouldShowDivisionForSide( const QgsComposerMapGrid::AnnotationCoordinate& coordinate, const QgsComposerMapGrid::BorderSide& side ) const
{
  switch ( side )
  {
    case QgsComposerMapGrid::Left:
      return shouldShowDivisionForDisplayMode( coordinate, mLeftFrameDivisions );
    case QgsComposerMapGrid::Right:
      return shouldShowDivisionForDisplayMode( coordinate, mRightFrameDivisions );
    case QgsComposerMapGrid::Top:
      return shouldShowDivisionForDisplayMode( coordinate, mTopFrameDivisions );
    case QgsComposerMapGrid::Bottom:
    default: //prevent warnings
      return shouldShowDivisionForDisplayMode( coordinate, mBottomFrameDivisions );
  }
}

bool QgsComposerMapGrid::shouldShowDivisionForDisplayMode( const QgsComposerMapGrid::AnnotationCoordinate& coordinate, const QgsComposerMapGrid::DisplayMode& mode ) const
{
  return mode == QgsComposerMapGrid::ShowAll
         || ( mode == QgsComposerMapGrid::LatitudeOnly && coordinate == QgsComposerMapGrid::Latitude )
         || ( mode == QgsComposerMapGrid::LongitudeOnly && coordinate == QgsComposerMapGrid::Longitude );
}

bool sortByDistance( const QPair<double, QgsComposerMapGrid::BorderSide>& a, const QPair<double, QgsComposerMapGrid::BorderSide>& b )
{
  return a.first < b.first;
}

QgsComposerMapGrid::BorderSide QgsComposerMapGrid::borderForLineCoord( const QPointF& p, const AnnotationCoordinate coordinateType ) const
{
  if ( !mComposerMap )
  {
    return QgsComposerMapGrid::Left;
  }

  double tolerance = qMax( mComposerMap->hasFrame() ? mComposerMap->pen().widthF() : 0.0, 1.0 );

  //check for corner coordinates
  if (( p.y() <= tolerance && p.x() <= tolerance )  // top left
      || ( p.y() <= tolerance && p.x() >= ( mComposerMap->rect().width() - tolerance ) ) //top right
      || ( p.y() >= ( mComposerMap->rect().height() - tolerance ) && p.x() <= tolerance ) //bottom left
      || ( p.y() >= ( mComposerMap->rect().height() - tolerance ) && p.x() >= ( mComposerMap->rect().width() - tolerance ) ) //bottom right
     )
  {
    //coordinate is in corner - fall back to preferred side for coordinate type
    if ( coordinateType == QgsComposerMapGrid::Latitude )
    {
      if ( p.x() <= tolerance )
      {
        return QgsComposerMapGrid::Left;
      }
      else
      {
        return QgsComposerMapGrid::Right;
      }
    }
    else
    {
      if ( p.y() <= tolerance )
      {
        return QgsComposerMapGrid::Top;
      }
      else
      {
        return QgsComposerMapGrid::Bottom;
      }
    }
  }

  //otherwise, guess side based on closest map side to point
  QList< QPair<double, QgsComposerMapGrid::BorderSide > > distanceToSide;
  distanceToSide << qMakePair( p.x(), QgsComposerMapGrid::Left );
  distanceToSide << qMakePair( mComposerMap->rect().width() - p.x(), QgsComposerMapGrid::Right );
  distanceToSide << qMakePair( p.y(), QgsComposerMapGrid::Top );
  distanceToSide << qMakePair( mComposerMap->rect().height() - p.y(), QgsComposerMapGrid::Bottom );

  qSort( distanceToSide.begin(), distanceToSide.end(), sortByDistance );
  return distanceToSide.at( 0 ).second;
}

void QgsComposerMapGrid::setLineSymbol( QgsLineSymbolV2* symbol )
{
  delete mGridLineSymbol;
  mGridLineSymbol = symbol;
}

void QgsComposerMapGrid::setMarkerSymbol( QgsMarkerSymbolV2 *symbol )
{
  delete mGridMarkerSymbol;
  mGridMarkerSymbol = symbol;
}

void QgsComposerMapGrid::setAnnotationDisplay( const QgsComposerMapGrid::DisplayMode display, const QgsComposerMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      mLeftGridAnnotationDisplay = display;
      break;
    case QgsComposerMapGrid::Right:
      mRightGridAnnotationDisplay = display;
      break;
    case QgsComposerMapGrid::Top:
      mTopGridAnnotationDisplay = display;
      break;
    case QgsComposerMapGrid::Bottom:
      mBottomGridAnnotationDisplay = display;
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

QgsComposerMapGrid::DisplayMode QgsComposerMapGrid::annotationDisplay( const QgsComposerMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      return mLeftGridAnnotationDisplay;
      break;
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationDisplay;
      break;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationDisplay;
      break;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDisplay;
      break;
  }
}

double QgsComposerMapGrid::maxExtension() const
{
  if ( !mComposerMap )
  {
    return 0;
  }

  if ( !mEnabled || ( mGridFrameStyle == QgsComposerMapGrid::NoFrame && ( !mShowGridAnnotation ||
                      (( mLeftGridAnnotationPosition != QgsComposerMapGrid::OutsideMapFrame || mLeftGridAnnotationDisplay == QgsComposerMapGrid::HideAll ) &&
                       ( mRightGridAnnotationPosition != QgsComposerMapGrid::OutsideMapFrame || mRightGridAnnotationDisplay == QgsComposerMapGrid::HideAll ) &&
                       ( mTopGridAnnotationPosition != QgsComposerMapGrid::OutsideMapFrame || mTopGridAnnotationDisplay == QgsComposerMapGrid::HideAll ) &&
                       ( mBottomGridAnnotationPosition != QgsComposerMapGrid::OutsideMapFrame || mBottomGridAnnotationDisplay == QgsComposerMapGrid::HideAll ) ) ) ) )
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
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMapGrid::Latitude ) );
    }
    it = yGridLines.constBegin();
    for ( ; it != yGridLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMapGrid::Longitude ) );
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
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMapGrid::Latitude ) );
    }

    it = yLines.constBegin();
    for ( ; it != yLines.constEnd(); ++it )
    {
      coordStrings.append( gridAnnotationString( it->first, QgsComposerMapGrid::Longitude ) );
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
  double gridFrameDist = ( mGridFrameStyle == QgsComposerMapGrid::NoFrame ) ? 0 : mGridFrameWidth + ( mGridFramePenThickness / 2.0 );
  return maxExtension + mAnnotationFrameDistance + gridFrameDist;
}

void QgsComposerMapGrid::setUnits( const QgsComposerMapGrid::GridUnit unit )
{
  if ( unit == mGridUnit )
  {
    return;
  }
  mGridUnit = unit;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setIntervalX( const double interval )
{
  if ( interval == mGridIntervalX )
  {
    return;
  }
  mGridIntervalX = interval;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setIntervalY( const double interval )
{
  if ( interval == mGridIntervalY )
  {
    return;
  }
  mGridIntervalY = interval;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setOffsetX( const double offset )
{
  if ( offset == mGridOffsetX )
  {
    return;
  }
  mGridOffsetX = offset;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setOffsetY( const double offset )
{
  if ( offset == mGridOffsetY )
  {
    return;
  }
  mGridOffsetY = offset;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setStyle( const QgsComposerMapGrid::GridStyle style )
{
  if ( style == mGridStyle )
  {
    return;
  }
  mGridStyle = style;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setAnnotationDirection( const QgsComposerMapGrid::AnnotationDirection direction, const QgsComposerMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      mLeftGridAnnotationDirection = direction;
      break;
    case QgsComposerMapGrid::Right:
      mRightGridAnnotationDirection = direction;
      break;
    case QgsComposerMapGrid::Top:
      mTopGridAnnotationDirection = direction;
      break;
    case QgsComposerMapGrid::Bottom:
      mBottomGridAnnotationDirection = direction;
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

void QgsComposerMapGrid::setFrameSideFlags( FrameSideFlags flags )
{
  mGridFrameSides = flags;
}

void QgsComposerMapGrid::setFrameSideFlag( QgsComposerMapGrid::FrameSideFlag flag, bool on )
{
  if ( on )
    mGridFrameSides |= flag;
  else
    mGridFrameSides &= ~flag;
}

QgsComposerMapGrid::FrameSideFlags QgsComposerMapGrid::frameSideFlags() const
{
  return mGridFrameSides;
}

bool QgsComposerMapGrid::testFrameSideFlag( QgsComposerMapGrid::FrameSideFlag flag ) const
{
  return mGridFrameSides.testFlag( flag );
}

void QgsComposerMapGrid::setAnnotationDirection( const AnnotationDirection direction )
{
  mLeftGridAnnotationDirection = direction;
  mRightGridAnnotationDirection = direction;
  mTopGridAnnotationDirection = direction;
  mBottomGridAnnotationDirection = direction;
}

void QgsComposerMapGrid::setAnnotationPosition( const AnnotationPosition position, const BorderSide border )
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      mLeftGridAnnotationPosition = position;
      break;
    case QgsComposerMapGrid::Right:
      mRightGridAnnotationPosition = position;
      break;
    case QgsComposerMapGrid::Top:
      mTopGridAnnotationPosition = position;
      break;
    case QgsComposerMapGrid::Bottom:
      mBottomGridAnnotationPosition = position;
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

QgsComposerMapGrid::AnnotationPosition QgsComposerMapGrid::annotationPosition( const QgsComposerMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      return mLeftGridAnnotationPosition;
      break;
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationPosition;
      break;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationPosition;
      break;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationPosition;
      break;
  }
}

QgsComposerMapGrid::AnnotationDirection QgsComposerMapGrid::annotationDirection( const BorderSide border ) const
{
  if ( !mComposerMap )
  {
    return mLeftGridAnnotationDirection;
  }

  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      return mLeftGridAnnotationDirection;
      break;
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationDirection;
      break;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationDirection;
      break;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDirection;
      break;
  }
}

void QgsComposerMapGrid::setFrameDivisions( const QgsComposerMapGrid::DisplayMode divisions, const QgsComposerMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      mLeftFrameDivisions = divisions;
      break;
    case QgsComposerMapGrid::Right:
      mRightFrameDivisions = divisions;
      break;
    case QgsComposerMapGrid::Top:
      mTopFrameDivisions = divisions;
      break;
    case QgsComposerMapGrid::Bottom:
      mBottomFrameDivisions = divisions;
      break;
    default:
      return;
  }

  if ( mComposerMap )
  {
    mComposerMap->update();
  }
}

QgsComposerMapGrid::DisplayMode QgsComposerMapGrid::frameDivisions( const QgsComposerMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsComposerMapGrid::Left:
      return mLeftFrameDivisions;
      break;
    case QgsComposerMapGrid::Right:
      return mRightFrameDivisions;
      break;
    case QgsComposerMapGrid::Top:
      return mTopFrameDivisions;
      break;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomFrameDivisions;
      break;
  }
}

int QgsComposerMapGrid::crsGridParams( QgsRectangle& crsRect, QgsCoordinateTransform& inverseTransform ) const
{
  if ( !mComposerMap )
  {
    return 1;
  }

  try
  {
    QgsCoordinateTransform tr( mComposerMap->composition()->mapSettings().destinationCrs(), mCRS );
    QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
    QRectF mbr = mapPolygon.boundingRect();
    QgsRectangle mapBoundingRect( mbr.left(), mbr.bottom(), mbr.right(), mbr.top() );


    if ( mCRS.geographicFlag() )
    {
      //handle crossing the 180 degree longitude line
      QgsPoint lowerLeft( mapBoundingRect.xMinimum(), mapBoundingRect.yMinimum() );
      QgsPoint upperRight( mapBoundingRect.xMaximum(), mapBoundingRect.yMaximum() );

      lowerLeft = tr.transform( lowerLeft.x(), lowerLeft.y() );
      upperRight = tr.transform( upperRight.x(), upperRight.y() );

      if ( lowerLeft.x() > upperRight.x() )
      {
        //we've crossed the line
        crsRect = tr.transformBoundingBox( mapBoundingRect, QgsCoordinateTransform::ForwardTransform, true );
      }
      else
      {
        //didn't cross the line
        crsRect = tr.transformBoundingBox( mapBoundingRect );
      }
    }
    else
    {
      crsRect = tr.transformBoundingBox( mapBoundingRect );
    }

    inverseTransform.setSourceCrs( mCRS );
    inverseTransform.setDestCRS( mComposerMap->composition()->mapSettings().destinationCrs() );
  }
  catch ( QgsCsException & cse )
  {
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
    return 1;
  }
  return 0;
}

QList<QPolygonF> QgsComposerMapGrid::trimLinesToMap( const QPolygonF& line, const QgsRectangle& rect )
{
  QgsGeometry* lineGeom = QgsGeometry::fromQPolygonF( line );
  QgsGeometry* rectGeom = QgsGeometry::fromRect( rect );

  QgsGeometry* intersected = lineGeom->intersection( rectGeom );
  QList<QgsGeometry*> intersectedParts = intersected->asGeometryCollection();

  QList<QPolygonF> trimmedLines;
  QList<QgsGeometry*>::const_iterator geomIt = intersectedParts.constBegin();
  for ( ; geomIt != intersectedParts.constEnd(); ++geomIt )
  {
    trimmedLines << ( *geomIt )->asQPolygonF();
  }

  qDeleteAll( intersectedParts );
  intersectedParts.clear();
  delete intersected;
  delete lineGeom;
  delete rectGeom;
  return trimmedLines;
}
