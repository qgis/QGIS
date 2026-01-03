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

#include <math.h>
#include <memory>

#include "qgscolorutils.h"
#include "qgscoordinateformatter.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsexception.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfontutils.h"
#include "qgsgeometry.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoututils.h"
#include "qgslinesymbol.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgssettings.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"
#include "qgstextdocument.h"
#include "qgstextdocumentmetrics.h"
#include "qgstextrenderer.h"
#include "qgsunittypes.h"

#include <QPainter>
#include <QPen>
#include <QVector2D>

#include "moc_qgslayoutitemmapgrid.cpp"

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

QList<QgsLayoutItemMapGrid *> QgsLayoutItemMapGridStack::asList() const // cppcheck-suppress duplInheritedMember
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

QgsLayoutItemMapGrid &QgsLayoutItemMapGridStack::operator[]( int idx ) // cppcheck-suppress duplInheritedMember
{
  QgsLayoutItemMapItem *item = mItems.at( idx );
  QgsLayoutItemMapGrid *grid = qobject_cast<QgsLayoutItemMapGrid *>( item );
  return *grid;
}

bool QgsLayoutItemMapGridStack::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  removeItems();

  //read grid stack
  const QDomNodeList mapGridNodeList = elem.elementsByTagName( u"ComposerMapGrid"_s );
  for ( int i = 0; i < mapGridNodeList.size(); ++i )
  {
    const QDomElement mapGridElem = mapGridNodeList.at( i ).toElement();
    QgsLayoutItemMapGrid *mapGrid = new QgsLayoutItemMapGrid( mapGridElem.attribute( u"name"_s ), mMap );
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

QVector2D borderToVector2D( Qgis::MapGridBorderSide border )
{
  // returns a border as a vector2D for vector arithmetic
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      return QVector2D( 0, 1 );
    case Qgis::MapGridBorderSide::Top:
      return QVector2D( -1, 0 );
    case Qgis::MapGridBorderSide::Right:
      return QVector2D( 0, -1 );
    case Qgis::MapGridBorderSide::Bottom:
      return QVector2D( 1, 0 );
  }
  return QVector2D();
}
QVector2D borderToNormal2D( Qgis::MapGridBorderSide border )
{
  // returns a border normal (towards center) as a vector2D for vector arithmetic
  const QVector2D borderVector = borderToVector2D( border );
  return QVector2D( borderVector.y(), -borderVector.x() );
}

QgsLayoutItemMapGrid::QgsLayoutItemMapGrid( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutItemMapItem( name, map )
  , mGridFrameSides( Qgis::MapGridFrameSideFlag::Left | Qgis::MapGridFrameSideFlag::Right |
                     Qgis::MapGridFrameSideFlag::Top | Qgis::MapGridFrameSideFlag::Bottom )
{
  //get default layout font from settings
  const QgsSettings settings;
  const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    QFont font;
    QgsFontUtils::setFontFamily( font, defaultFontString );
    mAnnotationFormat.setFont( font );
  }

  createDefaultGridLineSymbol();
  createDefaultGridMarkerSymbol();

  connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemMapGrid::refreshDataDefinedProperties );
  connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemMapGrid::refreshDataDefinedProperties );
  connect( mMap, &QgsLayoutItemMap::crsChanged, this, [this]
  {
    if ( !mCRS.isValid() )
      emit crsChanged();
  } );
}

QgsLayoutItemMapGrid::~QgsLayoutItemMapGrid() = default;

void QgsLayoutItemMapGrid::createDefaultGridLineSymbol()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"width"_s, u"0.3"_s );
  properties.insert( u"capstyle"_s, u"flat"_s );
  mGridLineSymbol = QgsLineSymbol::createSimple( properties );
}

