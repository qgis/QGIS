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

#include "qgsmessagelog.h"
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
#include "qgsstyleentityvisitor.h"
#include "qgstextrenderer.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

#include <QVector2D>
#include <math.h>

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
  return qobject_cast<QgsLayoutItemMapGrid *>( item );
}

QgsLayoutItemMapGrid *QgsLayoutItemMapGridStack::grid( const int index ) const
{
  QgsLayoutItemMapItem *item = QgsLayoutItemMapItemStack::item( index );
  return qobject_cast<QgsLayoutItemMapGrid *>( item );
}

QList<QgsLayoutItemMapGrid *> QgsLayoutItemMapGridStack::asList() const
{
  QList< QgsLayoutItemMapGrid * > list;
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( QgsLayoutItemMapGrid *grid = qobject_cast<QgsLayoutItemMapGrid *>( item ) )
    {
      list.append( grid );
    }
  }
  return list;
}

QgsLayoutItemMapGrid &QgsLayoutItemMapGridStack::operator[]( int idx )
{
  QgsLayoutItemMapItem *item = mItems.at( idx );
  QgsLayoutItemMapGrid *grid = qobject_cast<QgsLayoutItemMapGrid *>( item );
  return *grid;
}

bool QgsLayoutItemMapGridStack::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  removeItems();

  //read grid stack
  const QDomNodeList mapGridNodeList = elem.elementsByTagName( QStringLiteral( "ComposerMapGrid" ) );
  for ( int i = 0; i < mapGridNodeList.size(); ++i )
  {
    const QDomElement mapGridElem = mapGridNodeList.at( i ).toElement();
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
    if ( QgsLayoutItemMapGrid *grid = qobject_cast<QgsLayoutItemMapGrid *>( item ) )
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

QVector2D borderToVector2D( QgsLayoutItemMapGrid::BorderSide border )
{
  // returns a border as a vector2D for vector arithmetic
  switch ( border )
  {
    case QgsLayoutItemMapGrid::Left:
      return QVector2D( 0, 1 );
    case QgsLayoutItemMapGrid::Top:
      return QVector2D( -1, 0 );
    case QgsLayoutItemMapGrid::Right:
      return QVector2D( 0, -1 );
    case QgsLayoutItemMapGrid::Bottom:
      return QVector2D( 1, 0 );
  }
  return QVector2D();
}
QVector2D borderToNormal2D( QgsLayoutItemMapGrid::BorderSide border )
{
  // returns a border normal (towards center) as a vector2D for vector arithmetic
  const QVector2D borderVector = borderToVector2D( border );
  return QVector2D( borderVector.y(), -borderVector.x() );
}

QgsLayoutItemMapGrid::QgsLayoutItemMapGrid( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutItemMapItem( name, map )
  , mGridFrameSides( QgsLayoutItemMapGrid::FrameLeft | QgsLayoutItemMapGrid::FrameRight |
                     QgsLayoutItemMapGrid::FrameTop | QgsLayoutItemMapGrid::FrameBottom )
{
  //get default layout font from settings
  const QgsSettings settings;
  const QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    QFont font;
    font.setFamily( defaultFontString );
    mAnnotationFormat.setFont( font );
  }

  createDefaultGridLineSymbol();
  createDefaultGridMarkerSymbol();

  connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemMapGrid::refreshDataDefinedProperties );
  connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemMapGrid::refreshDataDefinedProperties );
  connect( mMap, &QgsLayoutItemMap::crsChanged, this, [ = ]
  {
    if ( !mCRS.isValid() )
      emit crsChanged();
  } );
}

QgsLayoutItemMapGrid::~QgsLayoutItemMapGrid() = default;

void QgsLayoutItemMapGrid::createDefaultGridLineSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  mGridLineSymbol.reset( QgsLineSymbol::createSimple( properties ) );
}

void QgsLayoutItemMapGrid::createDefaultGridMarkerSymbol()
{
  QVariantMap properties;
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
  const QDomElement gridLineStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridLineSymbol.get(), doc, context );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( QStringLiteral( "markerStyle" ) );
  const QDomElement gridMarkerStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridMarkerSymbol.get(), doc, context );
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
  mapGridElem.setAttribute( QStringLiteral( "rotatedTicksLengthMode" ), mRotatedTicksLengthMode );
  mapGridElem.setAttribute( QStringLiteral( "rotatedTicksEnabled" ), mRotatedTicksEnabled );
  mapGridElem.setAttribute( QStringLiteral( "rotatedTicksMinimumAngle" ), QString::number( mRotatedTicksMinimumAngle ) );
  mapGridElem.setAttribute( QStringLiteral( "rotatedTicksMarginToCorner" ), QString::number( mRotatedTicksMarginToCorner ) );
  mapGridElem.setAttribute( QStringLiteral( "rotatedAnnotationsLengthMode" ), mRotatedAnnotationsLengthMode );
  mapGridElem.setAttribute( QStringLiteral( "rotatedAnnotationsEnabled" ), mRotatedAnnotationsEnabled );
  mapGridElem.setAttribute( QStringLiteral( "rotatedAnnotationsMinimumAngle" ), QString::number( mRotatedAnnotationsMinimumAngle ) );
  mapGridElem.setAttribute( QStringLiteral( "rotatedAnnotationsMarginToCorner" ), QString::number( mRotatedAnnotationsMarginToCorner ) );
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
  mapGridElem.appendChild( mAnnotationFormat.writeXml( doc, context ) );
  mapGridElem.setAttribute( QStringLiteral( "annotationPrecision" ), mGridAnnotationPrecision );
  mapGridElem.setAttribute( QStringLiteral( "unit" ), mGridUnit );
  mapGridElem.setAttribute( QStringLiteral( "blendMode" ), mBlendMode );
  mapGridElem.setAttribute( QStringLiteral( "minimumIntervalWidth" ), QString::number( mMinimumIntervalWidth ) );
  mapGridElem.setAttribute( QStringLiteral( "maximumIntervalWidth" ), QString::number( mMaximumIntervalWidth ) );

  const bool ok = QgsLayoutItemMapItem::writeXml( mapGridElem, doc, context );
  elem.appendChild( mapGridElem );
  return ok;
}

