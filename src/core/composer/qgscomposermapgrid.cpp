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

QgsComposerMapGridStack::QgsComposerMapGridStack( QgsComposerMap *map )
  : QgsComposerMapItemStack( map )
{

}

void QgsComposerMapGridStack::addGrid( QgsComposerMapGrid *grid )
{
  QgsComposerMapItemStack::addItem( grid );
}

void QgsComposerMapGridStack::removeGrid( const QString &gridId )
{
  QgsComposerMapItemStack::removeItem( gridId );
}

void QgsComposerMapGridStack::moveGridUp( const QString &gridId )
{
  QgsComposerMapItemStack::moveItemUp( gridId );
}

void QgsComposerMapGridStack::moveGridDown( const QString &gridId )
{
  QgsComposerMapItemStack::moveItemDown( gridId );
}

const QgsComposerMapGrid *QgsComposerMapGridStack::constGrid( const QString &gridId ) const
{
  const QgsComposerMapItem *item = QgsComposerMapItemStack::constItem( gridId );
  return dynamic_cast<const QgsComposerMapGrid *>( item );
}

QgsComposerMapGrid *QgsComposerMapGridStack::grid( const QString &gridId ) const
{
  QgsComposerMapItem *item = QgsComposerMapItemStack::item( gridId );
  return dynamic_cast<QgsComposerMapGrid *>( item );
}

QgsComposerMapGrid *QgsComposerMapGridStack::grid( const int index ) const
{
  QgsComposerMapItem *item = QgsComposerMapItemStack::item( index );
  return dynamic_cast<QgsComposerMapGrid *>( item );
}

QList<QgsComposerMapGrid *> QgsComposerMapGridStack::asList() const
{
  QList< QgsComposerMapGrid * > list;
  QList< QgsComposerMapItem * >::const_iterator it = mItems.begin();
  for ( ; it != mItems.end(); ++it )
  {
    QgsComposerMapGrid *grid = dynamic_cast<QgsComposerMapGrid *>( *it );
    if ( grid )
    {
      list.append( grid );
    }
  }
  return list;
}

QgsComposerMapGrid &QgsComposerMapGridStack::operator[]( int idx )
{
  QgsComposerMapItem *item = mItems.at( idx );
  QgsComposerMapGrid *grid = dynamic_cast<QgsComposerMapGrid *>( item );
  return *grid;
}

bool QgsComposerMapGridStack::readXml( const QDomElement &elem, const QDomDocument &doc )
{
  removeItems();

  //read grid stack
  QDomNodeList mapGridNodeList = elem.elementsByTagName( QStringLiteral( "ComposerMapGrid" ) );
  for ( int i = 0; i < mapGridNodeList.size(); ++i )
  {
    QDomElement mapGridElem = mapGridNodeList.at( i ).toElement();
    QgsComposerMapGrid *mapGrid = new QgsComposerMapGrid( mapGridElem.attribute( QStringLiteral( "name" ) ), mComposerMap );
    mapGrid->readXml( mapGridElem, doc );
    mItems.append( mapGrid );
  }

  return true;
}

double QgsComposerMapGridStack::maxGridExtension() const
{
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  double left = 0.0;
  calculateMaxGridExtension( top, right, bottom, left );
  return std::max( std::max( std::max( top, right ), bottom ), left );
}

void QgsComposerMapGridStack::calculateMaxGridExtension( double &top, double &right, double &bottom, double &left ) const
{
  top = 0.0;
  right = 0.0;
  bottom = 0.0;
  left = 0.0;

  Q_FOREACH ( QgsComposerMapItem *item, mItems )
  {
    QgsComposerMapGrid *grid = dynamic_cast<QgsComposerMapGrid *>( item );
    if ( grid )
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
// QgsComposerMapGrid
//


QgsComposerMapGrid::QgsComposerMapGrid( const QString &name, QgsComposerMap *map )
  : QgsComposerMapItem( name, map )
{
  init();
}

QgsComposerMapGrid::QgsComposerMapGrid()
  : QgsComposerMapItem( QString(), nullptr )
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
  mGridLineSymbol = nullptr;
  mGridMarkerSymbol = nullptr;
  mGridUnit = MapUnit;
  mBlendMode = QPainter::CompositionMode_SourceOver;

  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "Composer/defaultFont" ) ).toString();
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
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  mGridLineSymbol = QgsLineSymbol::createSimple( properties );
}