void QgsLayoutItemMapGrid::createDefaultGridMarkerSymbol()
{
  QVariantMap properties;
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"2.0"_s );
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  mGridMarkerSymbol = QgsMarkerSymbol::createSimple( properties );
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

  QDomElement mapGridElem = doc.createElement( u"ComposerMapGrid"_s );
  mapGridElem.setAttribute( u"gridStyle"_s, static_cast< int >( mGridStyle ) );
  mapGridElem.setAttribute( u"intervalX"_s, qgsDoubleToString( mGridIntervalX ) );
  mapGridElem.setAttribute( u"intervalY"_s, qgsDoubleToString( mGridIntervalY ) );
  mapGridElem.setAttribute( u"offsetX"_s, qgsDoubleToString( mGridOffsetX ) );
  mapGridElem.setAttribute( u"offsetY"_s, qgsDoubleToString( mGridOffsetY ) );
  mapGridElem.setAttribute( u"crossLength"_s, qgsDoubleToString( mCrossLength ) );

  QDomElement lineStyleElem = doc.createElement( u"lineStyle"_s );
  const QDomElement gridLineStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridLineSymbol.get(), doc, context );
  lineStyleElem.appendChild( gridLineStyleElem );
  mapGridElem.appendChild( lineStyleElem );

  QDomElement markerStyleElem = doc.createElement( u"markerStyle"_s );
  const QDomElement gridMarkerStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mGridMarkerSymbol.get(), doc, context );
  markerStyleElem.appendChild( gridMarkerStyleElem );
  mapGridElem.appendChild( markerStyleElem );

  mapGridElem.setAttribute( u"gridFrameStyle"_s, static_cast< int >( mGridFrameStyle ) );
  mapGridElem.setAttribute( u"gridFrameSideFlags"_s, mGridFrameSides );
  mapGridElem.setAttribute( u"gridFrameWidth"_s, qgsDoubleToString( mGridFrameWidth ) );
  mapGridElem.setAttribute( u"gridFrameMargin"_s, qgsDoubleToString( mGridFrameMargin ) );
  mapGridElem.setAttribute( u"gridFramePenThickness"_s, qgsDoubleToString( mGridFramePenThickness ) );
  mapGridElem.setAttribute( u"gridFramePenColor"_s, QgsColorUtils::colorToString( mGridFramePenColor ) );
  mapGridElem.setAttribute( u"frameFillColor1"_s, QgsColorUtils::colorToString( mGridFrameFillColor1 ) );
  mapGridElem.setAttribute( u"frameFillColor2"_s, QgsColorUtils::colorToString( mGridFrameFillColor2 ) );
  mapGridElem.setAttribute( u"leftFrameDivisions"_s, static_cast< int >( mLeftFrameDivisions ) );
  mapGridElem.setAttribute( u"rightFrameDivisions"_s, static_cast< int >( mRightFrameDivisions ) );
  mapGridElem.setAttribute( u"topFrameDivisions"_s, static_cast< int >( mTopFrameDivisions ) );
  mapGridElem.setAttribute( u"bottomFrameDivisions"_s, static_cast< int >( mBottomFrameDivisions ) );
  mapGridElem.setAttribute( u"rotatedTicksLengthMode"_s, static_cast< int >( mRotatedTicksLengthMode ) );
  mapGridElem.setAttribute( u"rotatedTicksEnabled"_s, mRotatedTicksEnabled );
  mapGridElem.setAttribute( u"rotatedTicksMinimumAngle"_s, QString::number( mRotatedTicksMinimumAngle ) );
  mapGridElem.setAttribute( u"rotatedTicksMarginToCorner"_s, QString::number( mRotatedTicksMarginToCorner ) );
  mapGridElem.setAttribute( u"rotatedAnnotationsLengthMode"_s, static_cast< int >( mRotatedAnnotationsLengthMode ) );
  mapGridElem.setAttribute( u"rotatedAnnotationsEnabled"_s, mRotatedAnnotationsEnabled );
  mapGridElem.setAttribute( u"rotatedAnnotationsMinimumAngle"_s, QString::number( mRotatedAnnotationsMinimumAngle ) );
  mapGridElem.setAttribute( u"rotatedAnnotationsMarginToCorner"_s, QString::number( mRotatedAnnotationsMarginToCorner ) );
  if ( mCRS.isValid() )
  {
    mCRS.writeXml( mapGridElem, doc );
  }

  mapGridElem.setAttribute( u"annotationFormat"_s, static_cast< int >( mGridAnnotationFormat ) );
  mapGridElem.setAttribute( u"showAnnotation"_s, mShowGridAnnotation );
  mapGridElem.setAttribute( u"annotationExpression"_s, mGridAnnotationExpressionString );
  mapGridElem.setAttribute( u"leftAnnotationDisplay"_s, static_cast< int >( mLeftGridAnnotationDisplay ) );
  mapGridElem.setAttribute( u"rightAnnotationDisplay"_s, static_cast< int >( mRightGridAnnotationDisplay ) );
  mapGridElem.setAttribute( u"topAnnotationDisplay"_s, static_cast< int >( mTopGridAnnotationDisplay ) );
  mapGridElem.setAttribute( u"bottomAnnotationDisplay"_s, static_cast< int >( mBottomGridAnnotationDisplay ) );
  mapGridElem.setAttribute( u"leftAnnotationPosition"_s, static_cast< int >( mLeftGridAnnotationPosition ) );
  mapGridElem.setAttribute( u"rightAnnotationPosition"_s, static_cast< int >( mRightGridAnnotationPosition ) );
  mapGridElem.setAttribute( u"topAnnotationPosition"_s, static_cast< int >( mTopGridAnnotationPosition ) );
  mapGridElem.setAttribute( u"bottomAnnotationPosition"_s, static_cast< int >( mBottomGridAnnotationPosition ) );
  mapGridElem.setAttribute( u"leftAnnotationDirection"_s, static_cast< int >( mLeftGridAnnotationDirection ) );
  mapGridElem.setAttribute( u"rightAnnotationDirection"_s, static_cast< int >( mRightGridAnnotationDirection ) );
  mapGridElem.setAttribute( u"topAnnotationDirection"_s, static_cast< int >( mTopGridAnnotationDirection ) );
  mapGridElem.setAttribute( u"bottomAnnotationDirection"_s, static_cast< int >( mBottomGridAnnotationDirection ) );
  mapGridElem.setAttribute( u"frameAnnotationDistance"_s, QString::number( mAnnotationFrameDistance ) );
  mapGridElem.appendChild( mAnnotationFormat.writeXml( doc, context ) );
  mapGridElem.setAttribute( u"annotationPrecision"_s, mGridAnnotationPrecision );
  mapGridElem.setAttribute( u"unit"_s, static_cast< int >( mGridUnit ) );
  mapGridElem.setAttribute( u"blendMode"_s, mBlendMode );
  mapGridElem.setAttribute( u"minimumIntervalWidth"_s, QString::number( mMinimumIntervalWidth ) );
  mapGridElem.setAttribute( u"maximumIntervalWidth"_s, QString::number( mMaximumIntervalWidth ) );

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
  mGridStyle = static_cast< Qgis::MapGridStyle >( itemElem.attribute( u"gridStyle"_s, u"0"_s ).toInt() );
  mGridIntervalX = itemElem.attribute( u"intervalX"_s, u"0"_s ).toDouble();
  mGridIntervalY = itemElem.attribute( u"intervalY"_s, u"0"_s ).toDouble();
  mGridOffsetX = itemElem.attribute( u"offsetX"_s, u"0"_s ).toDouble();
  mGridOffsetY = itemElem.attribute( u"offsetY"_s, u"0"_s ).toDouble();
  mCrossLength = itemElem.attribute( u"crossLength"_s, u"3"_s ).toDouble();
  mGridFrameStyle = static_cast< Qgis::MapGridFrameStyle >( itemElem.attribute( u"gridFrameStyle"_s, u"0"_s ).toInt() );
  mGridFrameSides = static_cast< Qgis::MapGridFrameSideFlags >( itemElem.attribute( u"gridFrameSideFlags"_s, u"15"_s ).toInt() );
  mGridFrameWidth = itemElem.attribute( u"gridFrameWidth"_s, u"2.0"_s ).toDouble();
  mGridFrameMargin = itemElem.attribute( u"gridFrameMargin"_s, u"0.0"_s ).toDouble();
  mGridFramePenThickness = itemElem.attribute( u"gridFramePenThickness"_s, u"0.3"_s ).toDouble();
  mGridFramePenColor = QgsColorUtils::colorFromString( itemElem.attribute( u"gridFramePenColor"_s, u"0,0,0"_s ) );
  mGridFrameFillColor1 = QgsColorUtils::colorFromString( itemElem.attribute( u"frameFillColor1"_s, u"255,255,255,255"_s ) );
  mGridFrameFillColor2 = QgsColorUtils::colorFromString( itemElem.attribute( u"frameFillColor2"_s, u"0,0,0,255"_s ) );
  mLeftFrameDivisions = static_cast< Qgis::MapGridComponentVisibility >( itemElem.attribute( u"leftFrameDivisions"_s, u"0"_s ).toInt() );
  mRightFrameDivisions = static_cast< Qgis::MapGridComponentVisibility >( itemElem.attribute( u"rightFrameDivisions"_s, u"0"_s ).toInt() );
  mTopFrameDivisions = static_cast< Qgis::MapGridComponentVisibility >( itemElem.attribute( u"topFrameDivisions"_s, u"0"_s ).toInt() );
  mBottomFrameDivisions = static_cast< Qgis::MapGridComponentVisibility >( itemElem.attribute( u"bottomFrameDivisions"_s, u"0"_s ).toInt() );
  mRotatedTicksLengthMode = static_cast< Qgis::MapGridTickLengthMode >( itemElem.attribute( u"rotatedTicksLengthMode"_s, u"0"_s ).toInt() );
  mRotatedTicksEnabled = itemElem.attribute( u"rotatedTicksEnabled"_s, u"0"_s ) != "0"_L1;
  mRotatedTicksMinimumAngle = itemElem.attribute( u"rotatedTicksMinimumAngle"_s, u"0"_s ).toDouble();
  mRotatedTicksMarginToCorner = itemElem.attribute( u"rotatedTicksMarginToCorner"_s, u"0"_s ).toDouble();
  mRotatedAnnotationsLengthMode = static_cast< Qgis::MapGridTickLengthMode >( itemElem.attribute( u"rotatedAnnotationsLengthMode"_s, u"0"_s ).toInt() );
  mRotatedAnnotationsEnabled = itemElem.attribute( u"rotatedAnnotationsEnabled"_s, u"0"_s ) != "0"_L1;
  mRotatedAnnotationsMinimumAngle = itemElem.attribute( u"rotatedAnnotationsMinimumAngle"_s, u"0"_s ).toDouble();
  mRotatedAnnotationsMarginToCorner = itemElem.attribute( u"rotatedAnnotationsMarginToCorner"_s, u"0"_s ).toDouble();

  const QDomElement lineStyleElem = itemElem.firstChildElement( u"lineStyle"_s );
  if ( !lineStyleElem.isNull() )
  {
    const QDomElement symbolElem = lineStyleElem.firstChildElement( u"symbol"_s );
    if ( !symbolElem.isNull() )
    {
      mGridLineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context );
    }
  }
  else
  {
    //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
    mGridLineSymbol = QgsLineSymbol::createSimple( QVariantMap() );
    mGridLineSymbol->setWidth( itemElem.attribute( u"penWidth"_s, u"0"_s ).toDouble() );
    mGridLineSymbol->setColor( QColor( itemElem.attribute( u"penColorRed"_s, u"0"_s ).toInt(),
                                       itemElem.attribute( u"penColorGreen"_s, u"0"_s ).toInt(),
                                       itemElem.attribute( u"penColorBlue"_s, u"0"_s ).toInt() ) );
  }

  const QDomElement markerStyleElem = itemElem.firstChildElement( u"markerStyle"_s );
  if ( !markerStyleElem.isNull() )
  {
    const QDomElement symbolElem = markerStyleElem.firstChildElement( u"symbol"_s );
    if ( !symbolElem.isNull() )
    {
      mGridMarkerSymbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context );
    }
  }

  if ( !mCRS.readXml( itemElem ) )
    mCRS = QgsCoordinateReferenceSystem();

  mBlendMode = static_cast< QPainter::CompositionMode >( itemElem.attribute( u"blendMode"_s, u"0"_s ).toUInt() );

  //annotation
  mShowGridAnnotation = ( itemElem.attribute( u"showAnnotation"_s, u"0"_s ) != "0"_L1 );
  mGridAnnotationFormat = static_cast< Qgis::MapGridAnnotationFormat >( itemElem.attribute( u"annotationFormat"_s, u"0"_s ).toInt() );
  mGridAnnotationExpressionString = itemElem.attribute( u"annotationExpression"_s );
  mGridAnnotationExpression.reset();
  mLeftGridAnnotationPosition = static_cast< Qgis::MapGridAnnotationPosition >( itemElem.attribute( u"leftAnnotationPosition"_s, u"0"_s ).toInt() );
  mRightGridAnnotationPosition = static_cast< Qgis::MapGridAnnotationPosition >( itemElem.attribute( u"rightAnnotationPosition"_s, u"0"_s ).toInt() );
  mTopGridAnnotationPosition = static_cast< Qgis::MapGridAnnotationPosition >( itemElem.attribute( u"topAnnotationPosition"_s, u"0"_s ).toInt() );
  mBottomGridAnnotationPosition = static_cast< Qgis::MapGridAnnotationPosition >( itemElem.attribute( u"bottomAnnotationPosition"_s, u"0"_s ).toInt() );
  mLeftGridAnnotationDisplay = static_cast<Qgis::MapGridComponentVisibility >( itemElem.attribute( u"leftAnnotationDisplay"_s, u"0"_s ).toInt() );
  mRightGridAnnotationDisplay = static_cast<Qgis::MapGridComponentVisibility >( itemElem.attribute( u"rightAnnotationDisplay"_s, u"0"_s ).toInt() );
  mTopGridAnnotationDisplay = static_cast<Qgis::MapGridComponentVisibility >( itemElem.attribute( u"topAnnotationDisplay"_s, u"0"_s ).toInt() );
  mBottomGridAnnotationDisplay = static_cast<Qgis::MapGridComponentVisibility >( itemElem.attribute( u"bottomAnnotationDisplay"_s, u"0"_s ).toInt() );

  mLeftGridAnnotationDirection = static_cast<Qgis::MapGridAnnotationDirection >( itemElem.attribute( u"leftAnnotationDirection"_s, u"0"_s ).toInt() );
  mRightGridAnnotationDirection = static_cast<Qgis::MapGridAnnotationDirection >( itemElem.attribute( u"rightAnnotationDirection"_s, u"0"_s ).toInt() );
  mTopGridAnnotationDirection = static_cast<Qgis::MapGridAnnotationDirection >( itemElem.attribute( u"topAnnotationDirection"_s, u"0"_s ).toInt() );
  mBottomGridAnnotationDirection = static_cast<Qgis::MapGridAnnotationDirection >( itemElem.attribute( u"bottomAnnotationDirection"_s, u"0"_s ).toInt() );
  mAnnotationFrameDistance = itemElem.attribute( u"frameAnnotationDistance"_s, u"0"_s ).toDouble();

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
    mAnnotationFormat.setSizeUnit( Qgis::RenderUnit::Points );
    mAnnotationFormat.setColor( QgsColorUtils::colorFromString( itemElem.attribute( "annotationFontColor", "0,0,0,255" ) ) );
  }

  mGridAnnotationPrecision = itemElem.attribute( u"annotationPrecision"_s, u"3"_s ).toInt();
  const int gridUnitInt = itemElem.attribute( u"unit"_s, QString::number( static_cast< int >( Qgis::MapGridUnit::MapUnits ) ) ).toInt();
  mGridUnit = ( gridUnitInt <= static_cast< int >( Qgis::MapGridUnit::DynamicPageSizeBased ) ) ? static_cast< Qgis::MapGridUnit >( gridUnitInt ) : Qgis::MapGridUnit::MapUnits;
  mMinimumIntervalWidth = itemElem.attribute( u"minimumIntervalWidth"_s, u"50"_s ).toDouble();
  mMaximumIntervalWidth = itemElem.attribute( u"maximumIntervalWidth"_s, u"100"_s ).toDouble();

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
    int countLongitudeLines = 0;
    int countLatitudeLines = 0;
    for ( const GridLine &line : mGridLines )
    {
      switch ( line.coordinateType )
      {
        case Qgis::MapGridAnnotationType::Longitude:
          countLongitudeLines++;
          break;
        case Qgis::MapGridAnnotationType::Latitude:
          countLatitudeLines++;
          break;
      }
    }

    int latitudeLineIndex = 0;
    int longitudeLineIndex = 0;
    if ( mGridStyle == Qgis::MapGridStyle::Lines )
    {
      QList< GridLine >::const_iterator gridIt = mGridLines.constBegin();
      for ( ; gridIt != mGridLines.constEnd(); ++gridIt )
      {
        switch ( gridIt->coordinateType )
        {
          case Qgis::MapGridAnnotationType::Longitude:
            longitudeLineIndex++;
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLongitudeLines, true ) );
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, longitudeLineIndex, true ) );
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_axis"_s, u"x"_s, true ) );
            break;

          case Qgis::MapGridAnnotationType::Latitude:
            latitudeLineIndex++;
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLatitudeLines, true ) );
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, latitudeLineIndex, true ) );
            context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_axis"_s, u"y"_s, true ) );
            break;
        }
        context.expressionContext().lastScope()->setVariable( u"grid_number"_s, gridIt->coordinate );
        drawGridLine( scalePolygon( gridIt->line, dotsPerMM ), context );
      }
    }
    else if ( mGridStyle == Qgis::MapGridStyle::LineCrosses || mGridStyle == Qgis::MapGridStyle::Markers )
    {
      const double maxX = mMap->rect().width();
      const double maxY = mMap->rect().height();

      QList< QgsPointXY >::const_iterator intersectionIt = mTransformedIntersections.constBegin();
      for ( ; intersectionIt != mTransformedIntersections.constEnd(); ++intersectionIt )
      {
        const double x = intersectionIt->x();
        const double y = intersectionIt->y();
        if ( mGridStyle == Qgis::MapGridStyle::LineCrosses )
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
        else if ( mGridStyle == Qgis::MapGridStyle::Markers )
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

  if ( mGridStyle == Qgis::MapGridStyle::LineCrosses || mGridStyle == Qgis::MapGridStyle::Markers )
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
      if ( gridIt->coordinateType == Qgis::MapGridAnnotationType::Longitude )
        yLines << QgsGeometry::fromPolylineXY( line );
      else if ( gridIt->coordinateType == Qgis::MapGridAnnotationType::Latitude )
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
  p->setRenderHint( QPainter::Antialiasing, mMap->layout()->renderContext().flags() & Qgis::LayoutRenderFlag::Antialiasing );

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
  if ( context.rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::Default )
    context.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  const QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  //is grid in a different crs than map?
  switch ( mGridUnit )
  {
    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::DynamicPageSizeBased:
      if ( mCRS.isValid() && mCRS != mMap->crs() )
      {
        drawGridCrsTransform( context, dotsPerMM );
        break;
      }

      [[fallthrough]];
    case Qgis::MapGridUnit::Centimeters:
    case Qgis::MapGridUnit::Millimeters:
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


  if ( mGridFrameStyle != Qgis::MapGridFrameStyle::NoFrame || mShowGridAnnotation )
    updateGridLinesAnnotationsPositions();

  if ( mGridFrameStyle != Qgis::MapGridFrameStyle::NoFrame )
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

  if ( calculateLinesOnly || mGridLines.empty() )
    return;

  QList< GridLine >::const_iterator vIt = mGridLines.constBegin();
  QList< GridLine >::const_iterator hIt = mGridLines.constBegin();

  int countLongitudeLines = 0;
  int countLatitudeLines = 0;
  for ( const GridLine &line : mGridLines )
  {
    switch ( line.coordinateType )
    {
      case Qgis::MapGridAnnotationType::Longitude:
        countLongitudeLines++;
        break;
      case Qgis::MapGridAnnotationType::Latitude:
        countLatitudeLines++;
        break;
    }
  }

  int latitudeLineIndex = 0;
  int longitudeLineIndex = 0;

  //simple approach: draw vertical lines first, then horizontal ones
  if ( mGridStyle == Qgis::MapGridStyle::Lines )
  {
    //we need to scale line coordinates to dots, rather than mm, since the painter has already been scaled to dots
    //this is done by multiplying each line coordinate by dotsPerMM
    QLineF line;
    for ( ; vIt != mGridLines.constEnd(); ++vIt )
    {
      if ( vIt->coordinateType != Qgis::MapGridAnnotationType::Longitude )
        continue;
      line = QLineF( vIt->line.first() * dotsPerMM, vIt->line.last() * dotsPerMM );

      longitudeLineIndex++;
      context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLongitudeLines, true ) );
      context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, longitudeLineIndex, true ) );
      context.expressionContext().lastScope()->setVariable( u"grid_number"_s, vIt->coordinate );
      context.expressionContext().lastScope()->setVariable( u"grid_axis"_s, "x" );

      drawGridLine( line, context );
    }

    for ( ; hIt != mGridLines.constEnd(); ++hIt )
    {
      if ( hIt->coordinateType != Qgis::MapGridAnnotationType::Latitude )
        continue;
      line = QLineF( hIt->line.first() * dotsPerMM, hIt->line.last() * dotsPerMM );

      latitudeLineIndex++;
      context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLatitudeLines, true ) );
      context.expressionContext().lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, latitudeLineIndex, true ) );
      context.expressionContext().lastScope()->setVariable( u"grid_number"_s, hIt->coordinate );
      context.expressionContext().lastScope()->setVariable( u"grid_axis"_s, "y" );

      drawGridLine( line, context );
    }
  }
  else if ( mGridStyle != Qgis::MapGridStyle::FrameAndAnnotationsOnly ) //cross or markers
  {
    QLineF l1, l2;
    QPointF intersectionPoint, crossEnd1, crossEnd2;
    for ( ; vIt != mGridLines.constEnd(); ++vIt )
    {
      if ( vIt->coordinateType != Qgis::MapGridAnnotationType::Longitude )
        continue;

      l1 = QLineF( vIt->line.first(), vIt->line.last() );

      //test for intersection with every horizontal line
      hIt = mGridLines.constBegin();
      for ( ; hIt != mGridLines.constEnd(); ++hIt )
      {
        if ( hIt->coordinateType != Qgis::MapGridAnnotationType::Latitude )
          continue;

        l2 = QLineF( hIt->line.first(), hIt->line.last() );

        if ( l2.intersects( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
        {
          if ( mGridStyle == Qgis::MapGridStyle::LineCrosses )
          {
            //apply a threshold to avoid calculate point if the two points are very close together (can lead to artifacts)
            crossEnd1 = ( ( intersectionPoint - l1.p1() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p1(), mEvaluatedCrossLength ) : intersectionPoint;
            crossEnd2 = ( ( intersectionPoint - l1.p2() ).manhattanLength() > 0.01 ) ?
                        QgsSymbolLayerUtils::pointOnLineWithDistance( intersectionPoint, l1.p2(), mEvaluatedCrossLength ) : intersectionPoint;
            //draw line using coordinates scaled to dots
            drawGridLine( QLineF( crossEnd1  * dotsPerMM, crossEnd2  * dotsPerMM ), context );
          }
          else if ( mGridStyle == Qgis::MapGridStyle::Markers )
          {
            drawGridMarker( intersectionPoint * dotsPerMM, context );
          }
        }
      }
    }
    if ( mGridStyle == Qgis::MapGridStyle::Markers )
    {
      //markers mode, so we have no need to process horizontal lines (we've already
      //drawn markers on the intersections between horizontal and vertical lines)
      return;
    }

    hIt = mGridLines.constBegin();
    for ( ; hIt != mGridLines.constEnd(); ++hIt )
    {
      if ( hIt->coordinateType != Qgis::MapGridAnnotationType::Latitude )
        continue;

      l1 = QLineF( hIt->line.first(), hIt->line.last() );

      vIt = mGridLines.constBegin();
      for ( ; vIt != mGridLines.constEnd(); ++vIt )
      {
        if ( vIt->coordinateType != Qgis::MapGridAnnotationType::Longitude )
          continue;

        l2 = QLineF( vIt->line.first(), vIt->line.last() );

        if ( l2.intersects( l1, &intersectionPoint ) == QLineF::BoundedIntersection )
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
    p->setRenderHint( QPainter::Antialiasing, mMap->layout()->renderContext().flags() & Qgis::LayoutRenderFlag::Antialiasing );
  }


  switch ( mGridFrameStyle )
  {
    case Qgis::MapGridFrameStyle::Zebra:
    case Qgis::MapGridFrameStyle::ZebraNautical:
      drawGridFrameZebra( p, extension );
      break;
    case Qgis::MapGridFrameStyle::InteriorTicks:
    case Qgis::MapGridFrameStyle::ExteriorTicks:
    case Qgis::MapGridFrameStyle::InteriorExteriorTicks:
      drawGridFrameTicks( p, extension );
      break;

    case Qgis::MapGridFrameStyle::LineBorder:
    case Qgis::MapGridFrameStyle::LineBorderNautical:
      drawGridFrameLine( p, extension );
      break;

    case Qgis::MapGridFrameStyle::NoFrame:
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
  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) )
  {
    drawGridFrameZebraBorder( p, Qgis::MapGridBorderSide::Left, extension ? &extension->left : nullptr );
  }
  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) )
  {
    drawGridFrameZebraBorder( p, Qgis::MapGridBorderSide::Right, extension ? &extension->right : nullptr );
  }
  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) )
  {
    drawGridFrameZebraBorder( p, Qgis::MapGridBorderSide::Top, extension ? &extension->top : nullptr );
  }
  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) )
  {
    drawGridFrameZebraBorder( p, Qgis::MapGridBorderSide::Bottom, extension ? &extension->bottom : nullptr );
  }
}