bool QgsLayoutItemMapGrid::readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( doc )
  if ( itemElem.isNull() )
  {
    return false;
  }

  const bool ok = QgsLayoutItemMapItem::readXml( itemElem, doc, context );

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
  mRotatedTicksLengthMode = TickLengthMode( itemElem.attribute( QStringLiteral( "rotatedTicksLengthMode" ), QStringLiteral( "0" ) ).toInt() );
  mRotatedTicksEnabled = itemElem.attribute( QStringLiteral( "rotatedTicksEnabled" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  mRotatedTicksMinimumAngle = itemElem.attribute( QStringLiteral( "rotatedTicksMinimumAngle" ), QStringLiteral( "0" ) ).toDouble();
  mRotatedTicksMarginToCorner = itemElem.attribute( QStringLiteral( "rotatedTicksMarginToCorner" ), QStringLiteral( "0" ) ).toDouble();
  mRotatedAnnotationsLengthMode = TickLengthMode( itemElem.attribute( QStringLiteral( "rotatedAnnotationsLengthMode" ), QStringLiteral( "0" ) ).toInt() );
  mRotatedAnnotationsEnabled = itemElem.attribute( QStringLiteral( "rotatedAnnotationsEnabled" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  mRotatedAnnotationsMinimumAngle = itemElem.attribute( QStringLiteral( "rotatedAnnotationsMinimumAngle" ), QStringLiteral( "0" ) ).toDouble();
  mRotatedAnnotationsMarginToCorner = itemElem.attribute( QStringLiteral( "rotatedAnnotationsMarginToCorner" ), QStringLiteral( "0" ) ).toDouble();

  const QDomElement lineStyleElem = itemElem.firstChildElement( QStringLiteral( "lineStyle" ) );
  if ( !lineStyleElem.isNull() )
  {
    const QDomElement symbolElem = lineStyleElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      mGridLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    }
  }
  else
  {
    //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
    mGridLineSymbol.reset( QgsLineSymbol::createSimple( QVariantMap() ) );
    mGridLineSymbol->setWidth( itemElem.attribute( QStringLiteral( "penWidth" ), QStringLiteral( "0" ) ).toDouble() );
    mGridLineSymbol->setColor( QColor( itemElem.attribute( QStringLiteral( "penColorRed" ), QStringLiteral( "0" ) ).toInt(),
                                       itemElem.attribute( QStringLiteral( "penColorGreen" ), QStringLiteral( "0" ) ).toInt(),
                                       itemElem.attribute( QStringLiteral( "penColorBlue" ), QStringLiteral( "0" ) ).toInt() ) );
  }

  const QDomElement markerStyleElem = itemElem.firstChildElement( QStringLiteral( "markerStyle" ) );
  if ( !markerStyleElem.isNull() )
  {
    const QDomElement symbolElem = markerStyleElem.firstChildElement( QStringLiteral( "symbol" ) );
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

  if ( !itemElem.firstChildElement( "text-style" ).isNull() )
  {
    mAnnotationFormat.readXml( itemElem, context );
  }
  else
  {
    QFont font;
    if ( !QgsFontUtils::setFromXmlChildNode( font, itemElem, "annotationFontProperties" ) )
    {
      font.fromString( itemElem.attribute( "annotationFont", QString() ) );
    }
    mAnnotationFormat.setFont( font );
    mAnnotationFormat.setSize( font.pointSizeF() );
    mAnnotationFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
    mAnnotationFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( "annotationFontColor", "0,0,0,255" ) ) );
  }

  mGridAnnotationPrecision = itemElem.attribute( QStringLiteral( "annotationPrecision" ), QStringLiteral( "3" ) ).toInt();
  const int gridUnitInt = itemElem.attribute( QStringLiteral( "unit" ), QString::number( MapUnit ) ).toInt();
  mGridUnit = ( gridUnitInt <= static_cast< int >( DynamicPageSizeBased ) ) ? static_cast< GridUnit >( gridUnitInt ) : MapUnit;
  mMinimumIntervalWidth = itemElem.attribute( QStringLiteral( "minimumIntervalWidth" ), QStringLiteral( "50" ) ).toDouble();
  mMaximumIntervalWidth = itemElem.attribute( QStringLiteral( "maximumIntervalWidth" ), QStringLiteral( "100" ) ).toDouble();

  refreshDataDefinedProperties();
  return ok;
}

void QgsLayoutItemMapGrid::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCRS == crs )
    return;

  mCRS = crs;
  mTransformDirty = true;
  emit crsChanged();
}

bool QgsLayoutItemMapGrid::usesAdvancedEffects() const
{
  return mBlendMode != QPainter::CompositionMode_SourceOver;
}

QPolygonF QgsLayoutItemMapGrid::scalePolygon( const QPolygonF &polygon, const double scale ) const
{
  const QTransform t = QTransform::fromScale( scale, scale );
  return t.map( polygon );
}