void QgsComposerMapGrid::createDefaultGridMarkerSymbol()
{
  delete mGridMarkerSymbol;
  QgsStringMap properties;
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "2.0" ) );
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  mGridMarkerSymbol = QgsMarkerSymbol::createSimple( properties );
}

void QgsComposerMapGrid::setGridLineWidth( const double width )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setWidth( width );
  }
}

void QgsComposerMapGrid::setGridLineColor( const QColor &c )
{
  if ( mGridLineSymbol )
  {
    mGridLineSymbol->setColor( c );
  }
}

bool QgsComposerMapGrid::writeXml( QDomElement &elem, QDomDocument &doc ) const
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

  QgsReadWriteContext context;
  context.setPathResolver( mComposition->project()->pathResolver() );

  QDomElement lineStyleElem = doc.createElement( QStringLiteral( "lineStyle" ) );
  QDomElement gridLineStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridLineSymbol, doc, context );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( QStringLiteral( "markerStyle" ) );
  QDomElement gridMarkerStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridMarkerSymbol, doc, context );
  markerStyleElem.appendChild( gridMarkerStyleElem );
  mapGridElem.appendChild( markerStyleElem );

  mapGridElem.setAttribute( QStringLiteral( "gridFrameStyle" ), mGridFrameStyle );
  mapGridElem.setAttribute( QStringLiteral( "gridFrameSideFlags" ), mGridFrameSides );
  mapGridElem.setAttribute( QStringLiteral( "gridFrameWidth" ), qgsDoubleToString( mGridFrameWidth ) );
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

  bool ok = QgsComposerMapItem::writeXml( mapGridElem, doc );
  elem.appendChild( mapGridElem );
  return ok;
}