void QgsLayoutItemMapGrid::drawGridFrameZebraBorder( QPainter *p, Qgis::MapGridBorderSide border, double *extension ) const
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

      if ( border == Qgis::MapGridBorderSide::Left || border == Qgis::MapGridBorderSide::Right )
        pos.insert( annot.position.y(), it->coordinate );
      else
        pos.insert( annot.position.x(), it->coordinate );
    }
  }


  if ( border == Qgis::MapGridBorderSide::Left || border == Qgis::MapGridBorderSide::Right )
  {
    pos.insert( mMap->rect().height(), mMap->rect().height() );
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) )
    {
      drawBLBox = border == Qgis::MapGridBorderSide::Left;
      drawBRBox = border == Qgis::MapGridBorderSide::Right;
    }
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) )
    {
      drawTLBox = border == Qgis::MapGridBorderSide::Left;
      drawTRBox = border == Qgis::MapGridBorderSide::Right;
    }
    if ( !drawTLBox && border == Qgis::MapGridBorderSide::Left )
      color1 = true;
  }
  else if ( border == Qgis::MapGridBorderSide::Top || border == Qgis::MapGridBorderSide::Bottom )
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
    if ( border == Qgis::MapGridBorderSide::Left || border == Qgis::MapGridBorderSide::Right )
    {
      height = posIt.key() - currentCoord;
      width = mEvaluatedGridFrameWidth;
      x = ( border == Qgis::MapGridBorderSide::Left ) ? -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ) : mMap->rect().width() + mEvaluatedGridFrameMargin;
      y = currentCoord;
    }
    else //top or bottom
    {
      height = mEvaluatedGridFrameWidth;
      width = posIt.key() - currentCoord;
      x = currentCoord;
      y = ( border == Qgis::MapGridBorderSide::Top ) ? -( mEvaluatedGridFrameWidth + mEvaluatedGridFrameMargin ) : mMap->rect().height() + mEvaluatedGridFrameMargin;
    }
    p->drawRect( QRectF( x, y, width, height ) );
    currentCoord = posIt.key();
    color1 = !color1;
  }

  if ( mGridFrameStyle == Qgis::MapGridFrameStyle::ZebraNautical || qgsDoubleNear( mEvaluatedGridFrameMargin, 0.0 ) )
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
      if ( mGridFrameStyle == Qgis::MapGridFrameStyle::InteriorExteriorTicks )
      {
        facingLeft = ( annot.angle != 0 );
        facingRight = ( annot.angle != 0 );
      }
      else if ( mGridFrameStyle == Qgis::MapGridFrameStyle::ExteriorTicks )
      {
        facingLeft = ( annot.angle > 0 );
        facingRight = ( annot.angle < 0 );
      }
      else
      {
        facingLeft = ( annot.angle < 0 );
        facingRight = ( annot.angle > 0 );
      }

      if ( annot.border == Qgis::MapGridBorderSide::Top && ( ( facingLeft && annot.position.x() < mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.x() > mMap->rect().width() - mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == Qgis::MapGridBorderSide::Bottom && ( ( facingLeft && annot.position.x() > mMap->rect().width() - mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.x() < mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == Qgis::MapGridBorderSide::Left && ( ( facingLeft && annot.position.y() > mMap->rect().height() - mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.y() < mRotatedTicksMarginToCorner ) ) )
        continue;
      if ( annot.border == Qgis::MapGridBorderSide::Right && ( ( facingLeft && annot.position.y() < mRotatedTicksMarginToCorner ) ||
           ( facingRight && annot.position.y() > mMap->rect().height() - mRotatedTicksMarginToCorner ) ) )
        continue;

      const QVector2D normalVector = borderToNormal2D( annot.border );
      const QVector2D vector = ( mRotatedTicksEnabled ) ? annot.vector : normalVector;

      double fA = mEvaluatedGridFrameMargin; // point near to frame
      double fB = mEvaluatedGridFrameMargin + mEvaluatedGridFrameWidth; // point far from frame

      if ( mRotatedTicksEnabled && mRotatedTicksLengthMode == Qgis::MapGridTickLengthMode::OrthogonalTicks )
      {
        fA /= QVector2D::dotProduct( vector, normalVector );
        fB /= QVector2D::dotProduct( vector, normalVector );
      }

      // extents isn't computed accurately
      if ( extension )
      {
        if ( mGridFrameStyle != Qgis::MapGridFrameStyle::InteriorTicks )
          extension->UpdateBorder( annot.border, fB );
        continue;
      }

      QVector2D pA;
      QVector2D pB;
      if ( mGridFrameStyle == Qgis::MapGridFrameStyle::InteriorTicks )
      {
        pA = annot.position + fA * vector;
        pB = annot.position + fB * vector;
      }
      else if ( mGridFrameStyle == Qgis::MapGridFrameStyle::ExteriorTicks )
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

  const bool drawDiagonals = mGridFrameStyle == Qgis::MapGridFrameStyle::LineBorderNautical && !qgsDoubleNear( mEvaluatedGridFrameMargin, 0.0 );

  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) )
  {
    if ( extension )
      extension->UpdateBorder( Qgis::MapGridBorderSide::Left, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) )
  {
    if ( extension )
      extension->UpdateBorder( Qgis::MapGridBorderSide::Right, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( mMap->rect().width() + mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) )
  {
    if ( extension )
      extension->UpdateBorder( Qgis::MapGridBorderSide::Top, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, 0 - mEvaluatedGridFrameMargin ) );
  }

  if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) )
  {
    if ( extension )
      extension->UpdateBorder( Qgis::MapGridBorderSide::Bottom, mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 );
    else
      p->drawLine( QLineF( 0 - mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin, mMap->rect().width() + mEvaluatedGridFrameMargin, mMap->rect().height() + mEvaluatedGridFrameMargin ) );
  }

  if ( ! extension && drawDiagonals )
  {
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) || testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) )
    {
      //corner left-top
      const double X1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0;
      const double Y1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0;
      p->drawLine( QLineF( 0, 0, X1, Y1 ) );
    }
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) || testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) )
    {
      //corner right-bottom
      const double X1 = mMap->rect().width() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      const double Y1 = mMap->rect().height() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      p->drawLine( QLineF( mMap->rect().width(), mMap->rect().height(), X1, Y1 ) );
    }
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) || testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) )
    {
      //corner right-top
      const double X1 = mMap->rect().width() + mEvaluatedGridFrameMargin - mEvaluatedGridFrameLineThickness / 2.0 ;
      const double Y1 = 0 - mEvaluatedGridFrameMargin + mEvaluatedGridFrameLineThickness / 2.0 ;
      p->drawLine( QLineF( mMap->rect().width(), 0, X1, Y1 ) );
    }
    if ( testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) || testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) )
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
  if ( mGridLines.empty() )
    return;

  QString currentAnnotationString;
  QList< GridLine >::const_iterator it = mGridLines.constBegin();

  QgsExpressionContextScope *gridScope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( expressionContext, gridScope );

  bool geographic = false;
  if ( mCRS.isValid() )
  {
    geographic = mCRS.isGeographic();
  }
  else if ( mMap && mMap->layout() )
  {
    geographic = mMap->crs().isGeographic();
  }

  const bool forceWrap = ( geographic && it->coordinateType == Qgis::MapGridAnnotationType::Longitude &&
                           ( mGridAnnotationFormat == Qgis::MapGridAnnotationFormat::Decimal || mGridAnnotationFormat == Qgis::MapGridAnnotationFormat::DecimalWithSuffix ) );

  int countLongitudeLines = 0;
  int countLatitudeLines = 0;
  for ( const GridLine &line : mGridLines )
  {
    switch ( line.coordinateType )
    {
      case Qgis::MapGridAnnotationType::Longitude:
        countLongitudeLines++;
        break;
      case Qgis::MapGridAnnotationType::Latitude:
        countLatitudeLines++;
        break;
    }
  }

  int latitudeLineIndex = 0;
  int longitudeLineIndex = 0;
  for ( ; it != mGridLines.constEnd(); ++it )
  {
    double value = it->coordinate;
    switch ( it->coordinateType )
    {
      case Qgis::MapGridAnnotationType::Longitude:
        longitudeLineIndex++;
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLongitudeLines, true ) );
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, longitudeLineIndex, true ) );
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_axis"_s, u"x"_s, true ) );
        break;

      case Qgis::MapGridAnnotationType::Latitude:
        latitudeLineIndex++;
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_count"_s, countLatitudeLines, true ) );
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_index"_s, latitudeLineIndex, true ) );
        gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_axis"_s, u"y"_s, true ) );
        break;
    }

    if ( forceWrap )
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

    gridScope->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_number"_s, value, true ) );

    if ( mDrawAnnotationProperty )
    {
      bool ok = false;
      const bool display = mDrawAnnotationProperty->valueAsBool( expressionContext, true, &ok );
      if ( ok && !display )
        continue;
    }
    currentAnnotationString = gridAnnotationString( it->coordinate, it->coordinateType, expressionContext, geographic );
    drawCoordinateAnnotation( context, it->startAnnotation, currentAnnotationString, it->coordinateType, extension );
    drawCoordinateAnnotation( context, it->endAnnotation, currentAnnotationString, it->coordinateType, extension );
  }
}