void QgsLayoutItemMapGrid::drawGridCrsTransform( QgsRenderContext &context, double dotsPerMM, bool calculateLinesOnly ) const
{
  if ( !mMap || !mEvaluatedEnabled )
  {
    return;
  }

  //has map extent/scale changed?
  const QPolygonF mapPolygon = mMap->transformedMapPolygon();
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
      QList< GridLine >::const_iterator gridIt = mGridLines.constBegin();
      for ( ; gridIt != mGridLines.constEnd(); ++gridIt )
      {
        drawGridLine( scalePolygon( gridIt->line, dotsPerMM ), context );
      }
    }
    else if ( mGridStyle == QgsLayoutItemMapGrid::Cross || mGridStyle == QgsLayoutItemMapGrid::Markers )
    {
      const double maxX = mMap->rect().width();
      const double maxY = mMap->rect().height();

      QList< QgsPointXY >::const_iterator intersectionIt = mTransformedIntersections.constBegin();
      for ( ; intersectionIt != mTransformedIntersections.constEnd(); ++intersectionIt )
      {
        const double x = intersectionIt->x();
        const double y = intersectionIt->y();
        if ( mGridStyle == QgsLayoutItemMapGrid::Cross )
        {
          //ensure that crosses don't overshoot the map item bounds
          const QLineF line1 = QLineF( x - mEvaluatedCrossLength, y, x + mEvaluatedCrossLength, y );
          line1.p1().rx() = line1.p1().x() < 0 ? 0 : line1.p1().x();
          line1.p2().rx() = line1.p2().x() > maxX ? maxX : line1.p2().x();
          const QLineF line2 = QLineF( x, y - mEvaluatedCrossLength, x, y + mEvaluatedCrossLength );
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
}

void QgsLayoutItemMapGrid::calculateCrsTransformLines() const
{
  QgsRectangle crsBoundingRect;
  QgsCoordinateTransform inverseTr;
  if ( crsGridParams( crsBoundingRect, inverseTr ) != 0 )
  {
    return;
  }

  // calculate grid lines
  mGridLines.clear();
  xGridLinesCrsTransform( crsBoundingRect, inverseTr );
  yGridLinesCrsTransform( crsBoundingRect, inverseTr );

  if ( mGridStyle == QgsLayoutItemMapGrid::Cross || mGridStyle == QgsLayoutItemMapGrid::Markers )
  {
    //cross or markers style - we also need to calculate intersections of lines

    //first convert lines to QgsGeometry
    QList< QgsGeometry > xLines;
    QList< QgsGeometry > yLines;
    QList< GridLine >::const_iterator gridIt = mGridLines.constBegin();
    for ( ; gridIt != mGridLines.constEnd(); ++gridIt )
    {

      QgsPolylineXY line;
      for ( int i = 0; i < gridIt->line.size(); ++i )
      {
        line.append( QgsPointXY( gridIt->line.at( i ).x(), gridIt->line.at( i ).y() ) );
      }
      if ( gridIt->coordinateType == AnnotationCoordinate::Longitude )
        yLines << QgsGeometry::fromPolylineXY( line );
      else if ( gridIt->coordinateType == AnnotationCoordinate::Latitude )
        xLines << QgsGeometry::fromPolylineXY( line );
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
        const QgsGeometry intersects = ( *yLineIt ).intersection( ( *xLineIt ) );
        if ( intersects.isNull() )
          continue;

        //go through all intersections and draw grid markers/crosses
        int i = 0;
        QgsPointXY vertex = intersects.vertexAt( i );
        while ( !vertex.isEmpty() )
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
  if ( !mMap || !mEvaluatedEnabled )
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

  const QRectF thisPaintRect = QRectF( 0, 0, mMap->rect().width(), mMap->rect().height() );
  p->setClipRect( thisPaintRect );
  if ( thisPaintRect != mPrevPaintRect )
  {
    //rect has changed, so need to recalculate transform
    mTransformDirty = true;
    mPrevPaintRect = thisPaintRect;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  const double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
  p->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, p );
  context.setForceVectorOutput( true );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  const QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  //is grid in a different crs than map?
  switch ( mGridUnit )
  {
    case MapUnit:
    case DynamicPageSizeBased:
      if ( mCRS.isValid() && mCRS != mMap->crs() )
      {
        drawGridCrsTransform( context, dotsPerMM );
        break;
      }

      FALLTHROUGH
    case CM:
    case MM:
      drawGridNoTransform( context, dotsPerMM );
      break;
  }
  p->restore();

  p->setClipping( false );
#ifdef Q_OS_MAC
  //QPainter::setClipping(false) seems to be broken on OSX (#12747). So we hack around it by
  //setting a larger clip rect
  p->setClipRect( mMap->mapRectFromScene( mMap->sceneBoundingRect() ).adjusted( -10, -10, 10, 10 ) );
#endif


  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame || mShowGridAnnotation )
    updateGridLinesAnnotationsPositions();

  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame )
  {
    drawGridFrame( p );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( context, context.expressionContext() );
  }
}

void QgsLayoutItemMapGrid::updateGridLinesAnnotationsPositions() const
{
  QList< GridLine >::iterator it = mGridLines.begin();
  for ( ; it != mGridLines.end(); ++it )
  {
    it->startAnnotation.border = borderForLineCoord( it->line.first(), it->coordinateType );
    it->endAnnotation.border = borderForLineCoord( it->line.last(), it->coordinateType );
    it->startAnnotation.position = QVector2D( it->line.first() );
    it->endAnnotation.position = QVector2D( it->line.last() );
    it->startAnnotation.vector = QVector2D( it->line.at( 1 ) - it->line.first() ).normalized();
    it->endAnnotation.vector = QVector2D( it->line.at( it->line.count() - 2 ) - it->line.last() ).normalized();
    const QVector2D normS = borderToNormal2D( it->startAnnotation.border );
    it->startAnnotation.angle = atan2( it->startAnnotation.vector.x() * normS.y() - it->startAnnotation.vector.y() * normS.x(), it->startAnnotation.vector.x() * normS.x() + it->startAnnotation.vector.y() * normS.y() );
    const QVector2D normE = borderToNormal2D( it->endAnnotation.border );
    it->endAnnotation.angle = atan2( it->endAnnotation.vector.x() * normE.y() - it->endAnnotation.vector.y() * normE.x(), it->endAnnotation.vector.x() * normE.x() + it->endAnnotation.vector.y() * normE.y() );
  }
}

void QgsLayoutItemMapGrid::drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, bool calculateLinesOnly ) const
{
  //get line positions
  mGridLines.clear();
  yGridLines();
  xGridLines();

  if ( calculateLinesOnly )
    return;

  QList< GridLine >::const_iterator vIt = mGridLines.constBegin();
  QList< GridLine >::const_iterator hIt = mGridLines.constBegin();

  //simple approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == QgsLayoutItemMapGrid::Solid )
  {
    //we need to scale line coordinates to dots, rather than mm, since the painter has already been scaled to dots
    //this is done by multiplying each line coordinate by dotsPerMM
    QLineF line;
    for ( ; vIt != mGridLines.constEnd(); ++vIt )
    {
      if ( vIt->coordinateType != AnnotationCoordinate::Longitude )
        continue;
      line = QLineF( vIt->line.first() * dotsPerMM, vIt->line.last() * dotsPerMM );
      drawGridLine( line, context );
    }

    for ( ; hIt != mGridLines.constEnd(); ++hIt )
    {
      if ( hIt->coordinateType != AnnotationCoordinate::Latitude )
        continue;
      line = QLineF( hIt->line.first() * dotsPerMM, hIt->line.last() * dotsPerMM );
      drawGridLine( line, context );
    }
  }
  else if ( mGridStyle != QgsLayoutItemMapGrid::FrameAnnotationsOnly ) //cross or markers
  {
    QLineF l1, l2;
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != mGridLines.constEnd(); ++vIt )
    {
      if ( vIt->coordinateType != AnnotationCoordinate::Longitude )
        continue;

      l1 = QLineF( vIt->line.first(), vIt->line.last() );

      //test for intersection with every horizontal line
      hIt = mGridLines.constBegin();
      for ( ; hIt != mGridLines.constEnd(); ++hIt )
      {
        if ( hIt->coordinateType != AnnotationCoordinate::Latitude )
          continue;

        l2 = QLineF( hIt->line.first(), hIt->line.last() );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        if ( l2.intersect( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
        if ( l2.intersects( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
        {
          if ( mGridStyle == QgsLayoutItemMapGrid::Cross )
          {
            //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
            crossEnd1 = ( ( intersectionPoint - l1.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p1(), mEvaluatedCrossLength ) : intersectionPoint;
            crossEnd2 = ( ( intersectionPoint - l1.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p2(), mEvaluatedCrossLength ) : intersectionPoint;
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

    hIt = mGridLines.constBegin();
    for ( ; hIt != mGridLines.constEnd(); ++hIt )
    {
      if ( hIt->coordinateType != AnnotationCoordinate::Latitude )
        continue;

      l1 = QLineF( hIt->line.first(), hIt->line.last() );

      vIt = mGridLines.constBegin();
      for ( ; vIt != mGridLines.constEnd(); ++vIt )
      {
        if ( vIt->coordinateType != AnnotationCoordinate::Longitude )
          continue;

        l2 = QLineF( vIt->line.first(), vIt->line.last() );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        if ( l2.intersect( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
        if ( l2.intersects( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
        {
          //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
          crossEnd1 = ( ( intersectionPoint - l1.p1() ).manhattanLength() > 0.01 ) ?
                      QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p1(), mEvaluatedCrossLength ) : intersectionPoint;
          crossEnd2 = ( ( intersectionPoint - l1.p2() ).manhattanLength() > 0.01 )  ?
                      QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p2(), mEvaluatedCrossLength ) : intersectionPoint;
          //draw line using coordinates scaled to dots
          drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
        }
      }
    }
  }
}

void QgsLayoutItemMapGrid::drawGridFrame( QPainter *p, GridExtension *extension ) const
{
  if ( p )
  {
    p->save();
    p->setRenderHint( QPainter::Antialiasing, mMap->layout()->renderContext().flags() & QgsLayoutRenderContext::FlagAntialiasing );
  }


  switch ( mGridFrameStyle )
  {
    case QgsLayoutItemMapGrid::Zebra:
    case QgsLayoutItemMapGrid::ZebraNautical:
      drawGridFrameZebra( p, extension );
      break;
    case QgsLayoutItemMapGrid::InteriorTicks:
    case QgsLayoutItemMapGrid::ExteriorTicks:
    case QgsLayoutItemMapGrid::InteriorExteriorTicks:
      drawGridFrameTicks( p, extension );
      break;

    case QgsLayoutItemMapGrid::LineBorder:
    case QgsLayoutItemMapGrid::LineBorderNautical:
      drawGridFrameLine( p, extension );
      break;

    case QgsLayoutItemMapGrid::NoFrame:
      break;
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

void QgsLayoutItemMapGrid::drawGridFrameZebra( QPainter *p, GridExtension *extension ) const
{
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) )
  {
    drawGridFrameZebraBorder( p, QgsLayoutItemMapGrid::Left, extension ? &extension->left : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) )
  {
    drawGridFrameZebraBorder( p, QgsLayoutItemMapGrid::Right, extension ? &extension->right : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
  {
    drawGridFrameZebraBorder( p, QgsLayoutItemMapGrid::Top, extension ? &extension->top : nullptr );
  }
  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
  {
    drawGridFrameZebraBorder( p, QgsLayoutItemMapGrid::Bottom, extension ? &extension->bottom : nullptr );
  }
}

void QgsLayoutItemMapGrid::drawGridFrameZebraBorder( QPainter *p, BorderSide border, double *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( extension )
  {
    *extension = mEvaluatedGridFrameMargin + mEvaluatedGridFrameWidth + mEvaluatedGridFrameLineThickness / 2.0;
    return;
  }

  double currentCoord = 0.0;
  bool color1 = false;
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  bool drawTLBox = false;
  bool drawTRBox = false;
  bool drawBLBox = false;
  bool drawBRBox = false;

  QMap< double, double > pos = QMap< double, double >();
  QList< GridLine >::const_iterator it = mGridLines.constBegin();
  for ( ; it != mGridLines.constEnd(); ++it )
  {
    // for first and last point of the line
    for ( int i = 0 ; i < 2 ; ++i )
    {
      const GridLineAnnotation annot = ( i == 0 ) ? it->startAnnotation : it->endAnnotation;

      // we skip if the point is on another border
      if ( annot.border != border )
        continue;

      if ( ! shouldShowDivisionForSide( it->coordinateType, annot.border ) )
        continue;

      if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
        pos.insert( annot.position.y(), it->coordinate );
      else
        pos.insert( annot.position.x(), it->coordinate );
    }
  }


  if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
  {
    pos.insert( mMap->rect().height(), mMap->rect().height() );
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
    {
      drawBLBox = border == QgsLayoutItemMapGrid::Left;
      drawBRBox = border == QgsLayoutItemMapGrid::Right;
    }
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
    {
      drawTLBox = border == QgsLayoutItemMapGrid::Left;
      drawTRBox = border == QgsLayoutItemMapGrid::Right;
    }
    if ( !drawTLBox && border == QgsLayoutItemMapGrid::Left )
      color1 = true;
  }
  else if ( border == QgsLayoutItemMapGrid::Top || border == QgsLayoutItemMapGrid::Bottom )
  {
    pos.insert( mMap->rect().width(), mMap->rect().width() );
  }

  //set pen to current frame pen
  QPen framePen = QPen( mGridFramePenColor );
  framePen.setWidthF( mEvaluatedGridFrameLineThickness );
  framePen.setJoinStyle( Qt::MiterJoin );
  p->setPen( framePen );

  QMap< double, double >::const_iterator posIt = pos.constBegin();
  for ( ; posIt != pos.constEnd(); ++posIt )
  {
    p->setBrush( QBrush( color1 ? mGridFrameFillColor1 : mGridFrameFillColor2 ) );
    if ( border == QgsLayoutItemMapGrid::Left || border == QgsLayoutItemMapGrid::Right )
    {
      height = posIt.key() - currentCoord;
      width = mEvaluatedGridFrameWidth;
      x = ( border == QgsLayoutItemMapGrid::Left ) ? -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ) : mMap->rect().width() + mEvaluatedGridFrameMargin;
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mEvaluatedGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == QgsLayoutItemMapGrid::Top ) ? -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ) : mMap->rect().height() + mEvaluatedGridFrameMargin;
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    color1 = !color1;
  }

  if ( mGridFrameStyle == ZebraNautical || qgsDoubleNear( mEvaluatedGridFrameMargin, 0.0 ) )
  {
    //draw corners
    width = height = ( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ) ;
    p->setBrush( QBrush( mGridFrameFillColor1 ) );
    if ( drawTLBox )
      p->drawRect( QRectF( -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ), -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ), width, height ) );
    if ( drawTRBox )
      p->drawRect( QRectF( mMap->rect().width(), -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ), width, height ) );
    if ( drawBLBox )
      p->drawRect( QRectF( -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ), mMap->rect().height(), width, height ) );
    if ( drawBRBox )
      p->drawRect( QRectF( mMap->rect().width(), mMap->rect().height(), width, height ) );
  }
}

void QgsLayoutItemMapGrid::drawGridFrameTicks( QPainter *p, GridExtension *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  //set pen to current frame pen
  if ( p )
  {
    QPen framePen = QPen( mGridFramePenColor );
    framePen.setWidthF( mEvaluatedGridFrameLineThickness );
    framePen.setCapStyle( Qt::FlatCap );
    p->setBrush( Qt::NoBrush );
    p->setPen( framePen );
  }

  QList< GridLine >::iterator it = mGridLines.begin();
  for ( ; it != mGridLines.end(); ++it )
  {
    // for first and last point of the line
    for ( int i = 0 ; i < 2 ; ++i )
    {
      const GridLineAnnotation annot = ( i == 0 ) ? it->startAnnotation : it->endAnnotation;

      if ( ! shouldShowDivisionForSide( it->coordinateType, annot.border ) )
        continue;

      // If the angle is below the threshold, we don't draw the annotation
      if ( abs( annot.angle ) / M_PI * 180.0 > 90.0 - mRotatedTicksMinimumAngle + 0.0001 )
        continue;

      // Skip outwards facing annotations that are below mRotatedTicksMarginToCorner
      bool facingLeft;
      bool facingRight;
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorExteriorTicks )
      {
        facingLeft = ( annot.angle != 0 );
        facingRight = ( annot.angle != 0 );
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        facingLeft = ( annot.angle > 0 );
        facingRight = ( annot.angle < 0 );
      }
      else
      {
        facingLeft = ( annot.angle < 0 );
        facingRight = ( annot.angle > 0 );
      }

      if ( annot.border == BorderSide::Top && ( ( facingLeft && annot.position.x() < mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.x() > mMap->rect().width() - mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == BorderSide::Bottom && ( ( facingLeft && annot.position.x() > mMap->rect().width() - mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.x() < mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == BorderSide::Left && ( ( facingLeft && annot.position.y() > mMap->rect().height() - mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.y() < mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == BorderSide::Right && ( ( facingLeft && annot.position.y() < mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.y() > mMap->rect().height() - mRotatedTicksMarginToCorner ) ) )
        continue;

      const QVector2D normalVector = borderToNormal2D( annot.border );
      const QVector2D vector = ( mRotatedTicksEnabled ) ? annot.vector : normalVector;

      double fA = mEvaluatedGridFrameMargin; // point near to frame
      double fB = mEvaluatedGridFrameMargin + mEvaluatedGridFrameWidth; // point far from frame

      if ( mRotatedTicksEnabled && mRotatedTicksLengthMode == OrthogonalTicks )
      {
        fA /= QVector2D::dotProduct( vector, normalVector );
        fB /= QVector2D::dotProduct( vector, normalVector );
      }

      // extents isn't computed accurately
      if ( extension )
      {
        if ( mGridFrameStyle != QgsLayoutItemMapGrid::InteriorTicks )
          extension->UpdateBorder( annot.border, fB );
        continue;
      }

      QVector2D pA;
      QVector2D pB;
      if ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks )
      {
        pA = annot.position + fA * vector;
        pB = annot.position + fB * vector;
      }
      else if ( mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks )
      {
        pA = annot.position - fA * vector;
        pB = annot.position - fB * vector;
      }
      else // InteriorExteriorTicks
      {
        pA = annot.position - fB * vector;
        pB = annot.position + ( fB - 2.0 * mEvaluatedGridFrameMargin ) * vector;
      }
      p->drawLine( QLineF( pA.toPointF(), pB.toPointF() ) );

    }
  }
}

void QgsLayoutItemMapGrid::drawGridFrameLine( QPainter *p, GridExtension *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( p )
  {
    //set pen to current frame pen
    QPen framePen = QPen( mGridFramePenColor );
    framePen.setWidthF( mEvaluatedGridFrameLineThickness );
    framePen.setCapStyle( Qt::SquareCap );
    p->setBrush( Qt::NoBrush );
    p->setPen( framePen );
  }

  const bool drawDiagonals = mGridFrameStyle == LineBorderNautical && !qgsDoubleNear( mEvaluatedGridFrameMargin, 0.0 );

  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) )
  {
    if ( extension )
      extension->UpdateBorder( QgsLayoutItemMapGrid::Left, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) )
  {
    if ( extension )
      extension->UpdateBorder( QgsLayoutItemMapGrid::Right, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( mMap->rect().width() + mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
  {
    if ( extension )
      extension->UpdateBorder( QgsLayoutItemMapGrid::Top, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
  {
    if ( extension )
      extension->UpdateBorder( QgsLayoutItemMapGrid::Bottom, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( ! extension && drawDiagonals )
  {
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) || testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
    {
      //corner left-top
      const double X1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0;
      const double Y1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0;
      p->drawLine( QLineF( 0, 0, X1, Y1 ) );
    }
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) || testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
    {
      //corner right-bottom
      const double X1 = mMap->rect().width() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      const double Y1 = mMap->rect().height() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      p->drawLine( QLineF( mMap->rect().width(), mMap->rect().height(), X1, Y1 ) );
    }
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) || testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) )
    {
      //corner right-top
      const double X1 = mMap->rect().width() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      const double Y1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 ;
      p->drawLine( QLineF( mMap->rect().width(), 0, X1, Y1 ) );
    }
    if ( testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) || testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) )
    {
      //corner left-bottom
      const double X1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 ;
      const double Y1 = mMap->rect().height() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      p->drawLine( QLineF( 0, mMap->rect().height(), X1, Y1 ) );
    }
  }
}

void QgsLayoutItemMapGrid::drawCoordinateAnnotations( QgsRenderContext &context, QgsExpressionContext &expressionContext,
    GridExtension *extension ) const
{
  QString currentAnnotationString;
  QList< GridLine >::const_iterator it = mGridLines.constBegin();
  for ( ; it != mGridLines.constEnd(); ++it )
  {
    currentAnnotationString = gridAnnotationString( it->coordinate, it->coordinateType, expressionContext );
    drawCoordinateAnnotation( context, it->startAnnotation, currentAnnotationString, it->coordinateType, extension );
    drawCoordinateAnnotation( context, it->endAnnotation, currentAnnotationString, it->coordinateType, extension );
  }
}

void QgsLayoutItemMapGrid::drawCoordinateAnnotation( QgsRenderContext &context, GridLineAnnotation annot, const QString &annotationString, const AnnotationCoordinate coordinateType, GridExtension *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( ! shouldShowAnnotationForSide( coordinateType, annot.border ) )
    return;

  const QgsLayoutItemMapGrid::BorderSide frameBorder = annot.border;
  double textWidth = QgsTextRenderer::textWidth( context, mAnnotationFormat, QStringList() << annotationString ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  if ( extension )
    textWidth *= 1.1; // little bit of extra padding when we are calculating the bounding rect, to account for antialiasing

  //relevant for annotations is the height of digits
  const double textHeight = ( extension ? ( QgsTextRenderer::textHeight( context, mAnnotationFormat, QChar(), true ) )
                              : ( QgsTextRenderer::textHeight( context, mAnnotationFormat, '0', false ) ) ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  double xpos = annot.position.x();
  double ypos = annot.position.y();
  QPointF anchor = QPointF();
  int rotation = 0;

  const AnnotationPosition anotPos = annotationPosition( frameBorder );
  const AnnotationDirection anotDir = annotationDirection( frameBorder );

  // If the angle is below the threshold, we don't draw the annotation
  if ( abs( annot.angle ) / M_PI * 180.0 > 90.0 - mRotatedAnnotationsMinimumAngle + 0.0001 )
    return;

  const QVector2D normalVector = borderToNormal2D( annot.border );
  const QVector2D vector = ( mRotatedAnnotationsEnabled ) ? annot.vector : normalVector;

  // Distance to frame
  double f = mEvaluatedAnnotationFrameDistance;

  // Adapt distance to frame using the frame width and line thickness into account
  const bool isOverTick = ( anotDir == QgsLayoutItemMapGrid::AboveTick || anotDir == QgsLayoutItemMapGrid::OnTick || anotDir == QgsLayoutItemMapGrid::UnderTick );
  const bool hasInteriorMargin = ! isOverTick && ( mGridFrameStyle == QgsLayoutItemMapGrid::InteriorTicks || mGridFrameStyle == QgsLayoutItemMapGrid::InteriorExteriorTicks );
  const bool hasExteriorMargin = ! isOverTick && ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ExteriorTicks || mGridFrameStyle == QgsLayoutItemMapGrid::InteriorExteriorTicks || mGridFrameStyle == QgsLayoutItemMapGrid::ZebraNautical );
  const bool hasBorderWidth = ( mGridFrameStyle == QgsLayoutItemMapGrid::Zebra || mGridFrameStyle == QgsLayoutItemMapGrid::ZebraNautical || mGridFrameStyle == QgsLayoutItemMapGrid::LineBorder || mGridFrameStyle == QgsLayoutItemMapGrid::LineBorderNautical );
  if ( ( anotPos == QgsLayoutItemMapGrid::InsideMapFrame && hasInteriorMargin ) || ( anotPos == QgsLayoutItemMapGrid::OutsideMapFrame && hasExteriorMargin ) )
    f += mEvaluatedGridFrameWidth;
  if ( hasBorderWidth )
    f += mEvaluatedGridFrameLineThickness / 2.0;

  if ( anotPos == QgsLayoutItemMapGrid::OutsideMapFrame )
    f *= -1;

  if ( mRotatedAnnotationsEnabled && mRotatedAnnotationsLengthMode == OrthogonalTicks )
  {
    f /= QVector2D::dotProduct( vector, normalVector );
  }

  const QVector2D pos = annot.position + f * vector;
  xpos = pos.x();
  ypos = pos.y();

  const bool outside = ( anotPos == QgsLayoutItemMapGrid::OutsideMapFrame );

  if (
    anotDir == QgsLayoutItemMapGrid::AboveTick ||
    anotDir == QgsLayoutItemMapGrid::OnTick ||
    anotDir == QgsLayoutItemMapGrid::UnderTick
  )
  {

    rotation = atan2( vector.y(), vector.x() ) / M_PI * 180;

    if ( rotation <= -90 || rotation > 90 )
    {
      rotation += 180;
      anchor.setX( outside ? 0 : textWidth ); // left / right
    }
    else
    {
      anchor.setX( outside ? textWidth : 0 ); // right / left
    }

    if ( anotDir == QgsLayoutItemMapGrid::AboveTick )
      anchor.setY( 0.5 * textHeight ); // bottom
    else if ( anotDir == QgsLayoutItemMapGrid::UnderTick )
      anchor.setY( -1.5 * textHeight ); // top
    else // OnTick
      anchor.setY( -0.5 * textHeight ); // middle

  }
  else if ( anotDir == QgsLayoutItemMapGrid::Horizontal )
  {
    rotation = 0;
    anchor.setX( 0.5 * textWidth ); // center
    anchor.setY( -0.5 * textHeight ); // middle
    if ( frameBorder == QgsLayoutItemMapGrid::Top )
      anchor.setY( outside ? 0 : -textHeight ); // bottom / top
    else if ( frameBorder == QgsLayoutItemMapGrid::Right )
      anchor.setX( outside ? 0 : textWidth ); // left / right
    else if ( frameBorder == QgsLayoutItemMapGrid::Bottom )
      anchor.setY( outside ? -textHeight : 0 ); // top / bottom
    else if ( frameBorder == QgsLayoutItemMapGrid::Left )
      anchor.setX( outside ? textWidth : 0 ); // right / left
  }
  else if ( anotDir == QgsLayoutItemMapGrid::Vertical )
  {
    rotation = -90;
    anchor.setX( 0.5 * textWidth ); // center
    anchor.setY( -0.5 * textHeight ); // middle
    if ( frameBorder == QgsLayoutItemMapGrid::Top )
      anchor.setX( outside ? 0 : textWidth ); // left / right
    else if ( frameBorder == QgsLayoutItemMapGrid::Right )
      anchor.setY( outside ? -textHeight : 0 ); // top / bottom
    else if ( frameBorder == QgsLayoutItemMapGrid::Bottom )
      anchor.setX( outside ? textWidth : 0 ); // right / left
    else if ( frameBorder == QgsLayoutItemMapGrid::Left )
      anchor.setY( outside ? 0 : -textHeight ); // bottom / top
  }
  else if ( anotDir == QgsLayoutItemMapGrid::VerticalDescending )
  {
    rotation = 90;
    anchor.setX( 0.5 * textWidth ); // center
    anchor.setY( -0.5 * textHeight ); // middle
    if ( frameBorder == QgsLayoutItemMapGrid::Top )
      anchor.setX( outside ? textWidth : 0 ); // right / left
    else if ( frameBorder == QgsLayoutItemMapGrid::Right )
      anchor.setY( outside ? 0 : -textHeight ); // bottom / top
    else if ( frameBorder == QgsLayoutItemMapGrid::Bottom )
      anchor.setX( outside ? 0 : textWidth ); // left / right
    else if ( frameBorder == QgsLayoutItemMapGrid::Left )
      anchor.setY( outside ? -textHeight : 0 ); // top / bottom
  }
  else // ( anotDir == QgsLayoutItemMapGrid::BoundaryDirection )
  {
    const QVector2D borderVector = borderToVector2D( annot.border );
    rotation = atan2( borderVector.y(), borderVector.x() ) / M_PI * 180;
    anchor.setX( 0.5 * textWidth ); // center
    if ( anotPos == QgsLayoutItemMapGrid::OutsideMapFrame )
      anchor.setY( -textHeight ); // top
    else
      anchor.setY( 0 ); // bottom
  }

  // extents isn't computed accurately
  if ( extension && anotPos == QgsLayoutItemMapGrid::OutsideMapFrame )
  {
    extension->UpdateBorder( frameBorder, -f + textWidth );
    // We also add a general margin, can be useful for labels near corners
    extension->UpdateAll( textWidth / 2.0 );
  }

  if ( extension || !context.painter() )
    return;

  // Skip outwards facing annotations that are below mRotatedAnnotationsMarginToCorner
  bool facingLeft = ( annot.angle < 0 );
  bool facingRight = ( annot.angle > 0 );
  if ( anotPos == QgsLayoutItemMapGrid::OutsideMapFrame )
  {
    facingLeft = !facingLeft;
    facingRight = !facingRight;
  }
  if ( annot.border == BorderSide::Top && ( ( facingLeft && annot.position.x() < mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.x() > mMap->rect().width() - mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == BorderSide::Bottom && ( ( facingLeft && annot.position.x() > mMap->rect().width() - mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.x() < mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == BorderSide::Left && ( ( facingLeft && annot.position.y() > mMap->rect().height() - mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.y() < mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == BorderSide::Right && ( ( facingLeft && annot.position.y() < mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.y() > mMap->rect().height() - mRotatedAnnotationsMarginToCorner ) ) )
    return;

  const QgsScopedQPainterState painterState( context.painter() );
  context.painter()->translate( QPointF( xpos, ypos ) );
  context.painter()->rotate( rotation );
  context.painter()->translate( -anchor );
  const QgsScopedRenderContextScaleToPixels scale( context );
  QgsTextRenderer::drawText( QPointF( 0, 0 ), 0, QgsTextRenderer::AlignLeft, QStringList() << annotationString, context, mAnnotationFormat );
}

QString QgsLayoutItemMapGrid::gridAnnotationString( double value, QgsLayoutItemMapGrid::AnnotationCoordinate coord, QgsExpressionContext &expressionContext ) const
{
  //check if we are using degrees (ie, geographic crs)
  bool geographic = false;
  if ( mCRS.isValid() )
  {
    geographic = mCRS.isGeographic();
  }
  else if ( mMap && mMap->layout() )
  {
    geographic = mMap->crs().isGeographic();
  }

  if ( geographic && coord == QgsLayoutItemMapGrid::Longitude &&
       ( mGridAnnotationFormat == QgsLayoutItemMapGrid::Decimal || mGridAnnotationFormat == QgsLayoutItemMapGrid::DecimalWithSuffix ) )
  {
    // wrap around longitudes > 180 or < -180 degrees, so that, e.g., "190E" -> "170W"
    const double wrappedX = std::fmod( value, 360.0 );
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

    const double coordRounded = qgsRound( value, mGridAnnotationPrecision );
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
  QgsCoordinateFormatter::FormatFlags flags = QgsCoordinateFormatter::FormatFlags();
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
      flags = QgsCoordinateFormatter::FormatFlags();
      break;

    case DegreeMinutePadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;

    case DegreeMinuteSecondNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FormatFlags();
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

int QgsLayoutItemMapGrid::xGridLines() const
{
  if ( !mMap || mEvaluatedIntervalY <= 0.0 )
  {
    return 1;
  }


  QPolygonF mapPolygon = mMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalY = mEvaluatedIntervalY;
  double gridOffsetY = mEvaluatedOffsetY;
  double annotationScale = 1.0;
  switch ( mGridUnit )
  {
    case CM:
    case MM:
    {
      mapBoundingRect = mMap->rect();
      mapPolygon = QPolygonF( mMap->rect() );
      if ( mGridUnit == CM )
      {
        annotationScale = 0.1;
        gridIntervalY *= 10;
        gridOffsetY *= 10;
      }
      break;
    }

    case MapUnit:
    case DynamicPageSizeBased:
      break;
  }

  //consider to round up to the next step in case the left boundary is > 0
  const double roundCorrection = mapBoundingRect.top() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.top() - gridOffsetY ) / gridIntervalY + roundCorrection ) * gridIntervalY + gridOffsetY;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || ( mGridUnit != MapUnit && mGridUnit != DynamicPageSizeBased ) )
  {
    //no rotation. Do it 'the easy way'

    double yCanvasCoord;
    while ( currentLevel <= mapBoundingRect.bottom() && gridLineCount < MAX_GRID_LINES )
    {
      yCanvasCoord = mMap->rect().height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
      GridLine newLine;
      newLine.coordinate = currentLevel * annotationScale;
      newLine.coordinateType = AnnotationCoordinate::Latitude;
      newLine.line = QPolygonF() << QPointF( 0, yCanvasCoord ) << QPointF( mMap->rect().width(), yCanvasCoord );
      mGridLines.append( newLine );
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
    const QLineF gridLine( mapBoundingRect.left(), currentLevel, mapBoundingRect.right(), currentLevel );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
      if ( it->intersects( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
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
      GridLine newLine;
      newLine.coordinate = currentLevel;
      newLine.coordinateType = AnnotationCoordinate::Latitude;
      newLine.line = QPolygonF() << mMap->mapToItemCoords( intersectionList.at( 0 ) ) << mMap->mapToItemCoords( intersectionList.at( 1 ) );
      mGridLines.append( newLine );
      gridLineCount++;
    }
    currentLevel += gridIntervalY;
  }


  return 0;
}

int QgsLayoutItemMapGrid::yGridLines() const
{
  if ( !mMap || mEvaluatedIntervalX <= 0.0 )
  {
    return 1;
  }

  QPolygonF mapPolygon = mMap->transformedMapPolygon();
  QRectF mapBoundingRect = mapPolygon.boundingRect();
  double gridIntervalX = mEvaluatedIntervalX;
  double gridOffsetX = mEvaluatedOffsetX;
  double annotationScale = 1.0;
  switch ( mGridUnit )
  {
    case CM:
    case MM:
    {
      mapBoundingRect = mMap->rect();
      mapPolygon = QPolygonF( mMap->rect() );
      if ( mGridUnit == CM )
      {
        annotationScale = 0.1;
        gridIntervalX *= 10;
        gridOffsetX *= 10;
      }
      break;
    }

    case MapUnit:
    case DynamicPageSizeBased:
      break;
  }

  //consider to round up to the next step in case the left boundary is > 0
  const double roundCorrection = mapBoundingRect.left() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.left() - gridOffsetX ) / gridIntervalX + roundCorrection ) * gridIntervalX + gridOffsetX;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || ( mGridUnit != MapUnit && mGridUnit != DynamicPageSizeBased ) )
  {
    //no rotation. Do it 'the easy way'
    double xCanvasCoord;
    while ( currentLevel <= mapBoundingRect.right() && gridLineCount < MAX_GRID_LINES )
    {
      xCanvasCoord = mMap->rect().width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();

      GridLine newLine;
      newLine.coordinate = currentLevel * annotationScale;
      newLine.coordinateType = AnnotationCoordinate::Longitude;
      newLine.line = QPolygonF() << QPointF( xCanvasCoord, 0 ) << QPointF( xCanvasCoord, mMap->rect().height() );
      mGridLines.append( newLine );
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
    const QLineF gridLine( currentLevel, mapBoundingRect.bottom(), currentLevel, mapBoundingRect.top() );

    QVector<QLineF>::const_iterator it = borderLines.constBegin();
    for ( ; it != borderLines.constEnd(); ++it )
    {
      QPointF intersectionPoint;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      if ( it->intersect( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
#else
      if ( it->intersects( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
#endif
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
      GridLine newLine;
      newLine.coordinate = currentLevel;
      newLine.coordinateType = AnnotationCoordinate::Longitude;
      newLine.line = QPolygonF() << mMap->mapToItemCoords( intersectionList.at( 0 ) ) << mMap->mapToItemCoords( intersectionList.at( 1 ) );
      mGridLines.append( newLine );
      gridLineCount++;
    }
    currentLevel += gridIntervalX;
  }

  return 0;
}

int QgsLayoutItemMapGrid::xGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t ) const
{
  if ( !mMap || mEvaluatedIntervalY <= 0.0 )
  {
    return 1;
  }

  const double roundCorrection = bbox.yMaximum() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( bbox.yMaximum() - mEvaluatedOffsetY ) / mEvaluatedIntervalY + roundCorrection ) * mEvaluatedIntervalY + mEvaluatedOffsetY;

  const double minX = bbox.xMinimum();
  const double maxX = bbox.xMaximum();
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
        const QgsPointXY mapPoint = t.transform( currentX, currentLevel ); //transform back to map crs
        gridLine.append( mMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) ); //transform back to composer coords
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
        QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentX += step;
      if ( crosses180 && currentX > 180.0 )
      {
        currentX -= 360.0;
        crossed180 = true;
      }
    }
    crossed180 = false;

    const QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if ( !( *lineIt ).isEmpty() )
      {
        GridLine newLine;
        newLine.coordinate = currentLevel;
        newLine.coordinateType = AnnotationCoordinate::Latitude;
        newLine.line = QPolygonF( *lineIt );
        mGridLines.append( newLine );
        gridLineCount++;
      }
    }
    currentLevel -= mEvaluatedIntervalY;
  }

  return 0;
}

int QgsLayoutItemMapGrid::yGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t ) const
{
  if ( !mMap || mEvaluatedIntervalX <= 0.0 )
  {
    return 1;
  }

  const double roundCorrection = bbox.xMinimum() > 0 ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( bbox.xMinimum() - mEvaluatedOffsetX ) / mEvaluatedIntervalX + roundCorrection ) * mEvaluatedIntervalX + mEvaluatedOffsetX;

  const double minY = bbox.yMinimum();
  const double maxY = bbox.yMaximum();
  const double step = ( maxY - minY ) / 20;

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
        const QgsPointXY mapPoint = t.transform( currentLevel, currentY );
        //transform back to composer coords
        gridLine.append( mMap->mapToItemCoords( QPointF( mapPoint.x(), mapPoint.y() ) ) );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
        QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
      }

      currentY += step;
    }
    //clip grid line to map polygon
    const QList<QPolygonF> lineSegments = trimLinesToMap( gridLine, QgsRectangle( mMap->rect() ) );
    QList<QPolygonF>::const_iterator lineIt = lineSegments.constBegin();
    for ( ; lineIt != lineSegments.constEnd(); ++lineIt )
    {
      if ( !( *lineIt ).isEmpty() )
      {
        GridLine newLine;
        newLine.coordinate = currentLevel;
        newLine.coordinateType = AnnotationCoordinate::Longitude;
        newLine.line = QPolygonF( *lineIt );
        mGridLines.append( newLine );
        gridLineCount++;
      }
    }
    currentLevel += mEvaluatedIntervalX;
    if ( crosses180 && currentLevel > 180.0 )
    {
      currentLevel -= 360.0;
      crossed180 = true;
    }
  }

  return 0;
}

bool QgsLayoutItemMapGrid::shouldShowDivisionForSide( QgsLayoutItemMapGrid::AnnotationCoordinate coordinate, QgsLayoutItemMapGrid::BorderSide side ) const
{
  switch ( side )
  {
    case QgsLayoutItemMapGrid::Left:
      return testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) && shouldShowForDisplayMode( coordinate, mEvaluatedLeftFrameDivisions );
    case QgsLayoutItemMapGrid::Right:
      return testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) && shouldShowForDisplayMode( coordinate, mEvaluatedRightFrameDivisions );
    case QgsLayoutItemMapGrid::Top:
      return testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) && shouldShowForDisplayMode( coordinate, mEvaluatedTopFrameDivisions );
    case QgsLayoutItemMapGrid::Bottom:
      return testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) && shouldShowForDisplayMode( coordinate, mEvaluatedBottomFrameDivisions );
  }
  return false; // no warnings
}

bool QgsLayoutItemMapGrid::shouldShowAnnotationForSide( QgsLayoutItemMapGrid::AnnotationCoordinate coordinate, QgsLayoutItemMapGrid::BorderSide side ) const
{
  switch ( side )
  {
    case QgsLayoutItemMapGrid::Left:
      return shouldShowForDisplayMode( coordinate, mEvaluatedLeftGridAnnotationDisplay );
    case QgsLayoutItemMapGrid::Right:
      return shouldShowForDisplayMode( coordinate, mEvaluatedRightGridAnnotationDisplay );
    case QgsLayoutItemMapGrid::Top:
      return shouldShowForDisplayMode( coordinate, mEvaluatedTopGridAnnotationDisplay );
    case QgsLayoutItemMapGrid::Bottom:
      return shouldShowForDisplayMode( coordinate, mEvaluatedBottomGridAnnotationDisplay );
  }
  return false; // no warnings
}

bool QgsLayoutItemMapGrid::shouldShowForDisplayMode( QgsLayoutItemMapGrid::AnnotationCoordinate coordinate, QgsLayoutItemMapGrid::DisplayMode mode ) const
{
  return mode == QgsLayoutItemMapGrid::ShowAll
         || ( mode == QgsLayoutItemMapGrid::LatitudeOnly && coordinate == QgsLayoutItemMapGrid::Latitude )
         || ( mode == QgsLayoutItemMapGrid::LongitudeOnly && coordinate == QgsLayoutItemMapGrid::Longitude );
}


QgsLayoutItemMapGrid::DisplayMode gridAnnotationDisplayModeFromDD( QString ddValue, QgsLayoutItemMapGrid::DisplayMode defValue )
{
  if ( ddValue.compare( QLatin1String( "x_only" ), Qt::CaseInsensitive ) == 0 )
    return QgsLayoutItemMapGrid::LatitudeOnly;
  else if ( ddValue.compare( QLatin1String( "y_only" ), Qt::CaseInsensitive ) == 0 )
    return QgsLayoutItemMapGrid::LongitudeOnly;
  else if ( ddValue.compare( QLatin1String( "disabled" ), Qt::CaseInsensitive ) == 0 )
    return QgsLayoutItemMapGrid::HideAll;
  else if ( ddValue.compare( QLatin1String( "all" ), Qt::CaseInsensitive ) == 0 )
    return QgsLayoutItemMapGrid::ShowAll;
  else
    return defValue;
}


void QgsLayoutItemMapGrid::refreshDataDefinedProperties()
{
  const QgsExpressionContext context = createExpressionContext();

  // if we are changing the grid interval or offset, then we also have to mark the transform as dirty
  mTransformDirty = mTransformDirty
                    || mDataDefinedProperties.isActive( QgsLayoutObject::MapGridIntervalX )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::MapGridIntervalY )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::MapGridOffsetX )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::MapGridOffsetY );

  mEvaluatedEnabled = mDataDefinedProperties.valueAsBool( QgsLayoutObject::MapGridEnabled, context, enabled() );
  switch ( mGridUnit )
  {
    case MapUnit:
    case MM:
    case CM:
    {
      mEvaluatedIntervalX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridIntervalX, context, mGridIntervalX );
      mEvaluatedIntervalY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridIntervalY, context, mGridIntervalY );
      break;
    }

    case DynamicPageSizeBased:
    {
      if ( mMaximumIntervalWidth < mMinimumIntervalWidth )
      {
        mEvaluatedEnabled = false;
      }
      else
      {
        const double mapWidthMm = mLayout->renderContext().measurementConverter().convert( mMap->sizeWithUnits(), QgsUnitTypes::LayoutMillimeters ).width();
        const double mapWidthMapUnits = mapWidth();
        const double minUnitsPerSeg = ( mMinimumIntervalWidth * mapWidthMapUnits ) / mapWidthMm;
        const double maxUnitsPerSeg = ( mMaximumIntervalWidth * mapWidthMapUnits ) / mapWidthMm;
        const double interval = QgsLayoutUtils::calculatePrettySize( minUnitsPerSeg, maxUnitsPerSeg );
        mEvaluatedIntervalX = interval;
        mEvaluatedIntervalY = interval;
        mTransformDirty = true;
      }
      break;
    }
  }
  mEvaluatedOffsetX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridOffsetX, context, mGridOffsetX );
  mEvaluatedOffsetY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridOffsetY, context, mGridOffsetY );
  mEvaluatedGridFrameWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridFrameSize, context, mGridFrameWidth );
  mEvaluatedGridFrameMargin = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridFrameMargin, context, mGridFrameMargin );
  mEvaluatedAnnotationFrameDistance = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridLabelDistance, context, mAnnotationFrameDistance );
  mEvaluatedCrossLength = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridCrossSize, context, mCrossLength );
  mEvaluatedGridFrameLineThickness = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapGridFrameLineThickness, context, mGridFramePenThickness );
  mEvaluatedLeftGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridAnnotationDisplayLeft, context ), mLeftGridAnnotationDisplay );
  mEvaluatedRightGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridAnnotationDisplayRight, context ), mRightGridAnnotationDisplay );
  mEvaluatedTopGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridAnnotationDisplayTop, context ), mTopGridAnnotationDisplay );
  mEvaluatedBottomGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridAnnotationDisplayBottom, context ), mBottomGridAnnotationDisplay );
  mEvaluatedLeftFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridFrameDivisionsLeft, context ), mLeftFrameDivisions );
  mEvaluatedRightFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridFrameDivisionsRight, context ), mRightFrameDivisions );
  mEvaluatedTopFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridFrameDivisionsTop, context ), mTopFrameDivisions );
  mEvaluatedBottomFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::MapGridFrameDivisionsBottom, context ), mBottomFrameDivisions );

}