bool QgsComposerMapGrid::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  bool ok = QgsComposerMapItem::readXml( itemElem, doc );

  //grid
  mGridStyle = QgsComposerMapGrid::GridStyle( itemElem.attribute( QStringLiteral( "gridStyle" ), QStringLiteral( "0" ) ).toInt() );
  mGridIntervalX = itemElem.attribute( QStringLiteral( "intervalX" ), QStringLiteral( "0" ) ).toDouble();
  mGridIntervalY = itemElem.attribute( QStringLiteral( "intervalY" ), QStringLiteral( "0" ) ).toDouble();
  mGridOffsetX = itemElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble();
  mGridOffsetY = itemElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble();
  mCrossLength = itemElem.attribute( QStringLiteral( "crossLength" ), QStringLiteral( "3" ) ).toDouble();
  mGridFrameStyle = static_cast< QgsComposerMapGrid::FrameStyle >( itemElem.attribute( QStringLiteral( "gridFrameStyle" ), QStringLiteral( "0" ) ).toInt() );
  mGridFrameSides = static_cast< QgsComposerMapGrid::FrameSideFlags >( itemElem.attribute( QStringLiteral( "gridFrameSideFlags" ), QStringLiteral( "15" ) ).toInt() );
  mGridFrameWidth = itemElem.attribute( QStringLiteral( "gridFrameWidth" ), QStringLiteral( "2.0" ) ).toDouble();
  mGridFramePenThickness = itemElem.attribute( QStringLiteral( "gridFramePenThickness" ), QStringLiteral( "0.3" ) ).toDouble();
  mGridFramePenColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "gridFramePenColor" ), QStringLiteral( "0,0,0" ) ) );
  mGridFrameFillColor1 = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "frameFillColor1" ), QStringLiteral( "255,255,255,255" ) ) );
  mGridFrameFillColor2 = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "frameFillColor2" ), QStringLiteral( "0,0,0,255" ) ) );
  mLeftFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "leftFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mRightFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "rightFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mTopFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "topFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );
  mBottomFrameDivisions = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "bottomFrameDivisions" ), QStringLiteral( "0" ) ).toInt() );

  QgsReadWriteContext context;
  context.setPathResolver( mComposition->project()->pathResolver() );

  QDomElement lineStyleElem = itemElem.firstChildElement( QStringLiteral( "lineStyle" ) );
  if ( !lineStyleElem.isNull() )
  {
    QDomElement symbolElem = lineStyleElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      delete mGridLineSymbol;
      mGridLineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context );
    }
  }
  else
  {
    //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
    mGridLineSymbol = QgsLineSymbol::createSimple( QgsStringMap() );
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
      delete mGridMarkerSymbol;
      mGridMarkerSymbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context );
    }
  }

  if ( !mCRS.readXml( itemElem ) )
    mCRS = QgsCoordinateReferenceSystem();

  mBlendMode = static_cast< QPainter::CompositionMode >( itemElem.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() );

  //annotation
  mShowGridAnnotation = ( itemElem.attribute( QStringLiteral( "showAnnotation" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  mGridAnnotationFormat = QgsComposerMapGrid::AnnotationFormat( itemElem.attribute( QStringLiteral( "annotationFormat" ), QStringLiteral( "0" ) ).toInt() );
  mGridAnnotationExpressionString = itemElem.attribute( QStringLiteral( "annotationExpression" ) );
  mGridAnnotationExpression.reset();
  mLeftGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "leftAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "rightAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "topAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationPosition = QgsComposerMapGrid::AnnotationPosition( itemElem.attribute( QStringLiteral( "bottomAnnotationPosition" ), QStringLiteral( "0" ) ).toInt() );
  mLeftGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "leftAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "rightAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "topAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationDisplay = QgsComposerMapGrid::DisplayMode( itemElem.attribute( QStringLiteral( "bottomAnnotationDisplay" ), QStringLiteral( "0" ) ).toInt() );

  mLeftGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "leftAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mRightGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "rightAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mTopGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "topAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
  mBottomGridAnnotationDirection = QgsComposerMapGrid::AnnotationDirection( itemElem.attribute( QStringLiteral( "bottomAnnotationDirection" ), QStringLiteral( "0" ) ).toInt() );
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

void QgsComposerMapGrid::drawGridCrsTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
    QList< QPair< double, QLineF > > &verticalLines, bool calculateLinesOnly )
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
    calculateCrsTransformLines();
  }

  //draw lines
  if ( !calculateLinesOnly )
  {
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

      QList< QgsPointXY >::const_iterator intersectionIt = mTransformedIntersections.constBegin();
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

void QgsComposerMapGrid::calculateCrsTransformLines()
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

  if ( mGridStyle == QgsComposerMapGrid::Cross || mGridStyle == QgsComposerMapGrid::Markers )
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

void QgsComposerMapGrid::draw( QPainter *p )
{
  if ( !mComposerMap || !mEnabled )
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
  double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
  p->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, p );
  context.setForceVectorOutput( true );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  QList< QPair< double, QLineF > > verticalLines;
  QList< QPair< double, QLineF > > horizontalLines;

  //is grid in a different crs than map?
  if ( mGridUnit == MapUnit && mCRS.isValid() && mCRS != mComposerMap->crs() )
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
  p->setClipRect( mComposerMap->mapRectFromScene( mComposerMap->sceneBoundingRect() ).adjusted( -10, -10, 10, 10 ) );
#endif

  if ( mGridFrameStyle != QgsComposerMapGrid::NoFrame )
  {
    drawGridFrame( p, horizontalLines, verticalLines );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( p, horizontalLines, verticalLines, context.expressionContext() );
  }
}

void QgsComposerMapGrid::drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
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
            crossEnd1 = ( ( intersectionPoint - vIt->second.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, vIt->second.p1(), mCrossLength ) : intersectionPoint;
            crossEnd2 = ( ( intersectionPoint - vIt->second.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, vIt->second.p2(), mCrossLength ) : intersectionPoint;
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

void QgsComposerMapGrid::drawGridFrame( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, GridExtension *extension ) const
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

  if ( testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) )
  {
    drawGridFrameBorder( p, leftGridFrame, QgsComposerMapGrid::Left, extension ? &extension->left : nullptr );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameRight ) )
  {
    drawGridFrameBorder( p, rightGridFrame, QgsComposerMapGrid::Right, extension ? &extension->right : nullptr );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameTop ) )
  {
    drawGridFrameBorder( p, topGridFrame, QgsComposerMapGrid::Top, extension ? &extension->top : nullptr );
  }
  if ( testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) )
  {
    drawGridFrameBorder( p, bottomGridFrame, QgsComposerMapGrid::Bottom, extension ? &extension->bottom : nullptr );
  }
  if ( p )
    p->restore();
}