void QgsLayoutItemMapGrid::drawCoordinateAnnotation( QgsRenderContext &context, GridLineAnnotation annot, const QString &annotationString, const Qgis::MapGridAnnotationType coordinateType, GridExtension *extension ) const
{
  if ( !mMap )
  {
    return;
  }

  if ( ! shouldShowAnnotationForSide( coordinateType, annot.border ) )
    return;

  // painter is in MM, scale to dots
  std::unique_ptr< QgsScopedQPainterState > painterState;
  double dotsPerMM = 1;

  if ( context.painter() && context.painter()->device() )
  {
    painterState = std::make_unique< QgsScopedQPainterState >( context.painter() );
    dotsPerMM = context.painter()->device()->logicalDpiX() / 25.4;
    context.painter()->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots
  }

  const Qgis::MapGridBorderSide frameBorder = annot.border;

  const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( annotationString.split( '\n' ), mAnnotationFormat );
  const double textScaleFactor = QgsTextRenderer::calculateScaleFactorForFormat( context, mAnnotationFormat );
  const QgsTextDocumentMetrics documentMetrics = QgsTextDocumentMetrics::calculateMetrics( doc, mAnnotationFormat, context, textScaleFactor );
  const QSizeF sizePainterUnits = documentMetrics.documentSize( Qgis::TextLayoutMode::Point, Qgis::TextOrientation::Horizontal );
  const double painterUnitsToMM = 1 / context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  double textWidthPainterUnits = sizePainterUnits.width();
  if ( extension )
    textWidthPainterUnits *= 1.1; // little bit of extra padding when we are calculating the bounding rect, to account for antialiasing

  const double textWidthMM = textWidthPainterUnits * painterUnitsToMM ;

  double textHeightPainterUnits = 0;
  if ( extension || doc.size() > 1 )
  {
    textHeightPainterUnits = sizePainterUnits.height();
  }
  else
  {
    // special logic for single line annotations -- using fixed digit height only.
    // kept for pixel-perfect compatibility with existing renders prior to proper support for
    // multi-line annotation labels
    textHeightPainterUnits = QgsTextRenderer::textHeight( context, mAnnotationFormat, '0', false );
  }
  const double textHeightMM = textHeightPainterUnits * painterUnitsToMM;


  const Qgis::MapGridAnnotationPosition anotPos = annotationPosition( frameBorder );
  const Qgis::MapGridAnnotationDirection anotDir = annotationDirection( frameBorder );

  // If the angle is below the threshold, we don't draw the annotation
  if ( abs( annot.angle ) / M_PI * 180.0 > 90.0 - mRotatedAnnotationsMinimumAngle + 0.0001 )
    return;

  const QVector2D normalVector = borderToNormal2D( annot.border );
  const QVector2D vector = ( mRotatedAnnotationsEnabled ) ? annot.vector : normalVector;

  // Distance to frame
  double distanceToFrameMM = mEvaluatedAnnotationFrameDistance;

  // Adapt distance to frame using the frame width and line thickness into account
  const bool isOverTick = ( anotDir == Qgis::MapGridAnnotationDirection::AboveTick || anotDir == Qgis::MapGridAnnotationDirection::OnTick || anotDir == Qgis::MapGridAnnotationDirection::UnderTick );
  const bool hasInteriorMargin = ! isOverTick && ( mGridFrameStyle == Qgis::MapGridFrameStyle::InteriorTicks || mGridFrameStyle == Qgis::MapGridFrameStyle::InteriorExteriorTicks );
  const bool hasExteriorMargin = ! isOverTick && ( mGridFrameStyle == Qgis::MapGridFrameStyle::Zebra || mGridFrameStyle == Qgis::MapGridFrameStyle::ExteriorTicks || mGridFrameStyle == Qgis::MapGridFrameStyle::InteriorExteriorTicks || mGridFrameStyle == Qgis::MapGridFrameStyle::ZebraNautical );
  const bool hasBorderWidth = ( mGridFrameStyle == Qgis::MapGridFrameStyle::Zebra || mGridFrameStyle == Qgis::MapGridFrameStyle::ZebraNautical || mGridFrameStyle == Qgis::MapGridFrameStyle::LineBorder || mGridFrameStyle == Qgis::MapGridFrameStyle::LineBorderNautical );
  if ( ( anotPos == Qgis::MapGridAnnotationPosition::InsideMapFrame && hasInteriorMargin ) || ( anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame && hasExteriorMargin ) )
    distanceToFrameMM += mEvaluatedGridFrameWidth;
  if ( hasBorderWidth )
    distanceToFrameMM += mEvaluatedGridFrameLineThickness / 2.0;

  if ( anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame )
    distanceToFrameMM *= -1;

  if ( mRotatedAnnotationsEnabled && mRotatedAnnotationsLengthMode == Qgis::MapGridTickLengthMode::OrthogonalTicks )
  {
    distanceToFrameMM /= QVector2D::dotProduct( vector, normalVector );
  }

  const QVector2D annotationPositionMM = annot.position + static_cast< float >( distanceToFrameMM ) * vector;

  const bool outside = ( anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame );

  QPointF anchorMM;
  int rotation = 0;

  if (
    anotDir == Qgis::MapGridAnnotationDirection::AboveTick ||
    anotDir == Qgis::MapGridAnnotationDirection::OnTick ||
    anotDir == Qgis::MapGridAnnotationDirection::UnderTick
  )
  {
    rotation = atan2( vector.y(), vector.x() ) / M_PI * 180;

    if ( rotation <= -90 || rotation > 90 )
    {
      rotation += 180;
      anchorMM.setX( outside ? 0 : textWidthMM ); // left / right
    }
    else
    {
      anchorMM.setX( outside ? textWidthMM : 0 ); // right / left
    }

    if ( anotDir == Qgis::MapGridAnnotationDirection::AboveTick )
      anchorMM.setY( 0.5 * textHeightMM ); // bottom
    else if ( anotDir == Qgis::MapGridAnnotationDirection::UnderTick )
      anchorMM.setY( -1.5 * textHeightMM ); // top
    else // OnTick
      anchorMM.setY( -0.5 * textHeightMM ); // middle

  }
  else if ( anotDir == Qgis::MapGridAnnotationDirection::Horizontal )
  {
    rotation = 0;
    anchorMM.setX( 0.5 * textWidthMM ); // center
    anchorMM.setY( -0.5 * textHeightMM ); // middle
    if ( frameBorder == Qgis::MapGridBorderSide::Top )
      anchorMM.setY( outside ? 0 : -textHeightMM ); // bottom / top
    else if ( frameBorder == Qgis::MapGridBorderSide::Right )
      anchorMM.setX( outside ? 0 : textWidthMM ); // left / right
    else if ( frameBorder == Qgis::MapGridBorderSide::Bottom )
      anchorMM.setY( outside ? -textHeightMM : 0 ); // top / bottom
    else if ( frameBorder == Qgis::MapGridBorderSide::Left )
      anchorMM.setX( outside ? textWidthMM : 0 ); // right / left
  }
  else if ( anotDir == Qgis::MapGridAnnotationDirection::Vertical )
  {
    rotation = -90;
    anchorMM.setX( 0.5 * textWidthMM ); // center
    anchorMM.setY( -0.5 * textHeightMM ); // middle
    if ( frameBorder == Qgis::MapGridBorderSide::Top )
      anchorMM.setX( outside ? 0 : textWidthMM ); // left / right
    else if ( frameBorder == Qgis::MapGridBorderSide::Right )
      anchorMM.setY( outside ? -textHeightMM : 0 ); // top / bottom
    else if ( frameBorder == Qgis::MapGridBorderSide::Bottom )
      anchorMM.setX( outside ? textWidthMM : 0 ); // right / left
    else if ( frameBorder == Qgis::MapGridBorderSide::Left )
      anchorMM.setY( outside ? 0 : -textHeightMM ); // bottom / top
  }
  else if ( anotDir == Qgis::MapGridAnnotationDirection::VerticalDescending )
  {
    rotation = 90;
    anchorMM.setX( 0.5 * textWidthMM ); // center
    anchorMM.setY( -0.5 * textHeightMM ); // middle
    if ( frameBorder == Qgis::MapGridBorderSide::Top )
      anchorMM.setX( outside ? textWidthMM : 0 ); // right / left
    else if ( frameBorder == Qgis::MapGridBorderSide::Right )
      anchorMM.setY( outside ? 0 : -textHeightMM ); // bottom / top
    else if ( frameBorder == Qgis::MapGridBorderSide::Bottom )
      anchorMM.setX( outside ? 0 : textWidthMM ); // left / right
    else if ( frameBorder == Qgis::MapGridBorderSide::Left )
      anchorMM.setY( outside ? -textHeightMM : 0 ); // top / bottom
  }
  else // ( anotDir == QgsLayoutItemMapGrid::BoundaryDirection )
  {
    const QVector2D borderVector = borderToVector2D( annot.border );
    rotation = atan2( borderVector.y(), borderVector.x() ) / M_PI * 180;
    anchorMM.setX( 0.5 * textWidthMM ); // center
    if ( anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame )
      anchorMM.setY( -textHeightMM ); // top
    else
      anchorMM.setY( 0 ); // bottom
  }

  // extents isn't computed accurately
  if ( extension && anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame )
  {
    extension->UpdateBorder( frameBorder, -distanceToFrameMM + textWidthMM );
    // We also add a general margin, can be useful for labels near corners
    extension->UpdateAll( textWidthMM / 2.0 );
  }

  if ( extension || !context.painter() )
    return;

  // Skip outwards facing annotations that are below mRotatedAnnotationsMarginToCorner
  bool facingLeft = ( annot.angle < 0 );
  bool facingRight = ( annot.angle > 0 );
  if ( anotPos == Qgis::MapGridAnnotationPosition::OutsideMapFrame )
  {
    facingLeft = !facingLeft;
    facingRight = !facingRight;
  }
  if ( annot.border == Qgis::MapGridBorderSide::Top && ( ( facingLeft && annot.position.x() < mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.x() > mMap->rect().width() - mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == Qgis::MapGridBorderSide::Bottom && ( ( facingLeft && annot.position.x() > mMap->rect().width() - mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.x() < mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == Qgis::MapGridBorderSide::Left && ( ( facingLeft && annot.position.y() > mMap->rect().height() - mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.y() < mRotatedAnnotationsMarginToCorner ) ) )
    return;
  if ( annot.border == Qgis::MapGridBorderSide::Right && ( ( facingLeft && annot.position.y() < mRotatedAnnotationsMarginToCorner ) ||
       ( facingRight && annot.position.y() > mMap->rect().height() - mRotatedAnnotationsMarginToCorner ) ) )
    return;

  context.painter()->translate( QPointF( annotationPositionMM .x(), annotationPositionMM .y() ) / painterUnitsToMM );
  context.painter()->rotate( rotation );
  context.painter()->translate( -anchorMM / painterUnitsToMM );
  QgsTextRenderer::drawDocument( QPointF( 0, 0 ), mAnnotationFormat, doc, documentMetrics, context, Qgis::TextHorizontalAlignment::Left, 0, Qgis::TextLayoutMode::Point );
}

QString QgsLayoutItemMapGrid::gridAnnotationString( const double value, Qgis::MapGridAnnotationType coord, QgsExpressionContext &expressionContext, bool isGeographic ) const
{
  //check if we are using degrees (ie, geographic crs)

  if ( mGridAnnotationFormat == Qgis::MapGridAnnotationFormat::Decimal )
  {
    return QString::number( value, 'f', mGridAnnotationPrecision );
  }
  else if ( mGridAnnotationFormat == Qgis::MapGridAnnotationFormat::DecimalWithSuffix )
  {
    QString hemisphere;

    const double coordRounded = qgsRound( value, mGridAnnotationPrecision );
    if ( coord == Qgis::MapGridAnnotationType::Longitude )
    {
      //don't use E/W suffixes if ambiguous (e.g., 180 degrees)
      if ( !isGeographic || ( coordRounded != 180.0 && coordRounded != 0.0 ) )
      {
        hemisphere = value < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
      }
    }
    else
    {
      //don't use N/S suffixes if ambiguous (e.g., 0 degrees)
      if ( !isGeographic || coordRounded != 0.0 )
      {
        hemisphere = value < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
      }
    }
    if ( isGeographic )
    {
      //insert degree symbol for geographic coordinates
      return QString::number( std::fabs( value ), 'f', mGridAnnotationPrecision ) + QChar( 176 ) + hemisphere;
    }
    else
    {
      return QString::number( std::fabs( value ), 'f', mGridAnnotationPrecision ) + hemisphere;
    }
  }
  else if ( mGridAnnotationFormat == Qgis::MapGridAnnotationFormat::CustomFormat )
  {
    if ( !mGridAnnotationExpression )
    {
      mGridAnnotationExpression = std::make_unique<QgsExpression>( mGridAnnotationExpressionString );
      mGridAnnotationExpression->prepare( &expressionContext );
    }
    return mGridAnnotationExpression->evaluate( &expressionContext ).toString();
  }

  QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatDecimalDegrees;
  QgsCoordinateFormatter::FormatFlags flags = QgsCoordinateFormatter::FormatFlags();
  switch ( mGridAnnotationFormat )
  {
    case Qgis::MapGridAnnotationFormat::Decimal:
    case Qgis::MapGridAnnotationFormat::DecimalWithSuffix:
    case Qgis::MapGridAnnotationFormat::CustomFormat:
      break; // already handled above

    case Qgis::MapGridAnnotationFormat::DegreeMinute:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
      break;

    case Qgis::MapGridAnnotationFormat::DegreeMinuteSecond:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
      break;

    case Qgis::MapGridAnnotationFormat::DegreeMinuteNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FormatFlags();
      break;

    case Qgis::MapGridAnnotationFormat::DegreeMinutePadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutes;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;

    case Qgis::MapGridAnnotationFormat::DegreeMinuteSecondNoSuffix:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FormatFlags();
      break;

    case Qgis::MapGridAnnotationFormat::DegreeMinuteSecondPadded:
      format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
      flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
      break;
  }

  switch ( coord )
  {
    case Qgis::MapGridAnnotationType::Longitude:
      return QgsCoordinateFormatter::formatX( value, format, mGridAnnotationPrecision, flags );

    case Qgis::MapGridAnnotationType::Latitude:
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
    case Qgis::MapGridUnit::Centimeters:
    case Qgis::MapGridUnit::Millimeters:
    {
      mapBoundingRect = mMap->rect();
      mapPolygon = QPolygonF( mMap->rect() );
      if ( mGridUnit == Qgis::MapGridUnit::Centimeters )
      {
        annotationScale = 0.1;
        gridIntervalY *= 10;
        gridOffsetY *= 10;
      }
      break;
    }

    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::DynamicPageSizeBased:
      break;
  }

  //consider to round up to the next step in case the left boundary is > 0
  const double roundCorrection = mapBoundingRect.top() > gridOffsetY ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.top() - gridOffsetY ) / gridIntervalY + roundCorrection ) * gridIntervalY + gridOffsetY;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || ( mGridUnit != Qgis::MapGridUnit::MapUnits && mGridUnit != Qgis::MapGridUnit::DynamicPageSizeBased ) )
  {
    //no rotation. Do it 'the easy way'

    double yCanvasCoord;
    while ( currentLevel <= mapBoundingRect.bottom() && gridLineCount < MAX_GRID_LINES )
    {
      yCanvasCoord = mMap->rect().height() * ( 1 - ( currentLevel - mapBoundingRect.top() ) / mapBoundingRect.height() );
      GridLine newLine;
      newLine.coordinate = currentLevel * annotationScale;
      newLine.coordinateType = Qgis::MapGridAnnotationType::Latitude;
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
      if ( it->intersects( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
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
      newLine.coordinateType = Qgis::MapGridAnnotationType::Latitude;
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
    case Qgis::MapGridUnit::Centimeters:
    case Qgis::MapGridUnit::Millimeters:
    {
      mapBoundingRect = mMap->rect();
      mapPolygon = QPolygonF( mMap->rect() );
      if ( mGridUnit == Qgis::MapGridUnit::Centimeters )
      {
        annotationScale = 0.1;
        gridIntervalX *= 10;
        gridOffsetX *= 10;
      }
      break;
    }

    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::DynamicPageSizeBased:
      break;
  }

  //consider to round up to the next step in case the left boundary is > 0
  const double roundCorrection = mapBoundingRect.left() > gridOffsetX ? 1.0 : 0.0;
  double currentLevel = static_cast< int >( ( mapBoundingRect.left() - gridOffsetX ) / gridIntervalX + roundCorrection ) * gridIntervalX + gridOffsetX;

  int gridLineCount = 0;
  if ( qgsDoubleNear( mMap->mapRotation(), 0.0 ) || ( mGridUnit != Qgis::MapGridUnit::MapUnits && mGridUnit != Qgis::MapGridUnit::DynamicPageSizeBased ) )
  {
    //no rotation. Do it 'the easy way'
    double xCanvasCoord;
    while ( currentLevel <= mapBoundingRect.right() && gridLineCount < MAX_GRID_LINES )
    {
      xCanvasCoord = mMap->rect().width() * ( currentLevel - mapBoundingRect.left() ) / mapBoundingRect.width();

      GridLine newLine;
      newLine.coordinate = currentLevel * annotationScale;
      newLine.coordinateType = Qgis::MapGridAnnotationType::Longitude;
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
      if ( it->intersects( gridLine, &intersectionPoint ) == QLineF::BoundedIntersection )
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
      newLine.coordinateType = Qgis::MapGridAnnotationType::Longitude;
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

  const double roundCorrection = bbox.yMaximum() > mEvaluatedOffsetY ? 1.0 : 0.0;
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
        QgsDebugError( u"Caught CRS exception %1"_s.arg( cse.what() ) );
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
        newLine.coordinateType = Qgis::MapGridAnnotationType::Latitude;
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

  const double roundCorrection = bbox.xMinimum() > mEvaluatedOffsetX ? 1.0 : 0.0;
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
        QgsDebugError( u"Caught CRS exception %1"_s.arg( cse.what() ) );
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
        newLine.coordinateType = Qgis::MapGridAnnotationType::Longitude;
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

bool QgsLayoutItemMapGrid::shouldShowDivisionForSide( Qgis::MapGridAnnotationType coordinate, Qgis::MapGridBorderSide side ) const
{
  switch ( side )
  {
    case Qgis::MapGridBorderSide::Left:
      return testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) && shouldShowForDisplayMode( coordinate, mEvaluatedLeftFrameDivisions );
    case Qgis::MapGridBorderSide::Right:
      return testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) && shouldShowForDisplayMode( coordinate, mEvaluatedRightFrameDivisions );
    case Qgis::MapGridBorderSide::Top:
      return testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) && shouldShowForDisplayMode( coordinate, mEvaluatedTopFrameDivisions );
    case Qgis::MapGridBorderSide::Bottom:
      return testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) && shouldShowForDisplayMode( coordinate, mEvaluatedBottomFrameDivisions );
  }
  return false; // no warnings
}

bool QgsLayoutItemMapGrid::shouldShowAnnotationForSide( Qgis::MapGridAnnotationType coordinate, Qgis::MapGridBorderSide side ) const
{
  switch ( side )
  {
    case Qgis::MapGridBorderSide::Left:
      return shouldShowForDisplayMode( coordinate, mEvaluatedLeftGridAnnotationDisplay );
    case Qgis::MapGridBorderSide::Right:
      return shouldShowForDisplayMode( coordinate, mEvaluatedRightGridAnnotationDisplay );
    case Qgis::MapGridBorderSide::Top:
      return shouldShowForDisplayMode( coordinate, mEvaluatedTopGridAnnotationDisplay );
    case Qgis::MapGridBorderSide::Bottom:
      return shouldShowForDisplayMode( coordinate, mEvaluatedBottomGridAnnotationDisplay );
  }
  return false; // no warnings
}

bool QgsLayoutItemMapGrid::shouldShowForDisplayMode( Qgis::MapGridAnnotationType coordinate, Qgis::MapGridComponentVisibility mode ) const
{
  return mode == Qgis::MapGridComponentVisibility::ShowAll
         || ( mode == Qgis::MapGridComponentVisibility::LatitudeOnly && coordinate == Qgis::MapGridAnnotationType::Latitude )
         || ( mode == Qgis::MapGridComponentVisibility::LongitudeOnly && coordinate == Qgis::MapGridAnnotationType::Longitude );
}

Qgis::MapGridComponentVisibility gridAnnotationDisplayModeFromDD( QString ddValue, Qgis::MapGridComponentVisibility defValue )
{
  if ( ddValue.compare( "x_only"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MapGridComponentVisibility::LatitudeOnly;
  else if ( ddValue.compare( "y_only"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MapGridComponentVisibility::LongitudeOnly;
  else if ( ddValue.compare( "disabled"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MapGridComponentVisibility::HideAll;
  else if ( ddValue.compare( "all"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MapGridComponentVisibility::ShowAll;
  else
    return defValue;
}

void QgsLayoutItemMapGrid::refreshDataDefinedProperties()
{
  const QgsExpressionContext context = createExpressionContext();

  // if we are changing the grid interval or offset, then we also have to mark the transform as dirty
  mTransformDirty = mTransformDirty
                    || mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MapGridIntervalX )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MapGridIntervalY )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MapGridOffsetX )
                    || mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MapGridOffsetY );

  mEvaluatedEnabled = mDataDefinedProperties.valueAsBool( QgsLayoutObject::DataDefinedProperty::MapGridEnabled, context, enabled() );

  // suppress false positive clang tidy warning
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MapGridDrawAnnotation ) )
  {
    mDrawAnnotationProperty.reset( new QgsProperty( mDataDefinedProperties.property( QgsLayoutObject::DataDefinedProperty::MapGridDrawAnnotation ) ) );
    mDrawAnnotationProperty->prepare( context );
  }
  else
  {
    mDrawAnnotationProperty.reset();
  }
  // NOLINTEND(bugprone-branch-clone)

  switch ( mGridUnit )
  {
    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::Millimeters:
    case Qgis::MapGridUnit::Centimeters:
    {
      mEvaluatedIntervalX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridIntervalX, context, mGridIntervalX );
      mEvaluatedIntervalY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridIntervalY, context, mGridIntervalY );
      break;
    }

    case Qgis::MapGridUnit::DynamicPageSizeBased:
    {
      if ( mMaximumIntervalWidth < mMinimumIntervalWidth )
      {
        mEvaluatedEnabled = false;
      }
      else
      {
        const double mapWidthMm = mLayout->renderContext().measurementConverter().convert( mMap->sizeWithUnits(), Qgis::LayoutUnit::Millimeters ).width();
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
  mEvaluatedOffsetX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridOffsetX, context, mGridOffsetX );
  mEvaluatedOffsetY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridOffsetY, context, mGridOffsetY );
  mEvaluatedGridFrameWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridFrameSize, context, mGridFrameWidth );
  mEvaluatedGridFrameMargin = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridFrameMargin, context, mGridFrameMargin );
  mEvaluatedAnnotationFrameDistance = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridLabelDistance, context, mAnnotationFrameDistance );
  mEvaluatedCrossLength = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridCrossSize, context, mCrossLength );
  mEvaluatedGridFrameLineThickness = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MapGridFrameLineThickness, context, mGridFramePenThickness );
  mEvaluatedLeftGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayLeft, context ), mLeftGridAnnotationDisplay );
  mEvaluatedRightGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayRight, context ), mRightGridAnnotationDisplay );
  mEvaluatedTopGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayTop, context ), mTopGridAnnotationDisplay );
  mEvaluatedBottomGridAnnotationDisplay = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayBottom, context ), mBottomGridAnnotationDisplay );
  mEvaluatedLeftFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsLeft, context ), mLeftFrameDivisions );
  mEvaluatedRightFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsRight, context ), mRightFrameDivisions );
  mEvaluatedTopFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsTop, context ), mTopFrameDivisions );
  mEvaluatedBottomFrameDivisions = gridAnnotationDisplayModeFromDD( mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsBottom, context ), mBottomFrameDivisions );
}