double QgsLayoutItemMapGrid::mapWidth() const
{
  if ( !mMap )
  {
    return 0.0;
  }

  const QgsRectangle mapExtent = mMap->extent();
  const QgsUnitTypes::DistanceUnit distanceUnit = mCRS.isValid() ? mCRS.mapUnits() : mMap->crs().mapUnits();
  if ( distanceUnit == QgsUnitTypes::DistanceUnknownUnit )
  {
    return mapExtent.width();
  }
  else
  {
    QgsDistanceArea da;

    da.setSourceCrs( mMap->crs(), mLayout->project()->transformContext() );
    da.setEllipsoid( mLayout->project()->ellipsoid() );

    const QgsUnitTypes::DistanceUnit units = da.lengthUnits();
    double measure = da.measureLine( QgsPointXY( mapExtent.xMinimum(), mapExtent.yMinimum() ),
                                     QgsPointXY( mapExtent.xMaximum(), mapExtent.yMinimum() ) );
    measure /= QgsUnitTypes::fromUnitToUnitFactor( distanceUnit, units );
    return measure;
  }
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

  const double tolerance = std::max( mMap->frameEnabled() ? mMap->pen().widthF() : 0.0, 1.0 );

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

void QgsLayoutItemMapGrid::setAnnotationFont( const QFont &font )
{
  mAnnotationFormat.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    mAnnotationFormat.setSize( font.pointSizeF() );
    mAnnotationFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    mAnnotationFormat.setSize( font.pixelSize() );
    mAnnotationFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }
}