void QgsComposerMapGrid::drawGridLine( const QLineF &line, QgsRenderContext &context ) const
{
  QPolygonF poly;
  poly << line.p1() << line.p2();
  drawGridLine( poly, context );
}

void QgsComposerMapGrid::drawGridLine( const QPolygonF &line, QgsRenderContext &context ) const
{
  if ( !mComposerMap || !mComposerMap->composition() || !mGridLineSymbol )
  {
    return;
  }

  mGridLineSymbol->startRender( context );
  mGridLineSymbol->renderPolyline( line, nullptr, context );
  mGridLineSymbol->stopRender( context );
}

void QgsComposerMapGrid::drawGridMarker( QPointF point, QgsRenderContext &context ) const
{
  if ( !mComposerMap || !mComposerMap->composition() || !mGridMarkerSymbol )
  {
    return;
  }

  mGridMarkerSymbol->startRender( context );
  mGridMarkerSymbol->renderPoint( point, nullptr, context );
  mGridMarkerSymbol->stopRender( context );
}

void QgsComposerMapGrid::drawGridFrameBorder( QPainter *p, const QMap< double, double > &borderPos, QgsComposerMapGrid::BorderSide border, double *extension ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  switch ( mGridFrameStyle )
  {
    case QgsComposerMapGrid::Zebra:
      drawGridFrameZebraBorder( p, borderPos, border, extension );
      break;
    case QgsComposerMapGrid::InteriorTicks:
    case QgsComposerMapGrid::ExteriorTicks:
    case QgsComposerMapGrid::InteriorExteriorTicks:
      drawGridFrameTicks( p, borderPos, border, extension );
      break;

    case QgsComposerMapGrid::LineBorder:
      drawGridFrameLineBorder( p, border, extension );
      break;

    case QgsComposerMapGrid::NoFrame:
      break;
  }

}

void QgsComposerMapGrid::drawGridFrameZebraBorder( QPainter *p, const QMap< double, double > &borderPos, QgsComposerMapGrid::BorderSide border, double *extension ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( extension )
  {
    *extension = mGridFrameWidth + mGridFramePenThickness / 2.0;
    return;
  }

  QMap< double, double > pos = borderPos;

  double currentCoord = 0;
  if ( ( border == QgsComposerMapGrid::Left || border == QgsComposerMapGrid::Right ) && testFrameSideFlag( QgsComposerMapGrid::FrameTop ) )
  {
    currentCoord = - mGridFrameWidth;
    pos.insert( 0, 0 );
  }
  else if ( ( border == QgsComposerMapGrid::Top || border == QgsComposerMapGrid::Bottom ) && testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) )
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