double QgsLayoutItemMapGrid::mapWidth() const
{
  if ( !mMap )
  {
    return 0.0;
  }

  const QgsRectangle mapExtent = mMap->extent();
  const Qgis::DistanceUnit distanceUnit = mCRS.isValid() ? mCRS.mapUnits() : mMap->crs().mapUnits();
  if ( distanceUnit == Qgis::DistanceUnit::Unknown )
  {
    return mapExtent.width();
  }
  else
  {
    QgsDistanceArea da;

    da.setSourceCrs( mMap->crs(), mLayout->project()->transformContext() );
    da.setEllipsoid( mLayout->project()->ellipsoid() );

    const Qgis::DistanceUnit units = da.lengthUnits();
    double measure = 0;
    try
    {
      measure = da.measureLine( QgsPointXY( mapExtent.xMinimum(), mapExtent.yMinimum() ),
                                QgsPointXY( mapExtent.xMaximum(), mapExtent.yMinimum() ) );
      measure /= QgsUnitTypes::fromUnitToUnitFactor( distanceUnit, units );
    }
    catch ( QgsCsException & )
    {
      // TODO report errors to user
      QgsDebugError( u"An error occurred while calculating length"_s );
    }
    return measure;
  }
}

bool sortByDistance( QPair<qreal, Qgis::MapGridBorderSide> a, QPair<qreal, Qgis::MapGridBorderSide> b )
{
  return a.first < b.first;
}