QFont QgsLayoutItemMapGrid::annotationFont() const
{
  return mAnnotationFormat.toQFont();
}

void QgsLayoutItemMapGrid::setAnnotationFontColor( const QColor &color )
{
  mAnnotationFormat.setColor( color );
}

QColor QgsLayoutItemMapGrid::annotationFontColor() const
{
  return mAnnotationFormat.color();
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
  }

  refreshDataDefinedProperties();

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
      return mBottomGridAnnotationDisplay;
  }
  return mBottomGridAnnotationDisplay; // no warnings
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

  if ( !mMap || !mEvaluatedEnabled )
  {
    return;
  }

  //setup render context
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  const QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  GridExtension extension;

  //collect grid lines
  switch ( mGridUnit )
  {
    case MapUnit:
    case DynamicPageSizeBased:
    {
      if ( mCRS.isValid() && mCRS != mMap->crs() )
      {
        drawGridCrsTransform( context, 0, true );
        break;
      }
    }
    FALLTHROUGH
    case CM:
    case MM:
      drawGridNoTransform( context, 0, true );
      break;
  }

  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame || mShowGridAnnotation )
    updateGridLinesAnnotationsPositions();

  if ( mGridFrameStyle != QgsLayoutItemMapGrid::NoFrame )
  {
    drawGridFrame( nullptr, &extension );
  }

  if ( mShowGridAnnotation )
  {
    drawCoordinateAnnotations( context, context.expressionContext(), &extension );
  }

  top = extension.top;
  right = extension.right;
  bottom = extension.bottom;
  left = extension.left;
}

