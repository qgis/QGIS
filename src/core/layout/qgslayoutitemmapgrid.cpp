/***************************************************************************
                         qgslayoutitemmapgrid.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Marco Hugentobler, Nyall Dawson
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

#include "qgslayoutitemmapgrid.h"
#include "qgslayoututils.h"
#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgsmapsettings.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsfontutils.h"
#include "qgsexpressioncontext.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgscoordinateformatter.h"

#include <QPainter>
#include <QPen>

#define MAX_GRID_LINES 1000 //maximum number of horizontal or vertical grid lines to draw

QgsLayoutItemMapGridStack::QgsLayoutItemMapGridStack( QgsLayoutItemMap *map )
  : QgsLayoutItemMapItemStack( map )
{

}

void QgsLayoutItemMapGridStack::addGrid( QgsLayoutItemMapGrid *grid )
{
  QgsLayoutItemMapItemStack::addItem( grid );
}

void QgsLayoutItemMapGridStack::removeGrid( const QString &gridId )
{
  QgsLayoutItemMapItemStack::removeItem( gridId );
}

void QgsLayoutItemMapGridStack::moveGridUp( const QString &gridId )
{
  QgsLayoutItemMapItemStack::moveItemUp( gridId );
}

void QgsLayoutItemMapGridStack::moveGridDown( const QString &gridId )
{
  QgsLayoutItemMapItemStack::moveItemDown( gridId );
}

QgsLayoutItemMapGrid *QgsLayoutItemMapGridStack::grid( const QString &gridId ) const
{
  QgsLayoutItemMapItem *item = QgsLayoutItemMapItemStack::item( gridId );
  return dynamic_cast<QgsLayoutItemMapGrid *>( item );
}

QgsLayoutItemMapGrid *QgsLayoutItemMapGridStack::grid( const int index ) const
{
  QgsLayoutItemMapItem *item = QgsLayoutItemMapItemStack::item( index );
  return dynamic_cast<QgsLayoutItemMapGrid *>( item );
}

QList<QgsLayoutItemMapGrid *> QgsLayoutItemMapGridStack::asList() const
{
  QList< QgsLayoutItemMapGrid * > list;
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( QgsLayoutItemMapGrid *grid = dynamic_cast<QgsLayoutItemMapGrid *>( item ) )
    {
      list.append( grid );
    }
  }
  return list;
}

QgsLayoutItemMapGrid &QgsLayoutItemMapGridStack::operator[]( int idx )
{
  QgsLayoutItemMapItem *item = mItems.at( idx );
  QgsLayoutItemMapGrid *grid = dynamic_cast<QgsLayoutItemMapGrid *>( item );
  return *grid;
}

bool QgsLayoutItemMapGridStack::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  removeItems();

  //read grid stack
  QDomNodeList mapGridNodeList = elem.elementsByTagName( QStringLiteral( "ComposerMapGrid" ) );
  for ( int i = 0; i < mapGridNodeList.size(); ++i )
  {
    QDomElement mapGridElem = mapGridNodeList.at( i ).toElement();
    QgsLayoutItemMapGrid *mapGrid = new QgsLayoutItemMapGrid( mapGridElem.attribute( QStringLiteral( "name" ) ), mMap );
    mapGrid->readXml( mapGridElem, doc, context );
    mItems.append( mapGrid );
  }

  return true;
}

double QgsLayoutItemMapGridStack::maxGridExtension() const
{
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  double left = 0.0;
  calculateMaxGridExtension( top, right, bottom, left );
  return std::max( std::max( std::max( top, right ), bottom ), left );
}

void QgsLayoutItemMapGridStack::calculateMaxGridExtension( double &top, double &right, double &bottom, double &left ) const
{
  top = 0.0;
  right = 0.0;
  bottom = 0.0;
  left = 0.0;

  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( QgsLayoutItemMapGrid *grid = dynamic_cast<QgsLayoutItemMapGrid *>( item ) )
    {
      double gridTop = 0.0;
      double gridRight = 0.0;
      double gridBottom = 0.0;
      double gridLeft = 0.0;
      grid->calculateMaxExtension( gridTop, gridRight, gridBottom, gridLeft );
      top = std::max( top, gridTop );
      right = std::max( right, gridRight );
      bottom = std::max( bottom, gridBottom );
      left = std::max( left, gridLeft );
    }
  }
}


//
// QgsLayoutItemMapGrid
//


QgsLayoutItemMapGrid::QgsLayoutItemMapGrid( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutItemMapItem( name, map )
  , mGridFrameSides( QgsLayoutItemMapGrid::FrameLeft | QgsLayoutItemMapGrid::FrameRight |
                     QgsLayoutItemMapGrid::FrameTop | QgsLayoutItemMapGrid::FrameBottom )
{
  //get default layout font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mGridAnnotationFont.setFamily( defaultFontString );
  }

  createDefaultGridLineSymbol();
  createDefaultGridMarkerSymbol();
}

void QgsLayoutItemMapGrid::createDefaultGridLineSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  mGridLineSymbol.reset( QgsLineSymbol::createSimple( properties ) );
}

void QgsLayoutItemMapGrid::createDefaultGridMarkerSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "2.0" ) );
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  mGridMarkerSymbol.reset( QgsMarkerSymbol::createSimple( properties ) );
}

void QgsLayoutItemMapGrid::setGridLineWidth( const double width )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setWidth( width );
  }
}

void QgsLayoutItemMapGrid::setGridLineColor( const QColor &c )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setColor( c );
  }
}

bool QgsLayoutItemMapGrid::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement mapGridElem = doc.createElement( QStringLiteral( "ComposerMapGrid" ) );
  mapGridElem.setAttribute( QStringLiteral( "gridStyle" ), mGridStyle );
  mapGridElem.setAttribute( QStringLiteral( "intervalX" ), qgsDoubleToString( mGridIntervalX ) );
  mapGridElem.setAttribute( QStringLiteral( "intervalY" ), qgsDoubleToString( mGridIntervalY ) );
  mapGridElem.setAttribute( QStringLiteral( "offsetX" ), qgsDoubleToString( mGridOffsetX ) );
  mapGridElem.setAttribute( QStringLiteral( "offsetY" ), qgsDoubleToString( mGridOffsetY ) );
  mapGridElem.setAttribute( QStringLiteral( "crossLength" ), qgsDoubleToString( mCrossLength ) );

  QDomElement lineStyleElem = doc.createElement( QStringLiteral( "lineStyle" ) );
  QDomElement gridLineStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridLineSymbol.get(), doc, context );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( QStringLiteral( "markerStyle" ) );
  QDomElement gridMarkerStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridMarkerSymbol.get(), doc, context );
  markerStyleElem.appendChild( gridMarkerStyleElem );
  mapGridElem.appendChild( markerStyleElem );

  mapGridElem.setAttribute( QStringLiteral( "gridFrameStyle" ), mGridFrameStyle );
  mapGridElem.setAttribute( QStringLiteral( "gridFrameSideFlags" ), mGridFrameSides );
  mapGridElem.setAttribute( QStringLiteral( "gridFrameWidth" ), qgsDoubleToString( mGridFrameWidth ) );
  mapGridElem.setAttribute( QStringLiteral( "gridFrameMargin" ), qgsDoubleToString( mGridFrameMargin ) );
  mapGridElem.setAttribute( QStringLiteral( "gridFramePenThickness" ), qgsDoubleToString( mGridFramePenThickness ) );
  mapGridElem.setAttribute( QStringLiteral( "gridFramePenColor" ), QgsSymbolLayerUtils::encodeColor( mGridFramePenColor ) );
  mapGridElem.setAttribute( QStringLiteral( "frameFillColor1" ), QgsSymbolLayerUtils::encodeColor( mGridFrameFillColor1 ) );
  mapGridElem.setAttribute( QStringLiteral( "frameFillColor2" ), QgsSymbolLayerUtils::encodeColor( mGridFrameFillColor2 ) );
  mapGridElem.setAttribute( QStringLiteral( "leftFrameDivisions" ), mLeftFrameDivisions );
  mapGridElem.setAttribute( QStringLiteral( "rightFrameDivisions" ), mRightFrameDivisions );
  mapGridElem.setAttribute( QStringLiteral( "topFrameDivisions" ), mTopFrameDivisions );
  mapGridElem.setAttribute( QStringLiteral( "bottomFrameDivisions" ), mBottomFrameDivisions );
  if ( mCRS.isValid() )
  {
    mCRS.writeXml( mapGridElem, doc );
  }

  mapGridElem.setAttribute( QStringLiteral( "annotationFormat" ), mGridAnnotationFormat );
  mapGridElem.setAttribute( QStringLiteral( "showAnnotation" ), mShowGridAnnotation );
  mapGridElem.setAttribute( QStringLiteral( "annotationExpression" ), mGridAnnotationExpressionString );
  mapGridElem.setAttribute( QStringLiteral( "leftAnnotationDisplay" ), mLeftGridAnnotationDisplay );
  mapGridElem.setAttribute( QStringLiteral( "rightAnnotationDisplay" ), mRightGridAnnotationDisplay );
  mapGridElem.setAttribute( QStringLiteral( "topAnnotationDisplay" ), mTopGridAnnotationDisplay );
  mapGridElem.setAttribute( QStringLiteral( "bottomAnnotationDisplay" ), mBottomGridAnnotationDisplay );
  mapGridElem.setAttribute( QStringLiteral( "leftAnnotationPosition" ), mLeftGridAnnotationPosition );
  mapGridElem.setAttribute( QStringLiteral( "rightAnnotationPosition" ), mRightGridAnnotationPosition );
  mapGridElem.setAttribute( QStringLiteral( "topAnnotationPosition" ), mTopGridAnnotationPosition );
  mapGridElem.setAttribute( QStringLiteral( "bottomAnnotationPosition" ), mBottomGridAnnotationPosition );
  mapGridElem.setAttribute( QStringLiteral( "leftAnnotationDirection" ), mLeftGridAnnotationDirection );
  mapGridElem.setAttribute( QStringLiteral( "rightAnnotationDirection" ), mRightGridAnnotationDirection );
  mapGridElem.setAttribute( QStringLiteral( "topAnnotationDirection" ), mTopGridAnnotationDirection );
  mapGridElem.setAttribute( QStringLiteral( "bottomAnnotationDirection" ), mBottomGridAnnotationDirection );
  mapGridElem.setAttribute( QStringLiteral( "frameAnnotationDistance" ), QString::number( mAnnotationFrameDistance ) );
  mapGridElem.appendChild( QgsFontUtils::toXmlElement( mGridAnnotationFont, doc, QStringLiteral( "annotationFontProperties" ) ) );
  mapGridElem.setAttribute( QStringLiteral( "annotationFontColor" ), QgsSymbolLayerUtils::encodeColor( mGridAnnotationFontColor ) );
  mapGridElem.setAttribute( QStringLiteral( "annotationPrecision" ), mGridAnnotationPrecision );
  mapGridElem.setAttribute( QStringLiteral( "unit" ), mGridUnit );
  mapGridElem.setAttribute( QStringLiteral( "blendMode" ), mBlendMode );

  bool ok = QgsLayoutItemMapItem::writeXml( mapGridElem, doc, context );
  elem.appendChild( mapGridElem );
  return ok;
}

bool QgsLayoutItemMapGrid::readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  bool ok = QgsLayoutItemMapItem::readXml( itemElem, doc, context );

  //grid
  mGridStyle = QgsLayoutItemMapGrid::GridStyle( itemElem.attribute( QStringLiteral( "gridStyle" ), QStringLiteral( "0" ) ).toInt() );
  mGridIntervalX = itemElem.attribute( QStringLiteral( "intervalX" ), QStringLiteral( "0" ) ).toDouble();
  mGridIntervalY = itemElem.attribute( QStringLiteral( "intervalY" ), QStringLiteral( "0" ) ).toDouble();
  mGridOffsetX = itemElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble();
  mGridOffsetY = itemElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble();
  mCrossLength = itemElem.attribute( QStringLiteral( "crossLength" ), QStringLiteral( "3" ) ).toDouble();
  mGridFrameStyle = static_cast< QgsLayoutItemMapGrid::FrameStyle >( itemElem.attribute( QStringLiteral( "gridFrameStyle" ), QStringLiteral( "0" ) ).toInt() );
  mGridFrameSides = static_cast< QgsLayoutItemMapGrid::FrameSideFlags >( itemElem.attribute( QStringLiteral( "gridFrameSideFlags" ), QStringLiteral( "15" ) ).toInt() );
  mGridFrameWidth = itemElem.attribute( QStringLiteral( "gridFrameWidth" ), QStringLiteral( "2.0" ) ).toDouble();
  mGridFrameMargin = itemElem.attribute( QStringLiteral( "gridFrameMargin" ), QStringLiteral( "0.0" ) ).toDouble();
  mGridFramePenThickness = itemElem.attribute( QStringLiteral( "gridFramePenThickness" ), QStringLiteral( "0.3" ) ).toDouble();
  mGridFramePenColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "gridFramePenColor" ), QStringLiteral( "0,0,0" ) ) );
  mGridFrameFillColor1 = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "frameFillColor1" ), QStringLiteral( "255,255,255,255" ) ) );
  mGridFrameFillColor2 = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "frameFillColor2" ), QStringLiteral( "0,0,0,255" ) ) );
  mLeftFrameDivisions = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "leftFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mRightFrameDivisions = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "rightFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mTopFrameDivisions = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "topFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mBottomFrameDivisions = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "bottomFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );

  QDomElement lineStyleElem = itemElem.firstChildElement( QStringLiteral( "lineStyle" ) );
  if ( !lineStyleElem.isNull() )
  {
    QDomElement symbolElem = lineStyleElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      mGridLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    }
  }
  else
  {
    //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
    mGridLineSymbol.reset( QgsLineSymbol::createSimple( QgsStringMap() ) );
    mGridLineSymbol->setWidth( itemElem.attribute( QStringLiteral( "penWidth" ), QStringLiteral( "0" ) ).toDouble() );
    mGridLineSymbol->setColor( QColor( itemElem.attribute( QStringLiteral( "penColorRed" ), QStringLiteral( "0" ) ).toInt(),
                                       itemElem.attribute( QStringLiteral( "penColorGreen" ), QStringLiteral( "0" ) ).toInt(),
                                       itemElem.attribute( QStringLiteral( "penColorBlue" ), QStringLiteral( "0" ) ).toInt() ) );
  }

  QDomElement markerStyleElem = itemElem.firstChildElement( QStringLiteral( "markerStyle" ) );
  if ( !markerStyleElem.isNull() )
  {
    QDomElement symbolElem = markerStyleElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      mGridMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context ) );
    }
  }

  if ( !mCRS.readXml( itemElem ) )
    mCRS = QgsCoordinateReferenceSystem();

  mBlendMode = static_cast< QPainter::CompositionMode >( itemElem.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() );

  //annotation
  mShowGridAnnotation = ( itemElem.attribute( QStringLiteral( "showAnnotation" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  mGridAnnotationFormat = QgsLayoutItemMapGrid::AnnotationFormat( itemElem.attribute( QStringLiteral( "annotationFormat" ), QStringLiteral( "0" ) ).toInt() );
  mGridAnnotationExpressionString = itemElem.attribute( QStringLiteral( "annotationExpression" ) );
  mGridAnnotationExpression.reset();
  mLeftGridAnnotationPosition = QgsLayoutItemMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "leftAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationPosition = QgsLayoutItemMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "rightAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationPosition = QgsLayoutItemMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "topAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationPosition = QgsLayoutItemMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "bottomAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mLeftGridAnnotationDisplay = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "leftAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationDisplay = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "rightAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationDisplay = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "topAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationDisplay = QgsLayoutItemMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "bottomAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );

  mLeftGridAnnotationDirection = QgsLayoutItemMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "leftAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationDirection = QgsLayoutItemMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "rightAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationDirection = QgsLayoutItemMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "topAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationDirection = QgsLayoutItemMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "bottomAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mAnnotationFrameDistance = itemElem.attribute( QStringLiteral( "frameAnnotationDistance" ), QStringLiteral( "0" ) ).toDouble();
  if ( !QgsFontUtils::setFromXmlChildNode( mGridAnnotationFont, itemElem, QStringLiteral( "annotationFontProperties" ) ) )
  {
    mGridAnnotationFont.fromString( itemElem.attribute( QStringLiteral( "annotationFont" ), QLatin1String( "" ) ) );
  }
  mGridAnnotationFontColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "annotationFontColor" ), QStringLiteral( "0,0,0,255" ) ) );
  mGridAnnotationPrecision = itemElem.attribute( QStringLiteral( "annotationPrecision" ), QStringLiteral( "3" ) ).toInt();
  int gridUnitInt = itemElem.attribute( QStringLiteral( "unit" ), QString::number( MapUnit ) ).toInt();
  mGridUnit = ( gridUnitInt <= static_cast< int >( CM ) ) ? static_cast< GridUnit >( gridUnitInt ) : MapUnit;
  return ok;
}

void QgsLayoutItemMapGrid::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCRS = crs;
  mTransformDirty = true;
}

bool QgsLayoutItemMapGrid::usesAdvancedEffects() const
{
  return mBlendMode != QPainter::CompositionMode_SourceOver;
}

QPolygonF QgsLayoutItemMapGrid::scalePolygon( const QPolygonF &polygon, const double scale ) const
{
  QTransform t = QTransform::fromScale( scale, scale );
  return t.map( polygon );
}

void QgsLayoutItemMapGrid::drawGridCrsTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines, bool calculateLinesOnly ) const
{
  if ( !mMap || !mEnabled )
  {
    return;
  }

  //has map extent/scale changed?
  QPolygonF mapPolygon = mMap->transformedMapPolygon();
  if ( mapPolygon != mPrevMapPolygon )
  {
    mTransformDirty = true;
    mPrevMapPolygon = mapPolygon;
  }

  if ( mTransformDirty )
  {
    calculateCrsTransformLines();
  }

  //draw lines
  if ( !calculateLinesOnly )
  {
    if ( mGridStyle == QgsLayoutItemMapGrid::Solid )
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
    else if ( mGridStyle == QgsLayoutItemMapGrid::Cross || mGridStyle == QgsLayoutItemMapGrid::Markers )
    {
      double maxX = mMap->rect().width();
      double maxY = mMap->rect().height();

      QList< QgsPointXY >::const_iterator intersectionIt = mTransformedIntersections.constBegin();
      for ( ; intersectionIt != mTransformedIntersections.constEnd(); ++intersectionIt )
      {
        double x = intersectionIt->x();
        double y = intersectionIt->y();
        if ( mGridStyle == QgsLayoutItemMapGrid::Cross )
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
        else if ( mGridStyle == QgsLayoutItemMapGrid::Markers )
        {
          drawGridMarker( QPointF( x, y ) * dotsPerMM, context );
        }
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

void QgsLayoutItemMapGrid::calculateCrsTransformLines() const
{
  QgsRectangle crsBoundingRect;
  QgsCoordinateTransform inverseTr;
  if ( crsGridParams( crsBoundingRect, inverseTr ) != 0 )
  {
    return;
  }

  //calculate x grid lines
  mTransformedXLines.clear();
  xGridLinesCrsTransform( crsBoundingRect, inverseTr, mTransformedXLines );

  //calculate y grid lines
  mTransformedYLines.clear();
  yGridLinesCrsTransform( crsBoundingRect, inverseTr, mTransformedYLines );

  if ( mGridStyle == QgsLayoutItemMapGrid::Cross || mGridStyle == QgsLayoutItemMapGrid::Markers )
  {
    //cross or markers style - we also need to calculate intersections of lines

    //first convert lines to QgsGeometry
    QList< QgsGeometry > yLines;
    QList< QPair< double, QPolygonF > >::const_iterator yGridIt = mTransformedYLines.constBegin();
    for ( ; yGridIt != mTransformedYLines.constEnd(); ++yGridIt )
    {
      QgsPolylineXY yLine;
      for ( int i = 0; i < ( *yGridIt ).second.size(); ++i )
      {
        yLine.append( QgsPointXY( ( *yGridIt ).second.at( i ).x(), ( *yGridIt ).second.at( i ).y() ) );
      }
      yLines << QgsGeometry::fromPolylineXY( yLine );
    }
    QList< QgsGeometry > xLines;
    QList< QPair< double, QPolygonF > >::const_iterator xGridIt = mTransformedXLines.constBegin();
    for ( ; xGridIt != mTransformedXLines.constEnd(); ++xGridIt )
    {
      QgsPolylineXY xLine;
      for ( int i = 0; i < ( *xGridIt ).second.size(); ++i )
      {
        xLine.append( QgsPointXY( ( *xGridIt ).second.at( i ).x(), ( *xGridIt ).second.at( i ).y() ) );
      }
      xLines << QgsGeometry::fromPolylineXY( xLine );
    }

    //now, loop through geometries and calculate intersection points
    mTransformedIntersections.clear();
    QList< QgsGeometry >::const_iterator yLineIt = yLines.constBegin();
    for ( ; yLineIt != yLines.constEnd(); ++yLineIt )
    {
      QList< QgsGeometry >::const_iterator xLineIt = xLines.constBegin();
      for ( ; xLineIt != xLines.constEnd(); ++xLineIt )
      {
        //look for intersections between lines
        QgsGeometry intersects = ( *yLineIt ).intersection( ( *xLineIt ) );
        if ( intersects.isNull() )
          continue;

        //go through all intersections and draw grid markers/crosses
        int i = 0;
        QgsPointXY vertex = intersects.vertexAt( i );
        while ( vertex != QgsPointXY( 0, 0 ) )
        {
          mTransformedIntersections << vertex;
          i = i + 1;
          vertex = intersects.vertexAt( i );
        }
      }
    }
  }

  mTransformDirty = false;
}

void QgsLayoutItemMapGrid::draw( QPainter *p )
{
  if ( !mMap || !mEnabled )
  {
    return;
  }
  QPaintDevice *paintDevice = p->device();
  if ( !paintDevice )
  {
    return;
  }

  p->save();
  p->setCompositionMode( mBlendMode );
  p->setRenderHint( QPainter::Antialiasing, mMap->layout()->renderContext().flags() & QgsLayoutRenderContext::FlagAntialiasing );

  QRectF thisPaintRect = QRectF( 0, 0, mMap->rect().width(), mMap->rect().height() );
  p->setClipRect( thisPaintRect );
  if ( thisPaintRect != mPrevPaintRect )
  {
    //rect has changed, so need to recalculate transform
    mTransformDirty = true;
    mPrevPaintRect = thisPaintRect;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
  p->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, p );
  context.setForceVectorOutput( true );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  QList< QPair< double, QLineF > > verticalLines;
  QList< QPair< double, QLineF > > horizontalLines;

  //is grid in a different crs than map?
  if ( mGridUnit == MapUnit && mCRS.isValid() && mCRS != mMap->crs() )
  {
    drawGridCrsTransform( context, dotsPerMM, horizontalLines, verticalLines );
  }
  else
  {
    drawGridNoTransform( context, dotsPerMM, horizontalLines, verticalLines );
  }

  p->restore();

  p->setClipping( false );
#ifdef Q_OS_MAC
  //QPainter::setClipping(false) seems to be broken on OSX (#12747). So we hack around it by
  //setting a larger clip rect
  p->setClipRect( mMap->mapRectFromScene( mMap->sceneBoundingRect() ).adjusted( -10, -10, 10, 10 ) );
#endif

  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame )
  {
    drawGridFrame( p, horizontalLines, verticalLines );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines, context.expressionContext() );
  }
}

void QgsLayoutItemMapGrid::drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines, bool calculateLinesOnly ) const
{
  //get line positions
  yGridLines( verticalLines );
  xGridLines( horizontalLines );

  if ( calculateLinesOnly )
    return;

  QList< QPair< double, QLineF > >::const_iterator vIt = verticalLines.constBegin();
  QList< QPair< double, QLineF > >::const_iterator hIt = horizontalLines.constBegin();

  //simple approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsLayoutItemMapGrid::Solid )
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
  else if ( mGridStyle != QgsLayoutItemMapGrid::FrameAnnotationsOnly ) //cross or markers
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
          if ( mGridStyle == QgsLayoutItemMapGrid::Cross )
          {
            //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
            crossEnd1 = ( ( intersectionPoint - vIt->second.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength ) : intersectionPoint;
            crossEnd2 = ( ( intersectionPoint - vIt->second.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength ) : intersectionPoint;
            //draw line using coordinates scaled to dots
            drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
          }
          else if ( mGridStyle == QgsLayoutItemMapGrid::Markers )
          {
            drawGridMarker( intersectionPoint * dotsPerMM, context );
          }
        }
      }
    }
    if ( mGridStyle == QgsLayoutItemMapGrid::Markers )
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
          crossEnd1 = ( ( intersectionPoint - hIt->second.p1() ).manhattanLength() > 0.01 ) ?
                      QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, hIt->second.p1(), mCrossLength ) : intersectionPoint;
          crossEnd2 = ( ( intersectionPoint - hIt->second.p2() ).manhattanLength() > 0.01 )  ?
                      QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, hIt->second.p2(), mCrossLength ) : intersectionPoint;
          //draw line using coordinates scaled to dots
          drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
        }
      }
    }
  }
}

void QgsLayoutItemMapGrid::drawGridFrame( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, GridExtension *extension ) const
{
  if ( p )
  {
    p->save();
    p->setRenderHint( QPainter::Antialiasing );
  }

  //Sort the coordinate positions for each side
  QMap< double, double > leftGridFrame;
  QMap< double, double > rightGridFrame;
  QMap< double, double > topGridFrame;
  QMap< double, double > bottomGridFrame;

  sortGridLinesOnBorders( hLines, vLines, leftGridFrame, rightGridFrame, topGridFrame, bottomGridFrame );

  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) )
  {
    drawGridFrameBorder( p, leftGridFrame, QgsLayoutItemMapGrid::Left, extension ? &extension->left : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) )
  {
    drawGridFrameBorder( p, rightGridFrame, QgsLayoutItemMapGrid::Right, extension ? &extension->right : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
  {
    drawGridFrameBorder( p, topGridFrame, QgsLayoutItemMapGrid::Top, extension ? &extension->top : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
  {
    drawGridFrameBorder( p, bottomGridFrame, QgsLayoutItemMapGrid::Bottom, extension ? &extension->bottom : nullptr );
  }
  if ( p )
    p->restore();
}

void QgsLayoutItemMapGrid::drawGridLine( const QLineF &line, QgsRenderContext &context ) const
{
  QPolygonF poly;
  poly << line.p1() << line.p2();
  drawGridLine( poly, context );
}

void QgsLayoutItemMapGrid::drawGridLine( const QPolygonF &line, QgsRenderContext &context ) const
{
  if ( !mMap || !mMap->layout() || !mGridLineSymbol )
  {
    return;
  }

  mGridLineSymbol->startRender( context );
  mGridLineSymbol->renderPolyline( line, nullptr, context );
  mGridLineSymbol->stopRender( context );
}

void QgsLayoutItemMapGrid::drawGridMarker( QPointF point, QgsRenderContext &context ) const
{
  if ( !mMap || !mMap->layout() || !mGridMarkerSymbol )
  {
    return;
  }

  mGridMarkerSymbol->startRender( context );
  mGridMarkerSymbol->renderPoint( point, nullptr, context );
  mGridMarkerSymbol->stopRender( context );
}

void QgsLayoutItemMapGrid::drawGridFrameBorder( QPainter *p, const QMap< double, double > &borderPos, QgsLayoutItemMapGrid::BorderSide border, double *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  switch ( mGridFrameStyle )
  {
    case QgsLayoutItemMapGrid::Zebra:
      drawGridFrameZebraBorder( p, borderPos, border, extension );
      break;
    case QgsLayoutItemMapGrid::InteriorTicks:
    case QgsLayoutItemMapGrid::ExteriorTicks:
    case QgsLayoutItemMapGrid::InteriorExteriorTicks:
      drawGridFrameTicks( p, borderPos, border, extension );
      break;

    case QgsLayoutItemMapGrid::LineBorder:
      drawGridFrameLineBorder( p, border, extension );
      break;

    case QgsLayoutItemMapGrid::NoFrame:
      break;
  }

}

void QgsLayoutItemMapGrid::drawGridFrameZebraBorder( QPainter *p, const QMap< double, double > &borderPos, QgsLayoutItemMapGrid::BorderSide border, double *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( extension )
  {
    *extension = mGridFrameMargin + mGridFrameWidth + mGridFramePenThickness / 2.0;
    return;
  }

  QMap< double, double > pos = borderPos;

  double currentCoord = 0.0;
  if ( ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right ) && testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
  {
//    currentCoord = - (mGridFrameWidth + mGridFrameMargin);
    currentCoord = -mGridFramePenThickness / 2.0;
    pos.insert( 0, 0 );
  }
  else if ( ( border == QgsLayoutItemMapGrid::Top || border == QgsLayoutItemMapGrid::Bottom ) && testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) )
  {
//    currentCoord = - (mGridFrameWidth + mGridFrameMargin);
    currentCoord = -mGridFramePenThickness / 2.0;
    pos.insert( 0, 0 );
  }
  bool color1 = true;
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
  {
    pos.insert( mMap->rect().height(), mMap->rect().height() );
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
    {
//      pos.insert( mMap->rect().height() + mGridFrameWidth, mMap->rect().height() + mGridFrameWidth );
      pos.insert( mMap->rect().height(), mMap->rect().height() );
    }
  }
  else if ( border == QgsLayoutItemMapGrid::Top || border == QgsLayoutItemMapGrid::Bottom )
  {
    pos.insert( mMap->rect().width(), mMap->rect().width() );
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) )
    {
//      pos.insert( mMap->rect().width() + mGridFrameWidth, mMap->rect().width() + mGridFrameWidth );
      pos.insert( mMap->rect().width(), mMap->rect().width() );
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
    if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
    {
      height = posIt.key() - currentCoord;
      width = mGridFrameWidth;
      x = ( border == QgsLayoutItemMapGrid::Left ) ? -( mGridFrameWidth + mGridFrameMargin ) : mMap->rect().width() + mGridFrameMargin;
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == QgsLayoutItemMapGrid::Top ) ? -( mGridFrameWidth + mGridFrameMargin ) : mMap->rect().height() + mGridFrameMargin;
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    color1 = !color1;
  }
  //draw corners
  width = height = ( mGridFrameWidth + mGridFrameMargin ) ;
  p->setBrush( QBrush( mGridFrameFillColor1 ) );
  p->drawRect( QRectF( -( mGridFrameWidth + mGridFrameMargin ), -( mGridFrameWidth + mGridFrameMargin ), width, height ) );
  p->drawRect( QRectF( mMap->rect().width(), -( mGridFrameWidth + mGridFrameMargin ), width, height ) );
  p->drawRect( QRectF( -( mGridFrameWidth + mGridFrameMargin ), mMap->rect().height(),                  width, height ) );
  p->drawRect( QRectF( mMap->rect().width(), mMap->rect().height(), width, height ) );
}

void QgsLayoutItemMapGrid::drawGridFrameTicks( QPainter *p, const QMap< double, double > &borderPos, QgsLayoutItemMapGrid::BorderSide border, double *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( extension )
  {
    if ( mGridFrameStyle != QgsLayoutItemMapGrid::InteriorTicks )
      *extension = mGridFrameMargin + mGridFrameWidth;
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
    if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
    {
      y = posIt.key();
      height = 0;
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        width = mGridFrameWidth;
        x = ( border == QgsLayoutItemMapGrid::Left ) ? 0 - mGridFrameMargin : mMap->rect().width() - mGridFrameWidth + mGridFrameMargin;
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        width = mGridFrameWidth;
        x = ( border == QgsLayoutItemMapGrid::Left ) ? - mGridFrameWidth - mGridFrameMargin : mMap->rect().width() + mGridFrameMargin;
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorExteriorTicks )
      {
        width = mGridFrameWidth * 2;
        x = ( border == QgsLayoutItemMapGrid::Left ) ? - mGridFrameWidth - mGridFrameMargin : mMap->rect().width() - mGridFrameWidth + mGridFrameMargin;
      }
    }
    else //top or bottom
    {
      x = posIt.key();
      width = 0;
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        height = mGridFrameWidth;
        y = ( border == QgsLayoutItemMapGrid::Top ) ? 0 - mGridFrameMargin : mMap->rect().height() - mGridFrameWidth + mGridFrameMargin;
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        height = mGridFrameWidth;
        y = ( border == QgsLayoutItemMapGrid::Top ) ? -mGridFrameWidth - mGridFrameMargin : mMap->rect().height() + mGridFrameMargin;
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorExteriorTicks )
      {
        height = mGridFrameWidth * 2;
        y = ( border == QgsLayoutItemMapGrid::Top ) ? -mGridFrameWidth - mGridFrameMargin : mMap->rect().height() - mGridFrameWidth + mGridFrameMargin;
      }
    }
    p->drawLine( QLineF( x, y, x + width, y + height ) );
  }
}

void QgsLayoutItemMapGrid::drawGridFrameLineBorder( QPainter *p, QgsLayoutItemMapGrid::BorderSide border, double *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( extension )
  {
    *extension = mGridFrameMargin + mGridFramePenThickness / 2.0;
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
    case QgsLayoutItemMapGrid::Left:
      p->drawLine( QLineF( 0 - mGridFrameMargin, 0 - mGridFrameMargin, 0 - mGridFrameMargin, mMap->rect().height() + mGridFrameMargin ) );
      //corner left-top
      if ( mGridFrameMargin != 0 )
      {
        const double X1 = 0 - mGridFrameMargin + mGridFramePenThickness / 2.0;
        const double Y1 = 0 - mGridFrameMargin + mGridFramePenThickness / 2.0;
        p->drawLine( QLineF( 0, 0, X1, Y1 ) );
      }
      break;
    case QgsLayoutItemMapGrid::Right:
      p->drawLine( QLineF( mMap->rect().width() + mGridFrameMargin, 0 - mGridFrameMargin, mMap->rect().width() + mGridFrameMargin, mMap->rect().height() + mGridFrameMargin ) );
      //corner right-bottom
      if ( mGridFrameMargin != 0 )
      {
        const double X1 = mMap->rect().width() + mGridFrameMargin - mGridFramePenThickness / 2.0 ;
        const double Y1 = mMap->rect().height() + mGridFrameMargin - mGridFramePenThickness / 2.0 ;
        p->drawLine( QLineF( mMap->rect().width(), mMap->rect().height(), X1, Y1 ) );
      }
      break;
    case QgsLayoutItemMapGrid::Top:
      p->drawLine( QLineF( 0 - mGridFrameMargin, 0 - mGridFrameMargin, mMap->rect().width() + mGridFrameMargin, 0 - mGridFrameMargin ) );
      //corner right-top
      if ( mGridFrameMargin != 0 )
      {
        const double X1 = mMap->rect().width() + mGridFrameMargin - mGridFramePenThickness / 2.0 ;
        const double Y1 = 0 - mGridFrameMargin + mGridFramePenThickness / 2.0 ;
        p->drawLine( QLineF( mMap->rect().width(), 0, X1, Y1 ) );
      }
      break;
    case QgsLayoutItemMapGrid::Bottom:
      p->drawLine( QLineF( 0 - mGridFrameMargin, mMap->rect().height() + mGridFrameMargin, mMap->rect().width() + mGridFrameMargin, mMap->rect().height() + mGridFrameMargin ) );
      //corner left-bottom
      if ( mGridFrameMargin != 0 )
      {
        const double X1 = 0 - mGridFrameMargin + mGridFramePenThickness / 2.0 ;
        const double Y1 = mMap->rect().height() + mGridFrameMargin - mGridFramePenThickness / 2.0 ;
        p->drawLine( QLineF( 0, mMap->rect().height(), X1, Y1 ) );
      }
      break;
  }
}

void QgsLayoutItemMapGrid::drawCoordinateAnnotations( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QgsExpressionContext &expressionContext,
    GridExtension *extension ) const
{
  QString currentAnnotationString;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, QgsLayoutItemMapGrid::Latitude, expressionContext );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsLayoutItemMapGrid::Latitude, extension );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsLayoutItemMapGrid::Latitude, extension );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, QgsLayoutItemMapGrid::Longitude, expressionContext );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsLayoutItemMapGrid::Longitude, extension );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsLayoutItemMapGrid::Longitude, extension );
  }
}

void QgsLayoutItemMapGrid::drawCoordinateAnnotation( QPainter *p, QPointF pos, const QString &annotationString, const AnnotationCoordinate coordinateType, GridExtension *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  QgsLayoutItemMapGrid::BorderSide frameBorder = borderForLineCoord( pos, coordinateType );
  double textWidth = QgsLayoutUtils::textWidthMM( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = extension ? QgsLayoutUtils::fontAscentMM( mGridAnnotationFont )
                      : QgsLayoutUtils::fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
  double xpos = pos.x();
  double ypos = pos.y();
  int rotation = 0;

  double gridFrameDistance = 0;
  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame && mGridFrameStyle != QgsLayoutItemMapGrid::LineBorder )
  {
    gridFrameDistance = mGridFrameWidth;
  }
  if ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::LineBorder )
  {
    gridFrameDistance += ( mGridFramePenThickness / 2.0 );
  }

  if ( frameBorder == QgsLayoutItemMapGrid::Left )
  {
    if ( mLeftGridAnnotationDisplay == QgsLayoutItemMapGrid::HideAll ||
         ( coordinateType == Longitude && mLeftGridAnnotationDisplay == QgsLayoutItemMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mLeftGridAnnotationDisplay == QgsLayoutItemMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) )
    {
      gridFrameDistance = 0;
    }

    if ( mLeftGridAnnotationPosition == QgsLayoutItemMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::Vertical || mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        xpos += textHeight + mAnnotationFrameDistance + gridFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
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
    else if ( mLeftGridAnnotationPosition == QgsLayoutItemMapGrid::OutsideMapFrame ) //Outside map frame
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::Vertical || mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        xpos -= ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
        if ( extension )
          extension->left = std::max( extension->left, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mLeftGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
      {
        xpos -= textHeight + mAnnotationFrameDistance + gridFrameDistance;
        ypos -= textWidth / 2.0;
        rotation = 90;
        if ( extension )
          extension->left = std::max( extension->left, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else
      {
        xpos -= ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
        if ( extension )
          extension->left = std::max( extension->left, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
    }
    else
    {
      return;
    }
  }
  else if ( frameBorder == QgsLayoutItemMapGrid::Right )
  {
    if ( mRightGridAnnotationDisplay == QgsLayoutItemMapGrid::HideAll ||
         ( coordinateType == Longitude && mRightGridAnnotationDisplay == QgsLayoutItemMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mRightGridAnnotationDisplay == QgsLayoutItemMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) )
    {
      gridFrameDistance = 0;
    }

    if ( mRightGridAnnotationPosition == QgsLayoutItemMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mRightGridAnnotationDirection == QgsLayoutItemMapGrid::Vertical )
      {
        xpos -= mAnnotationFrameDistance + gridFrameDistance;
        ypos += textWidth / 2.0;
        rotation = 270;
      }
      else if ( mRightGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending || mRightGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
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
    else if ( mRightGridAnnotationPosition == QgsLayoutItemMapGrid::OutsideMapFrame )//OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mRightGridAnnotationDirection == QgsLayoutItemMapGrid::Vertical )
      {
        xpos += ( textHeight + mAnnotationFrameDistance + gridFrameDistance );
        ypos += textWidth / 2.0;
        rotation = 270;
        if ( extension )
          extension->right = std::max( extension->right, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mRightGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending || mRightGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos -= textWidth / 2.0;
        rotation = 90;
        if ( extension )
          extension->right = std::max( extension->right, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else //Horizontal
      {
        xpos += ( mAnnotationFrameDistance + gridFrameDistance );
        ypos += textHeight / 2.0;
        if ( extension )
          extension->right = std::max( extension->right, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
    }
    else
    {
      return;
    }
  }
  else if ( frameBorder == QgsLayoutItemMapGrid::Bottom )
  {
    if ( mBottomGridAnnotationDisplay == QgsLayoutItemMapGrid::HideAll ||
         ( coordinateType == Longitude && mBottomGridAnnotationDisplay == QgsLayoutItemMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mBottomGridAnnotationDisplay == QgsLayoutItemMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
    {
      gridFrameDistance = 0;
    }

    if ( mBottomGridAnnotationPosition == QgsLayoutItemMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::Horizontal || mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        ypos -= mAnnotationFrameDistance + gridFrameDistance;
        xpos -= textWidth / 2.0;
      }
      else if ( mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
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
    else if ( mBottomGridAnnotationPosition == QgsLayoutItemMapGrid::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::Horizontal || mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        ypos += ( mAnnotationFrameDistance + textHeight + gridFrameDistance );
        xpos -= textWidth / 2.0;
        if ( extension )
        {
          extension->bottom = std::max( extension->bottom, mAnnotationFrameDistance + gridFrameDistance + textHeight );
          extension->left = std::max( extension->left, textWidth / 2.0 ); // annotation at bottom left/right may extend outside the bounds
          extension->right = std::max( extension->right, textWidth / 2.0 );
        }
      }
      else if ( mBottomGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos += gridFrameDistance + mAnnotationFrameDistance;
        rotation = 90;
        if ( extension )
          extension->bottom = std::max( extension->bottom, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos += ( textWidth + mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
        if ( extension )
          extension->bottom = std::max( extension->bottom, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
    }
    else
    {
      return;
    }
  }
  else //top
  {
    if ( mTopGridAnnotationDisplay == QgsLayoutItemMapGrid::HideAll ||
         ( coordinateType == Longitude && mTopGridAnnotationDisplay == QgsLayoutItemMapGrid::LatitudeOnly ) ||
         ( coordinateType == Latitude && mTopGridAnnotationDisplay == QgsLayoutItemMapGrid::LongitudeOnly ) )
    {
      return;
    }
    if ( !testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
    {
      gridFrameDistance = 0;
    }

    if ( mTopGridAnnotationPosition == QgsLayoutItemMapGrid::InsideMapFrame )
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mTopGridAnnotationDirection == QgsLayoutItemMapGrid::Horizontal || mTopGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos += textHeight + mAnnotationFrameDistance + gridFrameDistance;
      }
      else if ( mTopGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
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
    else if ( mTopGridAnnotationPosition == QgsLayoutItemMapGrid::OutsideMapFrame ) //OutsideMapFrame
    {
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        gridFrameDistance = 0;
      }
      if ( mTopGridAnnotationDirection == QgsLayoutItemMapGrid::Horizontal || mTopGridAnnotationDirection == QgsLayoutItemMapGrid::BoundaryDirection )
      {
        xpos -= textWidth / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
        if ( extension )
          extension->top = std::max( extension->top, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mTopGridAnnotationDirection == QgsLayoutItemMapGrid::VerticalDescending )
      {
        xpos -= textHeight / 2.0;
        ypos -= textWidth + mAnnotationFrameDistance + gridFrameDistance;
        rotation = 90;
        if ( extension )
          extension->top = std::max( extension->top, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
      else //Vertical
      {
        xpos += textHeight / 2.0;
        ypos -= ( mAnnotationFrameDistance + gridFrameDistance );
        rotation = 270;
        if ( extension )
          extension->top = std::max( extension->top, mAnnotationFrameDistance + gridFrameDistance + textWidth );
      }
    }
    else
    {
      return;
    }
  }

  if ( extension || !p )
    return;

  drawAnnotation( p, QPointF( xpos, ypos ), rotation, annotationString );
}

void QgsLayoutItemMapGrid::drawAnnotation( QPainter *p, QPointF pos, int rotation, const QString &annotationText ) const
{
  if ( !mMap )
  {
    return;
  }

  p->save();
  p->translate( pos );
  p->rotate( rotation );
  QgsLayoutUtils::drawText( p, QPointF( 0, 0 ), annotationText, mGridAnnotationFont, mGridAnnotationFontColor );
  p->restore();
}

QString QgsLayoutItemMapGrid::gridAnnotationString( double value, QgsLayoutItemMapGrid::AnnotationCoordinate coord, QgsExpressionContext &expressionContext ) const
{
  //check if we are using degrees (ie, geographic crs)
  bool geographic = false;
  if ( mCRS.isValid() && mCRS.isGeographic() )
  {
    geographic = true;
  }
  else if ( mMap && mMap->layout() )
  {
    geographic = mMap->crs().isGeographic();
  }

  if ( geographic && coord == QgsLayoutItemMapGrid::Longitude &&
       ( mGridAnnotationFormat == QgsLayoutItemMapGrid::Decimal || mGridAnnotationFormat == QgsLayoutItemMapGrid::DecimalWithSuffix ) )
  {
    // wrap around longitudes > 180 or < -180 degrees, so that, e.g., "190E" -> "170W"
    double wrappedX = std::fmod( value, 360.0 );
    if ( wrappedX > 180.0 )
    {
      value = wrappedX - 360.0;
    }
    else if ( wrappedX < -180.0 )
    {
      value = wrappedX + 360.0;
    }
  }

  if ( mGridAnnotationFormat == QgsLayoutItemMapGrid::Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == QgsLayoutItemMapGrid::DecimalWithSuffix )
  {
    QString hemisphere;

    double coordRounded = std::round( value * std::pow( 10.0, mGridAnnotationPrecision ) ) / std::pow( 10.0, mGridAnnotationPrecision );
    if ( coord == QgsLayoutItemMapGrid::Longitude )
    {
      //don't use E/W suffixes if ambiguous (e.g., 180 degrees)
      if ( !geographic || ( coordRounded != 180.0 && coordRounded != 0.0 ) )
      {
        hemisphere = value < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
      }
    }
    else
    {
      //don't use N/S suffixes if ambiguous (e.g., 0 degrees)
      if ( !geographic || coordRounded != 0.0 )
      {
        hemisphere = value < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
      }
    }
    if ( geographic )
    {
      //insert degree symbol for geographic coordinates
      return QString::number( std::fabs( value ), 'f', mGridAnnotationPrecision ) + QChar( 176 ) + hemisphere;
    }
    else
    {
      return QString::number( std::fabs( value ), 'f', mGridAnnotationPrecision ) + hemisphere;
    }
  }
  else if ( mGridAnnotationFormat == CustomFormat )
  {
    expressionContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_number" ), value, true ) );
    expressionContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_axis" ), coord == QgsLayoutItemMapGrid::Longitude ? "x" : "y", true ) );
    if ( !mGridAnnotationExpression )
    {
      mGridAnnotationExpression.reset( new QgsExpression( mGridAnnotationExpressionString ) );
      mGridAnnotationExpression->prepare( &expressionContext );
    }
    return mGridAnnotationExpression->evaluate( &expressionContext ).toString();
  }

  QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatDecimalDegrees;
  QgsCoordinateFormatter::FormatFlags flags = nullptr;
  switch ( mGridAnnotationFormat )
  {
    case Decimal:
    case DecimalWithSuffix:
    case CustomFormat:
      break; // already handled above

    case DegreeMinute:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
      break;

    case DegreeMinuteSecond:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
      break;

    case DegreeMinuteNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = nullptr;
      break;

    case DegreeMinutePadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;

    case DegreeMinuteSecondNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = nullptr;
      break;

    case DegreeMinuteSecondPadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;
  }

  switch ( coord )
  {
    case Longitude:
      return QgsCoordinateFormatter::formatX( value, format, mGridAnnotationPrecision, flags );

    case Latitude:
      return QgsCoordinateFormatter::formatY( value, format, mGridAnnotationPrecision, flags );
  }

  return QString(); // no warnings
}

int QgsLayoutItemMapGrid::xGridLines( QList< QPair< double, QLineF > > &lines ) const
{
  lines.clear();
  if ( !mMap || mGridIntervalY <= 0.0 )
  {
    return 1;
  }


  QPolygonF mapPolygon = mMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalY = mGridIntervalY;
  double gridOffsetY = mGridOffsetY;
  double annotationScale = 1.0;
  if ( mGridUnit != MapUnit )
  {
    mapBoundingRect = mMap->rect();
    mapPolygon = QPolygonF( mMap->rect() );
    if ( mGridUnit == CM )
    {
      annotationScale = 0.1;
      gridIntervalY *= 10;
      gridOffsetY *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.top() - gridOffsetY ) / gridIntervalY + roundCorrection ) * gridIntervalY + gridOffsetY;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || mGridUnit != MapUnit )
  {
    //no rotation. Do it 'the easy way'

    double yCanvasCoord;
    while ( currentLevel <= mapBoundingRect.bottom() && gridLineCount < MAX_GRID_LINES )
    {
      yCanvasCoord = mMap->rect().height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
      lines.push_back( qMakePair( currentLevel * annotationScale, QLineF( 0, yCanvasCoord, mMap->rect().width(), yCanvasCoord ) ) );
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

  QVector<QPointF> intersectionList; //intersects between border lines and grid lines

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
      lines.push_back( qMakePair( currentLevel, QLineF( mMap->mapToItemCoords( intersectionList.at( 0 ) ), mMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalY;
  }


  return 0;
}

int QgsLayoutItemMapGrid::yGridLines( QList< QPair< double, QLineF > > &lines ) const
{
  lines.clear();
  if ( !mMap || mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  QPolygonF mapPolygon = mMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalX = mGridIntervalX;
  double gridOffsetX = mGridOffsetX;
  double annotationScale = 1.0;
  if ( mGridUnit != MapUnit )
  {
    mapBoundingRect = mMap->rect();
    mapPolygon = QPolygonF( mMap->rect() );
    if ( mGridUnit == CM )
    {
      annotationScale = 0.1;
      gridIntervalX *= 10;
      gridOffsetX *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.left() - gridOffsetX ) / gridIntervalX + roundCorrection ) * gridIntervalX + gridOffsetX;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || mGridUnit != MapUnit )
  {
    //no rotation. Do it 'the easy way'
    double xCanvasCoord;
    while ( currentLevel <= mapBoundingRect.right() && gridLineCount < MAX_GRID_LINES )
    {
      xCanvasCoord = mMap->rect().width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();
      lines.push_back( qMakePair( currentLevel * annotationScale, QLineF( xCanvasCoord, 0, xCanvasCoord, mMap->rect().height() ) ) );
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

  QVector<QPointF> intersectionList; //intersects between border lines and grid lines

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
      lines.push_back( qMakePair( currentLevel, QLineF( mMap->mapToItemCoords( intersectionList.at( 0 ) ), mMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalX;
  }

  return 0;
}

int QgsLayoutItemMapGrid::xGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const
{
  lines.clear();
  if ( !mMap || mGridIntervalY <= 0.0 )
  {
    return 1;
  }

  double roundCorrection = bbox.yMaximum() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( bbox.yMaximum() - mGridOffsetY ) / mGridIntervalY + roundCorrection ) * mGridIntervalY + mGridOffsetY;

  double minX = bbox.xMinimum();
  double maxX = bbox.xMaximum();
  double step = ( maxX - minX ) / 20;

  bool crosses180 = false;
  bool crossed180 = false;
  if ( mCRS.isGeographic() && ( minX > maxX ) )
  {
    //handle 180 degree longitude crossover
    crosses180 = true;
    step = ( maxX + 360.0 - minX ) / 20;
  }

  if ( qgsDoubleNear( step, 0.0 ) )
    return 1;

  int gridLineCount = 0;
  while ( currentLevel >= bbox.yMinimum() && gridLineCount < MAX_GRID_LINES )
  {
    QPolygonF gridLine;
    double currentX = minX;
    bool cont = true;
    while ( cont )
    {
      if ( ( !crosses180 || crossed180 ) && ( currentX > maxX ) )
      {
        cont = false;
      }

      try
      {
        QgsPointXY mapPoint = t.transform( currentX, currentLevel ); //transform back to map crs
        gridLine.append( mMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) ); //transform back to composer coords
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
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

    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if ( !( *lineIt ).isEmpty() )
      {
        lines.append( qMakePair( currentLevel, *lineIt ) );
        gridLineCount++;
      }
    }
    currentLevel -= mGridIntervalY;
  }

  return 0;
}

int QgsLayoutItemMapGrid::yGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const
{
  lines.clear();
  if ( !mMap || mGridIntervalX <= 0.0 )
  {
    return 1;
  }

  double roundCorrection = bbox.xMinimum() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( bbox.xMinimum() - mGridOffsetX ) / mGridIntervalX + roundCorrection ) * mGridIntervalX + mGridOffsetX;

  double minY = bbox.yMinimum();
  double maxY = bbox.yMaximum();
  double step = ( maxY - minY ) / 20;

  if ( qgsDoubleNear( step, 0.0 ) )
    return 1;

  bool crosses180 = false;
  bool crossed180 = false;
  if ( mCRS.isGeographic() && ( bbox.xMinimum() > bbox.xMaximum() ) )
  {
    //handle 180 degree longitude crossover
    crosses180 = true;
  }

  int gridLineCount = 0;
  while ( ( currentLevel <= bbox.xMaximum() || ( crosses180 && !crossed180 ) ) && gridLineCount < MAX_GRID_LINES )
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
        QgsPointXY mapPoint = t.transform( currentLevel, currentY );
        //transform back to composer coords
        gridLine.append( mMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentY += step;
    }
    //clip grid line to map polygon
    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if ( !( *lineIt ).isEmpty() )
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

void QgsLayoutItemMapGrid::sortGridLinesOnBorders( const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QMap< double, double > &leftFrameEntries,
    QMap< double, double > &rightFrameEntries, QMap< double, double > &topFrameEntries, QMap< double, double > &bottomFrameEntries ) const
{
  QList< QgsMapAnnotation > borderPositions;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    QgsMapAnnotation p1;
    p1.coordinate = it->first;
    p1.itemPosition = it->second.p1();
    p1.coordinateType = QgsLayoutItemMapGrid::Latitude;
    borderPositions << p1;

    QgsMapAnnotation p2;
    p2.coordinate = it->first;
    p2.itemPosition = it->second.p2();
    p2.coordinateType = QgsLayoutItemMapGrid::Latitude;
    borderPositions << p2;
  }
  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    QgsMapAnnotation p1;
    p1.coordinate = it->first;
    p1.itemPosition = it->second.p1();
    p1.coordinateType = QgsLayoutItemMapGrid::Longitude;
    borderPositions << p1;

    QgsMapAnnotation p2;
    p2.coordinate = it->first;
    p2.itemPosition = it->second.p2();
    p2.coordinateType = QgsLayoutItemMapGrid::Longitude;
    borderPositions << p2;
  }

  QList< QgsMapAnnotation >::const_iterator bIt = borderPositions.constBegin();
  for ( ; bIt != borderPositions.constEnd(); ++bIt )
  {
    QgsLayoutItemMapGrid::BorderSide frameBorder = borderForLineCoord( bIt->itemPosition, bIt->coordinateType );
    if ( frameBorder == QgsLayoutItemMapGrid::Left && shouldShowDivisionForSide( bIt->coordinateType, QgsLayoutItemMapGrid::Left ) )
    {
      leftFrameEntries.insert( bIt->itemPosition.y(), bIt->coordinate );
    }
    else if ( frameBorder == QgsLayoutItemMapGrid::Right && shouldShowDivisionForSide( bIt->coordinateType, QgsLayoutItemMapGrid::Right ) )
    {
      rightFrameEntries.insert( bIt->itemPosition.y(), bIt->coordinate );
    }
    else if ( frameBorder == QgsLayoutItemMapGrid::Top && shouldShowDivisionForSide( bIt->coordinateType, QgsLayoutItemMapGrid::Top ) )
    {
      topFrameEntries.insert( bIt->itemPosition.x(), bIt->coordinate );
    }
    else if ( frameBorder == QgsLayoutItemMapGrid::Bottom && shouldShowDivisionForSide( bIt->coordinateType, QgsLayoutItemMapGrid::Bottom ) )
    {
      bottomFrameEntries.insert( bIt->itemPosition.x(), bIt->coordinate );
    }
  }
}

bool QgsLayoutItemMapGrid::shouldShowDivisionForSide( QgsLayoutItemMapGrid::AnnotationCoordinate coordinate, QgsLayoutItemMapGrid::BorderSide side ) const
{
  switch ( side )
  {
    case QgsLayoutItemMapGrid::Left:
      return shouldShowDivisionForDisplayMode( coordinate, mLeftFrameDivisions );
    case QgsLayoutItemMapGrid::Right:
      return shouldShowDivisionForDisplayMode( coordinate, mRightFrameDivisions );
    case QgsLayoutItemMapGrid::Top:
      return shouldShowDivisionForDisplayMode( coordinate, mTopFrameDivisions );
    case QgsLayoutItemMapGrid::Bottom:
    default: //prevent warnings
      return shouldShowDivisionForDisplayMode( coordinate, mBottomFrameDivisions );
  }
}

bool QgsLayoutItemMapGrid::shouldShowDivisionForDisplayMode( QgsLayoutItemMapGrid::AnnotationCoordinate coordinate, QgsLayoutItemMapGrid::DisplayMode mode ) const
{
  return mode == QgsLayoutItemMapGrid::ShowAll
         || ( mode == QgsLayoutItemMapGrid::LatitudeOnly && coordinate == QgsLayoutItemMapGrid::Latitude )
         || ( mode == QgsLayoutItemMapGrid::LongitudeOnly && coordinate == QgsLayoutItemMapGrid::Longitude );
}

bool sortByDistance( QPair<qreal, QgsLayoutItemMapGrid::BorderSide> a, QPair<qreal, QgsLayoutItemMapGrid::BorderSide> b )
{
  return a.first < b.first;
}

QgsLayoutItemMapGrid::BorderSide QgsLayoutItemMapGrid::borderForLineCoord( QPointF p, const AnnotationCoordinate coordinateType ) const
{
  if ( !mMap )
  {
    return QgsLayoutItemMapGrid::Left;
  }

  double tolerance = std::max( mMap->frameEnabled() ? mMap->pen().widthF() : 0.0, 1.0 );

  //check for corner coordinates
  if ( ( p.y() <= tolerance && p.x() <= tolerance ) // top left
       || ( p.y() <= tolerance && p.x() >= ( mMap->rect().width() - tolerance ) ) //top right
       || ( p.y() >= ( mMap->rect().height() - tolerance ) && p.x() <= tolerance ) //bottom left
       || ( p.y() >= ( mMap->rect().height() - tolerance ) && p.x() >= ( mMap->rect().width() - tolerance ) ) //bottom right
     )
  {
    //coordinate is in corner - fall back to preferred side for coordinate type
    if ( coordinateType == QgsLayoutItemMapGrid::Latitude )
    {
      if ( p.x() <= tolerance )
      {
        return QgsLayoutItemMapGrid::Left;
      }
      else
      {
        return QgsLayoutItemMapGrid::Right;
      }
    }
    else
    {
      if ( p.y() <= tolerance )
      {
        return QgsLayoutItemMapGrid::Top;
      }
      else
      {
        return QgsLayoutItemMapGrid::Bottom;
      }
    }
  }

  //otherwise, guess side based on closest map side to point
  QList< QPair<qreal, QgsLayoutItemMapGrid::BorderSide > > distanceToSide;
  distanceToSide << qMakePair( p.x(), QgsLayoutItemMapGrid::Left );
  distanceToSide << qMakePair( mMap->rect().width() - p.x(), QgsLayoutItemMapGrid::Right );
  distanceToSide << qMakePair( p.y(), QgsLayoutItemMapGrid::Top );
  distanceToSide << qMakePair( mMap->rect().height() - p.y(), QgsLayoutItemMapGrid::Bottom );

  std::sort( distanceToSide.begin(), distanceToSide.end(), sortByDistance );
  return distanceToSide.at( 0 ).second;
}

void QgsLayoutItemMapGrid::setLineSymbol( QgsLineSymbol *symbol )
{
  mGridLineSymbol.reset( symbol );
}

const QgsLineSymbol *QgsLayoutItemMapGrid::lineSymbol() const
{
  return mGridLineSymbol.get();
}

QgsLineSymbol *QgsLayoutItemMapGrid::lineSymbol()
{
  return mGridLineSymbol.get();
}

void QgsLayoutItemMapGrid::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mGridMarkerSymbol.reset( symbol );
}

const QgsMarkerSymbol *QgsLayoutItemMapGrid::markerSymbol() const
{
  return mGridMarkerSymbol.get();
}

QgsMarkerSymbol *QgsLayoutItemMapGrid::markerSymbol()
{
  return mGridMarkerSymbol.get();
}

void QgsLayoutItemMapGrid::setAnnotationDisplay( const QgsLayoutItemMapGrid::DisplayMode display, const QgsLayoutItemMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      mLeftGridAnnotationDisplay = display;
      break;
    case QgsLayoutItemMapGrid::Right:
      mRightGridAnnotationDisplay = display;
      break;
    case QgsLayoutItemMapGrid::Top:
      mTopGridAnnotationDisplay = display;
      break;
    case QgsLayoutItemMapGrid::Bottom:
      mBottomGridAnnotationDisplay = display;
      break;
    default:
      return;
  }

  if ( mMap )
  {
    mMap->updateBoundingRect();
    mMap->update();
  }
}

QgsLayoutItemMapGrid::DisplayMode QgsLayoutItemMapGrid::annotationDisplay( const QgsLayoutItemMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      return mLeftGridAnnotationDisplay;
    case QgsLayoutItemMapGrid::Right:
      return mRightGridAnnotationDisplay;
    case QgsLayoutItemMapGrid::Top:
      return mTopGridAnnotationDisplay;
    case QgsLayoutItemMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDisplay;
  }
}

double QgsLayoutItemMapGrid::maxExtension() const
{
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  double left = 0.0;
  calculateMaxExtension( top, right, bottom, left );
  return std::max( std::max( std::max( top, right ), bottom ), left );
}

void QgsLayoutItemMapGrid::calculateMaxExtension( double &top, double &right, double &bottom, double &left ) const
{
  top = 0.0;
  right = 0.0;
  bottom = 0.0;
  left = 0.0;

  if ( !mMap || !mEnabled )
  {
    return;
  }

  //setup render context
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  GridExtension extension;

  //collect grid lines
  QList< QPair< double, QLineF > > verticalLines;
  QList< QPair< double, QLineF > > horizontalLines;
  if ( mGridUnit == MapUnit && mCRS.isValid() && mCRS != mMap->crs() )
  {
    drawGridCrsTransform( context, 0, horizontalLines, verticalLines, true );
  }
  else
  {
    drawGridNoTransform( context, 0, horizontalLines, verticalLines, true );
  }

  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame )
  {
    drawGridFrame( nullptr, horizontalLines, verticalLines, &extension );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( nullptr, horizontalLines, verticalLines, context.expressionContext(), &extension );
  }

  top = extension.top;
  right = extension.right;
  bottom = extension.bottom;
  left = extension.left;
}

void QgsLayoutItemMapGrid::setUnits( const QgsLayoutItemMapGrid::GridUnit unit )
{
  if ( unit == mGridUnit )
  {
    return;
  }
  mGridUnit = unit;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setIntervalX( const double interval )
{
  if ( qgsDoubleNear( interval, mGridIntervalX ) )
  {
    return;
  }
  mGridIntervalX = interval;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setIntervalY( const double interval )
{
  if ( qgsDoubleNear( interval, mGridIntervalY ) )
  {
    return;
  }
  mGridIntervalY = interval;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setOffsetX( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetX ) )
  {
    return;
  }
  mGridOffsetX = offset;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setOffsetY( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetY ) )
  {
    return;
  }
  mGridOffsetY = offset;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setStyle( const QgsLayoutItemMapGrid::GridStyle style )
{
  if ( style == mGridStyle )
  {
    return;
  }
  mGridStyle = style;
  mTransformDirty = true;
}

void QgsLayoutItemMapGrid::setAnnotationDirection( const QgsLayoutItemMapGrid::AnnotationDirection direction, const QgsLayoutItemMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      mLeftGridAnnotationDirection = direction;
      break;
    case QgsLayoutItemMapGrid::Right:
      mRightGridAnnotationDirection = direction;
      break;
    case QgsLayoutItemMapGrid::Top:
      mTopGridAnnotationDirection = direction;
      break;
    case QgsLayoutItemMapGrid::Bottom:
      mBottomGridAnnotationDirection = direction;
      break;
    default:
      return;
  }

  if ( mMap )
  {
    mMap->updateBoundingRect();
    mMap->update();
  }
}

void QgsLayoutItemMapGrid::setFrameSideFlags( FrameSideFlags flags )
{
  mGridFrameSides = flags;
}

void QgsLayoutItemMapGrid::setFrameSideFlag( QgsLayoutItemMapGrid::FrameSideFlag flag, bool on )
{
  if ( on )
    mGridFrameSides |= flag;
  else
    mGridFrameSides &= ~flag;
}

QgsLayoutItemMapGrid::FrameSideFlags QgsLayoutItemMapGrid::frameSideFlags() const
{
  return mGridFrameSides;
}

QgsExpressionContext QgsLayoutItemMapGrid::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutObject::createExpressionContext();
  context.appendScope( new QgsExpressionContextScope( tr( "Grid" ) ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_number" ), 0, true ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_axis" ), "x", true ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "grid_number" ) << QStringLiteral( "grid_axis" ) );
  return context;
}

bool QgsLayoutItemMapGrid::testFrameSideFlag( QgsLayoutItemMapGrid::FrameSideFlag flag ) const
{
  return mGridFrameSides.testFlag( flag );
}

void QgsLayoutItemMapGrid::setAnnotationDirection( const AnnotationDirection direction )
{
  mLeftGridAnnotationDirection = direction;
  mRightGridAnnotationDirection = direction;
  mTopGridAnnotationDirection = direction;
  mBottomGridAnnotationDirection = direction;
}

void QgsLayoutItemMapGrid::setAnnotationPosition( const AnnotationPosition position, const BorderSide border )
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      mLeftGridAnnotationPosition = position;
      break;
    case QgsLayoutItemMapGrid::Right:
      mRightGridAnnotationPosition = position;
      break;
    case QgsLayoutItemMapGrid::Top:
      mTopGridAnnotationPosition = position;
      break;
    case QgsLayoutItemMapGrid::Bottom:
      mBottomGridAnnotationPosition = position;
      break;
    default:
      return;
  }

  if ( mMap )
  {
    mMap->updateBoundingRect();
    mMap->update();
  }
}

QgsLayoutItemMapGrid::AnnotationPosition QgsLayoutItemMapGrid::annotationPosition( const QgsLayoutItemMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      return mLeftGridAnnotationPosition;
    case QgsLayoutItemMapGrid::Right:
      return mRightGridAnnotationPosition;
    case QgsLayoutItemMapGrid::Top:
      return mTopGridAnnotationPosition;
    case QgsLayoutItemMapGrid::Bottom:
    default:
      return mBottomGridAnnotationPosition;
  }
}

QgsLayoutItemMapGrid::AnnotationDirection QgsLayoutItemMapGrid::annotationDirection( const BorderSide border ) const
{
  if ( !mMap )
  {
    return mLeftGridAnnotationDirection;
  }

  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      return mLeftGridAnnotationDirection;
    case QgsLayoutItemMapGrid::Right:
      return mRightGridAnnotationDirection;
    case QgsLayoutItemMapGrid::Top:
      return mTopGridAnnotationDirection;
    case QgsLayoutItemMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDirection;
  }
}

void QgsLayoutItemMapGrid::setFrameDivisions( const QgsLayoutItemMapGrid::DisplayMode divisions, const QgsLayoutItemMapGrid::BorderSide border )
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      mLeftFrameDivisions = divisions;
      break;
    case QgsLayoutItemMapGrid::Right:
      mRightFrameDivisions = divisions;
      break;
    case QgsLayoutItemMapGrid::Top:
      mTopFrameDivisions = divisions;
      break;
    case QgsLayoutItemMapGrid::Bottom:
      mBottomFrameDivisions = divisions;
      break;
    default:
      return;
  }

  if ( mMap )
  {
    mMap->update();
  }
}

QgsLayoutItemMapGrid::DisplayMode QgsLayoutItemMapGrid::frameDivisions( const QgsLayoutItemMapGrid::BorderSide border ) const
{
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      return mLeftFrameDivisions;
    case QgsLayoutItemMapGrid::Right:
      return mRightFrameDivisions;
    case QgsLayoutItemMapGrid::Top:
      return mTopFrameDivisions;
    case QgsLayoutItemMapGrid::Bottom:
    default:
      return mBottomFrameDivisions;
  }
}

int QgsLayoutItemMapGrid::crsGridParams( QgsRectangle &crsRect, QgsCoordinateTransform &inverseTransform ) const
{
  if ( !mMap )
  {
    return 1;
  }

  try
  {
    QgsCoordinateTransform tr( mMap->crs(), mCRS, mLayout->project() );
    QPolygonF mapPolygon = mMap->transformedMapPolygon();
    QRectF mbr = mapPolygon.boundingRect();
    QgsRectangle mapBoundingRect( mbr.left(), mbr.bottom(), mbr.right(), mbr.top() );


    if ( mCRS.isGeographic() )
    {
      //handle crossing the 180 degree longitude line
      QgsPointXY lowerLeft( mapBoundingRect.xMinimum(), mapBoundingRect.yMinimum() );
      QgsPointXY upperRight( mapBoundingRect.xMaximum(), mapBoundingRect.yMaximum() );

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

    inverseTransform = QgsCoordinateTransform( mCRS, mMap->crs(), mLayout->project() );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
    return 1;
  }
  return 0;
}

QList<QPolygonF> QgsLayoutItemMapGrid::trimLinesToMap( const QPolygonF &line, const QgsRectangle &rect )
{
  QgsGeometry lineGeom = QgsGeometry::fromQPolygonF( line );
  QgsGeometry rectGeom = QgsGeometry::fromRect( rect );

  QgsGeometry intersected = lineGeom.intersection( rectGeom );
  QVector<QgsGeometry> intersectedParts = intersected.asGeometryCollection();

  QList<QPolygonF> trimmedLines;
  QVector<QgsGeometry>::const_iterator geomIt = intersectedParts.constBegin();
  for ( ; geomIt != intersectedParts.constEnd(); ++geomIt )
  {
    trimmedLines << ( *geomIt ).asQPolygonF();
  }
  return trimmedLines;
}