Qgis::MapGridBorderSide QgsLayoutItemMapGrid::borderForLineCoord( QPointF p, const Qgis::MapGridAnnotationType coordinateType ) const
{
  if ( !mMap )
  {
    return Qgis::MapGridBorderSide::Left;
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
    if ( coordinateType == Qgis::MapGridAnnotationType::Latitude )
    {
      if ( p.x() <= tolerance )
      {
        return Qgis::MapGridBorderSide::Left;
      }
      else
      {
        return Qgis::MapGridBorderSide::Right;
      }
    }
    else
    {
      if ( p.y() <= tolerance )
      {
        return Qgis::MapGridBorderSide::Top;
      }
      else
      {
        return Qgis::MapGridBorderSide::Bottom;
      }
    }
  }

  //otherwise, guess side based on closest map side to point
  QList< QPair<qreal, Qgis::MapGridBorderSide > > distanceToSide;
  distanceToSide << qMakePair( p.x(), Qgis::MapGridBorderSide::Left );
  distanceToSide << qMakePair( mMap->rect().width() - p.x(), Qgis::MapGridBorderSide::Right );
  distanceToSide << qMakePair( p.y(), Qgis::MapGridBorderSide::Top );
  distanceToSide << qMakePair( mMap->rect().height() - p.y(), Qgis::MapGridBorderSide::Bottom );

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
    mAnnotationFormat.setSizeUnit( Qgis::RenderUnit::Points );
  }
  else if ( font.pixelSize() > 0 )
  {
    mAnnotationFormat.setSize( font.pixelSize() );
    mAnnotationFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
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

void QgsLayoutItemMapGrid::setAnnotationDisplay( const Qgis::MapGridComponentVisibility display, const Qgis::MapGridBorderSide border )
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      mLeftGridAnnotationDisplay = display;
      break;
    case Qgis::MapGridBorderSide::Right:
      mRightGridAnnotationDisplay = display;
      break;
    case Qgis::MapGridBorderSide::Top:
      mTopGridAnnotationDisplay = display;
      break;
    case Qgis::MapGridBorderSide::Bottom:
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

Qgis::MapGridComponentVisibility QgsLayoutItemMapGrid::annotationDisplay( const Qgis::MapGridBorderSide border ) const
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      return mLeftGridAnnotationDisplay;
    case Qgis::MapGridBorderSide::Right:
      return mRightGridAnnotationDisplay;
    case Qgis::MapGridBorderSide::Top:
      return mTopGridAnnotationDisplay;
    case Qgis::MapGridBorderSide::Bottom:
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
    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::DynamicPageSizeBased:
    {
      if ( mCRS.isValid() && mCRS != mMap->crs() )
      {
        drawGridCrsTransform( context, 0, true );
        break;
      }
    }
    [[fallthrough]];
    case Qgis::MapGridUnit::Centimeters:
    case Qgis::MapGridUnit::Millimeters:
      drawGridNoTransform( context, 0, true );
      break;
  }

  if ( mGridFrameStyle != Qgis::MapGridFrameStyle::NoFrame || mShowGridAnnotation )
    updateGridLinesAnnotationsPositions();

  if ( mGridFrameStyle != Qgis::MapGridFrameStyle::NoFrame )
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