void QgsLayoutItemMapGrid::setEnabled( bool enabled )
{
  QgsLayoutItemMapItem::setEnabled( enabled );
  refreshDataDefinedProperties();
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
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setIntervalY( const double interval )
{
  if ( qgsDoubleNear( interval, mGridIntervalY ) )
  {
    return;
  }
  mGridIntervalY = interval;
  mTransformDirty = true;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setOffsetX( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetX ) )
  {
    return;
  }
  mGridOffsetX = offset;
  mTransformDirty = true;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setOffsetY( const double offset )
{
  if ( qgsDoubleNear( offset, mGridOffsetY ) )
  {
    return;
  }
  mGridOffsetY = offset;
  mTransformDirty = true;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setMinimumIntervalWidth( double minWidth )
{
  if ( qgsDoubleNear( minWidth, mMinimumIntervalWidth ) )
  {
    return;
  }
  mMinimumIntervalWidth = minWidth;
  mTransformDirty = true;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setMaximumIntervalWidth( double maxWidth )
{
  if ( qgsDoubleNear( maxWidth, mMaximumIntervalWidth ) )
  {
    return;
  }
  mMaximumIntervalWidth = maxWidth;
  mTransformDirty = true;
  refreshDataDefinedProperties();
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

void QgsLayoutItemMapGrid::setCrossLength( const double length )
{
  mCrossLength = length;
  refreshDataDefinedProperties();
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
  QgsExpressionContext context = QgsLayoutItemMapItem::createExpressionContext();
  context.appendScope( new QgsExpressionContextScope( tr( "Grid" ) ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_number" ), 0, true ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "grid_axis" ), "x", true ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "grid_number" ) << QStringLiteral( "grid_axis" ) );
  return context;
}

bool QgsLayoutItemMapGrid::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mGridLineSymbol )
  {
    QgsStyleSymbolEntity entity( mGridLineSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "grid" ), QObject::tr( "Grid" ) ) ) )
      return false;
  }
  if ( mGridMarkerSymbol )
  {
    QgsStyleSymbolEntity entity( mGridMarkerSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "grid" ), QObject::tr( "Grid" ) ) ) )
      return false;
  }

  return true;
}