void QgsComposerMapGrid::drawGridFrameTicks( QPainter *p, const QMap< double, double > &borderPos, QgsComposerMapGrid::BorderSide border, double *extension ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( extension )
  {
    if ( mGridFrameStyle != QgsComposerMapGrid::InteriorTicks )
      *extension = mGridFrameWidth;
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

void QgsComposerMapGrid::drawGridFrameLineBorder( QPainter *p, QgsComposerMapGrid::BorderSide border, double *extension ) const
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( extension )
  {
    *extension = mGridFramePenThickness / 2.0;
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

void QgsComposerMapGrid::drawCoordinateAnnotations( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QgsExpressionContext &expressionContext,
    GridExtension *extension ) const
{
  QString currentAnnotationString;
  QList< QPair< double, QLineF > >::const_iterator it = hLines.constBegin();
  for ( ; it != hLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, QgsComposerMapGrid::Latitude, expressionContext );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsComposerMapGrid::Latitude, extension );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsComposerMapGrid::Latitude, extension );
  }

  it = vLines.constBegin();
  for ( ; it != vLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->first, QgsComposerMapGrid::Longitude, expressionContext );
    drawCoordinateAnnotation( p, it->second.p1(), currentAnnotationString, QgsComposerMapGrid::Longitude, extension );
    drawCoordinateAnnotation( p, it->second.p2(), currentAnnotationString, QgsComposerMapGrid::Longitude, extension );
  }
}

void QgsComposerMapGrid::drawCoordinateAnnotation( QPainter *p, QPointF pos, const QString &annotationString, const AnnotationCoordinate coordinateType, GridExtension *extension ) const
{
  if ( !mComposerMap )
  {
    return;
  }
  QgsComposerMapGrid::BorderSide frameBorder = borderForLineCoord( pos, coordinateType );
  double textWidth = QgsComposerUtils::textWidthMM( mGridAnnotationFont, annotationString );
  //relevant for annotations is the height of digits
  double textHeight = extension ? QgsComposerUtils::fontAscentMM( mGridAnnotationFont )
                      : QgsComposerUtils::fontHeightCharacterMM( mGridAnnotationFont, QChar( '0' ) );
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
        if ( extension )
          extension->left = std::max( extension->left, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mLeftGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
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
        if ( extension )
          extension->right = std::max( extension->right, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mRightGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending || mRightGridAnnotationDirection == QgsComposerMapGrid::BoundaryDirection )
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
        if ( extension )
          extension->bottom = std::max( extension->bottom, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mBottomGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
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
        if ( extension )
          extension->top = std::max( extension->top, mAnnotationFrameDistance + gridFrameDistance + textHeight );
      }
      else if ( mTopGridAnnotationDirection == QgsComposerMapGrid::VerticalDescending )
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

void QgsComposerMapGrid::drawAnnotation( QPainter *p, QPointF pos, int rotation, const QString &annotationText ) const
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

QString QgsComposerMapGrid::gridAnnotationString( double value, QgsComposerMapGrid::AnnotationCoordinate coord, QgsExpressionContext &expressionContext ) const
{
  //check if we are using degrees (ie, geographic crs)
  bool geographic = false;
  if ( mCRS.isValid() && mCRS.isGeographic() )
  {
    geographic = true;
  }
  else if ( mComposerMap && mComposerMap->composition() )
  {
    geographic = mComposerMap->crs().isGeographic();
  }

  if ( geographic && coord == QgsComposerMapGrid::Longitude &&
       ( mGridAnnotationFormat == QgsComposerMapGrid::Decimal || mGridAnnotationFormat == QgsComposerMapGrid::DecimalWithSuffix ) )
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

  if ( mGridAnnotationFormat == QgsComposerMapGrid::Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == QgsComposerMapGrid::DecimalWithSuffix )
  {
    QString hemisphere;

    double coordRounded = std::round( value * std::pow( 10.0, mGridAnnotationPrecision ) ) / std::pow( 10.0, mGridAnnotationPrecision );
    if ( coord == QgsComposerMapGrid::Longitude )
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
    expressionContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_axis" ), coord == QgsComposerMapGrid::Longitude ? "x" : "y", true ) );
    if ( !mGridAnnotationExpression )
    {
      mGridAnnotationExpression.reset( new QgsExpression( mGridAnnotationExpressionString ) );
      mGridAnnotationExpression->prepare( &expressionContext );
    }
    return mGridAnnotationExpression->evaluate( &expressionContext ).toString();
  }

  QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatDecimalDegrees;
  QgsCoordinateFormatter::FormatFlags flags = 0;
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
      flags = 0;
      break;

    case DegreeMinutePadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;

    case DegreeMinuteSecondNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = 0;
      break;

    case DegreeMinuteSecondPadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;
  }

  switch ( coord )
  {
    case Longitude:
      return QgsCoordinateFormatter::formatX( value, format, flags );

    case Latitude:
      return QgsCoordinateFormatter::formatY( value, format, flags );
  }

  return QString(); // no warnings
}

int QgsComposerMapGrid::xGridLines( QList< QPair< double, QLineF > > &lines ) const
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
      gridIntervalY *= 10;
      gridOffsetY *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.top() - gridOffsetY ) / gridIntervalY + roundCorrection ) * gridIntervalY + gridOffsetY;

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
      lines.push_back( qMakePair( currentLevel, QLineF( mComposerMap->mapToItemCoords( intersectionList.at( 0 ) ), mComposerMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalY;
  }


  return 0;
}

int QgsComposerMapGrid::yGridLines( QList< QPair< double, QLineF > > &lines ) const
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
      gridIntervalX *= 10;
      gridOffsetX *= 10;
    }
  }

  //consider to round up to the next step in case the left boundary is > 0
  double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.left() - gridOffsetX ) / gridIntervalX + roundCorrection ) * gridIntervalX + gridOffsetX;

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
      lines.push_back( qMakePair( currentLevel, QLineF( mComposerMap->mapToItemCoords( intersectionList.at( 0 ) ), mComposerMap->mapToItemCoords( intersectionList.at( 1 ) ) ) ) );
      gridLineCount++;
    }
    currentLevel += gridIntervalX;
  }

  return 0;
}