void QgsLayoutItemMapGrid::setUnits( const Qgis::MapGridUnit unit )
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

void QgsLayoutItemMapGrid::setStyle( const Qgis::MapGridStyle style )
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

void QgsLayoutItemMapGrid::setAnnotationDirection( const Qgis::MapGridAnnotationDirection direction, const Qgis::MapGridBorderSide border )
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      mLeftGridAnnotationDirection = direction;
      break;
    case Qgis::MapGridBorderSide::Right:
      mRightGridAnnotationDirection = direction;
      break;
    case Qgis::MapGridBorderSide::Top:
      mTopGridAnnotationDirection = direction;
      break;
    case Qgis::MapGridBorderSide::Bottom:
      mBottomGridAnnotationDirection = direction;
      break;
  }

  if ( mMap )
  {
    mMap->updateBoundingRect();
    mMap->update();
  }
}

void QgsLayoutItemMapGrid::setFrameSideFlags( Qgis::MapGridFrameSideFlags flags )
{
  mGridFrameSides = flags;
}

void QgsLayoutItemMapGrid::setFrameSideFlag( Qgis::MapGridFrameSideFlag flag, bool on )
{
  mGridFrameSides.setFlag( flag, on );
}

Qgis::MapGridFrameSideFlags QgsLayoutItemMapGrid::frameSideFlags() const
{
  return mGridFrameSides;
}