void QgsLayoutItemMapGrid::refresh()
{
  mTransformDirty = true;
  refreshDataDefinedProperties();
  mMap->updateBoundingRect();
  mMap->update();
}

bool QgsLayoutItemMapGrid::testFrameSideFlag( QgsLayoutItemMapGrid::FrameSideFlag flag ) const
{
  return mGridFrameSides.testFlag( flag );
}

void QgsLayoutItemMapGrid::setFrameWidth( const double width )
{
  mGridFrameWidth = width;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setFrameMargin( const double margin )
{
  mGridFrameMargin = margin;
  refreshDataDefinedProperties();
}

void QgsLayoutItemMapGrid::setFramePenSize( const double width )
{
  mGridFramePenThickness = width;
  refreshDataDefinedProperties();
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
      return mBottomGridAnnotationPosition;
  }
  return mLeftGridAnnotationPosition; // no warnings
}

void QgsLayoutItemMapGrid::setAnnotationFrameDistance( const double distance )
{
  mAnnotationFrameDistance = distance;
  refreshDataDefinedProperties();
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
      return mBottomGridAnnotationDirection;
  }
  return mLeftGridAnnotationDirection; // no warnings
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
  }

  refreshDataDefinedProperties();

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
      return mBottomFrameDivisions;
  }
  return mLeftFrameDivisions; // no warnings
}