int QgsComposerMapGrid::xGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalY <= 0.0 )
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
        gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) ); //transform back to composer coords
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

    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
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

int QgsComposerMapGrid::yGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const
{
  lines.clear();
  if ( !mComposerMap || mGridIntervalX <= 0.0 )
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
        gridLine.append( mComposerMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentY += step;
    }
    //clip grid line to map polygon
    QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mComposerMap->rect() ) );
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

void QgsComposerMapGrid::sortGridLinesOnBorders( const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QMap< double, double > &leftFrameEntries,
    QMap< double, double > &rightFrameEntries, QMap< double, double > &topFrameEntries, QMap< double, double > &bottomFrameEntries ) const
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

bool QgsComposerMapGrid::shouldShowDivisionForSide( QgsComposerMapGrid::AnnotationCoordinate coordinate, QgsComposerMapGrid::BorderSide side ) const
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

bool QgsComposerMapGrid::shouldShowDivisionForDisplayMode( QgsComposerMapGrid::AnnotationCoordinate coordinate, QgsComposerMapGrid::DisplayMode mode ) const
{
  return mode == QgsComposerMapGrid::ShowAll
         || ( mode == QgsComposerMapGrid::LatitudeOnly && coordinate == QgsComposerMapGrid::Latitude )
         || ( mode == QgsComposerMapGrid::LongitudeOnly && coordinate == QgsComposerMapGrid::Longitude );
}

bool sortByDistance( QPair<qreal, QgsComposerMapGrid::BorderSide> a, QPair<qreal, QgsComposerMapGrid::BorderSide> b )
{
  return a.first < b.first;
}