QgsExpressionContext QgsLayoutItemMapGrid::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutItemMapItem::createExpressionContext();
  context.appendScope( new QgsExpressionContextScope( tr( "Grid" ) ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_number"_s, 0, true ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"grid_axis"_s, "x", true ) );
  context.setHighlightedVariables( QStringList() << u"grid_number"_s << u"grid_axis"_s );
  return context;
}

bool QgsLayoutItemMapGrid::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mGridLineSymbol )
  {
    QgsStyleSymbolEntity entity( mGridLineSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"grid"_s, QObject::tr( "Grid" ) ) ) )
      return false;
  }
  if ( mGridMarkerSymbol )
  {
    QgsStyleSymbolEntity entity( mGridMarkerSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"grid"_s, QObject::tr( "Grid" ) ) ) )
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

bool QgsLayoutItemMapGrid::testFrameSideFlag( Qgis::MapGridFrameSideFlag flag ) const
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

void QgsLayoutItemMapGrid::setAnnotationDirection( const Qgis::MapGridAnnotationDirection direction )
{
  mLeftGridAnnotationDirection = direction;
  mRightGridAnnotationDirection = direction;
  mTopGridAnnotationDirection = direction;
  mBottomGridAnnotationDirection = direction;
}

void QgsLayoutItemMapGrid::setAnnotationPosition( const Qgis::MapGridAnnotationPosition position, const Qgis::MapGridBorderSide border )
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      mLeftGridAnnotationPosition = position;
      break;
    case Qgis::MapGridBorderSide::Right:
      mRightGridAnnotationPosition = position;
      break;
    case Qgis::MapGridBorderSide::Top:
      mTopGridAnnotationPosition = position;
      break;
    case Qgis::MapGridBorderSide::Bottom:
      mBottomGridAnnotationPosition = position;
      break;
  }

  if ( mMap )
  {
    mMap->updateBoundingRect();
    mMap->update();
  }
}

Qgis::MapGridAnnotationPosition QgsLayoutItemMapGrid::annotationPosition( const Qgis::MapGridBorderSide border ) const
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      return mLeftGridAnnotationPosition;
    case Qgis::MapGridBorderSide::Right:
      return mRightGridAnnotationPosition;
    case Qgis::MapGridBorderSide::Top:
      return mTopGridAnnotationPosition;
    case Qgis::MapGridBorderSide::Bottom:
      return mBottomGridAnnotationPosition;
  }
  return mLeftGridAnnotationPosition; // no warnings
}

void QgsLayoutItemMapGrid::setAnnotationFrameDistance( const double distance )
{
  mAnnotationFrameDistance = distance;
  refreshDataDefinedProperties();
}

Qgis::MapGridAnnotationDirection QgsLayoutItemMapGrid::annotationDirection( const Qgis::MapGridBorderSide border ) const
{
  if ( !mMap )
  {
    return mLeftGridAnnotationDirection;
  }

  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      return mLeftGridAnnotationDirection;
    case Qgis::MapGridBorderSide::Right:
      return mRightGridAnnotationDirection;
    case Qgis::MapGridBorderSide::Top:
      return mTopGridAnnotationDirection;
    case Qgis::MapGridBorderSide::Bottom:
      return mBottomGridAnnotationDirection;
  }
  return mLeftGridAnnotationDirection; // no warnings
}

void QgsLayoutItemMapGrid::setAnnotationExpression( const QString &expression )
{
  mGridAnnotationExpressionString = expression;
  mGridAnnotationExpression.reset();
}

void QgsLayoutItemMapGrid::setFrameDivisions( const Qgis::MapGridComponentVisibility divisions, const Qgis::MapGridBorderSide border )
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      mLeftFrameDivisions = divisions;
      break;
    case Qgis::MapGridBorderSide::Right:
      mRightFrameDivisions = divisions;
      break;
    case Qgis::MapGridBorderSide::Top:
      mTopFrameDivisions = divisions;
      break;
    case Qgis::MapGridBorderSide::Bottom:
      mBottomFrameDivisions = divisions;
      break;
  }

  refreshDataDefinedProperties();

  if ( mMap )
  {
    mMap->update();
  }
}

Qgis::MapGridComponentVisibility QgsLayoutItemMapGrid::frameDivisions( const Qgis::MapGridBorderSide border ) const
{
  switch ( border )
  {
    case Qgis::MapGridBorderSide::Left:
      return mLeftFrameDivisions;
    case Qgis::MapGridBorderSide::Right:
      return mRightFrameDivisions;
    case Qgis::MapGridBorderSide::Top:
      return mTopFrameDivisions;
    case Qgis::MapGridBorderSide::Bottom:
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
    QgsDebugError( u"Caught CRS exception %1"_s.arg( cse.what() ) );
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

void QgsLayoutItemMapGrid::copyProperties( const QgsLayoutItemMapGrid *other )
{
  // grid
  setStyle( other->style() );
  setIntervalX( other->intervalX() );
  setIntervalY( other->intervalY() );
  setOffsetX( other->offsetX() );
  setOffsetY( other->offsetX() );
  setCrossLength( other->crossLength() );
  setFrameStyle( other->frameStyle() );
  setFrameSideFlags( other->frameSideFlags() );
  setFrameWidth( other->frameWidth() );
  setFrameMargin( other->frameMargin() );
  setFramePenSize( other->framePenSize() );
  setFramePenColor( other->framePenColor() );
  setFrameFillColor1( other->frameFillColor1() );
  setFrameFillColor2( other->frameFillColor2() );

  setFrameDivisions( other->frameDivisions( Qgis::MapGridBorderSide::Left ), Qgis::MapGridBorderSide::Left );
  setFrameDivisions( other->frameDivisions( Qgis::MapGridBorderSide::Right ), Qgis::MapGridBorderSide::Right );
  setFrameDivisions( other->frameDivisions( Qgis::MapGridBorderSide::Bottom ), Qgis::MapGridBorderSide::Bottom );
  setFrameDivisions( other->frameDivisions( Qgis::MapGridBorderSide::Top ), Qgis::MapGridBorderSide::Top );

  setRotatedTicksLengthMode( other->rotatedTicksLengthMode() );
  setRotatedTicksEnabled( other->rotatedTicksEnabled() );
  setRotatedTicksMinimumAngle( other->rotatedTicksMinimumAngle() );
  setRotatedTicksMarginToCorner( other->rotatedTicksMarginToCorner() );
  setRotatedAnnotationsLengthMode( other->rotatedAnnotationsLengthMode() );
  setRotatedAnnotationsEnabled( other->rotatedAnnotationsEnabled() );
  setRotatedAnnotationsMinimumAngle( other->rotatedAnnotationsMinimumAngle() );
  setRotatedAnnotationsMarginToCorner( other->rotatedAnnotationsMarginToCorner() );

  if ( other->lineSymbol() )
  {
    setLineSymbol( other->lineSymbol()->clone() );
  }

  if ( other->markerSymbol() )
  {
    setMarkerSymbol( other->markerSymbol()->clone() );
  }

  setCrs( other->crs() );

  setBlendMode( other->blendMode() );

  //annotation
  setAnnotationEnabled( other->annotationEnabled() );
  setAnnotationFormat( other->annotationFormat() );
  setAnnotationExpression( other->annotationExpression() );

  setAnnotationPosition( other->annotationPosition( Qgis::MapGridBorderSide::Left ), Qgis::MapGridBorderSide::Left );
  setAnnotationPosition( other->annotationPosition( Qgis::MapGridBorderSide::Right ), Qgis::MapGridBorderSide::Right );
  setAnnotationPosition( other->annotationPosition( Qgis::MapGridBorderSide::Bottom ), Qgis::MapGridBorderSide::Bottom );
  setAnnotationPosition( other->annotationPosition( Qgis::MapGridBorderSide::Top ), Qgis::MapGridBorderSide::Top );
  setAnnotationDisplay( other->annotationDisplay( Qgis::MapGridBorderSide::Left ), Qgis::MapGridBorderSide::Left );
  setAnnotationDisplay( other->annotationDisplay( Qgis::MapGridBorderSide::Right ), Qgis::MapGridBorderSide::Right );
  setAnnotationDisplay( other->annotationDisplay( Qgis::MapGridBorderSide::Bottom ), Qgis::MapGridBorderSide::Bottom );
  setAnnotationDisplay( other->annotationDisplay( Qgis::MapGridBorderSide::Top ), Qgis::MapGridBorderSide::Top );
  setAnnotationDirection( other->annotationDirection( Qgis::MapGridBorderSide::Left ), Qgis::MapGridBorderSide::Left );
  setAnnotationDirection( other->annotationDirection( Qgis::MapGridBorderSide::Right ), Qgis::MapGridBorderSide::Right );
  setAnnotationDirection( other->annotationDirection( Qgis::MapGridBorderSide::Bottom ), Qgis::MapGridBorderSide::Bottom );
  setAnnotationDirection( other->annotationDirection( Qgis::MapGridBorderSide::Top ), Qgis::MapGridBorderSide::Top );
  setAnnotationFrameDistance( other->annotationFrameDistance() );
  setAnnotationTextFormat( other->annotationTextFormat() );

  setAnnotationPrecision( other->annotationPrecision() );
  setUnits( other->units() );
  setMinimumIntervalWidth( other->minimumIntervalWidth() );
  setMaximumIntervalWidth( other->maximumIntervalWidth() );

  setDataDefinedProperties( other->dataDefinedProperties() );
  refreshDataDefinedProperties();
}