int QgsLayoutItemMapGrid::crsGridParams( QgsRectangle &crsRect, QgsCoordinateTransform &inverseTransform ) const
{
  if ( !mMap )
  {
    return 1;
  }

  try
  {
    const QgsCoordinateTransform tr( mMap->crs(), mCRS, mLayout->project() );
    QgsCoordinateTransform extentTransform = tr;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    const QPolygonF mapPolygon = mMap->transformedMapPolygon();
    const QRectF mbr = mapPolygon.boundingRect();
    const QgsRectangle mapBoundingRect( mbr.left(), mbr.bottom(), mbr.right(), mbr.top() );


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
        crsRect = extentTransform.transformBoundingBox( mapBoundingRect, Qgis::TransformDirection::Forward, true );
      }
      else
      {
        //didn't cross the line
        crsRect = extentTransform.transformBoundingBox( mapBoundingRect );
      }
    }
    else
    {
      crsRect = extentTransform.transformBoundingBox( mapBoundingRect );
    }

    inverseTransform = QgsCoordinateTransform( mCRS, mMap->crs(), mLayout->project() );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
    return 1;
  }
  return 0;
}

QList<QPolygonF> QgsLayoutItemMapGrid::trimLinesToMap( const QPolygonF &line, const QgsRectangle &rect )
{
  const QgsGeometry lineGeom = QgsGeometry::fromQPolygonF( line );
  const QgsGeometry rectGeom = QgsGeometry::fromRect( rect );

  const QgsGeometry intersected = lineGeom.intersection( rectGeom );
  const QVector<QgsGeometry> intersectedParts = intersected.asGeometryCollection();

  QList<QPolygonF> trimmedLines;
  QVector<QgsGeometry>::const_iterator geomIt = intersectedParts.constBegin();
  for ( ; geomIt != intersectedParts.constEnd(); ++geomIt )
  {
    trimmedLines << ( *geomIt ).asQPolygonF();
  }
  return trimmedLines;
}