QgsComposerMapGrid::BorderSide QgsComposerMapGrid::borderForLineCoord( QPointF p, const AnnotationCoordinate coordinateType ) const
{
  if ( !mComposerMap )
  {
    return QgsComposerMapGrid::Left;
  }

  double tolerance = std::max( mComposerMap->hasFrame() ? mComposerMap->pen().widthF() : 0.0, 1.0 );

  //check for corner coordinates
  if ( ( p.y() <= tolerance && p.x() <= tolerance ) // top left
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
  QList< QPair<qreal, QgsComposerMapGrid::BorderSide > > distanceToSide;
  distanceToSide << qMakePair( p.x(), QgsComposerMapGrid::Left );
  distanceToSide << qMakePair( mComposerMap->rect().width() - p.x(), QgsComposerMapGrid::Right );
  distanceToSide << qMakePair( p.y(), QgsComposerMapGrid::Top );
  distanceToSide << qMakePair( mComposerMap->rect().height() - p.y(), QgsComposerMapGrid::Bottom );

  std::sort( distanceToSide.begin(), distanceToSide.end(), sortByDistance );
  return distanceToSide.at( 0 ).second;
}

void QgsComposerMapGrid::setLineSymbol( QgsLineSymbol *symbol )
{
  delete mGridLineSymbol;
  mGridLineSymbol = symbol;
}

void QgsComposerMapGrid::setMarkerSymbol( QgsMarkerSymbol *symbol )
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
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationDisplay;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationDisplay;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDisplay;
  }
}

double QgsComposerMapGrid::maxExtension()
{
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  double left = 0.0;
  calculateMaxExtension( top, right, bottom, left );
  return std::max( std::max( std::max( top, right ), bottom ), left );
}

void QgsComposerMapGrid::calculateMaxExtension( double &top, double &right, double &bottom, double &left )
{
  top = 0.0;
  right = 0.0;
  bottom = 0.0;
  left = 0.0;

  if ( !mComposerMap || !mEnabled )
  {
    return;
  }

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, nullptr );
  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  GridExtension extension;

  //collect grid lines
  QList< QPair< double, QLineF > > verticalLines;
  QList< QPair< double, QLineF > > horizontalLines;
  if ( mGridUnit == MapUnit && mCRS.isValid() && mCRS != mComposerMap->crs() )
  {
    drawGridCrsTransform( context, 0, horizontalLines, verticalLines, false );
  }
  else
  {
    drawGridNoTransform( context, 0, horizontalLines, verticalLines, false );
  }

  if ( mGridFrameStyle != QgsComposerMapGrid::NoFrame )
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
  if ( qgsDoubleNear( interval, mGridIntervalX ) )
  {
    return;
  }
  mGridIntervalX = interval;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setIntervalY( const double interval )
{
  if ( qgsDoubleNear( interval, mGridIntervalY ) )
  {
    return;
  }
  mGridIntervalY = interval;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setOffsetX( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetX ) )
  {
    return;
  }
  mGridOffsetX = offset;
  mTransformDirty = true;
}

void QgsComposerMapGrid::setOffsetY( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetY ) )
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

QgsExpressionContext QgsComposerMapGrid::createExpressionContext() const
{
  QgsExpressionContext context = QgsComposerObject::createExpressionContext();
  context.appendScope( new QgsExpressionContextScope( tr( "Grid" ) ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_number" ), 0, true ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_axis" ), "x", true ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "grid_number" ) << QStringLiteral( "grid_axis" ) );
  return context;
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
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationPosition;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationPosition;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationPosition;
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
    case QgsComposerMapGrid::Right:
      return mRightGridAnnotationDirection;
    case QgsComposerMapGrid::Top:
      return mTopGridAnnotationDirection;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomGridAnnotationDirection;
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
    case QgsComposerMapGrid::Right:
      return mRightFrameDivisions;
    case QgsComposerMapGrid::Top:
      return mTopFrameDivisions;
    case QgsComposerMapGrid::Bottom:
    default:
      return mBottomFrameDivisions;
  }
}

int QgsComposerMapGrid::crsGridParams( QgsRectangle &crsRect, QgsCoordinateTransform &inverseTransform ) const
{
  if ( !mComposerMap )
  {
    return 1;
  }

  try
  {
    QgsCoordinateTransform tr( mComposerMap->crs(), mCRS );
    QPolygonF mapPolygon = mComposerMap->transformedMapPolygon();
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

    inverseTransform.setSourceCrs( mCRS );
    inverseTransform.setDestinationCrs( mComposerMap->crs() );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
    return 1;
  }
  return 0;
}

QList<QPolygonF> QgsComposerMapGrid::trimLinesToMap( const QPolygonF &line, const QgsRectangle &rect )
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
