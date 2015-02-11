/***************************************************************************
  qgspallabeling.cpp
  Smart labeling for vector layers
  -------------------
   begin                : June 2009
   copyright            : (C) Martin Dobias
   email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspallabeling.h"
#include "qgspalgeometry.h"

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

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>

#include "diagram/qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsfontutils.h"
#include "qgslabelsearchtree.h"
#include "qgsexpression.h"
#include "qgsdatadefined.h"

#include <qgslogger.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaprenderer.h>
#include <qgsmarkersymbollayerv2.h>
#include <qgsproject.h>
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include <QMessageBox>


Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter* p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale(( double )qt_defaultDpiX() / p->device()->logicalDpiX(),
           ( double )qt_defaultDpiY() / p->device()->logicalDpiY() );
}


using namespace pal;

// -------------

QgsPalLayerSettings::QgsPalLayerSettings()
    : upsidedownLabels( Upright )
    , palLayer( NULL )
    , mCurFeat( 0 )
    , mCurFields( 0 )
    , xform( NULL )
    , ct( NULL )
    , extentGeom( NULL )
    , mFeaturesToLabel( 0 )
    , mFeatsSendingToPal( 0 )
    , mFeatsRegPal( 0 )
    , expression( 0 )
{
  enabled = false;
  isExpression = false;
  fieldIndex = 0;

  // text style
  textFont = QApplication::font();
  textNamedStyle = QString( "" );
  fontSizeInMapUnits = false;
  textColor = Qt::black;
  textTransp = 0;
  blendMode = QPainter::CompositionMode_SourceOver;
  previewBkgrdColor = Qt::white;
  // font processing info
  mTextFontFound = true;
  mTextFontFamily = QApplication::font().family();

  // text formatting
  wrapChar = "";
  multilineHeight = 1.0;
  multilineAlign = MultiLeft;
  addDirectionSymbol = false;
  leftDirectionSymbol = QString( "<" );
  rightDirectionSymbol = QString( ">" );
  reverseDirectionSymbol = false;
  placeDirectionSymbol = SymbolLeftRight;
  formatNumbers = false;
  decimals = 3;
  plusSign = false;

  // text buffer
  bufferDraw = false;
  bufferSize = 1.0;
  bufferSizeInMapUnits = false;
  bufferColor = Qt::white;
  bufferTransp = 0;
  bufferNoFill = false;
  bufferJoinStyle = Qt::BevelJoin;
  bufferBlendMode = QPainter::CompositionMode_SourceOver;

  // shape background
  shapeDraw = false;
  shapeType = ShapeRectangle;
  shapeSVGFile = QString();
  shapeSizeType = SizeBuffer;
  shapeSize = QPointF( 0.0, 0.0 );
  shapeSizeUnits = MM;
  shapeRotationType = RotationSync;
  shapeRotation = 0.0;
  shapeOffset = QPointF( 0.0, 0.0 );
  shapeOffsetUnits = MM;
  shapeRadii = QPointF( 0.0, 0.0 );
  shapeRadiiUnits = MM;
  shapeFillColor = Qt::white;
  shapeBorderColor = Qt::darkGray;
  shapeBorderWidth = 0.0;
  shapeBorderWidthUnits = MM;
  shapeJoinStyle = Qt::BevelJoin;
  shapeTransparency = 0;
  shapeBlendMode = QPainter::CompositionMode_SourceOver;

  // drop shadow
  shadowDraw = false;
  shadowUnder = ShadowLowest;
  shadowOffsetAngle = 135;
  shadowOffsetDist = 1.0;
  shadowOffsetUnits = MM;
  shadowOffsetGlobal = true;
  shadowRadius = 1.5;
  shadowRadiusUnits = MM;
  shadowRadiusAlphaOnly = false;
  shadowTransparency = 30;
  shadowScale = 100;
  shadowColor = Qt::black;
  shadowBlendMode = QPainter::CompositionMode_Multiply;

  // placement
  placement = AroundPoint;
  placementFlags = 0;
  centroidWhole = false;
  centroidInside = false;
  quadOffset = QuadrantOver;
  xOffset = 0;
  yOffset = 0;
  labelOffsetInMapUnits = true;
  dist = 0;
  distInMapUnits = false;
  angleOffset = 0;
  preserveRotation = true;
  maxCurvedCharAngleIn = 20.0;
  maxCurvedCharAngleOut = -20.0;
  priority = 5;
  repeatDistance = 0;
  repeatDistanceUnit = MM;

  // rendering
  scaleVisibility = false;
  scaleMin = 1;
  scaleMax = 10000000;
  fontLimitPixelSize = false;
  fontMinPixelSize = 0; //trigger to turn it on by default for map unit labels
  fontMaxPixelSize = 10000;
  displayAll = false;
  upsidedownLabels = Upright;

  labelPerPart = false;
  mergeLines = false;
  minFeatureSize = 0.0;
  limitNumLabels = false;
  maxNumLabels = 2000;
  obstacle = true;

  // scale factors
  vectorScaleFactor = 1.0;
  rasterCompressFactor = 1.0;

  // data defined string and old-style index values
  // NOTE: in QPair use -1 for second value (other values are for old-style layer properties migration)

  // text style
  mDataDefinedNames.insert( Size, QPair<QString, int>( "Size", 0 ) );
  mDataDefinedNames.insert( Bold, QPair<QString, int>( "Bold", 1 ) );
  mDataDefinedNames.insert( Italic, QPair<QString, int>( "Italic", 2 ) );
  mDataDefinedNames.insert( Underline, QPair<QString, int>( "Underline", 3 ) );
  mDataDefinedNames.insert( Color, QPair<QString, int>( "Color", 4 ) );
  mDataDefinedNames.insert( Strikeout, QPair<QString, int>( "Strikeout", 5 ) );
  mDataDefinedNames.insert( Family, QPair<QString, int>( "Family", 6 ) );
  mDataDefinedNames.insert( FontStyle, QPair<QString, int>( "FontStyle", -1 ) );
  mDataDefinedNames.insert( FontSizeUnit, QPair<QString, int>( "FontSizeUnit", -1 ) );
  mDataDefinedNames.insert( FontTransp, QPair<QString, int>( "FontTransp", 18 ) );
  mDataDefinedNames.insert( FontCase, QPair<QString, int>( "FontCase", -1 ) );
  mDataDefinedNames.insert( FontLetterSpacing, QPair<QString, int>( "FontLetterSpacing", -1 ) );
  mDataDefinedNames.insert( FontWordSpacing, QPair<QString, int>( "FontWordSpacing", -1 ) );
  mDataDefinedNames.insert( FontBlendMode, QPair<QString, int>( "FontBlendMode", -1 ) );

  // text formatting
  mDataDefinedNames.insert( MultiLineWrapChar, QPair<QString, int>( "MultiLineWrapChar", -1 ) );
  mDataDefinedNames.insert( MultiLineHeight, QPair<QString, int>( "MultiLineHeight", -1 ) );
  mDataDefinedNames.insert( MultiLineAlignment, QPair<QString, int>( "MultiLineAlignment", -1 ) );
  mDataDefinedNames.insert( DirSymbDraw, QPair<QString, int>( "DirSymbDraw", -1 ) );
  mDataDefinedNames.insert( DirSymbLeft, QPair<QString, int>( "DirSymbLeft", -1 ) );
  mDataDefinedNames.insert( DirSymbRight, QPair<QString, int>( "DirSymbRight", -1 ) );
  mDataDefinedNames.insert( DirSymbPlacement, QPair<QString, int>( "DirSymbPlacement", -1 ) );
  mDataDefinedNames.insert( DirSymbReverse, QPair<QString, int>( "DirSymbReverse", -1 ) );
  mDataDefinedNames.insert( NumFormat, QPair<QString, int>( "NumFormat", -1 ) );
  mDataDefinedNames.insert( NumDecimals, QPair<QString, int>( "NumDecimals", -1 ) );
  mDataDefinedNames.insert( NumPlusSign, QPair<QString, int>( "NumPlusSign", -1 ) );

  // text buffer
  mDataDefinedNames.insert( BufferDraw, QPair<QString, int>( "BufferDraw", -1 ) );
  mDataDefinedNames.insert( BufferSize, QPair<QString, int>( "BufferSize", 7 ) );
  mDataDefinedNames.insert( BufferUnit, QPair<QString, int>( "BufferUnit", -1 ) );
  mDataDefinedNames.insert( BufferColor, QPair<QString, int>( "BufferColor", 8 ) );
  mDataDefinedNames.insert( BufferTransp, QPair<QString, int>( "BufferTransp", 19 ) );
  mDataDefinedNames.insert( BufferJoinStyle, QPair<QString, int>( "BufferJoinStyle", -1 ) );
  mDataDefinedNames.insert( BufferBlendMode, QPair<QString, int>( "BufferBlendMode", -1 ) );

  // background
  mDataDefinedNames.insert( ShapeDraw, QPair<QString, int>( "ShapeDraw", -1 ) );
  mDataDefinedNames.insert( ShapeKind, QPair<QString, int>( "ShapeKind", -1 ) );
  mDataDefinedNames.insert( ShapeSVGFile, QPair<QString, int>( "ShapeSVGFile", -1 ) );
  mDataDefinedNames.insert( ShapeSizeType, QPair<QString, int>( "ShapeSizeType", -1 ) );
  mDataDefinedNames.insert( ShapeSizeX, QPair<QString, int>( "ShapeSizeX", -1 ) );
  mDataDefinedNames.insert( ShapeSizeY, QPair<QString, int>( "ShapeSizeY", -1 ) );
  mDataDefinedNames.insert( ShapeSizeUnits, QPair<QString, int>( "ShapeSizeUnits", -1 ) );
  mDataDefinedNames.insert( ShapeRotationType, QPair<QString, int>( "ShapeRotationType", -1 ) );
  mDataDefinedNames.insert( ShapeRotation, QPair<QString, int>( "ShapeRotation", -1 ) );
  mDataDefinedNames.insert( ShapeOffset, QPair<QString, int>( "ShapeOffset", -1 ) );
  mDataDefinedNames.insert( ShapeOffsetUnits, QPair<QString, int>( "ShapeOffsetUnits", -1 ) );
  mDataDefinedNames.insert( ShapeRadii, QPair<QString, int>( "ShapeRadii", -1 ) );
  mDataDefinedNames.insert( ShapeRadiiUnits, QPair<QString, int>( "ShapeRadiiUnits", -1 ) );
  mDataDefinedNames.insert( ShapeTransparency, QPair<QString, int>( "ShapeTransparency", -1 ) );
  mDataDefinedNames.insert( ShapeBlendMode, QPair<QString, int>( "ShapeBlendMode", -1 ) );
  mDataDefinedNames.insert( ShapeFillColor, QPair<QString, int>( "ShapeFillColor", -1 ) );
  mDataDefinedNames.insert( ShapeBorderColor, QPair<QString, int>( "ShapeBorderColor", -1 ) );
  mDataDefinedNames.insert( ShapeBorderWidth, QPair<QString, int>( "ShapeBorderWidth", -1 ) );
  mDataDefinedNames.insert( ShapeBorderWidthUnits, QPair<QString, int>( "ShapeBorderWidthUnits", -1 ) );
  mDataDefinedNames.insert( ShapeJoinStyle, QPair<QString, int>( "ShapeJoinStyle", -1 ) );

  // drop shadow
  mDataDefinedNames.insert( ShadowDraw, QPair<QString, int>( "ShadowDraw", -1 ) );
  mDataDefinedNames.insert( ShadowUnder, QPair<QString, int>( "ShadowUnder", -1 ) );
  mDataDefinedNames.insert( ShadowOffsetAngle, QPair<QString, int>( "ShadowOffsetAngle", -1 ) );
  mDataDefinedNames.insert( ShadowOffsetDist, QPair<QString, int>( "ShadowOffsetDist", -1 ) );
  mDataDefinedNames.insert( ShadowOffsetUnits, QPair<QString, int>( "ShadowOffsetUnits", -1 ) );
  mDataDefinedNames.insert( ShadowRadius, QPair<QString, int>( "ShadowRadius", -1 ) );
  mDataDefinedNames.insert( ShadowRadiusUnits, QPair<QString, int>( "ShadowRadiusUnits", -1 ) );
  mDataDefinedNames.insert( ShadowTransparency, QPair<QString, int>( "ShadowTransparency", -1 ) );
  mDataDefinedNames.insert( ShadowScale, QPair<QString, int>( "ShadowScale", -1 ) );
  mDataDefinedNames.insert( ShadowColor, QPair<QString, int>( "ShadowColor", -1 ) );
  mDataDefinedNames.insert( ShadowBlendMode, QPair<QString, int>( "ShadowBlendMode", -1 ) );

  // placement
  mDataDefinedNames.insert( CentroidWhole, QPair<QString, int>( "CentroidWhole", -1 ) );
  mDataDefinedNames.insert( OffsetQuad, QPair<QString, int>( "OffsetQuad", -1 ) );
  mDataDefinedNames.insert( OffsetXY, QPair<QString, int>( "OffsetXY", -1 ) );
  mDataDefinedNames.insert( OffsetUnits, QPair<QString, int>( "OffsetUnits", -1 ) );
  mDataDefinedNames.insert( LabelDistance, QPair<QString, int>( "LabelDistance", 13 ) );
  mDataDefinedNames.insert( DistanceUnits, QPair<QString, int>( "DistanceUnits", -1 ) );
  mDataDefinedNames.insert( OffsetRotation, QPair<QString, int>( "OffsetRotation", -1 ) );
  mDataDefinedNames.insert( CurvedCharAngleInOut, QPair<QString, int>( "CurvedCharAngleInOut", -1 ) );
  mDataDefinedNames.insert( RepeatDistance, QPair<QString, int>( "RepeatDistance", -1 ) );
  mDataDefinedNames.insert( RepeatDistanceUnit, QPair<QString, int>( "RepeatDistanceUnit", -1 ) );
  // (data defined only)
  mDataDefinedNames.insert( PositionX, QPair<QString, int>( "PositionX", 9 ) );
  mDataDefinedNames.insert( PositionY, QPair<QString, int>( "PositionY", 10 ) );
  mDataDefinedNames.insert( Hali, QPair<QString, int>( "Hali", 11 ) );
  mDataDefinedNames.insert( Vali, QPair<QString, int>( "Vali", 12 ) );
  mDataDefinedNames.insert( Rotation, QPair<QString, int>( "Rotation", 14 ) );

  //rendering
  mDataDefinedNames.insert( ScaleVisibility, QPair<QString, int>( "ScaleVisibility", -1 ) );
  mDataDefinedNames.insert( MinScale, QPair<QString, int>( "MinScale", 16 ) );
  mDataDefinedNames.insert( MaxScale, QPair<QString, int>( "MaxScale", 17 ) );
  mDataDefinedNames.insert( FontLimitPixel, QPair<QString, int>( "FontLimitPixel", -1 ) );
  mDataDefinedNames.insert( FontMinPixel, QPair<QString, int>( "FontMinPixel", -1 ) );
  mDataDefinedNames.insert( FontMaxPixel, QPair<QString, int>( "FontMaxPixel", -1 ) );
  // (data defined only)
  mDataDefinedNames.insert( Show, QPair<QString, int>( "Show", 15 ) );
  mDataDefinedNames.insert( AlwaysShow, QPair<QString, int>( "AlwaysShow", 20 ) );

  // temp stuff for when drawing label components (don't copy)
  showingShadowRects = false;
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings& s )
    : palLayer( NULL )
    , mCurFeat( NULL )
    , mCurFields( NULL )
    , fieldIndex( 0 )
    , xform( NULL )
    , ct( NULL )
    , extentGeom( NULL )
    , mFeaturesToLabel( 0 )
    , mFeatsSendingToPal( 0 )
    , mFeatsRegPal( 0 )
    , showingShadowRects( false )
    , expression( NULL )
{
  // copy only permanent stuff

  enabled = s.enabled;

  // text style
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  textFont = s.textFont;
  textNamedStyle = s.textNamedStyle;
  fontSizeInMapUnits = s.fontSizeInMapUnits;
  fontSizeMapUnitScale = s.fontSizeMapUnitScale;
  textColor = s.textColor;
  textTransp = s.textTransp;
  blendMode = s.blendMode;
  previewBkgrdColor = s.previewBkgrdColor;
  // font processing info
  mTextFontFound = s.mTextFontFound;
  mTextFontFamily = s.mTextFontFamily;

  // text formatting
  wrapChar = s.wrapChar;
  multilineHeight = s.multilineHeight;
  multilineAlign = s.multilineAlign;
  addDirectionSymbol = s.addDirectionSymbol;
  leftDirectionSymbol = s.leftDirectionSymbol;
  rightDirectionSymbol = s.rightDirectionSymbol;
  reverseDirectionSymbol = s.reverseDirectionSymbol;
  placeDirectionSymbol = s.placeDirectionSymbol;
  formatNumbers = s.formatNumbers;
  decimals = s.decimals;
  plusSign = s.plusSign;

  // text buffer
  bufferDraw = s.bufferDraw;
  bufferSize = s.bufferSize;
  bufferSizeInMapUnits = s.bufferSizeInMapUnits;
  bufferSizeMapUnitScale = s.bufferSizeMapUnitScale;
  bufferColor = s.bufferColor;
  bufferTransp = s.bufferTransp;
  bufferNoFill = s.bufferNoFill;
  bufferJoinStyle = s.bufferJoinStyle;
  bufferBlendMode = s.bufferBlendMode;

  // placement
  placement = s.placement;
  placementFlags = s.placementFlags;
  centroidWhole = s.centroidWhole;
  centroidInside = s.centroidInside;
  quadOffset = s.quadOffset;
  xOffset = s.xOffset;
  yOffset = s.yOffset;
  labelOffsetInMapUnits = s.labelOffsetInMapUnits;
  labelOffsetMapUnitScale = s.labelOffsetMapUnitScale;
  dist = s.dist;
  distInMapUnits = s.distInMapUnits;
  distMapUnitScale = s.distMapUnitScale;
  angleOffset = s.angleOffset;
  preserveRotation = s.preserveRotation;
  maxCurvedCharAngleIn = s.maxCurvedCharAngleIn;
  maxCurvedCharAngleOut = s.maxCurvedCharAngleOut;
  priority = s.priority;
  repeatDistance = s.repeatDistance;
  repeatDistanceUnit = s.repeatDistanceUnit;
  repeatDistanceMapUnitScale = s.repeatDistanceMapUnitScale;

  // rendering
  scaleVisibility = s.scaleVisibility;
  scaleMin = s.scaleMin;
  scaleMax = s.scaleMax;
  fontLimitPixelSize = s.fontLimitPixelSize;
  fontMinPixelSize = s.fontMinPixelSize;
  fontMaxPixelSize = s.fontMaxPixelSize;
  displayAll = s.displayAll;
  upsidedownLabels = s.upsidedownLabels;

  labelPerPart = s.labelPerPart;
  mergeLines = s.mergeLines;
  minFeatureSize = s.minFeatureSize;
  limitNumLabels = s.limitNumLabels;
  maxNumLabels = s.maxNumLabels;
  obstacle = s.obstacle;

  // shape background
  shapeDraw = s.shapeDraw;
  shapeType = s.shapeType;
  shapeSVGFile = s.shapeSVGFile;
  shapeSizeType = s.shapeSizeType;
  shapeSize = s.shapeSize;
  shapeSizeUnits = s.shapeSizeUnits;
  shapeSizeMapUnitScale = s.shapeSizeMapUnitScale;
  shapeRotationType = s.shapeRotationType;
  shapeRotation = s.shapeRotation;
  shapeOffset = s.shapeOffset;
  shapeOffsetUnits = s.shapeOffsetUnits;
  shapeOffsetMapUnitScale = s.shapeOffsetMapUnitScale;
  shapeRadii = s.shapeRadii;
  shapeRadiiUnits = s.shapeRadiiUnits;
  shapeRadiiMapUnitScale = s.shapeRadiiMapUnitScale;
  shapeFillColor = s.shapeFillColor;
  shapeBorderColor = s.shapeBorderColor;
  shapeBorderWidth = s.shapeBorderWidth;
  shapeBorderWidthUnits = s.shapeBorderWidthUnits;
  shapeBorderWidthMapUnitScale = s.shapeBorderWidthMapUnitScale;
  shapeJoinStyle = s.shapeJoinStyle;
  shapeTransparency = s.shapeTransparency;
  shapeBlendMode = s.shapeBlendMode;

  // drop shadow
  shadowDraw = s.shadowDraw;
  shadowUnder = s.shadowUnder;
  shadowOffsetAngle = s.shadowOffsetAngle;
  shadowOffsetDist = s.shadowOffsetDist;
  shadowOffsetUnits = s.shadowOffsetUnits;
  shadowOffsetMapUnitScale = s.shadowOffsetMapUnitScale;
  shadowOffsetGlobal = s.shadowOffsetGlobal;
  shadowRadius = s.shadowRadius;
  shadowRadiusUnits = s.shadowRadiusUnits;
  shadowRadiusMapUnitScale = s.shadowRadiusMapUnitScale;
  shadowRadiusAlphaOnly = s.shadowRadiusAlphaOnly;
  shadowTransparency = s.shadowTransparency;
  shadowScale = s.shadowScale;
  shadowColor = s.shadowColor;
  shadowBlendMode = s.shadowBlendMode;

  // data defined
  dataDefinedProperties = s.dataDefinedProperties;
  mDataDefinedNames = s.mDataDefinedNames;

  // scale factors
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
{
  // pal layer is deleted internally in PAL

  delete ct;
  delete expression;
  delete extentGeom;

  // clear pointers to QgsDataDefined objects
  dataDefinedProperties.clear();
}


QgsPalLayerSettings QgsPalLayerSettings::fromLayer( QgsVectorLayer* layer )
{
  QgsPalLayerSettings settings;
  settings.readFromLayer( layer );
  return settings;
}


QgsExpression* QgsPalLayerSettings::getLabelExpression()
{
  if ( expression == NULL )
  {
    expression = new QgsExpression( fieldName );
  }
  return expression;
}

static QColor _readColor( QgsVectorLayer* layer, QString property, QColor defaultColor = Qt::black, bool withAlpha = true )
{
  int r = layer->customProperty( property + "R", QVariant( defaultColor.red() ) ).toInt();
  int g = layer->customProperty( property + "G", QVariant( defaultColor.green() ) ).toInt();
  int b = layer->customProperty( property + "B", QVariant( defaultColor.blue() ) ).toInt();
  int a = withAlpha ? layer->customProperty( property + "A", QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

static void _writeColor( QgsVectorLayer* layer, QString property, QColor color, bool withAlpha = true )
{
  layer->setCustomProperty( property + "R", color.red() );
  layer->setCustomProperty( property + "G", color.green() );
  layer->setCustomProperty( property + "B", color.blue() );
  if ( withAlpha )
    layer->setCustomProperty( property + "A", color.alpha() );
}

static QgsPalLayerSettings::SizeUnit _decodeUnits( const QString& str )
{
  if ( str.compare( "Point", Qt::CaseInsensitive ) == 0
       || str.compare( "Points", Qt::CaseInsensitive ) == 0 ) return QgsPalLayerSettings::Points;
  if ( str.compare( "MapUnit", Qt::CaseInsensitive ) == 0
       || str.compare( "MapUnits", Qt::CaseInsensitive ) == 0 ) return QgsPalLayerSettings::MapUnits;
  if ( str.compare( "Percent", Qt::CaseInsensitive ) == 0 ) return QgsPalLayerSettings::Percent;
  return QgsPalLayerSettings::MM; // "MM"
}

static Qt::PenJoinStyle _decodePenJoinStyle( const QString& str )
{
  if ( str.compare( "Miter", Qt::CaseInsensitive ) == 0 ) return Qt::MiterJoin;
  if ( str.compare( "Round", Qt::CaseInsensitive ) == 0 ) return Qt::RoundJoin;
  return Qt::BevelJoin; // "Bevel"
}

void QgsPalLayerSettings::readDataDefinedPropertyMap( QgsVectorLayer* layer,
    QMap < QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > & propertyMap )
{
  if ( !layer )
  {
    return;
  }

  QMapIterator<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > i( mDataDefinedNames );
  while ( i.hasNext() )
  {
    i.next();
    readDataDefinedProperty( layer, i.key(), propertyMap );
  }
}

void QgsPalLayerSettings::writeDataDefinedPropertyMap( QgsVectorLayer* layer,
    const QMap < QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > & propertyMap )
{
  if ( !layer )
  {
    return;
  }

  QMapIterator<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > i( mDataDefinedNames );
  while ( i.hasNext() )
  {
    i.next();
    QString newPropertyName = "labeling/dataDefined/" + i.value().first;
    QVariant propertyValue = QVariant();

    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = propertyMap.find( i.key() );
    if ( it != propertyMap.constEnd() )
    {
      QgsDataDefined* dd = it.value();
      if ( dd )
      {
        bool active = dd->isActive();
        bool useExpr = dd->useExpression();
        QString expr = dd->expressionString();
        QString field = dd->field();

        bool defaultVals = ( !active && !useExpr && expr.isEmpty() && field.isEmpty() );

        if ( !defaultVals )
        {
          // TODO: update this when project settings for labeling are migrated to better XML layout
          QStringList values;
          values << ( active ? "1" : "0" );
          values << ( useExpr ? "1" : "0" );
          values << expr;
          values << field;
          if ( !values.isEmpty() )
          {
            propertyValue = QVariant( values.join( "~~" ) );
          }
        }
      }
    }

    if ( propertyValue.isValid() )
    {
      layer->setCustomProperty( newPropertyName, propertyValue );
    }
    else
    {
      // remove unused properties
      layer->removeCustomProperty( newPropertyName );
    }

    if ( layer->customProperty( newPropertyName, QVariant() ).isValid() && i.value().second > -1 )
    {
      // remove old-style field index-based property, if still present
      layer->removeCustomProperty( QString( "labeling/dataDefinedProperty" ) + QString::number( i.value().second ) );
    }
  }
}

void QgsPalLayerSettings::readDataDefinedProperty( QgsVectorLayer* layer,
    QgsPalLayerSettings::DataDefinedProperties p,
    QMap < QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > & propertyMap )
{
  QString newPropertyName = "labeling/dataDefined/" + mDataDefinedNames.value( p ).first;
  QVariant newPropertyField = layer->customProperty( newPropertyName, QVariant() );

  QString ddString = QString();
  if ( newPropertyField.isValid() )
  {
    ddString = newPropertyField.toString();
  }
  else // maybe working with old-style field index-based property (< QGIS 2.0)
  {
    int oldIndx = mDataDefinedNames.value( p ).second;

    if ( oldIndx < 0 ) // something went wrong and we are working with new-style
    {
      return;
    }

    QString oldPropertyName = "labeling/dataDefinedProperty" + QString::number( oldIndx );
    QVariant oldPropertyField = layer->customProperty( oldPropertyName, QVariant() );

    if ( !oldPropertyField.isValid() )
    {
      return;
    }

    // switch from old-style field index- to name-based properties
    bool conversionOk;
    int indx = oldPropertyField.toInt( &conversionOk );

    if ( conversionOk )
    {
      // Fix to migrate from old-style vector api, where returned QMap keys possibly
      //   had 'holes' in sequence of field indices, e.g. 0,2,3
      // QgsAttrPalIndexNameHash provides a means of access field name in sequences from
      //   providers that procuded holes (e.g. PostGIS skipped geom column), otherwise it is empty
      QgsAttrPalIndexNameHash oldIndicesToNames = layer->dataProvider()->palAttributeIndexNames();

      if ( !oldIndicesToNames.isEmpty() )
      {
        ddString = oldIndicesToNames.value( indx );
      }
      else
      {
        QgsFields fields = layer->dataProvider()->fields();
        if ( indx < fields.size() ) // in case field count has changed
        {
          ddString = fields.at( indx ).name();
        }
      }
    }

    if ( !ddString.isEmpty() )
    {
      //upgrade any existing property to field name-based
      layer->setCustomProperty( newPropertyName, QVariant( updateDataDefinedString( ddString ) ) );

      // fix for buffer drawing triggered off of just its data defined size in the past (<2.0)
      if ( oldIndx == 7 ) // old bufferSize enum
      {
        bufferDraw = true;
        layer->setCustomProperty( "labeling/bufferDraw", true );
      }

      // fix for scale visibility limits triggered off of just its data defined values in the past (<2.0)
      if ( oldIndx == 16 || oldIndx == 17 ) // old minScale and maxScale enums
      {
        scaleVisibility = true;
        layer->setCustomProperty( "labeling/scaleVisibility", true );
      }
    }

    // remove old-style field index-based property
    layer->removeCustomProperty( oldPropertyName );
  }

  if ( !ddString.isEmpty() && ddString != QString( "0~~0~~~~" ) )
  {
    // TODO: update this when project settings for labeling are migrated to better XML layout
    QString newStyleString = updateDataDefinedString( ddString );
    QStringList ddv = newStyleString.split( "~~" );

    QgsDataDefined* dd = new QgsDataDefined( ddv.at( 0 ).toInt(), ddv.at( 1 ).toInt(), ddv.at( 2 ), ddv.at( 3 ) );
    propertyMap.insert( p, dd );
  }
  else
  {
    // remove unused properties
    layer->removeCustomProperty( newPropertyName );
  }
}

void QgsPalLayerSettings::readFromLayer( QgsVectorLayer* layer )
{
  if ( layer->customProperty( "labeling" ).toString() != QString( "pal" ) )
  {
    // for polygons the "over point" (over centroid) placement is better than the default
    // "around point" (around centroid) which is more suitable for points
    if ( layer->geometryType() == QGis::Polygon )
      placement = OverPoint;

    return; // there's no information available
  }

  // NOTE: set defaults for newly added properties, for backwards compatibility

  enabled = layer->customProperty( "labeling/enabled" ).toBool();

  // text style
  fieldName = layer->customProperty( "labeling/fieldName" ).toString();
  isExpression = layer->customProperty( "labeling/isExpression" ).toBool();
  QFont appFont = QApplication::font();
  mTextFontFamily = layer->customProperty( "labeling/fontFamily", QVariant( appFont.family() ) ).toString();
  QString fontFamily = mTextFontFamily;
  if ( mTextFontFamily != appFont.family() && !QgsFontUtils::fontFamilyMatchOnSystem( mTextFontFamily ) )
  {
    // trigger to notify user about font family substitution (signal emitted in QgsVectorLayer::prepareLabelingAndDiagrams)
    mTextFontFound = false;

    // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
    // currently only defaults to matching algorithm for resolving [foundry], if a font of similar family is found (default for QFont)

    // for now, do not use matching algorithm for substitution if family not found, substitute default instead
    fontFamily = appFont.family();
  }

  double fontSize = layer->customProperty( "labeling/fontSize" ).toDouble();
  fontSizeInMapUnits = layer->customProperty( "labeling/fontSizeInMapUnits" ).toBool();
  fontSizeMapUnitScale.minScale = layer->customProperty( "labeling/fontSizeMapUnitMinScale", 0.0 ).toDouble();
  fontSizeMapUnitScale.maxScale = layer->customProperty( "labeling/fontSizeMapUnitMaxScale", 0.0 ).toDouble();
  int fontWeight = layer->customProperty( "labeling/fontWeight" ).toInt();
  bool fontItalic = layer->customProperty( "labeling/fontItalic" ).toBool();
  textFont = QFont( fontFamily, fontSize, fontWeight, fontItalic );
  textFont.setPointSizeF( fontSize ); //double precision needed because of map units
  textNamedStyle = layer->customProperty( "labeling/namedStyle", QVariant( "" ) ).toString();
  QgsFontUtils::updateFontViaStyle( textFont, textNamedStyle ); // must come after textFont.setPointSizeF()
  textFont.setCapitalization(( QFont::Capitalization )layer->customProperty( "labeling/fontCapitals", QVariant( 0 ) ).toUInt() );
  textFont.setUnderline( layer->customProperty( "labeling/fontUnderline" ).toBool() );
  textFont.setStrikeOut( layer->customProperty( "labeling/fontStrikeout" ).toBool() );
  textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( "labeling/fontLetterSpacing", QVariant( 0.0 ) ).toDouble() );
  textFont.setWordSpacing( layer->customProperty( "labeling/fontWordSpacing", QVariant( 0.0 ) ).toDouble() );
  textColor = _readColor( layer, "labeling/textColor", Qt::black, false );
  textTransp = layer->customProperty( "labeling/textTransp" ).toInt();
  blendMode = QgsMapRenderer::getCompositionMode(
                ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/blendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );
  previewBkgrdColor = QColor( layer->customProperty( "labeling/previewBkgrdColor", QVariant( "#ffffff" ) ).toString() );


  // text formatting
  wrapChar = layer->customProperty( "labeling/wrapChar" ).toString();
  multilineHeight = layer->customProperty( "labeling/multilineHeight", QVariant( 1.0 ) ).toDouble();
  multilineAlign = ( MultiLineAlign )layer->customProperty( "labeling/multilineAlign", QVariant( MultiLeft ) ).toUInt();
  addDirectionSymbol = layer->customProperty( "labeling/addDirectionSymbol" ).toBool();
  leftDirectionSymbol = layer->customProperty( "labeling/leftDirectionSymbol", QVariant( "<" ) ).toString();
  rightDirectionSymbol = layer->customProperty( "labeling/rightDirectionSymbol", QVariant( ">" ) ).toString();
  reverseDirectionSymbol = layer->customProperty( "labeling/reverseDirectionSymbol" ).toBool();
  placeDirectionSymbol = ( DirectionSymbols )layer->customProperty( "labeling/placeDirectionSymbol", QVariant( SymbolLeftRight ) ).toUInt();
  formatNumbers = layer->customProperty( "labeling/formatNumbers" ).toBool();
  decimals = layer->customProperty( "labeling/decimals" ).toInt();
  plusSign = layer->customProperty( "labeling/plussign" ).toInt();

  // text buffer
  double bufSize = layer->customProperty( "labeling/bufferSize", QVariant( 0.0 ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  QVariant drawBuffer = layer->customProperty( "labeling/bufferDraw", QVariant() );
  if ( drawBuffer.isValid() )
  {
    bufferDraw = drawBuffer.toBool();
    bufferSize = bufSize;
  }
  else if ( bufSize != 0.0 )
  {
    bufferDraw = true;
    bufferSize = bufSize;
  }
  else
  {
    // keep bufferSize at new 1.0 default
    bufferDraw = false;
  }

  bufferSizeInMapUnits = layer->customProperty( "labeling/bufferSizeInMapUnits" ).toBool();
  bufferSizeMapUnitScale.minScale = layer->customProperty( "labeling/bufferSizeMapUnitMinScale", 0.0 ).toDouble();
  bufferSizeMapUnitScale.maxScale = layer->customProperty( "labeling/bufferSizeMapUnitMaxScale", 0.0 ).toDouble();
  bufferColor = _readColor( layer, "labeling/bufferColor", Qt::white, false );
  bufferTransp = layer->customProperty( "labeling/bufferTransp" ).toInt();
  bufferBlendMode = QgsMapRenderer::getCompositionMode(
                      ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/bufferBlendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );
  bufferJoinStyle = ( Qt::PenJoinStyle )layer->customProperty( "labeling/bufferJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt();
  bufferNoFill = layer->customProperty( "labeling/bufferNoFill", QVariant( false ) ).toBool();

  // background
  shapeDraw = layer->customProperty( "labeling/shapeDraw", QVariant( false ) ).toBool();
  shapeType = ( ShapeType )layer->customProperty( "labeling/shapeType", QVariant( ShapeRectangle ) ).toUInt();
  shapeSVGFile = layer->customProperty( "labeling/shapeSVGFile", QVariant( "" ) ).toString();
  shapeSizeType = ( SizeType )layer->customProperty( "labeling/shapeSizeType", QVariant( SizeBuffer ) ).toUInt();
  shapeSize = QPointF( layer->customProperty( "labeling/shapeSizeX", QVariant( 0.0 ) ).toDouble(),
                       layer->customProperty( "labeling/shapeSizeY", QVariant( 0.0 ) ).toDouble() );
  shapeSizeUnits = ( SizeUnit )layer->customProperty( "labeling/shapeSizeUnits", QVariant( MM ) ).toUInt();
  shapeSizeMapUnitScale.minScale = layer->customProperty( "labeling/shapeSizeMapUnitMinScale", 0.0 ).toDouble();
  shapeSizeMapUnitScale.maxScale = layer->customProperty( "labeling/shapeSizeMapUnitMaxScale", 0.0 ).toDouble();
  shapeRotationType = ( RotationType )layer->customProperty( "labeling/shapeRotationType", QVariant( RotationSync ) ).toUInt();
  shapeRotation = layer->customProperty( "labeling/shapeRotation", QVariant( 0.0 ) ).toDouble();
  shapeOffset = QPointF( layer->customProperty( "labeling/shapeOffsetX", QVariant( 0.0 ) ).toDouble(),
                         layer->customProperty( "labeling/shapeOffsetY", QVariant( 0.0 ) ).toDouble() );
  shapeOffsetUnits = ( SizeUnit )layer->customProperty( "labeling/shapeOffsetUnits", QVariant( MM ) ).toUInt();
  shapeOffsetMapUnitScale.minScale = layer->customProperty( "labeling/shapeOffsetMapUnitMinScale", 0.0 ).toDouble();
  shapeOffsetMapUnitScale.maxScale = layer->customProperty( "labeling/shapeOffsetMapUnitMaxScale", 0.0 ).toDouble();
  shapeRadii = QPointF( layer->customProperty( "labeling/shapeRadiiX", QVariant( 0.0 ) ).toDouble(),
                        layer->customProperty( "labeling/shapeRadiiY", QVariant( 0.0 ) ).toDouble() );
  shapeRadiiUnits = ( SizeUnit )layer->customProperty( "labeling/shapeRadiiUnits", QVariant( MM ) ).toUInt();
  shapeRadiiMapUnitScale.minScale = layer->customProperty( "labeling/shapeRaddiMapUnitMinScale", 0.0 ).toDouble();
  shapeRadiiMapUnitScale.maxScale = layer->customProperty( "labeling/shapeRaddiMapUnitMaxScale", 0.0 ).toDouble();
  shapeFillColor = _readColor( layer, "labeling/shapeFillColor", Qt::white, true );
  shapeBorderColor = _readColor( layer, "labeling/shapeBorderColor", Qt::darkGray, true );
  shapeBorderWidth = layer->customProperty( "labeling/shapeBorderWidth", QVariant( .0 ) ).toDouble();
  shapeBorderWidthUnits = ( SizeUnit )layer->customProperty( "labeling/shapeBorderWidthUnits", QVariant( MM ) ).toUInt();
  shapeBorderWidthMapUnitScale.minScale = layer->customProperty( "labeling/shapeBorderWidthMapUnitMinScale", 0.0 ).toDouble();
  shapeBorderWidthMapUnitScale.maxScale = layer->customProperty( "labeling/shapeBorderWidthMapUnitMaxScale", 0.0 ).toDouble();
  shapeJoinStyle = ( Qt::PenJoinStyle )layer->customProperty( "labeling/shapeJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt();
  shapeTransparency = layer->customProperty( "labeling/shapeTransparency", QVariant( 0 ) ).toInt();
  shapeBlendMode = QgsMapRenderer::getCompositionMode(
                     ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/shapeBlendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );

  // drop shadow
  shadowDraw = layer->customProperty( "labeling/shadowDraw", QVariant( false ) ).toBool();
  shadowUnder = ( ShadowType )layer->customProperty( "labeling/shadowUnder", QVariant( ShadowLowest ) ).toUInt();//ShadowLowest;
  shadowOffsetAngle = layer->customProperty( "labeling/shadowOffsetAngle", QVariant( 135 ) ).toInt();
  shadowOffsetDist = layer->customProperty( "labeling/shadowOffsetDist", QVariant( 1.0 ) ).toDouble();
  shadowOffsetUnits = ( SizeUnit )layer->customProperty( "labeling/shadowOffsetUnits", QVariant( MM ) ).toUInt();
  shadowOffsetMapUnitScale.minScale = layer->customProperty( "labeling/shadowOffsetMapUnitMinScale", 0.0 ).toDouble();
  shadowOffsetMapUnitScale.maxScale = layer->customProperty( "labeling/shadowOffsetMapUnitMaxScale", 0.0 ).toDouble();
  shadowOffsetGlobal = layer->customProperty( "labeling/shadowOffsetGlobal", QVariant( true ) ).toBool();
  shadowRadius = layer->customProperty( "labeling/shadowRadius", QVariant( 1.5 ) ).toDouble();
  shadowRadiusUnits = ( SizeUnit )layer->customProperty( "labeling/shadowRadiusUnits", QVariant( MM ) ).toUInt();
  shadowRadiusMapUnitScale.minScale = layer->customProperty( "labeling/shadowRadiusMapUnitMinScale", 0.0 ).toDouble();
  shadowRadiusMapUnitScale.maxScale = layer->customProperty( "labeling/shadowRadiusMapUnitMaxScale", 0.0 ).toDouble();
  shadowRadiusAlphaOnly = layer->customProperty( "labeling/shadowRadiusAlphaOnly", QVariant( false ) ).toBool();
  shadowTransparency = layer->customProperty( "labeling/shadowTransparency", QVariant( 30 ) ).toInt();
  shadowScale = layer->customProperty( "labeling/shadowScale", QVariant( 100 ) ).toInt();
  shadowColor = _readColor( layer, "labeling/shadowColor", Qt::black, false );
  shadowBlendMode = QgsMapRenderer::getCompositionMode(
                      ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/shadowBlendMode", QVariant( QgsMapRenderer::BlendMultiply ) ).toUInt() );

  // placement
  placement = ( Placement )layer->customProperty( "labeling/placement" ).toInt();
  placementFlags = layer->customProperty( "labeling/placementFlags" ).toUInt();
  centroidWhole = layer->customProperty( "labeling/centroidWhole", QVariant( false ) ).toBool();
  centroidInside = layer->customProperty( "labeling/centroidInside", QVariant( false ) ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  distInMapUnits = layer->customProperty( "labeling/distInMapUnits" ).toBool();
  distMapUnitScale.minScale = layer->customProperty( "labeling/distMapUnitMinScale", 0.0 ).toDouble();
  distMapUnitScale.maxScale = layer->customProperty( "labeling/distMapUnitMaxScale", 0.0 ).toDouble();
  quadOffset = ( QuadrantPosition )layer->customProperty( "labeling/quadOffset", QVariant( QuadrantOver ) ).toUInt();
  xOffset = layer->customProperty( "labeling/xOffset", QVariant( 0.0 ) ).toDouble();
  yOffset = layer->customProperty( "labeling/yOffset", QVariant( 0.0 ) ).toDouble();
  labelOffsetInMapUnits = layer->customProperty( "labeling/labelOffsetInMapUnits", QVariant( true ) ).toBool();
  labelOffsetMapUnitScale.minScale = layer->customProperty( "labeling/labelOffsetMapUnitMinScale", 0.0 ).toDouble();
  labelOffsetMapUnitScale.maxScale = layer->customProperty( "labeling/labelOffsetMapUnitMaxScale", 0.0 ).toDouble();
  angleOffset = layer->customProperty( "labeling/angleOffset", QVariant( 0.0 ) ).toDouble();
  preserveRotation = layer->customProperty( "labeling/preserveRotation", QVariant( true ) ).toBool();
  maxCurvedCharAngleIn = layer->customProperty( "labeling/maxCurvedCharAngleIn", QVariant( 20.0 ) ).toDouble();
  maxCurvedCharAngleOut = layer->customProperty( "labeling/maxCurvedCharAngleOut", QVariant( -20.0 ) ).toDouble();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  repeatDistance = layer->customProperty( "labeling/repeatDistance", 0.0 ).toDouble();
  repeatDistanceUnit = ( SizeUnit ) layer->customProperty( "labeling/repeatDistanceUnit", QVariant( MM ) ).toUInt();
  repeatDistanceMapUnitScale.minScale = layer->customProperty( "labeling/repeatDistanceMapUnitMinScale", 0.0 ).toDouble();
  repeatDistanceMapUnitScale.maxScale = layer->customProperty( "labeling/repeatDistanceMapUnitMaxScale", 0.0 ).toDouble();

  // rendering
  int scalemn = layer->customProperty( "labeling/scaleMin", QVariant( 0 ) ).toInt();
  int scalemx = layer->customProperty( "labeling/scaleMax", QVariant( 0 ) ).toInt();

  // fix for scale visibility limits being keyed off of just its values in the past (<2.0)
  QVariant scalevis = layer->customProperty( "labeling/scaleVisibility", QVariant() );
  if ( scalevis.isValid() )
  {
    scaleVisibility = scalevis.toBool();
    scaleMin = scalemn;
    scaleMax = scalemx;
  }
  else if ( scalemn > 0 || scalemx > 0 )
  {
    scaleVisibility = true;
    scaleMin = scalemn;
    scaleMax = scalemx;
  }
  else
  {
    // keep scaleMin and scaleMax at new 1.0 defaults (1 and 10000000, were 0 and 0)
    scaleVisibility = false;
  }


  fontLimitPixelSize = layer->customProperty( "labeling/fontLimitPixelSize", QVariant( false ) ).toBool();
  fontMinPixelSize = layer->customProperty( "labeling/fontMinPixelSize", QVariant( 0 ) ).toInt();
  fontMaxPixelSize = layer->customProperty( "labeling/fontMaxPixelSize", QVariant( 10000 ) ).toInt();
  displayAll = layer->customProperty( "labeling/displayAll", QVariant( false ) ).toBool();
  upsidedownLabels = ( UpsideDownLabels )layer->customProperty( "labeling/upsidedownLabels", QVariant( Upright ) ).toUInt();

  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
  minFeatureSize = layer->customProperty( "labeling/minFeatureSize" ).toDouble();
  limitNumLabels = layer->customProperty( "labeling/limitNumLabels", QVariant( false ) ).toBool();
  maxNumLabels = layer->customProperty( "labeling/maxNumLabels", QVariant( 2000 ) ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle", QVariant( true ) ).toBool();

  readDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::writeToLayer( QgsVectorLayer* layer )
{
  // this is a mark that labeling information is present
  layer->setCustomProperty( "labeling", "pal" );

  layer->setCustomProperty( "labeling/enabled", enabled );

  // text style
  layer->setCustomProperty( "labeling/fieldName", fieldName );
  layer->setCustomProperty( "labeling/isExpression", isExpression );
  layer->setCustomProperty( "labeling/fontFamily", textFont.family() );
  layer->setCustomProperty( "labeling/namedStyle", textNamedStyle );
  layer->setCustomProperty( "labeling/fontSize", textFont.pointSizeF() );
  layer->setCustomProperty( "labeling/fontSizeInMapUnits", fontSizeInMapUnits );
  layer->setCustomProperty( "labeling/fontSizeMapUnitMinScale", fontSizeMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/fontSizeMapUnitMaxScale", fontSizeMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/fontWeight", textFont.weight() );
  layer->setCustomProperty( "labeling/fontItalic", textFont.italic() );
  layer->setCustomProperty( "labeling/fontBold", textFont.bold() );
  layer->setCustomProperty( "labeling/fontStrikeout", textFont.strikeOut() );
  layer->setCustomProperty( "labeling/fontUnderline", textFont.underline() );
  _writeColor( layer, "labeling/textColor", textColor );
  layer->setCustomProperty( "labeling/fontCapitals", ( unsigned int )textFont.capitalization() );
  layer->setCustomProperty( "labeling/fontLetterSpacing", textFont.letterSpacing() );
  layer->setCustomProperty( "labeling/fontWordSpacing", textFont.wordSpacing() );
  layer->setCustomProperty( "labeling/textTransp", textTransp );
  layer->setCustomProperty( "labeling/blendMode", QgsMapRenderer::getBlendModeEnum( blendMode ) );
  layer->setCustomProperty( "labeling/previewBkgrdColor", previewBkgrdColor.name() );

  // text formatting
  layer->setCustomProperty( "labeling/wrapChar", wrapChar );
  layer->setCustomProperty( "labeling/multilineHeight", multilineHeight );
  layer->setCustomProperty( "labeling/multilineAlign", ( unsigned int )multilineAlign );
  layer->setCustomProperty( "labeling/addDirectionSymbol", addDirectionSymbol );
  layer->setCustomProperty( "labeling/leftDirectionSymbol", leftDirectionSymbol );
  layer->setCustomProperty( "labeling/rightDirectionSymbol", rightDirectionSymbol );
  layer->setCustomProperty( "labeling/reverseDirectionSymbol", reverseDirectionSymbol );
  layer->setCustomProperty( "labeling/placeDirectionSymbol", ( unsigned int )placeDirectionSymbol );
  layer->setCustomProperty( "labeling/formatNumbers", formatNumbers );
  layer->setCustomProperty( "labeling/decimals", decimals );
  layer->setCustomProperty( "labeling/plussign", plusSign );

  // text buffer
  layer->setCustomProperty( "labeling/bufferDraw", bufferDraw );
  layer->setCustomProperty( "labeling/bufferSize", bufferSize );
  layer->setCustomProperty( "labeling/bufferSizeInMapUnits", bufferSizeInMapUnits );
  layer->setCustomProperty( "labeling/bufferSizeMapUnitMinScale", bufferSizeMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/bufferSizeMapUnitMaxScale", bufferSizeMapUnitScale.maxScale );
  _writeColor( layer, "labeling/bufferColor", bufferColor );
  layer->setCustomProperty( "labeling/bufferNoFill", bufferNoFill );
  layer->setCustomProperty( "labeling/bufferTransp", bufferTransp );
  layer->setCustomProperty( "labeling/bufferJoinStyle", ( unsigned int )bufferJoinStyle );
  layer->setCustomProperty( "labeling/bufferBlendMode", QgsMapRenderer::getBlendModeEnum( bufferBlendMode ) );

  // background
  layer->setCustomProperty( "labeling/shapeDraw", shapeDraw );
  layer->setCustomProperty( "labeling/shapeType", ( unsigned int )shapeType );
  layer->setCustomProperty( "labeling/shapeSVGFile", shapeSVGFile );
  layer->setCustomProperty( "labeling/shapeSizeType", ( unsigned int )shapeSizeType );
  layer->setCustomProperty( "labeling/shapeSizeX", shapeSize.x() );
  layer->setCustomProperty( "labeling/shapeSizeY", shapeSize.y() );
  layer->setCustomProperty( "labeling/shapeSizeUnits", ( unsigned int )shapeSizeUnits );
  layer->setCustomProperty( "labeling/shapeSizeMapUnitMinScale", shapeSizeMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shapeSizeMapUnitMaxScale", shapeSizeMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/shapeRotationType", ( unsigned int )shapeRotationType );
  layer->setCustomProperty( "labeling/shapeRotation", shapeRotation );
  layer->setCustomProperty( "labeling/shapeOffsetX", shapeOffset.x() );
  layer->setCustomProperty( "labeling/shapeOffsetY", shapeOffset.y() );
  layer->setCustomProperty( "labeling/shapeOffsetUnits", ( unsigned int )shapeOffsetUnits );
  layer->setCustomProperty( "labeling/shapeOffsetMapUnitMinScale", shapeOffsetMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shapeOffsetMapUnitMaxScale", shapeOffsetMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/shapeRadiiX", shapeRadii.x() );
  layer->setCustomProperty( "labeling/shapeRadiiY", shapeRadii.y() );
  layer->setCustomProperty( "labeling/shapeRadiiUnits", ( unsigned int )shapeRadiiUnits );
  layer->setCustomProperty( "labeling/shapeRadiiMapUnitMinScale", shapeRadiiMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shapeRadiiMapUnitMaxScale", shapeRadiiMapUnitScale.maxScale );
  _writeColor( layer, "labeling/shapeFillColor", shapeFillColor, true );
  _writeColor( layer, "labeling/shapeBorderColor", shapeBorderColor, true );
  layer->setCustomProperty( "labeling/shapeBorderWidth", shapeBorderWidth );
  layer->setCustomProperty( "labeling/shapeBorderWidthUnits", ( unsigned int )shapeBorderWidthUnits );
  layer->setCustomProperty( "labeling/shapeBorderWidthMapUnitMinScale", shapeBorderWidthMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shapeBorderWidthMapUnitMaxScale", shapeBorderWidthMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/shapeJoinStyle", ( unsigned int )shapeJoinStyle );
  layer->setCustomProperty( "labeling/shapeTransparency", shapeTransparency );
  layer->setCustomProperty( "labeling/shapeBlendMode", QgsMapRenderer::getBlendModeEnum( shapeBlendMode ) );

  // drop shadow
  layer->setCustomProperty( "labeling/shadowDraw", shadowDraw );
  layer->setCustomProperty( "labeling/shadowUnder", ( unsigned int )shadowUnder );
  layer->setCustomProperty( "labeling/shadowOffsetAngle", shadowOffsetAngle );
  layer->setCustomProperty( "labeling/shadowOffsetDist", shadowOffsetDist );
  layer->setCustomProperty( "labeling/shadowOffsetUnits", ( unsigned int )shadowOffsetUnits );
  layer->setCustomProperty( "labeling/shadowOffsetMapUnitMinScale", shadowOffsetMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shadowOffsetMapUnitMaxScale", shadowOffsetMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/shadowOffsetGlobal", shadowOffsetGlobal );
  layer->setCustomProperty( "labeling/shadowRadius", shadowRadius );
  layer->setCustomProperty( "labeling/shadowRadiusUnits", ( unsigned int )shadowRadiusUnits );
  layer->setCustomProperty( "labeling/shadowRadiusMapUnitMinScale", shadowRadiusMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/shadowRadiusMapUnitMaxScale", shadowRadiusMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/shadowRadiusAlphaOnly", shadowRadiusAlphaOnly );
  layer->setCustomProperty( "labeling/shadowTransparency", shadowTransparency );
  layer->setCustomProperty( "labeling/shadowScale", shadowScale );
  _writeColor( layer, "labeling/shadowColor", shadowColor, false );
  layer->setCustomProperty( "labeling/shadowBlendMode", QgsMapRenderer::getBlendModeEnum( shadowBlendMode ) );

  // placement
  layer->setCustomProperty( "labeling/placement", placement );
  layer->setCustomProperty( "labeling/placementFlags", ( unsigned int )placementFlags );
  layer->setCustomProperty( "labeling/centroidWhole", centroidWhole );
  layer->setCustomProperty( "labeling/centroidInside", centroidInside );
  layer->setCustomProperty( "labeling/dist", dist );
  layer->setCustomProperty( "labeling/distInMapUnits", distInMapUnits );
  layer->setCustomProperty( "labeling/distMapUnitMinScale", distMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/distMapUnitMaxScale", distMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/quadOffset", ( unsigned int )quadOffset );
  layer->setCustomProperty( "labeling/xOffset", xOffset );
  layer->setCustomProperty( "labeling/yOffset", yOffset );
  layer->setCustomProperty( "labeling/labelOffsetInMapUnits", labelOffsetInMapUnits );
  layer->setCustomProperty( "labeling/labelOffsetMapUnitMinScale", labelOffsetMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/labelOffsetMapUnitMaxScale", labelOffsetMapUnitScale.maxScale );
  layer->setCustomProperty( "labeling/angleOffset", angleOffset );
  layer->setCustomProperty( "labeling/preserveRotation", preserveRotation );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleIn", maxCurvedCharAngleIn );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleOut", maxCurvedCharAngleOut );
  layer->setCustomProperty( "labeling/priority", priority );
  layer->setCustomProperty( "labeling/repeatDistance", repeatDistance );
  layer->setCustomProperty( "labeling/repeatDistanceUnit", repeatDistanceUnit );
  layer->setCustomProperty( "labeling/repeatDistanceMapUnitMinScale", repeatDistanceMapUnitScale.minScale );
  layer->setCustomProperty( "labeling/repeatDistanceMapUnitMaxScale", repeatDistanceMapUnitScale.maxScale );

  // rendering
  layer->setCustomProperty( "labeling/scaleVisibility", scaleVisibility );
  layer->setCustomProperty( "labeling/scaleMin", scaleMin );
  layer->setCustomProperty( "labeling/scaleMax", scaleMax );
  layer->setCustomProperty( "labeling/fontLimitPixelSize", fontLimitPixelSize );
  layer->setCustomProperty( "labeling/fontMinPixelSize", fontMinPixelSize );
  layer->setCustomProperty( "labeling/fontMaxPixelSize", fontMaxPixelSize );
  layer->setCustomProperty( "labeling/displayAll", displayAll );
  layer->setCustomProperty( "labeling/upsidedownLabels", ( unsigned int )upsidedownLabels );

  layer->setCustomProperty( "labeling/labelPerPart", labelPerPart );
  layer->setCustomProperty( "labeling/mergeLines", mergeLines );
  layer->setCustomProperty( "labeling/minFeatureSize", minFeatureSize );
  layer->setCustomProperty( "labeling/limitNumLabels", limitNumLabels );
  layer->setCustomProperty( "labeling/maxNumLabels", maxNumLabels );
  layer->setCustomProperty( "labeling/obstacle", obstacle );

  writeDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::setDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p,
    bool active, bool useExpr, const QString& expr, const QString& field )
{
  bool defaultVals = ( !active && !useExpr && expr.isEmpty() && field.isEmpty() );

  if ( dataDefinedProperties.contains( p ) )
  {
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
    if ( it != dataDefinedProperties.constEnd() )
    {
      QgsDataDefined* dd = it.value();
      dd->setActive( active );
      dd->setUseExpression( useExpr );
      dd->setExpressionString( expr );
      dd->setField( field );
    }
  }
  else if ( !defaultVals )
  {
    QgsDataDefined* dd = new QgsDataDefined( active, useExpr, expr, field );
    dataDefinedProperties.insert( p, dd );
  }
}

void QgsPalLayerSettings::removeDataDefinedProperty( DataDefinedProperties p )
{
  QMap< DataDefinedProperties, QgsDataDefined* >::iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.end() )
  {
    delete( it.value() );
    dataDefinedProperties.erase( it );
  }
}

QString QgsPalLayerSettings::updateDataDefinedString( const QString& value )
{
  // TODO: update or remove this when project settings for labeling are migrated to better XML layout
  QString newValue = value;
  if ( !value.isEmpty() && !value.contains( "~~" ) )
  {
    QStringList values;
    values << "1"; // all old-style values are active if not empty
    values << "0";
    values << "";
    values << value; // all old-style values are only field names
    newValue = values.join( "~~" );
  }

  return newValue;
}

QgsDataDefined* QgsPalLayerSettings::dataDefinedProperty( DataDefinedProperties p )
{
  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return 0;
}

QMap<QString, QString> QgsPalLayerSettings::dataDefinedMap( DataDefinedProperties p ) const
{
  QMap<QString, QString> map;
  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    return it.value()->toMap();
  }
  return map;
}

QVariant QgsPalLayerSettings::dataDefinedValue( DataDefinedProperties p, QgsFeature& f, const QgsFields& fields ) const
{
  if ( !dataDefinedProperties.contains( p ) )
  {
    return QVariant();
  }

  QgsDataDefined* dd = 0;
  QMap< DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    dd = it.value();
  }

  if ( !dd )
  {
    return QVariant();
  }

  if ( !dd->isActive() )
  {
    return QVariant();
  }

  QVariant result = QVariant();
  bool useExpression = dd->useExpression();
  QString field = dd->field();

  //QgsDebugMsgLevel( QString( "isActive:" ) + isActive ? "1" : "0", 4 );
  //QgsDebugMsgLevel( QString( "useExpression:" ) + useExpression ? "1" : "0", 4 );
  //QgsDebugMsgLevel( QString( "expression:" ) + dd->expressionString(), 4 );
  //QgsDebugMsgLevel( QString( "field:" ) + field, 4 );

  if ( useExpression && dd->expressionIsPrepared() )
  {
    QgsExpression* expr = dd->expression();
    //QgsDebugMsgLevel( QString( "expr columns:" ) + expr->referencedColumns().join( "," ), 4 );

    result = expr->evaluate( &f );
    if ( expr->hasEvalError() )
    {
      QgsDebugMsgLevel( QString( "Evaluate error:" ) + expr->evalErrorString(), 4 );
      return QVariant();
    }
  }
  else if ( !useExpression && !field.isEmpty() )
  {
    // use direct attribute access instead of evaluating "field" expression (much faster)
    int indx = fields.indexFromName( field );
    if ( indx != -1 )
    {
      result = f.attribute( indx );
    }
  }
  return result;
}

bool QgsPalLayerSettings::dataDefinedEvaluate( DataDefinedProperties p, QVariant& exprVal ) const
{
  // null passed-around QVariant
  exprVal.clear();

  QVariant result = dataDefinedValue( p, *mCurFeat, *mCurFields );

  if ( result.isValid() ) // filter NULL values? i.e. && !result.isNull()
  {
    //QgsDebugMsgLevel( QString( "result type:" ) + QString( result.typeName() ), 4 );
    //QgsDebugMsgLevel( QString( "result string:" ) + result.toString(), 4 );
    exprVal = result;
    return true;
  }

  return false;
}

bool QgsPalLayerSettings::dataDefinedIsActive( DataDefinedProperties p ) const
{
  bool isActive = false;
  QMap< DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    isActive = it.value()->isActive();
  }

  return isActive;
}

bool QgsPalLayerSettings::dataDefinedUseExpression( DataDefinedProperties p ) const
{
  bool useExpression = false;
  QMap< DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    useExpression = it.value()->useExpression();
  }

  return useExpression;
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

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f )
{
  if ( !fm || !f )
  {
    return;
  }

  QString wrapchr = wrapChar;
  double multilineH = multilineHeight;

  bool addDirSymb = addDirectionSymbol;
  QString leftDirSymb = leftDirectionSymbol;
  QString rightDirSymb = rightDirectionSymbol;
  QgsPalLayerSettings::DirectionSymbols placeDirSymb = placeDirectionSymbol;

  if ( f == mCurFeat ) // called internally, use any stored data defined values
  {
    if ( dataDefinedValues.contains( QgsPalLayerSettings::MultiLineWrapChar ) )
    {
      wrapchr = dataDefinedValues.value( QgsPalLayerSettings::MultiLineWrapChar ).toString();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::MultiLineHeight ) )
    {
      multilineH = dataDefinedValues.value( QgsPalLayerSettings::MultiLineHeight ).toDouble();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::DirSymbDraw ) )
    {
      addDirSymb = dataDefinedValues.value( QgsPalLayerSettings::DirSymbDraw ).toBool();
    }

    if ( addDirSymb )
    {

      if ( dataDefinedValues.contains( QgsPalLayerSettings::DirSymbLeft ) )
      {
        leftDirSymb = dataDefinedValues.value( QgsPalLayerSettings::DirSymbLeft ).toString();
      }
      if ( dataDefinedValues.contains( QgsPalLayerSettings::DirSymbRight ) )
      {
        rightDirSymb = dataDefinedValues.value( QgsPalLayerSettings::DirSymbRight ).toString();
      }

      if ( dataDefinedValues.contains( QgsPalLayerSettings::DirSymbPlacement ) )
      {
        placeDirSymb = ( QgsPalLayerSettings::DirectionSymbols )dataDefinedValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt();
      }

    }

  }
  else // called externally with passed-in feature, evaluate data defined
  {
    QVariant exprVal = dataDefinedValue( QgsPalLayerSettings::MultiLineWrapChar, *f, *mCurFields );
    if ( exprVal.isValid() )
    {
      wrapchr = exprVal.toString();
    }
    exprVal.clear();
    exprVal = dataDefinedValue( QgsPalLayerSettings::MultiLineHeight, *f, *mCurFields );
    if ( exprVal.isValid() )
    {
      bool ok;
      double size = exprVal.toDouble( &ok );
      if ( ok )
      {
        multilineH = size;
      }
    }

    exprVal.clear();
    exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbDraw, *f, *mCurFields );
    if ( exprVal.isValid() )
    {
      addDirSymb = exprVal.toBool();
    }

    if ( addDirSymb ) // don't do extra evaluations if not adding a direction symbol
    {
      exprVal.clear();
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbLeft, *f, *mCurFields );
      if ( exprVal.isValid() )
      {
        leftDirSymb = exprVal.toString();
      }
      exprVal.clear();
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbRight, *f, *mCurFields );
      if ( exprVal.isValid() )
      {
        rightDirSymb = exprVal.toString();
      }
      exprVal.clear();
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbPlacement, *f, *mCurFields );
      if ( exprVal.isValid() )
      {
        bool ok;
        int enmint = exprVal.toInt( &ok );
        if ( ok )
        {
          placeDirSymb = ( QgsPalLayerSettings::DirectionSymbols )enmint;
        }
      }
    }

  }

  if ( wrapchr.isEmpty() )
  {
    wrapchr = QString( "\n" ); // default to new line delimiter
  }

  //consider the space needed for the direction symbol
  if ( addDirSymb && placement == QgsPalLayerSettings::Line
       && ( !leftDirSymb.isEmpty() || !rightDirSymb.isEmpty() ) )
  {
    QString dirSym = leftDirSymb;

    if ( fm->width( rightDirSymb ) > fm->width( dirSym ) )
      dirSym = rightDirSymb;

    if ( placeDirSymb == QgsPalLayerSettings::SymbolLeftRight )
    {
      text.append( dirSym );
    }
    else
    {
      text.prepend( dirSym + wrapchr ); // SymbolAbove or SymbolBelow
    }
  }

  double w = 0.0, h = 0.0;
  QStringList multiLineSplit = text.split( wrapchr );
  int lines = multiLineSplit.size();

  double labelHeight = fm->ascent() + fm->descent(); // ignore +1 for baseline

  h += fm->height() + ( double )(( lines - 1 ) * labelHeight * multilineH );
  h /= rasterCompressFactor;

  for ( int i = 0; i < lines; ++i )
  {
    double width = fm->width( multiLineSplit.at( i ) );
    if ( width > w )
    {
      w = width;
    }
  }
  w /= rasterCompressFactor;

#if 0 // XXX strk
  QgsPoint ptSize = xform->toMapCoordinatesF( w, h );
  labelX = qAbs( ptSize.x() - ptZero.x() );
  labelY = qAbs( ptSize.y() - ptZero.y() );
#else
  double uPP = xform->mapUnitsPerPixel();
  labelX = w * uPP;
  labelY = h * uPP;
#endif
}

void QgsPalLayerSettings::registerFeature( QgsFeature& f, const QgsRenderContext& context, QString dxfLayer )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful
  mCurFeat = &f;
//  mCurFields = &layer->pendingFields();

  // store data defined-derived values for later adding to QgsPalGeometry for use during rendering
  dataDefinedValues.clear();

  // data defined show label? defaults to show label if not 0
  if ( dataDefinedIsActive( QgsPalLayerSettings::Show ) )
  {
    bool showLabel = dataDefinedEvaluate( QgsPalLayerSettings::Show, exprVal );
    showLabel = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Show:%1" ).arg( showLabel ? "true" : "false" ), 4 );
    if ( !showLabel )
    {
      return;
    }
  }

  // data defined scale visibility?
  bool useScaleVisibility = scaleVisibility;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ScaleVisibility, exprVal ) )
  {
    QgsDebugMsgLevel( QString( "exprVal ScaleVisibility:%1" ).arg( exprVal.toBool() ? "true" : "false" ), 4 );
    useScaleVisibility = exprVal.toBool();
  }

  if ( useScaleVisibility )
  {
    // data defined min scale?
    double minScale = scaleMin;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::MinScale, exprVal ) )
    {
      QgsDebugMsgLevel( QString( "exprVal MinScale:%1" ).arg( exprVal.toDouble() ), 4 );
      bool conversionOk;
      double mins = exprVal.toDouble( &conversionOk );
      if ( conversionOk )
      {
        minScale = mins;
      }
    }

    // scales closer than 1:1
    if ( minScale < 0 )
    {
      minScale = 1 / qAbs( minScale );
    }

    if ( minScale != 0 && context.rendererScale() < minScale )
    {
      return;
    }

    // data defined max scale?
    double maxScale = scaleMax;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::MaxScale, exprVal ) )
    {
      QgsDebugMsgLevel( QString( "exprVal MaxScale:%1" ).arg( exprVal.toDouble() ), 4 );
      bool conversionOk;
      double maxs = exprVal.toDouble( &conversionOk );
      if ( conversionOk )
      {
        maxScale = maxs;
      }
    }

    // scales closer than 1:1
    if ( maxScale < 0 )
    {
      maxScale = 1 / qAbs( maxScale );
    }

    if ( maxScale != 0 && context.rendererScale() > maxScale )
    {
      return;
    }
  }

  QFont labelFont = textFont;
  // labelFont will be added to label's QgsPalGeometry for use during label painting

  // data defined font units?
  SizeUnit fontunits = fontSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::Points;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontSizeUnit, exprVal ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal Font units:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      fontunits = _decodeUnits( units );
    }
  }

  //data defined label size?
  double fontSize = labelFont.pointSizeF(); // font size doesn't have its own class data member
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Size, exprVal ) )
  {
    QgsDebugMsgLevel( QString( "exprVal Size:%1" ).arg( exprVal.toDouble() ), 4 );
    bool ok;
    double size = exprVal.toDouble( &ok );
    if ( ok )
    {
      fontSize = size;
    }
  }
  if ( fontSize <= 0.0 )
  {
    return;
  }

  int fontPixelSize = sizeToPixel( fontSize, context, fontunits, true, fontSizeMapUnitScale );
  // don't try to show font sizes less than 1 pixel (Qt complains)
  if ( fontPixelSize < 1 )
  {
    return;
  }
  labelFont.setPixelSize( fontPixelSize );

  // NOTE: labelFont now always has pixelSize set, so pointSize or pointSizeF might return -1

  // defined 'minimum/maximum pixel font size'?
  if ( fontunits == QgsPalLayerSettings::MapUnits )
  {
    bool useFontLimitPixelSize = fontLimitPixelSize;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::FontLimitPixel, exprVal ) )
    {
      QgsDebugMsgLevel( QString( "exprVal FontLimitPixel:%1" ).arg( exprVal.toBool() ? "true" : "false" ), 4 );
      useFontLimitPixelSize = exprVal.toBool();
    }

    if ( useFontLimitPixelSize )
    {
      int fontMinPixel = fontMinPixelSize;
      if ( dataDefinedEvaluate( QgsPalLayerSettings::FontMinPixel, exprVal ) )
      {
        bool ok;
        int sizeInt = exprVal.toInt( &ok );
        QgsDebugMsgLevel( QString( "exprVal FontMinPixel:%1" ).arg( sizeInt ), 4 );
        if ( ok )
        {
          fontMinPixel = sizeInt;
        }
      }

      int fontMaxPixel = fontMaxPixelSize;
      if ( dataDefinedEvaluate( QgsPalLayerSettings::FontMaxPixel, exprVal ) )
      {
        bool ok;
        int sizeInt = exprVal.toInt( &ok );
        QgsDebugMsgLevel( QString( "exprVal FontMaxPixel:%1" ).arg( sizeInt ), 4 );
        if ( ok )
        {
          fontMaxPixel = sizeInt;
        }
      }

      if ( fontMinPixel > labelFont.pixelSize() || labelFont.pixelSize() > fontMaxPixel )
      {
        return;
      }
    }
  }

  // NOTE: the following parsing functions calculate and store any data defined values for later use in QgsPalLabeling::drawLabeling
  // this is done to provide clarity, and because such parsing is not directly related to PAL feature registration calculations

  // calculate rest of font attributes and store any data defined values
  // this is done here for later use in making label backgrounds part of collision management (when implemented)
  parseTextStyle( labelFont, fontunits, context );
  parseTextFormatting();
  parseTextBuffer();
  parseShapeBackground();
  parseDropShadow();

  QString labelText;

  // Check to see if we are a expression string.
  if ( isExpression )
  {
    QgsExpression* exp = getLabelExpression();
    if ( exp->hasParserError() )
    {
      QgsDebugMsgLevel( QString( "Expression parser error:%1" ).arg( exp->parserErrorString() ), 4 );
      return;
    }
    exp->setScale( context.rendererScale() );
//    QVariant result = exp->evaluate( &f, layer->pendingFields() );
    QVariant result = exp->evaluate( &f ); // expression prepared in QgsPalLabeling::prepareLayer()
    if ( exp->hasEvalError() )
    {
      QgsDebugMsgLevel( QString( "Expression parser eval error:%1" ).arg( exp->evalErrorString() ), 4 );
      return;
    }
    labelText = result.isNull() ? "" : result.toString();
  }
  else
  {
    const QVariant &v = f.attribute( fieldIndex );
    labelText = v.isNull() ? "" : v.toString();
  }

  // data defined format numbers?
  bool formatnum = formatNumbers;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::NumFormat, exprVal ) )
  {
    formatnum = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal NumFormat:%1" ).arg( formatnum ? "true" : "false" ), 4 );
  }

  // format number if label text is coercible to a number
  if ( formatnum )
  {
    // data defined decimal places?
    int decimalPlaces = decimals;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::NumDecimals, exprVal ) )
    {
      bool ok;
      int dInt = exprVal.toInt( &ok );
      QgsDebugMsgLevel( QString( "exprVal NumDecimals:%1" ).arg( dInt ), 4 );
      if ( ok && dInt > 0 ) // needs to be positive
      {
        decimalPlaces = dInt;
      }
    }

    // data defined plus sign?
    bool signPlus = plusSign;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::NumPlusSign, exprVal ) )
    {
      signPlus = exprVal.toBool();
      QgsDebugMsgLevel( QString( "exprVal NumPlusSign:%1" ).arg( signPlus ? "true" : "false" ), 4 );
    }

    QVariant textV( labelText );
    bool ok;
    double d = textV.toDouble( &ok );
    if ( ok )
    {
      QString numberFormat;
      if ( d > 0 && signPlus )
      {
        numberFormat.append( "+" );
      }
      numberFormat.append( "%1" );
      labelText = numberFormat.arg( d, 0, 'f', decimalPlaces );
    }
  }


  // NOTE: this should come AFTER any option that affects font metrics
  QFontMetricsF* labelFontMetrics = new QFontMetricsF( labelFont );
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelFontMetrics, labelText, labelX, labelY, mCurFeat );


  // maximum angle between curved label characters (hardcoded defaults used in QGIS <2.0)
  //
  double maxcharanglein = 20.0; // range 20.0-60.0
  double maxcharangleout = -20.0; // range 20.0-95.0

  if ( placement == QgsPalLayerSettings::Curved )
  {
    maxcharanglein = maxCurvedCharAngleIn;
    maxcharangleout = maxCurvedCharAngleOut;

    //data defined maximum angle between curved label characters?
    if ( dataDefinedEvaluate( QgsPalLayerSettings::CurvedCharAngleInOut, exprVal ) )
    {
      QString ptstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QString( "exprVal CurvedCharAngleInOut:%1" ).arg( ptstr ), 4 );

      if ( !ptstr.isEmpty() )
      {
        QPointF maxcharanglePt = QgsSymbolLayerV2Utils::decodePoint( ptstr );
        maxcharanglein = qBound( 20.0, ( double )maxcharanglePt.x(), 60.0 );
        maxcharangleout = qBound( 20.0, ( double )maxcharanglePt.y(), 95.0 );
      }
    }
    // make sure maxcharangleout is always negative
    maxcharangleout = -( qAbs( maxcharangleout ) );
  }

  QgsGeometry* geom = f.geometry();
  if ( !geom )
  {
    return;
  }

  // reproject the geometry if necessary (but don't modify the features
  // geometry so that geometry based expression keep working)
  QScopedPointer<QgsGeometry> clonedGeometry;
  if ( ct )
  {
    geom = new QgsGeometry( *geom );
    clonedGeometry.reset( geom );

    try
    {
      geom->transform( *ct );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsgLevel( QString( "Ignoring feature %1 due transformation exception" ).arg( f.id() ), 4 );
      return;
    }
  }

  if ( !checkMinimumSizeMM( context, geom, minFeatureSize ) )
  {
    return;
  }

  // whether we're going to create a centroid for polygon
  bool centroidPoly = (( placement == QgsPalLayerSettings::AroundPoint
                         || placement == QgsPalLayerSettings::OverPoint )
                       && geom->type() == QGis::Polygon );

  // data defined centroid whole or clipped?
  bool wholeCentroid = centroidWhole;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::CentroidWhole, exprVal ) )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal CentroidWhole:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      if ( str.compare( "Visible", Qt::CaseInsensitive ) == 0 )
      {
        wholeCentroid = false;
      }
      else if ( str.compare( "Whole", Qt::CaseInsensitive ) == 0 )
      {
        wholeCentroid = true;
      }
    }
  }

  if ( !geom->asGeos() )
    return;  // there is something really wrong with the geometry

  // fix invalid polygons
  if ( geom->type() == QGis::Polygon && !geom->isGeosValid() )
  {
    QgsGeometry* bufferGeom = geom->buffer( 0, 0 );
    if ( !bufferGeom )
    {
      return;
    }
    geom = bufferGeom;
    clonedGeometry.reset( geom );
  }

  // Rotate the geometry if needed, before clipping
  const QgsMapToPixel& m2p = context.mapToPixel();
  if ( m2p.mapRotation() )
  {
    if ( geom->rotate( m2p.mapRotation(), context.extent().center() ) )
    {
      QgsDebugMsg( QString( "Error rotating geometry" ).arg( geom->exportToWkt() ) );
      return; // really ?
    }
  }

  // CLIP the geometry if it is bigger than the extent
  // don't clip if centroid is requested for whole feature
  bool do_clip = false;
  if ( !centroidPoly || ( centroidPoly && !wholeCentroid ) )
  {
    do_clip = !extentGeom->contains( geom );
    if ( do_clip )
    {
      QgsGeometry* clipGeom = geom->intersection( extentGeom ); // creates new geometry
      if ( !clipGeom )
      {
        return;
      }
      geom = clipGeom;
      clonedGeometry.reset( geom );
    }
  }

  const GEOSGeometry* geos_geom = geom->asGeos();

  if ( geos_geom == NULL )
    return; // invalid geometry

  // likelihood exists label will be registered with PAL and may be drawn
  // check if max number of features to label (already registered with PAL) has been reached
  // Debug output at end of QgsPalLabeling::drawLabeling(), when deleting temp geometries
  if ( limitNumLabels )
  {
    if ( !maxNumLabels )
    {
      return;
    }
    mFeatsRegPal = palLayer->getNbFeatures();
    if ( mFeatsRegPal >= maxNumLabels )
    {
      return;
    }

    int divNum = ( int )((( double )mFeaturesToLabel / maxNumLabels ) + 0.5 );
    if ( divNum && ( mFeatsRegPal == ( int )( mFeatsSendingToPal / divNum ) ) )
    {
      mFeatsSendingToPal += 1;
      if ( divNum &&  mFeatsSendingToPal % divNum )
      {
        return;
      }
    }
  }

  GEOSGeometry* geos_geom_clone;
  if ( GEOSGeomTypeId_r( QgsGeometry::getGEOSHandler(), geos_geom ) == GEOS_POLYGON && repeatDistance > 0 && placement == Line )
  {
    geos_geom_clone = GEOSBoundary_r( QgsGeometry::getGEOSHandler(), geos_geom );
  }
  else
  {
    geos_geom_clone = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom );
  }

  //data defined position / alignment / rotation?
  bool dataDefinedPosition = false;
  bool labelIsPinned = false;
  bool layerDefinedRotation = false;
  bool dataDefinedRotation = false;
  double xPos = 0.0, yPos = 0.0, angle = 0.0;
  bool ddXPos = false, ddYPos = false;
  double quadOffsetX = 0.0, quadOffsetY = 0.0;
  double offsetX = 0.0, offsetY = 0.0;

  //data defined quadrant offset?
  QuadrantPosition quadOff = quadOffset;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetQuad, exprVal ) )
  {
    bool ok;
    int quadInt = exprVal.toInt( &ok );
    QgsDebugMsgLevel( QString( "exprVal OffsetQuad:%1" ).arg( quadInt ), 4 );
    if ( ok && 0 <= quadInt && quadInt <= 8 )
    {
      quadOff = ( QuadrantPosition )quadInt;
    }
  }

  // adjust quadrant offset of labels
  switch ( quadOff )
  {
    case QuadrantAboveLeft:
      quadOffsetX = -1.0;
      quadOffsetY = 1.0;
      break;
    case QuadrantAbove:
      quadOffsetX = 0.0;
      quadOffsetY = 1.0;
      break;
    case QuadrantAboveRight:
      quadOffsetX = 1.0;
      quadOffsetY = 1.0;
      break;
    case QuadrantLeft:
      quadOffsetX = -1.0;
      quadOffsetY = 0.0;
      break;
    case QuadrantRight:
      quadOffsetX = 1.0;
      quadOffsetY = 0.0;
      break;
    case QuadrantBelowLeft:
      quadOffsetX = -1.0;
      quadOffsetY = -1.0;
      break;
    case QuadrantBelow:
      quadOffsetX = 0.0;
      quadOffsetY = -1.0;
      break;
    case QuadrantBelowRight:
      quadOffsetX = 1.0;
      quadOffsetY = -1.0;
      break;
    case QuadrantOver:
    default:
      break;
  }

  //data defined label offset?
  double xOff = xOffset;
  double yOff = yOffset;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetXY, exprVal ) )
  {
    QString ptstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal OffsetXY:%1" ).arg( ptstr ), 4 );

    if ( !ptstr.isEmpty() )
    {
      QPointF ddOffPt = QgsSymbolLayerV2Utils::decodePoint( ptstr );
      xOff = ddOffPt.x();
      yOff = ddOffPt.y();
    }
  }

  // data defined label offset units?
  bool offinmapunits = labelOffsetInMapUnits;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetUnits, exprVal ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal OffsetUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      offinmapunits = ( _decodeUnits( units ) == QgsPalLayerSettings::MapUnits );
    }
  }

  // adjust offset of labels to match chosen unit and map scale
  // offsets match those of symbology: -x = left, -y = up
  double mapUntsPerMM = labelOffsetMapUnitScale.computeMapUnitsPerPixel( context ) * context.scaleFactor();
  if ( xOff != 0 )
  {
    offsetX = xOff;  // must be positive to match symbology offset direction
    if ( !offinmapunits )
    {
      offsetX *= mapUntsPerMM; //convert offset from mm to map units
    }
  }
  if ( yOff != 0 )
  {
    offsetY = -yOff; // must be negative to match symbology offset direction
    if ( !offinmapunits )
    {
      offsetY *= mapUntsPerMM; //convert offset from mm to map units
    }
  }

  // layer defined rotation?
  // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
  if ( placement == QgsPalLayerSettings::OverPoint && angleOffset != 0 )
  {
    layerDefinedRotation = true;
    angle = angleOffset * M_PI / 180; // convert to radians
  }

  //data defined rotation?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Rotation, exprVal ) )
  {
    bool ok;
    double rotD = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QString( "exprVal Rotation:%1" ).arg( rotD ), 4 );
    if ( ok )
    {
      dataDefinedRotation = true;
      // TODO: add setting to disable having data defined rotation follow
      //       map rotation ?
      rotD -= m2p.mapRotation();
      angle = rotD * M_PI / 180.0;
    }
  }

  if ( dataDefinedEvaluate( QgsPalLayerSettings::PositionX, exprVal ) )
  {
    if ( !exprVal.isNull() )
      xPos = exprVal.toDouble( &ddXPos );
    QgsDebugMsgLevel( QString( "exprVal PositionX:%1" ).arg( xPos ), 4 );

    if ( dataDefinedEvaluate( QgsPalLayerSettings::PositionY, exprVal ) )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      if ( !exprVal.isNull() )
        yPos = exprVal.toDouble( &ddYPos );
      QgsDebugMsgLevel( QString( "exprVal PositionY:%1" ).arg( yPos ), 4 );

      if ( ddXPos && ddYPos )
      {
        dataDefinedPosition = true;
        labelIsPinned = true;
        // layer rotation set, but don't rotate pinned labels unless data defined
        if ( layerDefinedRotation && !dataDefinedRotation )
        {
          angle = 0.0;
        }

        //x/y shift in case of alignment
        double xdiff = 0.0;
        double ydiff = 0.0;

        //horizontal alignment
        if ( dataDefinedEvaluate( QgsPalLayerSettings::Hali, exprVal ) )
        {
          QString haliString = exprVal.toString();
          QgsDebugMsgLevel( QString( "exprVal Hali:%1" ).arg( haliString ), 4 );
          if ( haliString.compare( "Center", Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX / 2.0;
          }
          else if ( haliString.compare( "Right", Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX;
          }
        }

        //vertical alignment
        if ( dataDefinedEvaluate( QgsPalLayerSettings::Vali, exprVal ) )
        {
          QString valiString = exprVal.toString();
          QgsDebugMsgLevel( QString( "exprVal Vali:%1" ).arg( valiString ), 4 );

          if ( valiString.compare( "Bottom", Qt::CaseInsensitive ) != 0 )
          {
            if ( valiString.compare( "Top", Qt::CaseInsensitive ) == 0 )
            {
              ydiff -= labelY;
            }
            else
            {
              double descentRatio = labelFontMetrics->descent() / labelFontMetrics->height();
              if ( valiString.compare( "Base", Qt::CaseInsensitive ) == 0 )
              {
                ydiff -= labelY * descentRatio;
              }
              else //'Cap' or 'Half'
              {
                double capHeightRatio = ( labelFontMetrics->boundingRect( 'H' ).height() + 1 + labelFontMetrics->descent() ) / labelFontMetrics->height();
                ydiff -= labelY * capHeightRatio;
                if ( valiString.compare( "Half", Qt::CaseInsensitive ) == 0 )
                {
                  ydiff += labelY * ( capHeightRatio - descentRatio ) / 2.0;
                }
              }
            }
          }
        }

        if ( dataDefinedRotation )
        {
          //adjust xdiff and ydiff because the hali/vali point needs to be the rotation center
          double xd = xdiff * cos( angle ) - ydiff * sin( angle );
          double yd = xdiff * sin( angle ) + ydiff * cos( angle );
          xdiff = xd;
          ydiff = yd;
        }

        //project xPos and yPos from layer to map CRS
        double z = 0;
        if ( ct )
        {
          try
          {
            ct->transformInPlace( xPos, yPos, z );
          }
          catch ( QgsCsException &e )
          {
            Q_UNUSED( e );
            QgsDebugMsgLevel( QString( "Ignoring feature %1 due transformation exception on data-defined position" ).arg( f.id() ), 4 );
            return;
          }
        }

        //rotate position with map if data-defined
        if ( dataDefinedPosition && m2p.mapRotation() )
        {
          const QgsPoint& center = context.extent().center();
          QTransform t = QTransform::fromTranslate( center.x(), center.y() );
          t.rotate( -m2p.mapRotation() );
          t.translate( -center.x(), -center.y() );
          double xPosR, yPosR;
          t.map( xPos, yPos, &xPosR, &yPosR );
          xPos = xPosR; yPos = yPosR;
        }

        xPos += xdiff;
        yPos += ydiff;
      }
      else
      {
        // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
        if ( dataDefinedRotation && placement != QgsPalLayerSettings::OverPoint )
        {
          angle = 0.0;
        }
      }
    }
  }

  // data defined always show?
  bool alwaysShow = false;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::AlwaysShow, exprVal ) )
  {
    alwaysShow = exprVal.toBool();
  }

  QgsPalGeometry* lbl = new QgsPalGeometry(
    f.id(),
    labelText,
    geos_geom_clone,
    labelFont.letterSpacing(),
    labelFont.wordSpacing(),
    placement == QgsPalLayerSettings::Curved );

  lbl->setDxfLayer( dxfLayer );

  // record the created geometry - it will be deleted at the end.
  geometries.append( lbl );

  // store the label's calculated font for later use during painting
#if QT_VERSION >= 0x040800
  QgsDebugMsgLevel( QString( "PAL font stored definedFont: %1, Style: %2" ).arg( labelFont.toString() ).arg( labelFont.styleName() ), 4 );
#endif
  lbl->setDefinedFont( labelFont );

  // set repeat distance
  // data defined repeat distance?
  double repeatDist = repeatDistance;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::RepeatDistance, exprVal ) )
  {
    bool ok;
    double distD = exprVal.toDouble( &ok );
    if ( ok )
    {
      repeatDist = distD;
    }
  }

  // data defined label-repeat distance units?
  bool repeatdistinmapunit = repeatDistanceUnit == QgsPalLayerSettings::MapUnits;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::RepeatDistanceUnit, exprVal ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal RepeatDistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      repeatdistinmapunit = ( _decodeUnits( units ) == QgsPalLayerSettings::MapUnits );
    }
  }

  if ( repeatDist != 0 )
  {
    if ( repeatdistinmapunit ) //convert distance from mm/map units to pixels
    {
      repeatDist /= repeatDistanceMapUnitScale.computeMapUnitsPerPixel( context ) * context.scaleFactor();
    }
    else //mm
    {
      repeatDist *= vectorScaleFactor;
    }
    repeatDist *= qAbs( ptOne.x() - ptZero.x() );
  }

  //  feature to the layer
  try
  {
    if ( !palLayer->registerFeature( lbl->strId(), lbl, labelX, labelY, labelText.toUtf8().constData(),
                                     xPos, yPos, dataDefinedPosition, angle, dataDefinedRotation,
                                     quadOffsetX, quadOffsetY, offsetX, offsetY, alwaysShow, repeatDist ) )
      return;
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsgLevel( QString( "Ignoring feature %1 due PAL exception:" ).arg( f.id() ) + QString::fromLatin1( e.what() ), 4 );
    return;
  }

  // TODO: only for placement which needs character info
  pal::Feature* feat = palLayer->getFeature( lbl->strId() );
  // account for any data defined font metrics adjustments
  feat->setLabelInfo( lbl->info( labelFontMetrics, xform, rasterCompressFactor, maxcharanglein, maxcharangleout ) );
  delete labelFontMetrics;

  // TODO: allow layer-wide feature dist in PAL...?

  // data defined label-feature distance?
  double distance = dist;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::LabelDistance, exprVal ) )
  {
    bool ok;
    double distD = exprVal.toDouble( &ok );
    if ( ok )
    {
      distance = distD;
    }
  }

  // data defined label-feature distance units?
  bool distinmapunit = distInMapUnits;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::DistanceUnits, exprVal ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal DistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      distinmapunit = ( _decodeUnits( units ) == QgsPalLayerSettings::MapUnits );
    }
  }

  if ( distance != 0 )
  {
    if ( distinmapunit ) //convert distance from mm/map units to pixels
    {
      distance /= distMapUnitScale.computeMapUnitsPerPixel( context );
    }
    else //mm
    {
      distance *= vectorScaleFactor;
    }
    feat->setDistLabel( qAbs( ptOne.x() - ptZero.x() )* distance );
  }


  //add parameters for data defined labeling to QgsPalGeometry
  QMap< DataDefinedProperties, QVariant >::const_iterator dIt = dataDefinedValues.constBegin();
  for ( ; dIt != dataDefinedValues.constEnd(); ++dIt )
  {
    lbl->addDataDefinedValue( dIt.key(), dIt.value() );
  }

  // set geometry's pinned property
  lbl->setIsPinned( labelIsPinned );
}

bool QgsPalLayerSettings::dataDefinedValEval( const QString& valType,
    QgsPalLayerSettings::DataDefinedProperties p,
    QVariant& exprVal )
{
  if ( dataDefinedEvaluate( p, exprVal ) )
  {
    QString dbgStr = QString( "exprVal %1:" ).arg( mDataDefinedNames.value( p ).first ) + "%1";

    if ( valType == QString( "bool" ) )
    {
      bool bol = exprVal.toBool();
      QgsDebugMsgLevel( dbgStr.arg( bol ? "true" : "false" ), 4 );
      dataDefinedValues.insert( p, QVariant( bol ) );
      return true;
    }
    if ( valType == QString( "int" ) )
    {
      bool ok;
      int size = exprVal.toInt( &ok );
      QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

      if ( ok )
      {
        dataDefinedValues.insert( p, QVariant( size ) );
        return true;
      }
    }
    if ( valType == QString( "intpos" ) )
    {
      bool ok;
      int size = exprVal.toInt( &ok );
      QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

      if ( ok && size > 0 )
      {
        dataDefinedValues.insert( p, QVariant( size ) );
        return true;
      }
    }
    if ( valType == QString( "double" ) )
    {
      bool ok;
      double size = exprVal.toDouble( &ok );
      QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

      if ( ok )
      {
        dataDefinedValues.insert( p, QVariant( size ) );
        return true;
      }
    }
    if ( valType == QString( "doublepos" ) )
    {
      bool ok;
      double size = exprVal.toDouble( &ok );
      QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

      if ( ok && size > 0.0 )
      {
        dataDefinedValues.insert( p, QVariant( size ) );
        return true;
      }
    }
    if ( valType == QString( "rotation180" ) )
    {
      bool ok;
      double rot = exprVal.toDouble( &ok );
      QgsDebugMsgLevel( dbgStr.arg( rot ), 4 );
      if ( ok )
      {
        if ( rot < -180.0 && rot >= -360 )
        {
          rot += 360;
        }
        if ( rot > 180.0 && rot <= 360 )
        {
          rot -= 360;
        }
        if ( rot >= -180 && rot <= 180 )
        {
          dataDefinedValues.insert( p, QVariant( rot ) );
          return true;
        }
      }
    }
    if ( valType == QString( "transp" ) )
    {
      bool ok;
      int size = exprVal.toInt( &ok );
      QgsDebugMsgLevel( dbgStr.arg( size ), 4 );
      if ( ok && size >= 0 && size <= 100 )
      {
        dataDefinedValues.insert( p, QVariant( size ) );
        return true;
      }
    }
    if ( valType == QString( "string" ) )
    {
      QString str = exprVal.toString(); // don't trim whitespace
      QgsDebugMsgLevel( dbgStr.arg( str ), 4 );

      dataDefinedValues.insert( p, QVariant( str ) ); // let it stay empty if it is
      return true;
    }
    if ( valType == QString( "units" ) )
    {
      QString unitstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( dbgStr.arg( unitstr ), 4 );

      if ( !unitstr.isEmpty() )
      {
        dataDefinedValues.insert( p, QVariant(( int )_decodeUnits( unitstr ) ) );
        return true;
      }
    }
    if ( valType == QString( "color" ) )
    {
      QString colorstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( dbgStr.arg( colorstr ), 4 );
      QColor color = QgsSymbolLayerV2Utils::decodeColor( colorstr );

      if ( color.isValid() )
      {
        dataDefinedValues.insert( p, QVariant( color ) );
        return true;
      }
    }
    if ( valType == QString( "joinstyle" ) )
    {
      QString joinstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( dbgStr.arg( joinstr ), 4 );

      if ( !joinstr.isEmpty() )
      {
        dataDefinedValues.insert( p, QVariant(( int )_decodePenJoinStyle( joinstr ) ) );
        return true;
      }
    }
    if ( valType == QString( "blendmode" ) )
    {
      QString blendstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( dbgStr.arg( blendstr ), 4 );

      if ( !blendstr.isEmpty() )
      {
        dataDefinedValues.insert( p, QVariant(( int )QgsSymbolLayerV2Utils::decodeBlendMode( blendstr ) ) );
        return true;
      }
    }
    if ( valType == QString( "pointf" ) )
    {
      QString ptstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( dbgStr.arg( ptstr ), 4 );

      if ( !ptstr.isEmpty() )
      {
        dataDefinedValues.insert( p, QVariant( QgsSymbolLayerV2Utils::decodePoint( ptstr ) ) );
        return true;
      }
    }
  }
  return false;
}

void QgsPalLayerSettings::parseTextStyle( QFont& labelFont,
    QgsPalLayerSettings::SizeUnit fontunits,
    const QgsRenderContext& context )
{
  // NOTE: labelFont already has pixelSize set, so pointSize or pointSizeF might return -1

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // Two ways to generate new data defined font:
  // 1) Family + [bold] + [italic] (named style is ignored and font is built off of base family)
  // 2) Family + named style  (bold or italic is ignored)

  // data defined font family?
  QString ddFontFamily( "" );
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Family, exprVal ) )
  {
    QString family = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal Font family:%1" ).arg( family ), 4 );

    if ( labelFont.family() != family )
    {
      // testing for ddFontFamily in QFontDatabase.families() may be slow to do for every feature
      // (i.e. don't use QgsFontUtils::fontFamilyMatchOnSystem( family ) here)
      if ( QgsFontUtils::fontFamilyOnSystem( family ) )
      {
        ddFontFamily = family;
      }
    }
  }

  // data defined named font style?
  QString ddFontStyle( "" );
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontStyle, exprVal ) )
  {
    QString fontstyle = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal Font style:%1" ).arg( fontstyle ), 4 );
    ddFontStyle = fontstyle;
  }

  // data defined bold font style?
  bool ddBold = false;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Bold, exprVal ) )
  {
    bool bold = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font bold:%1" ).arg( bold ? "true" : "false" ), 4 );
    ddBold = bold;
  }

  // data defined italic font style?
  bool ddItalic = false;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Italic, exprVal ) )
  {
    bool italic = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font italic:%1" ).arg( italic ? "true" : "false" ), 4 );
    ddItalic = italic;
  }

  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  //       (currently defaults to what has been read in from layer settings)
  QFont newFont;
  QFont appFont = QApplication::font();
  bool newFontBuilt = false;
  if ( ddBold || ddItalic )
  {
    // new font needs built, since existing style needs removed
    newFont = QFont( !ddFontFamily.isEmpty() ? ddFontFamily : labelFont.family() );
    newFontBuilt = true;
    newFont.setBold( ddBold );
    newFont.setItalic( ddItalic );
  }
  else if ( !ddFontStyle.isEmpty()
            && ddFontStyle.compare( "Ignore", Qt::CaseInsensitive ) != 0 )
  {
    if ( !ddFontFamily.isEmpty() )
    {
      // both family and style are different, build font from database
      QFont styledfont = mFontDB.font( ddFontFamily, ddFontStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }

    // update the font face style
    QgsFontUtils::updateFontViaStyle( newFontBuilt ? newFont : labelFont, ddFontStyle );
  }
  else if ( !ddFontFamily.isEmpty() )
  {
    if ( ddFontStyle.compare( "Ignore", Qt::CaseInsensitive ) != 0 )
    {
      // just family is different, build font from database
      QFont styledfont = mFontDB.font( ddFontFamily, textNamedStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }
    else
    {
      newFont = QFont( ddFontFamily );
      newFontBuilt = true;
    }
  }

  if ( newFontBuilt )
  {
    // copy over existing font settings
    //newFont = newFont.resolve( labelFont ); // should work, but let's be sure what's being copied
    newFont.setPixelSize( labelFont.pixelSize() );
    newFont.setCapitalization( labelFont.capitalization() );
    newFont.setUnderline( labelFont.underline() );
    newFont.setStrikeOut( labelFont.strikeOut() );
    newFont.setWordSpacing( labelFont.wordSpacing() );
    newFont.setLetterSpacing( QFont::AbsoluteSpacing, labelFont.letterSpacing() );

    labelFont = newFont;
  }

  // data defined word spacing?
  double wordspace = labelFont.wordSpacing();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontWordSpacing, exprVal ) )
  {
    bool ok;
    double wspacing = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QString( "exprVal FontWordSpacing:%1" ).arg( wspacing ), 4 );
    if ( ok )
    {
      wordspace = wspacing;
    }
  }
  labelFont.setWordSpacing( sizeToPixel( wordspace, context, fontunits, false, fontSizeMapUnitScale ) );

  // data defined letter spacing?
  double letterspace = labelFont.letterSpacing();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontLetterSpacing, exprVal ) )
  {
    bool ok;
    double lspacing = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QString( "exprVal FontLetterSpacing:%1" ).arg( lspacing ), 4 );
    if ( ok )
    {
      letterspace = lspacing;
    }
  }
  labelFont.setLetterSpacing( QFont::AbsoluteSpacing, sizeToPixel( letterspace, context, fontunits, false, fontSizeMapUnitScale ) );

  // data defined font capitalization?
  QFont::Capitalization fontcaps = labelFont.capitalization();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontCase, exprVal ) )
  {
    QString fcase = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal FontCase:%1" ).arg( fcase ), 4 );

    if ( !fcase.isEmpty() )
    {
      if ( fcase.compare( "NoChange", Qt::CaseInsensitive ) == 0 )
      {
        fontcaps = QFont::MixedCase;
      }
      else if ( fcase.compare( "Upper", Qt::CaseInsensitive ) == 0 )
      {
        fontcaps = QFont::AllUppercase;
      }
      else if ( fcase.compare( "Lower", Qt::CaseInsensitive ) == 0 )
      {
        fontcaps = QFont::AllLowercase;
      }
      else if ( fcase.compare( "Capitalize", Qt::CaseInsensitive ) == 0 )
      {
        fontcaps = QFont::Capitalize;
      }

      if ( fontcaps != labelFont.capitalization() )
      {
        labelFont.setCapitalization( fontcaps );
      }
    }
  }

  // data defined strikeout font style?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Strikeout, exprVal ) )
  {
    bool strikeout = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font strikeout:%1" ).arg( strikeout ? "true" : "false" ), 4 );
    labelFont.setStrikeOut( strikeout );
  }

  // data defined underline font style?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Underline, exprVal ) )
  {
    bool underline = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font underline:%1" ).arg( underline ? "true" : "false" ), 4 );
    labelFont.setUnderline( underline );
  }

  // pass the rest on to QgsPalLabeling::drawLabeling

  // data defined font color?
  dataDefinedValEval( "color", QgsPalLayerSettings::Color, exprVal );

  // data defined font transparency?
  dataDefinedValEval( "transp", QgsPalLayerSettings::FontTransp, exprVal );

  // data defined font blend mode?
  dataDefinedValEval( "blendmode", QgsPalLayerSettings::FontBlendMode, exprVal );

}

void QgsPalLayerSettings::parseTextBuffer()
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined draw buffer?
  bool drawBuffer = bufferDraw;
  if ( dataDefinedValEval( "bool", QgsPalLayerSettings::BufferDraw, exprVal ) )
  {
    drawBuffer = exprVal.toBool();
  }

  if ( !drawBuffer )
  {
    return;
  }

  // data defined buffer size?
  double bufrSize = bufferSize;
  if ( dataDefinedValEval( "doublepos", QgsPalLayerSettings::BufferSize, exprVal ) )
  {
    bufrSize = exprVal.toDouble();
  }

  // data defined buffer transparency?
  int bufTransp = bufferTransp;
  if ( dataDefinedValEval( "transp", QgsPalLayerSettings::BufferTransp, exprVal ) )
  {
    bufTransp = exprVal.toInt();
  }

  drawBuffer = ( drawBuffer && bufrSize > 0.0 && bufTransp < 100 );

  if ( !drawBuffer )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::BufferDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::BufferSize );
    dataDefinedValues.remove( QgsPalLayerSettings::BufferTransp );
    return; // don't bother evaluating values that won't be used
  }

  // data defined buffer units?
  dataDefinedValEval( "units", QgsPalLayerSettings::BufferUnit, exprVal );

  // data defined buffer color?
  dataDefinedValEval( "color", QgsPalLayerSettings::BufferColor, exprVal );

  // data defined buffer pen join style?
  dataDefinedValEval( "joinstyle", QgsPalLayerSettings::BufferJoinStyle, exprVal );

  // data defined buffer blend mode?
  dataDefinedValEval( "blendmode", QgsPalLayerSettings::BufferBlendMode, exprVal );
}

void QgsPalLayerSettings::parseTextFormatting()
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined multiline wrap character?
  QString wrapchr = wrapChar;
  if ( dataDefinedValEval( "string", QgsPalLayerSettings::MultiLineWrapChar, exprVal ) )
  {
    wrapchr = exprVal.toString();
  }

  // data defined multiline height?
  dataDefinedValEval( "double", QgsPalLayerSettings::MultiLineHeight, exprVal );

  // data defined multiline text align?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::MultiLineAlignment, exprVal ) )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal MultiLineAlignment:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      // "Left"
      QgsPalLayerSettings::MultiLineAlign aligntype = QgsPalLayerSettings::MultiLeft;

      if ( str.compare( "Center", Qt::CaseInsensitive ) == 0 )
      {
        aligntype = QgsPalLayerSettings::MultiCenter;
      }
      else if ( str.compare( "Right", Qt::CaseInsensitive ) == 0 )
      {
        aligntype = QgsPalLayerSettings::MultiRight;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::MultiLineAlignment, QVariant(( int )aligntype ) );
    }
  }

  // data defined direction symbol?
  bool drawDirSymb = addDirectionSymbol;
  if ( dataDefinedValEval( "bool", QgsPalLayerSettings::DirSymbDraw, exprVal ) )
  {
    drawDirSymb = exprVal.toBool();
  }

  if ( drawDirSymb )
  {
    // data defined direction left symbol?
    dataDefinedValEval( "string", QgsPalLayerSettings::DirSymbLeft, exprVal );

    // data defined direction right symbol?
    dataDefinedValEval( "string", QgsPalLayerSettings::DirSymbRight, exprVal );

    // data defined direction symbol placement?
    if ( dataDefinedEvaluate( QgsPalLayerSettings::DirSymbPlacement, exprVal ) )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QString( "exprVal DirSymbPlacement:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        // "LeftRight"
        QgsPalLayerSettings::DirectionSymbols placetype = QgsPalLayerSettings::SymbolLeftRight;

        if ( str.compare( "Above", Qt::CaseInsensitive ) == 0 )
        {
          placetype = QgsPalLayerSettings::SymbolAbove;
        }
        else if ( str.compare( "Below", Qt::CaseInsensitive ) == 0 )
        {
          placetype = QgsPalLayerSettings::SymbolBelow;
        }
        dataDefinedValues.insert( QgsPalLayerSettings::DirSymbPlacement, QVariant(( int )placetype ) );
      }
    }

    // data defined direction symbol reversed?
    dataDefinedValEval( "bool", QgsPalLayerSettings::DirSymbReverse, exprVal );
  }

  // formatting for numbers is inline with generation of base label text and not passed to label painting
}

void QgsPalLayerSettings::parseShapeBackground()
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined draw shape?
  bool drawShape = shapeDraw;
  if ( dataDefinedValEval( "bool", QgsPalLayerSettings::ShapeDraw, exprVal ) )
  {
    drawShape = exprVal.toBool();
  }

  if ( !drawShape )
  {
    return;
  }

  // data defined shape transparency?
  int shapeTransp = shapeTransparency;
  if ( dataDefinedValEval( "transp", QgsPalLayerSettings::ShapeTransparency, exprVal ) )
  {
    shapeTransp = exprVal.toInt();
  }

  drawShape = ( drawShape && shapeTransp < 100 ); // size is not taken into account (could be)

  if ( !drawShape )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeTransparency );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape kind?
  QgsPalLayerSettings::ShapeType shapeKind = shapeType;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeKind, exprVal ) )
  {
    QString skind = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeKind:%1" ).arg( skind ), 4 );

    if ( !skind.isEmpty() )
    {
      // "Rectangle"
      QgsPalLayerSettings::ShapeType shpkind = QgsPalLayerSettings::ShapeRectangle;

      if ( skind.compare( "Square", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsPalLayerSettings::ShapeSquare;
      }
      else if ( skind.compare( "Ellipse", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsPalLayerSettings::ShapeEllipse;
      }
      else if ( skind.compare( "Circle", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsPalLayerSettings::ShapeCircle;
      }
      else if ( skind.compare( "SVG", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsPalLayerSettings::ShapeSVG;
      }
      shapeKind = shpkind;
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeKind, QVariant(( int )shpkind ) );
    }
  }

  // data defined shape SVG path?
  QString svgPath = shapeSVGFile;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeSVGFile, exprVal ) )
  {
    QString svgfile = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeSVGFile:%1" ).arg( svgfile ), 4 );

    // '' empty paths are allowed
    svgPath = svgfile;
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeSVGFile, QVariant( svgfile ) );
  }

  // data defined shape size type?
  QgsPalLayerSettings::SizeType shpSizeType = shapeSizeType;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeSizeType, exprVal ) )
  {
    QString stype = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeSizeType:%1" ).arg( stype ), 4 );

    if ( !stype.isEmpty() )
    {
      // "Buffer"
      QgsPalLayerSettings::SizeType sizType = QgsPalLayerSettings::SizeBuffer;

      if ( stype.compare( "Fixed", Qt::CaseInsensitive ) == 0 )
      {
        sizType = QgsPalLayerSettings::SizeFixed;
      }
      shpSizeType = sizType;
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeSizeType, QVariant(( int )sizType ) );
    }
  }

  // data defined shape size X? (SVGs only use X for sizing)
  double ddShpSizeX = shapeSize.x();
  if ( dataDefinedValEval( "double", QgsPalLayerSettings::ShapeSizeX, exprVal ) )
  {
    ddShpSizeX = exprVal.toDouble();
  }

  // data defined shape size Y?
  double ddShpSizeY = shapeSize.y();
  if ( dataDefinedValEval( "double", QgsPalLayerSettings::ShapeSizeY, exprVal ) )
  {
    ddShpSizeY = exprVal.toDouble();
  }

  // don't continue under certain circumstances (e.g. size is fixed)
  bool skip = false;
  if ( shapeKind == QgsPalLayerSettings::ShapeSVG
       && ( svgPath.isEmpty()
            || ( !svgPath.isEmpty()
                 && shpSizeType == QgsPalLayerSettings::SizeFixed
                 && ddShpSizeX == 0.0 ) ) )
  {
    skip = true;
  }
  if ( shapeKind != QgsPalLayerSettings::ShapeSVG
       && shpSizeType == QgsPalLayerSettings::SizeFixed
       && ( ddShpSizeX == 0.0 || ddShpSizeY == 0.0 ) )
  {
    skip = true;
  }

  if ( skip )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeTransparency );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeKind );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSVGFile );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSizeX );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSizeY );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape size units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShapeSizeUnits, exprVal );

  // data defined shape rotation type?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeRotationType, exprVal ) )
  {
    QString rotstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeRotationType:%1" ).arg( rotstr ), 4 );

    if ( !rotstr.isEmpty() )
    {
      // "Sync"
      QgsPalLayerSettings::RotationType rottype = QgsPalLayerSettings::RotationSync;

      if ( rotstr.compare( "Offset", Qt::CaseInsensitive ) == 0 )
      {
        rottype = QgsPalLayerSettings::RotationOffset;
      }
      else if ( rotstr.compare( "Fixed", Qt::CaseInsensitive ) == 0 )
      {
        rottype = QgsPalLayerSettings::RotationFixed;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeRotationType, QVariant(( int )rottype ) );
    }
  }

  // data defined shape rotation?
  dataDefinedValEval( "rotation180", QgsPalLayerSettings::ShapeRotation, exprVal );

  // data defined shape offset?
  dataDefinedValEval( "pointf", QgsPalLayerSettings::ShapeOffset, exprVal );

  // data defined shape offset units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShapeOffsetUnits, exprVal );

  // data defined shape radii?
  dataDefinedValEval( "pointf", QgsPalLayerSettings::ShapeRadii, exprVal );

  // data defined shape radii units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShapeRadiiUnits, exprVal );

  // data defined shape blend mode?
  dataDefinedValEval( "blendmode", QgsPalLayerSettings::ShapeBlendMode, exprVal );

  // data defined shape fill color?
  dataDefinedValEval( "color", QgsPalLayerSettings::ShapeFillColor, exprVal );

  // data defined shape border color?
  dataDefinedValEval( "color", QgsPalLayerSettings::ShapeBorderColor, exprVal );

  // data defined shape border width?
  dataDefinedValEval( "doublepos", QgsPalLayerSettings::ShapeBorderWidth, exprVal );

  // data defined shape border width units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShapeBorderWidthUnits, exprVal );

  // data defined shape join style?
  dataDefinedValEval( "joinstyle", QgsPalLayerSettings::ShapeJoinStyle, exprVal );

}

void QgsPalLayerSettings::parseDropShadow()
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined draw shadow?
  bool drawShadow = shadowDraw;
  if ( dataDefinedValEval( "bool", QgsPalLayerSettings::ShadowDraw, exprVal ) )
  {
    drawShadow = exprVal.toBool();
  }

  if ( !drawShadow )
  {
    return;
  }

  // data defined shadow transparency?
  int shadowTransp = shadowTransparency;
  if ( dataDefinedValEval( "transp", QgsPalLayerSettings::ShadowTransparency, exprVal ) )
  {
    shadowTransp = exprVal.toInt();
  }

  // data defined shadow offset distance?
  double shadowOffDist = shadowOffsetDist;
  if ( dataDefinedValEval( "doublepos", QgsPalLayerSettings::ShadowOffsetDist, exprVal ) )
  {
    shadowOffDist = exprVal.toDouble();
  }

  // data defined shadow offset distance?
  double shadowRad = shadowRadius;
  if ( dataDefinedValEval( "doublepos", QgsPalLayerSettings::ShadowRadius, exprVal ) )
  {
    shadowRad = exprVal.toDouble();
  }

  drawShadow = ( drawShadow && shadowTransp < 100 && !( shadowOffDist == 0.0 && shadowRad == 0.0 ) );

  if ( !drawShadow )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::ShadowDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowTransparency );
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowOffsetDist );
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowRadius );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shadow under type?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShadowUnder, exprVal ) )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShadowUnder:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      // "Lowest"
      QgsPalLayerSettings::ShadowType shdwtype = QgsPalLayerSettings::ShadowLowest;

      if ( str.compare( "Text", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsPalLayerSettings::ShadowText;
      }
      else if ( str.compare( "Buffer", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsPalLayerSettings::ShadowBuffer;
      }
      else if ( str.compare( "Background", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsPalLayerSettings::ShadowShape;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::ShadowUnder, QVariant(( int )shdwtype ) );
    }
  }

  // data defined shadow offset angle?
  dataDefinedValEval( "rotation180", QgsPalLayerSettings::ShadowOffsetAngle, exprVal );

  // data defined shadow offset units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShadowOffsetUnits, exprVal );

  // data defined shadow radius?
  dataDefinedValEval( "double", QgsPalLayerSettings::ShadowRadius, exprVal );

  // data defined shadow radius units?
  dataDefinedValEval( "units", QgsPalLayerSettings::ShadowRadiusUnits, exprVal );

  // data defined shadow scale?  ( gui bounds to 0-2000, no upper bound here )
  dataDefinedValEval( "intpos", QgsPalLayerSettings::ShadowScale, exprVal );

  // data defined shadow color?
  dataDefinedValEval( "color", QgsPalLayerSettings::ShadowColor, exprVal );

  // data defined shadow blend mode?
  dataDefinedValEval( "blendmode", QgsPalLayerSettings::ShadowBlendMode, exprVal );
}

int QgsPalLayerSettings::sizeToPixel( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor, const QgsMapUnitScale& mapUnitScale ) const
{
  return ( int )( scaleToPixelContext( size, c, unit, rasterfactor, mapUnitScale ) + 0.5 );
}

double QgsPalLayerSettings::scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor, const QgsMapUnitScale& mapUnitScale ) const
{
  // if render context is that of device (i.e. not a scaled map), just return size
  double mapUnitsPerPixel = mapUnitScale.computeMapUnitsPerPixel( c );

  if ( unit == MapUnits && mapUnitsPerPixel > 0.0 )
  {
    size = size / mapUnitsPerPixel * ( rasterfactor ? c.rasterScaleFactor() : 1 );
  }
  else // e.g. in points or mm
  {
    double ptsTomm = ( unit == Points ? 0.352778 : 1 );
    size *= ptsTomm * c.scaleFactor() * ( rasterfactor ? c.rasterScaleFactor() : 1 );
  }
  return size;
}

// -------------

QgsPalLabeling::QgsPalLabeling()
    : mMapSettings( NULL ), mPal( NULL )
    , mResults( 0 )
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
  mShowingShadowRects = false;
  mShowingAllLabels = false;
  mShowingPartialsLabels = p.getShowPartial();
  mDrawOutlineLabels = true;
}

QgsPalLabeling::~QgsPalLabeling()
{
  // make sure we've freed everything
  exit();

  clearActiveLayers();

  delete mResults;
  mResults = 0;
}

bool QgsPalLabeling::willUseLayer( QgsVectorLayer* layer )
{
  return staticWillUseLayer( layer );
}

bool QgsPalLabeling::staticWillUseLayer( const QString& layerID )
{
  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
  if ( !layer )
    return false;
  return staticWillUseLayer( layer );
}


bool QgsPalLabeling::staticWillUseLayer( QgsVectorLayer* layer )
{
  // don't do QgsPalLayerSettings::readFromLayer( layer ) if not needed
  bool enabled = false;
  if ( layer->customProperty( "labeling" ).toString() == QString( "pal" ) )
    enabled = layer->customProperty( "labeling/enabled", QVariant( false ) ).toBool();

  return enabled;
}


void QgsPalLabeling::clearActiveLayers()
{
  QHash<QString, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    clearActiveLayer( lit.key() );
  }
  mActiveLayers.clear();
}

void QgsPalLabeling::clearActiveLayer( const QString &layerID )
{
  QgsPalLayerSettings& lyr = mActiveLayers[layerID];

  // delete all QgsDataDefined objects (which also deletes their QgsExpression object)
  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::iterator it = lyr.dataDefinedProperties.begin();
  for ( ; it != lyr.dataDefinedProperties.constEnd(); ++it )
  {
    delete( it.value() );
    it.value() = 0;
  }
  lyr.dataDefinedProperties.clear();
}

int QgsPalLabeling::prepareLayer( QgsVectorLayer* layer, QStringList& attrNames, QgsRenderContext& ctx )
{
  Q_ASSERT( mMapSettings != NULL );

  if ( !willUseLayer( layer ) )
  {
    return 0;
  }

  QgsDebugMsgLevel( "PREPARE LAYER " + layer->id(), 4 );

  // start with a temporary settings class, find out labeling info
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );

  if ( lyrTmp.fieldName.isEmpty() )
  {
    return 0;
  }

  if ( lyrTmp.isExpression )
  {
    QgsExpression exp( lyrTmp.fieldName );
    if ( exp.hasEvalError() )
    {
      QgsDebugMsgLevel( "Prepare error:" + exp.evalErrorString(), 4 );
      return 0;
    }
  }
  else
  {
    // If we aren't an expression, we check to see if we can find the column.
    if ( layer->fieldNameIndex( lyrTmp.fieldName ) == -1 )
    {
      return 0;
    }
  }

  // add layer settings to the pallabeling hashtable: <QgsVectorLayer*, QgsPalLayerSettings>
  mActiveLayers.insert( layer->id(), lyrTmp );
  // start using the reference to the layer in hashtable instead of local instance
  QgsPalLayerSettings& lyr = mActiveLayers[layer->id()];

  lyr.mCurFields = &( layer->pendingFields() );

  // add field indices for label's text, from expression or field
  if ( lyr.isExpression )
  {
    // prepare expression for use in QgsPalLayerSettings::registerFeature()
    QgsExpression* exp = lyr.getLabelExpression();
    exp->prepare( layer->pendingFields() );
    if ( exp->hasEvalError() )
    {
      QgsDebugMsgLevel( "Prepare error:" + exp->evalErrorString(), 4 );
    }
    foreach ( QString name, exp->referencedColumns() )
    {
      QgsDebugMsgLevel( "REFERENCED COLUMN = " + name, 4 );
      attrNames.append( name );
    }
  }
  else
  {
    attrNames.append( lyr.fieldName );
  }

  // add field indices of data defined expression or field
  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator dIt = lyr.dataDefinedProperties.constBegin();
  for ( ; dIt != lyr.dataDefinedProperties.constEnd(); ++dIt )
  {
    QgsDataDefined* dd = dIt.value();
    if ( !dd->isActive() )
    {
      continue;
    }

    // NOTE: the following also prepares any expressions for later use

    // store parameters for data defined expressions
    QMap<QString, QVariant> exprParams;
    exprParams.insert( "scale", ctx.rendererScale() );

    dd->setExpressionParams( exprParams );

    // this will return columns for expressions or field name, depending upon what is set to be used
    QStringList cols = dd->referencedColumns( layer ); // <-- prepares any expressions, too

    //QgsDebugMsgLevel( QString( "Data defined referenced columns:" ) + cols.join( "," ), 4 );
    foreach ( QString name, cols )
    {
      attrNames.append( name );
    }
  }

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
    default: Q_ASSERT( "unsupported placement" && 0 ); return 0;
  }

  // create the pal layer
  double priority = 1 - lyr.priority / 10.0; // convert 0..10 --> 1..0
  double min_scale = -1, max_scale = -1;

  // handled in QgsPalLayerSettings::registerFeature now
  //if ( lyr.scaleVisibility && !lyr.dataDefinedIsActive( QgsPalLayerSettings::ScaleVisibility ) )
  //{
  //  min_scale = lyr.scaleMin;
  //  max_scale = lyr.scaleMax;
  //}

  Layer* l = mPal->addLayer( layer->id().toUtf8().data(),
                             min_scale, max_scale, arrangement,
                             METER, priority, lyr.obstacle, true, true,
                             lyr.displayAll );

  if ( lyr.placementFlags )
    l->setArrangementFlags( lyr.placementFlags );

  // set label mode (label per feature is the default)
  l->setLabelMode( lyr.labelPerPart ? Layer::LabelPerFeaturePart : Layer::LabelPerFeature );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( lyr.mergeLines );


  // set whether location of centroid must be inside of polygons
  l->setCentroidInside( lyr.centroidInside );

  // set how to show upside-down labels
  Layer::UpsideDownLabels upsdnlabels;
  switch ( lyr.upsidedownLabels )
  {
    case QgsPalLayerSettings::Upright:     upsdnlabels = Layer::Upright; break;
    case QgsPalLayerSettings::ShowDefined: upsdnlabels = Layer::ShowDefined; break;
    case QgsPalLayerSettings::ShowAll:     upsdnlabels = Layer::ShowAll; break;
    default: Q_ASSERT( "unsupported upside-down label setting" && 0 ); return 0;
  }
  l->setUpsidedownLabels( upsdnlabels );

//  // fix for font size in map units causing font to show pointsize at small map scales
//  int pixelFontSize = lyr.sizeToPixel( lyr.textFont.pointSizeF(), ctx,
//                                       lyr.fontSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::Points,
//                                       true );

//  if ( pixelFontSize < 1 )
//  {
//    lyr.textFont.setPointSize( 1 );
//    lyr.textFont.setPixelSize( 1 );
//  }
//  else
//  {
//    lyr.textFont.setPixelSize( pixelFontSize );
//  }

//  // scale spacing sizes if using map units
//  if ( lyr.fontSizeInMapUnits )
//  {
//    double spacingPixelSize;
//    if ( lyr.textFont.wordSpacing() != 0 )
//    {
//      spacingPixelSize = lyr.textFont.wordSpacing() / ctx.mapToPixel().mapUnitsPerPixel() * ctx.rasterScaleFactor();
//      lyr.textFont.setWordSpacing( spacingPixelSize );
//    }

//    if ( lyr.textFont.letterSpacing() != 0 )
//    {
//      spacingPixelSize = lyr.textFont.letterSpacing() / ctx.mapToPixel().mapUnitsPerPixel() * ctx.rasterScaleFactor();
//      lyr.textFont.setLetterSpacing( QFont::AbsoluteSpacing, spacingPixelSize );
//    }
//  }

  //raster and vector scale factors
  lyr.vectorScaleFactor = ctx.scaleFactor();
  lyr.rasterCompressFactor = ctx.rasterScaleFactor();

  // save the pal layer to our layer context (with some additional info)
  lyr.palLayer = l;
  lyr.fieldIndex = layer->fieldNameIndex( lyr.fieldName );

  lyr.xform = &mMapSettings->mapToPixel();
  lyr.ct = 0;
  if ( mMapSettings->hasCrsTransformEnabled() )
    lyr.ct = new QgsCoordinateTransform( layer->crs(), mMapSettings->destinationCrs() );
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  // rect for clipping
  lyr.extentGeom = QgsGeometry::fromRect( mMapSettings->visibleExtent() );

  lyr.mFeatsSendingToPal = 0;

  return 1; // init successful
}

int QgsPalLabeling::addDiagramLayer( QgsVectorLayer* layer, const QgsDiagramLayerSettings *s )
{
  Layer* l = mPal->addLayer( layer->id().append( "d" ).toUtf8().data(), -1, -1, pal::Arrangement( s->placement ), METER, s->priority, s->obstacle, true, true );
  l->setArrangementFlags( s->placementFlags );

  mActiveDiagramLayers.insert( layer->id(), *s );
  // initialize the local copy
  QgsDiagramLayerSettings& s2 = mActiveDiagramLayers[layer->id()];

  s2.palLayer = l;
  s2.ct = 0;
  if ( mMapSettings->hasCrsTransformEnabled() )
    s2.ct = new QgsCoordinateTransform( layer->crs(), mMapSettings->destinationCrs() );

  s2.xform = &mMapSettings->mapToPixel();

  s2.fields = layer->pendingFields();

  s2.renderer = layer->diagramRenderer()->clone();

  return 1;
}

void QgsPalLabeling::registerFeature( const QString& layerID, QgsFeature& f, const QgsRenderContext& context, QString dxfLayer )
{
  QgsPalLayerSettings& lyr = mActiveLayers[layerID];
  lyr.registerFeature( f, context, dxfLayer );
}

void QgsPalLabeling::registerDiagramFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context )
{
  //get diagram layer settings, diagram renderer
  QHash<QString, QgsDiagramLayerSettings>::iterator layerIt = mActiveDiagramLayers.find( layerID );
  if ( layerIt == mActiveDiagramLayers.constEnd() )
  {
    return;
  }

  //convert geom to geos
  QgsGeometry* geom = feat.geometry();

  if ( layerIt.value().ct && staticWillUseLayer( layerID ) ) // reproject the geometry if feature not already transformed for labeling
  {
    geom->transform( *( layerIt.value().ct ) );
  }

  const GEOSGeometry* geos_geom = geom->asGeos();
  if ( geos_geom == 0 )
  {
    return; // invalid geometry
  }

  //create PALGeometry with diagram = true
  QgsPalGeometry* lbl = new QgsPalGeometry( feat.id(), "", GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom ) );
  lbl->setIsDiagram( true );

  // record the created geometry - it will be deleted at the end.
  layerIt.value().geometries.append( lbl );

  double diagramWidth = 0;
  double diagramHeight = 0;
  QgsDiagramRendererV2* dr = layerIt.value().renderer;
  if ( dr )
  {
    QSizeF diagSize = dr->sizeMapUnits( feat, context );
    if ( diagSize.isValid() )
    {
      diagramWidth = diagSize.width();
      diagramHeight = diagSize.height();
    }

    //append the diagram attributes to lbl
    lbl->setDiagramAttributes( feat.attributes() );
  }

  //  feature to the layer
  int ddColX = layerIt.value().xPosColumn;
  int ddColY = layerIt.value().yPosColumn;
  double ddPosX = 0.0;
  double ddPosY = 0.0;
  bool ddPos = ( ddColX >= 0 && ddColY >= 0 );
  if ( ddPos )
  {
    bool posXOk, posYOk;
    //data defined diagram position is always centered
    ddPosX = feat.attribute( ddColX ).toDouble( &posXOk ) - diagramWidth / 2.0;
    ddPosY = feat.attribute( ddColY ).toDouble( &posYOk ) - diagramHeight / 2.0;
    if ( !posXOk || !posYOk )
    {
      ddPos = false;
    }
    else
    {
      const QgsCoordinateTransform* ct = layerIt.value().ct;
      if ( ct )
      {
        double z = 0;
        ct->transformInPlace( ddPosX, ddPosY, z );
      }
    }
  }

  try
  {
    if ( !layerIt.value().palLayer->registerFeature( lbl->strId(), lbl, diagramWidth, diagramHeight, "", ddPosX, ddPosY, ddPos ) )
    {
      return;
    }
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsgLevel( QString( "Ignoring feature %1 due PAL exception:" ).arg( feat.id() ) + QString::fromLatin1( e.what() ), 4 );
    return;
  }

  pal::Feature* palFeat = layerIt.value().palLayer->getFeature( lbl->strId() );
  QgsPoint ptZero = layerIt.value().xform->toMapCoordinates( 0, 0 );
  QgsPoint ptOne = layerIt.value().xform->toMapCoordinates( 1, 0 );
  palFeat->setDistLabel( qAbs( ptOne.x() - ptZero.x() ) * layerIt.value().dist );
}


void QgsPalLabeling::init( QgsMapRenderer* mr )
{
  init( mr->mapSettings() );
}

void QgsPalLabeling::init( const QgsMapSettings& mapSettings )
{
  mMapSettings = &mapSettings;

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

  mPal->setShowPartial( mShowingPartialsLabels );

  clearActiveLayers(); // free any previous QgsDataDefined objects
  mActiveDiagramLayers.clear();
}

void QgsPalLabeling::exit()
{
  delete mPal;
  mPal = NULL;
  mMapSettings = NULL;
}

QgsPalLayerSettings& QgsPalLabeling::layer( const QString& layerName )
{
  QHash<QString, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    if ( lit.key() == layerName )
    {
      return lit.value();
    }
  }
  return mInvalidLayerSettings;
}

void QgsPalLabeling::dataDefinedTextStyle( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  //font color
  if ( ddValues.contains( QgsPalLayerSettings::Color ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Color );
    tmpLyr.textColor = ddColor.value<QColor>();
  }

  //font transparency
  if ( ddValues.contains( QgsPalLayerSettings::FontTransp ) )
  {
    tmpLyr.textTransp = ddValues.value( QgsPalLayerSettings::FontTransp ).toInt();
  }

  tmpLyr.textColor.setAlphaF(( 100.0 - ( double )( tmpLyr.textTransp ) ) / 100.0 );

  //font blend mode
  if ( ddValues.contains( QgsPalLayerSettings::FontBlendMode ) )
  {
    tmpLyr.blendMode = ( QPainter::CompositionMode )ddValues.value( QgsPalLayerSettings::FontBlendMode ).toInt();
  }
}

void QgsPalLabeling::dataDefinedTextFormatting( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  if ( ddValues.contains( QgsPalLayerSettings::MultiLineWrapChar ) )
  {
    tmpLyr.wrapChar = ddValues.value( QgsPalLayerSettings::MultiLineWrapChar ).toString();
  }

  if ( !tmpLyr.wrapChar.isEmpty() || tmpLyr.getLabelExpression()->expression().contains( "wordwrap" ) )
  {

    if ( ddValues.contains( QgsPalLayerSettings::MultiLineHeight ) )
    {
      tmpLyr.multilineHeight = ddValues.value( QgsPalLayerSettings::MultiLineHeight ).toDouble();
    }

    if ( ddValues.contains( QgsPalLayerSettings::MultiLineAlignment ) )
    {
      tmpLyr.multilineAlign = ( QgsPalLayerSettings::MultiLineAlign )ddValues.value( QgsPalLayerSettings::MultiLineAlignment ).toInt();
    }

  }

  if ( ddValues.contains( QgsPalLayerSettings::DirSymbDraw ) )
  {
    tmpLyr.addDirectionSymbol = ddValues.value( QgsPalLayerSettings::DirSymbDraw ).toBool();
  }

  if ( tmpLyr.addDirectionSymbol )
  {

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbLeft ) )
    {
      tmpLyr.leftDirectionSymbol = ddValues.value( QgsPalLayerSettings::DirSymbLeft ).toString();
    }
    if ( ddValues.contains( QgsPalLayerSettings::DirSymbRight ) )
    {
      tmpLyr.rightDirectionSymbol = ddValues.value( QgsPalLayerSettings::DirSymbRight ).toString();
    }

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbPlacement ) )
    {
      tmpLyr.placeDirectionSymbol = ( QgsPalLayerSettings::DirectionSymbols )ddValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt();
    }

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbReverse ) )
    {
      tmpLyr.reverseDirectionSymbol = ddValues.value( QgsPalLayerSettings::DirSymbReverse ).toBool();
    }

  }
}

void QgsPalLabeling::dataDefinedTextBuffer( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  //buffer draw
  if ( ddValues.contains( QgsPalLayerSettings::BufferDraw ) )
  {
    tmpLyr.bufferDraw = ddValues.value( QgsPalLayerSettings::BufferDraw ).toBool();
  }

  if ( !tmpLyr.bufferDraw )
  {
    // tmpLyr.bufferSize > 0.0 && tmpLyr.bufferTransp < 100 figured in during evaluation
    return; // don't continue looking for unused values
  }

  //buffer size
  if ( ddValues.contains( QgsPalLayerSettings::BufferSize ) )
  {
    tmpLyr.bufferSize = ddValues.value( QgsPalLayerSettings::BufferSize ).toDouble();
  }

  //buffer transparency
  if ( ddValues.contains( QgsPalLayerSettings::BufferTransp ) )
  {
    tmpLyr.bufferTransp = ddValues.value( QgsPalLayerSettings::BufferTransp ).toInt();
  }

  //buffer size units
  if ( ddValues.contains( QgsPalLayerSettings::BufferUnit ) )
  {
    QgsPalLayerSettings::SizeUnit bufunit = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::BufferUnit ).toInt();
    tmpLyr.bufferSizeInMapUnits = ( bufunit == QgsPalLayerSettings::MapUnits );
  }

  //buffer color
  if ( ddValues.contains( QgsPalLayerSettings::BufferColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::BufferColor );
    tmpLyr.bufferColor = ddColor.value<QColor>();
  }

  // apply any transparency
  tmpLyr.bufferColor.setAlphaF(( 100.0 - ( double )( tmpLyr.bufferTransp ) ) / 100.0 );

  //buffer pen join style
  if ( ddValues.contains( QgsPalLayerSettings::BufferJoinStyle ) )
  {
    tmpLyr.bufferJoinStyle = ( Qt::PenJoinStyle )ddValues.value( QgsPalLayerSettings::BufferJoinStyle ).toInt();
  }

  //buffer blend mode
  if ( ddValues.contains( QgsPalLayerSettings::BufferBlendMode ) )
  {
    tmpLyr.bufferBlendMode = ( QPainter::CompositionMode )ddValues.value( QgsPalLayerSettings::BufferBlendMode ).toInt();
  }
}

void QgsPalLabeling::dataDefinedShapeBackground( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  //shape draw
  if ( ddValues.contains( QgsPalLayerSettings::ShapeDraw ) )
  {
    tmpLyr.shapeDraw = ddValues.value( QgsPalLayerSettings::ShapeDraw ).toBool();
  }

  if ( !tmpLyr.shapeDraw )
  {
    return; // don't continue looking for unused values
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeKind ) )
  {
    tmpLyr.shapeType = ( QgsPalLayerSettings::ShapeType )ddValues.value( QgsPalLayerSettings::ShapeKind ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSVGFile ) )
  {
    tmpLyr.shapeSVGFile = ddValues.value( QgsPalLayerSettings::ShapeSVGFile ).toString();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeType ) )
  {
    tmpLyr.shapeSizeType = ( QgsPalLayerSettings::SizeType )ddValues.value( QgsPalLayerSettings::ShapeSizeType ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeX ) )
  {
    tmpLyr.shapeSize.setX( ddValues.value( QgsPalLayerSettings::ShapeSizeX ).toDouble() );
  }
  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeY ) )
  {
    tmpLyr.shapeSize.setY( ddValues.value( QgsPalLayerSettings::ShapeSizeY ).toDouble() );
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeUnits ) )
  {
    tmpLyr.shapeSizeUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShapeSizeUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRotationType ) )
  {
    tmpLyr.shapeRotationType = ( QgsPalLayerSettings::RotationType )ddValues.value( QgsPalLayerSettings::ShapeRotationType ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRotation ) )
  {
    tmpLyr.shapeRotation = ddValues.value( QgsPalLayerSettings::ShapeRotation ).toDouble();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeOffset ) )
  {
    tmpLyr.shapeOffset = ddValues.value( QgsPalLayerSettings::ShapeOffset ).toPointF();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeOffsetUnits ) )
  {
    tmpLyr.shapeOffsetUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShapeOffsetUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRadii ) )
  {
    tmpLyr.shapeRadii = ddValues.value( QgsPalLayerSettings::ShapeRadii ).toPointF();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRadiiUnits ) )
  {
    tmpLyr.shapeRadiiUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShapeRadiiUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeTransparency ) )
  {
    tmpLyr.shapeTransparency = ddValues.value( QgsPalLayerSettings::ShapeTransparency ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBlendMode ) )
  {
    tmpLyr.shapeBlendMode = ( QPainter::CompositionMode )ddValues.value( QgsPalLayerSettings::ShapeBlendMode ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeFillColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShapeFillColor );
    tmpLyr.shapeFillColor = ddColor.value<QColor>();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShapeBorderColor );
    tmpLyr.shapeBorderColor = ddColor.value<QColor>();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderWidth ) )
  {
    tmpLyr.shapeBorderWidth = ddValues.value( QgsPalLayerSettings::ShapeBorderWidth ).toDouble();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderWidthUnits ) )
  {
    tmpLyr.shapeBorderWidthUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShapeBorderWidthUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeJoinStyle ) )
  {
    tmpLyr.shapeJoinStyle = ( Qt::PenJoinStyle )ddValues.value( QgsPalLayerSettings::ShapeJoinStyle ).toInt();
  }
}

void QgsPalLabeling::dataDefinedDropShadow( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  //shadow draw
  if ( ddValues.contains( QgsPalLayerSettings::ShadowDraw ) )
  {
    tmpLyr.shadowDraw = ddValues.value( QgsPalLayerSettings::ShadowDraw ).toBool();
  }

  if ( !tmpLyr.shadowDraw )
  {
    return; // don't continue looking for unused values
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowUnder ) )
  {
    tmpLyr.shadowUnder = ( QgsPalLayerSettings::ShadowType )ddValues.value( QgsPalLayerSettings::ShadowUnder ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetAngle ) )
  {
    tmpLyr.shadowOffsetAngle = ddValues.value( QgsPalLayerSettings::ShadowOffsetAngle ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetDist ) )
  {
    tmpLyr.shadowOffsetDist = ddValues.value( QgsPalLayerSettings::ShadowOffsetDist ).toDouble();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetUnits ) )
  {
    tmpLyr.shadowOffsetUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShadowOffsetUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowRadius ) )
  {
    tmpLyr.shadowRadius = ddValues.value( QgsPalLayerSettings::ShadowRadius ).toDouble();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowRadiusUnits ) )
  {
    tmpLyr.shadowRadiusUnits = ( QgsPalLayerSettings::SizeUnit )ddValues.value( QgsPalLayerSettings::ShadowRadiusUnits ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowTransparency ) )
  {
    tmpLyr.shadowTransparency = ddValues.value( QgsPalLayerSettings::ShadowTransparency ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowScale ) )
  {
    tmpLyr.shadowScale = ddValues.value( QgsPalLayerSettings::ShadowScale ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShadowColor );
    tmpLyr.shadowColor = ddColor.value<QColor>();
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowBlendMode ) )
  {
    tmpLyr.shadowBlendMode = ( QPainter::CompositionMode )ddValues.value( QgsPalLayerSettings::ShadowBlendMode ).toInt();
  }
}


// helper function for checking for job cancellation within PAL
static bool _palIsCancelled( void* ctx )
{
  return (( QgsRenderContext* ) ctx )->renderingStopped();
}

void QgsPalLabeling::drawLabeling( QgsRenderContext& context )
{
  Q_ASSERT( mMapSettings != NULL );
  QPainter* painter = context.painter();
  QgsRectangle extent = context.extent();

  mPal->registerCancellationCallback( &_palIsCancelled, &context );

  delete mResults;
  mResults = new QgsLabelingResults;

  QTime t;
  t.start();

  // do the labeling itself
  double scale = mMapSettings->scale(); // scale denominator
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
    Q_UNUSED( e );
    QgsDebugMsgLevel( "PAL EXCEPTION :-( " + QString::fromLatin1( e.what() ), 4 );
    //mActiveLayers.clear(); // clean up
    return;
  }

  if ( context.renderingStopped() )
    return; // it has been cancelled

#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // http://hub.qgis.org/issues/11856
  QgsMapToPixel xform = mMapSettings->mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel& xform = mMapSettings->mapToPixel();
#endif

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

        drawLabelCandidateRect( lp, painter, &xform );
      }
    }
  }

  // find the solution
  labels = mPal->solveProblem( problem, mShowingAllLabels );

  QgsDebugMsgLevel( QString( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( labels->size() ), 4 );
  t.restart();

  if ( context.renderingStopped() )
  {
    delete problem;
    delete labels;
    deleteTemporaryData();
    return;
  }

  painter->setRenderHint( QPainter::Antialiasing );

  // draw the labels
  std::list<LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it )
  {
    if ( context.renderingStopped() )
      break;

    QgsPalGeometry* palGeometry = dynamic_cast< QgsPalGeometry* >(( *it )->getFeaturePart()->getUserGeometry() );
    if ( !palGeometry )
    {
      continue;
    }

    //layer names
    QString layerName = QString::fromUtf8(( *it )->getLayerName() );
    if ( palGeometry->isDiagram() )
    {
      QgsFeature feature;
      //render diagram
      QHash<QString, QgsDiagramLayerSettings>::iterator dit = mActiveDiagramLayers.begin();
      for ( dit = mActiveDiagramLayers.begin(); dit != mActiveDiagramLayers.end(); ++dit )
      {
        if ( QString( dit.key() + "d" ) == layerName )
        {
          feature.setFields( &dit.value().fields );
          palGeometry->feature( feature );
          QgsPoint outPt = xform.transform(( *it )->getX(), ( *it )->getY() );
          dit.value().renderer->renderDiagram( feature, context, QPointF( outPt.x(), outPt.y() ) );
        }
      }

      //insert into label search tree to manipulate position interactively
      if ( mResults->mLabelSearchTree )
      {
        //for diagrams, remove the additional 'd' at the end of the layer id
        QString layerId = layerName;
        layerId.chop( 1 );
        mResults->mLabelSearchTree->insertLabel( *it, QString( palGeometry->strId() ).toInt(), QString( "" ), layerId, QFont(), true, false );
      }
      continue;
    }

    const QgsPalLayerSettings& lyr = layer( layerName );

    // Copy to temp, editable layer settings
    // these settings will be changed by any data defined values, then used for rendering label components
    // settings may be adjusted during rendering of components
    QgsPalLayerSettings tmpLyr( lyr );

    // apply any previously applied data defined settings for the label
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues = palGeometry->dataDefinedValues();

    //font
    QFont dFont = palGeometry->definedFont();
    // following debug is >= Qt 4.8 only ( because of QFont::styleName() )
#if QT_VERSION >= 0x040800
    QgsDebugMsgLevel( QString( "PAL font tmpLyr: %1, Style: %2" ).arg( tmpLyr.textFont.toString() ).arg( QFontInfo( tmpLyr.textFont ).styleName() ), 4 );
    QgsDebugMsgLevel( QString( "PAL font definedFont: %1, Style: %2" ).arg( dFont.toString() ).arg( dFont.styleName() ), 4 );
#endif
    tmpLyr.textFont = dFont;

    // update tmpLyr with any data defined text style values
    dataDefinedTextStyle( tmpLyr, ddValues );

    // update tmpLyr with any data defined text buffer values
    dataDefinedTextBuffer( tmpLyr, ddValues );

    // update tmpLyr with any data defined text formatting values
    dataDefinedTextFormatting( tmpLyr, ddValues );

    // update tmpLyr with any data defined shape background values
    dataDefinedShapeBackground( tmpLyr, ddValues );

    // update tmpLyr with any data defined drop shadow values
    dataDefinedDropShadow( tmpLyr, ddValues );


    tmpLyr.showingShadowRects = mShowingShadowRects;

    // Render the components of a label in reverse order
    //   (backgrounds -> text)

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowLowest )
    {
      if ( tmpLyr.shapeDraw )
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowShape;
      }
      else if ( tmpLyr.bufferDraw )
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowBuffer;
      }
      else
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowText;
      }
    }

    if ( tmpLyr.shapeDraw )
    {
      drawLabel( *it, context, tmpLyr, LabelShape );
    }

    if ( tmpLyr.bufferDraw )
    {
      drawLabel( *it, context, tmpLyr, LabelBuffer );
    }

    drawLabel( *it, context, tmpLyr, LabelText );

    if ( mResults->mLabelSearchTree )
    {
      QString labeltext = (( QgsPalGeometry* )( *it )->getFeaturePart()->getUserGeometry() )->text();
      mResults->mLabelSearchTree->insertLabel( *it, QString( palGeometry->strId() ).toInt(), layerName, labeltext, dFont, false, palGeometry->isPinned() );
    }
  }

  // Reset composition mode for further drawing operations
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  QgsDebugMsgLevel( QString( "LABELING draw:  %1 ms" ).arg( t.elapsed() ), 4 );

  delete problem;
  delete labels;
  deleteTemporaryData();
}

void QgsPalLabeling::deleteTemporaryData()
{
  // delete all allocated geometries for features
  QHash<QString, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    QgsPalLayerSettings& lyr = lit.value();
    for ( QList<QgsPalGeometry*>::iterator git = lyr.geometries.begin(); git != lyr.geometries.end(); ++git )
      delete *git;
    if ( lyr.limitNumLabels )
    {
      QgsDebugMsgLevel( QString( "mFeaturesToLabel: %1" ).arg( lyr.mFeaturesToLabel ), 4 );
      QgsDebugMsgLevel( QString( "maxNumLabels: %1" ).arg( lyr.maxNumLabels ), 4 );
      QgsDebugMsgLevel( QString( "mFeatsSendingToPal: %1" ).arg( lyr.mFeatsSendingToPal ), 4 );
      QgsDebugMsgLevel( QString( "mFeatsRegPal: %1" ).arg( lyr.geometries.count() ), 4 );
    }
    lyr.geometries.clear();
  }

  //delete all allocated geometries for diagrams
  QHash<QString, QgsDiagramLayerSettings>::iterator dIt = mActiveDiagramLayers.begin();
  for ( ; dIt != mActiveDiagramLayers.end(); ++dIt )
  {
    QgsDiagramLayerSettings& dls = dIt.value();
    for ( QList<QgsPalGeometry*>::iterator git = dls.geometries.begin(); git != dls.geometries.end(); ++git )
    {
      delete *git;
    }
    dls.geometries.clear();
  }
}

QList<QgsLabelPosition> QgsPalLabeling::labelsAtPosition( const QgsPoint& p )
{
  return mResults ? mResults->labelsAtPosition( p ) : QList<QgsLabelPosition>();
}

QList<QgsLabelPosition> QgsPalLabeling::labelsWithinRect( const QgsRectangle& r )
{
  return mResults ? mResults->labelsWithinRect( r ) : QList<QgsLabelPosition>();
}

QgsLabelingResults *QgsPalLabeling::takeResults()
{
  if ( mResults )
  {
    QgsLabelingResults* tmp = mResults;
    mResults = 0;
    return tmp; // ownership passed to the caller
  }
  else
    return 0;
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

  painter->save();

#if 0 // TODO: generalize some of this
  double w = lp->getWidth();
  double h = lp->getHeight();
  double cx = lp->getX() + w / 2.0;
  double cy = lp->getY() + h / 2.0;
  double scale = 1.0 / xform->mapUnitsPerPixel();
  double rotation = xform->mapRotation();
  double sw = w * scale;
  double sh = h * scale;
  QRectF rect( -sw / 2, -sh / 2, sw, sh );

  painter->translate( xform->transform( QPointF( cx, cy ) ).toQPointF() );
  if ( rotation )
  {
    // Only if not horizontal
    if ( lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT &&
         lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT_OVER &&
         lp->getFeaturePart()->getLayer()->getArrangement() != P_HORIZ )
    {
      painter->rotate( rotation );
    }
  }
  painter->translate( rect.bottomLeft() );
  painter->rotate( -lp->getAlpha() * 180 / M_PI );
  painter->translate( -rect.bottomLeft() );
#else
  QgsPoint outPt2 = xform->transform( lp->getX() + lp->getWidth(), lp->getY() + lp->getHeight() );
  QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -lp->getAlpha() * 180 / M_PI );
#endif

  painter->drawRect( rect );
  painter->restore();

  // save the rect
  rect.moveTo( outPt.x(), outPt.y() );
  mCandidates.append( QgsLabelCandidate( rect, lp->getCost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->getNextPart() )
    drawLabelCandidateRect( lp->getNextPart(), painter, xform );
}

void QgsPalLabeling::drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType, double dpiRatio )
{
  // NOTE: this is repeatedly called for multi-part labels
  QPainter* painter = context.painter();
#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // http://hub.qgis.org/issues/11856
  QgsMapToPixel xform = context.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel& xform = context.mapToPixel();
#endif

  QgsLabelComponent component;
  component.setDpiRatio( dpiRatio );

  QgsPoint outPt = xform.transform( label->getX(), label->getY() );
//  QgsPoint outPt2 = xform->transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
//  QRectF labelRect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );

  component.setOrigin( outPt );
  component.setRotation( label->getAlpha() );

  if ( drawType == QgsPalLabeling::LabelShape )
  {
    // get rotated label's center point
    QgsPoint centerPt( outPt );
    QgsPoint outPt2 = xform.transform( label->getX() + label->getWidth() / 2,
                                       label->getY() + label->getHeight() / 2 );

    double xc = outPt2.x() - outPt.x();
    double yc = outPt2.y() - outPt.y();

    double angle = -label->getAlpha();
    double xd = xc * cos( angle ) - yc * sin( angle );
    double yd = xc * sin( angle ) + yc * cos( angle );

    centerPt.setX( centerPt.x() + xd );
    centerPt.setY( centerPt.y() + yd );

    component.setCenter( centerPt );
    component.setSize( QgsPoint( label->getWidth(), label->getHeight() ) );

    drawLabelBackground( context, component, tmpLyr );
  }

  else if ( drawType == QgsPalLabeling::LabelBuffer
            || drawType == QgsPalLabeling::LabelText )
  {

    // TODO: optimize access :)
    QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();
    QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );
    QFontMetricsF* labelfm = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->getLabelFontMetrics();

    QString wrapchr = !tmpLyr.wrapChar.isEmpty() ? tmpLyr.wrapChar : QString( "\n" );

    //add the direction symbol if needed
    if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line &&
         tmpLyr.addDirectionSymbol )
    {
      bool prependSymb = false;
      QString symb = tmpLyr.rightDirectionSymbol;

      if ( label->getReversed() )
      {
        prependSymb = true;
        symb = tmpLyr.leftDirectionSymbol;
      }

      if ( tmpLyr.reverseDirectionSymbol )
      {
        if ( symb == tmpLyr.rightDirectionSymbol )
        {
          prependSymb = true;
          symb = tmpLyr.leftDirectionSymbol;
        }
        else
        {
          prependSymb = false;
          symb = tmpLyr.rightDirectionSymbol;
        }
      }

      if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
      {
        prependSymb = true;
        symb = symb + wrapchr;
      }
      else if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
      {
        prependSymb = false;
        symb = wrapchr + symb;
      }

      if ( prependSymb )
      {
        txt.prepend( symb );
      }
      else
      {
        txt.append( symb );
      }
    }

    //QgsDebugMsgLevel( "drawLabel " + txt, 4 );

    QStringList multiLineList = txt.split( wrapchr );
    int lines = multiLineList.size();

    double labelWidest = 0.0;
    for ( int i = 0; i < lines; ++i )
    {
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( labelWidth > labelWidest )
      {
        labelWidest = labelWidth;
      }
    }

    double labelHeight = labelfm->ascent() + labelfm->descent(); // ignore +1 for baseline
    //  double labelHighest = labelfm->height() + ( double )(( lines - 1 ) * labelHeight * tmpLyr.multilineHeight );

    // needed to move bottom of text's descender to within bottom edge of label
    double ascentOffset = 0.25 * labelfm->ascent(); // labelfm->descent() is not enough

    for ( int i = 0; i < lines; ++i )
    {
      painter->save();
#if 0 // TODO: generalize some of this
      LabelPosition* lp = label;
      double w = lp->getWidth();
      double h = lp->getHeight();
      double cx = lp->getX() + w / 2.0;
      double cy = lp->getY() + h / 2.0;
      double scale = 1.0 / xform->mapUnitsPerPixel();
      double rotation = xform->mapRotation();
      double sw = w * scale;
      double sh = h * scale;
      QRectF rect( -sw / 2, -sh / 2, sw, sh );
      painter->translate( xform->transform( QPointF( cx, cy ) ).toQPointF() );
      if ( rotation )
      {
        // Only if not horizontal
        if ( lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT &&
             lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT_OVER &&
             lp->getFeaturePart()->getLayer()->getArrangement() != P_HORIZ )
        {
          painter->rotate( rotation );
        }
      }
      painter->translate( rect.bottomLeft() );
      painter->rotate( -lp->getAlpha() * 180 / M_PI );
#else
      painter->translate( QPointF( outPt.x(), outPt.y() ) );
      painter->rotate( -label->getAlpha() * 180 / M_PI );
#endif

      // scale down painter: the font size has been multiplied by raster scale factor
      // to workaround a Qt font scaling bug with small font sizes
      painter->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );

      // figure x offset for horizontal alignment of multiple lines
      double xMultiLineOffset = 0.0;
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( lines > 1 && tmpLyr.multilineAlign != QgsPalLayerSettings::MultiLeft )
      {
        double labelWidthDiff = labelWidest - labelWidth;
        if ( tmpLyr.multilineAlign == QgsPalLayerSettings::MultiCenter )
        {
          labelWidthDiff /= 2;
        }
        xMultiLineOffset = labelWidthDiff;
        //QgsDebugMsgLevel( QString( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ), 4 );
      }

      double yMultiLineOffset = ( lines - 1 - i ) * labelHeight * tmpLyr.multilineHeight;
      painter->translate( QPointF( xMultiLineOffset, - ascentOffset - yMultiLineOffset ) );

      component.setText( multiLineList.at( i ) );
      component.setSize( QgsPoint( labelWidth, labelHeight ) );
      component.setOffset( QgsPoint( 0.0, -ascentOffset ) );
      component.setRotation( -component.rotation() * 180 / M_PI );
      component.setRotationOffset( 0.0 );

      if ( drawType == QgsPalLabeling::LabelBuffer )
      {
        // draw label's buffer
        drawLabelBuffer( context, component, tmpLyr );
      }
      else
      {
        // draw label's text, QPainterPath method
        QPainterPath path;
        path.addText( 0, 0, tmpLyr.textFont, component.text() );

        // store text's drawing in QPicture for drop shadow call
        QPicture textPict;
        QPainter textp;
        textp.begin( &textPict );
        textp.setPen( Qt::NoPen );
        textp.setBrush( tmpLyr.textColor );
        textp.drawPath( path );
        // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
        //       e.g. some capitalization options, but not others
        //textp.setFont( tmpLyr.textFont );
        //textp.setPen( tmpLyr.textColor );
        //textp.drawText( 0, 0, component.text() );
        textp.end();

        if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowText )
        {
          component.setPicture( &textPict );
          component.setPictureBuffer( 0.0 ); // no pen width to deal with
          component.setOrigin( QgsPoint( 0.0, 0.0 ) );

          drawLabelShadow( context, component, tmpLyr );
        }

        // paint the text
        if ( context.useAdvancedEffects() )
        {
          painter->setCompositionMode( tmpLyr.blendMode );
        }

        // scale for any print output or image saving @ specific dpi
        painter->scale( component.dpiRatio(), component.dpiRatio() );

        if ( mDrawOutlineLabels )
        {
          // draw outlined text
          _fixQPictureDPI( painter );
          painter->drawPicture( 0, 0, textPict );
        }
        else
        {
          // draw text as text (for SVG and PDF exports)
          painter->setFont( tmpLyr.textFont );
          painter->setPen( tmpLyr.textColor );
          painter->setRenderHint( QPainter::TextAntialiasing );
          painter->drawText( 0, 0, component.text() );
        }
      }
      painter->restore();
    }
  }

  // NOTE: this used to be within above multi-line loop block, at end. (a mistake since 2010? [LS])
  if ( label->getNextPart() )
    drawLabel( label->getNextPart(), context, tmpLyr, drawType, dpiRatio );
}

void QgsPalLabeling::drawLabelBuffer( QgsRenderContext& context,
                                      const QgsLabelComponent& component,
                                      const QgsPalLayerSettings& tmpLyr )
{
  QPainter* p = context.painter();

  double penSize = tmpLyr.scaleToPixelContext( tmpLyr.bufferSize, context,
                   ( tmpLyr.bufferSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::MM ), true, tmpLyr.bufferSizeMapUnitScale );

  QPainterPath path;
  path.addText( 0, 0, tmpLyr.textFont, component.text() );
  QPen pen( tmpLyr.bufferColor );
  pen.setWidthF( penSize );
  pen.setJoinStyle( tmpLyr.bufferJoinStyle );
  QColor tmpColor( tmpLyr.bufferColor );
  // honor pref for whether to fill buffer interior
  if ( tmpLyr.bufferNoFill )
  {
    tmpColor.setAlpha( 0 );
  }

  // store buffer's drawing in QPicture for drop shadow call
  QPicture buffPict;
  QPainter buffp;
  buffp.begin( &buffPict );
  buffp.setPen( pen );
  buffp.setBrush( tmpColor );
  buffp.drawPath( path );
  buffp.end();

  if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowBuffer )
  {
    QgsLabelComponent bufferComponent = component;
    bufferComponent.setOrigin( QgsPoint( 0.0, 0.0 ) );
    bufferComponent.setPicture( &buffPict );
    bufferComponent.setPictureBuffer( penSize / 2.0 );
    drawLabelShadow( context, bufferComponent, tmpLyr );
  }

  p->save();
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( tmpLyr.bufferBlendMode );
  }
//  p->setPen( pen );
//  p->setBrush( tmpColor );
//  p->drawPath( path );

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio(), component.dpiRatio() );
  _fixQPictureDPI( p );
  p->drawPicture( 0, 0, buffPict );
  p->restore();
}

void QgsPalLabeling::drawLabelBackground( QgsRenderContext& context,
    QgsLabelComponent component,
    const QgsPalLayerSettings& tmpLyr )
{
  QPainter* p = context.painter();
  double labelWidth = component.size().x(), labelHeight = component.size().y();
  //QgsDebugMsgLevel( QString( "Background label rotation: %1" ).arg( component.rotation() ), 4 );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  if ( tmpLyr.shapeRotationType != QgsPalLayerSettings::RotationFixed )
  {
    component.setRotation( -( component.rotation() * 180 / M_PI ) ); // RotationSync
    component.setRotationOffset(
      tmpLyr.shapeRotationType == QgsPalLayerSettings::RotationOffset ? tmpLyr.shapeRotation : 0.0 );
  }
  else // RotationFixed
  {
    component.setRotation( 0.0 ); // don't use label's rotation
    component.setRotationOffset( tmpLyr.shapeRotation );
  }

  // mm to map units conversion factor
  double mmToMapUnits = tmpLyr.shapeSizeMapUnitScale.computeMapUnitsPerPixel( context ) * context.scaleFactor();

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeSVG )
  {
    // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

    if ( tmpLyr.shapeSVGFile.isEmpty() )
      return;

    double sizeOut = 0.0;
    // only one size used for SVG sizing/scaling (no use of shapeSize.y() or Y field in gui)
    if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeFixed )
    {
      sizeOut = tmpLyr.shapeSize.x();
    }
    else if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeBuffer )
    {
      // add buffer to greatest dimension of label
      if ( labelWidth >= labelHeight )
        sizeOut = labelWidth;
      else if ( labelHeight > labelWidth )
        sizeOut = labelHeight;

      // label size in map units, convert to shapeSizeUnits, if different
      if ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM )
      {
        sizeOut /= mmToMapUnits;
      }

      // add buffer
      sizeOut += tmpLyr.shapeSize.x() * 2;
    }

    // don't bother rendering symbols smaller than 1x1 pixels in size
    // TODO: add option to not show any svgs under/over a certian size
    if ( tmpLyr.scaleToPixelContext( sizeOut, context, tmpLyr.shapeSizeUnits, false, tmpLyr.shapeSizeMapUnitScale ) < 1.0 )
      return;

    QgsStringMap map; // for SVG symbology marker
    map["name"] = QgsSymbolLayerV2Utils::symbolNameToPath( tmpLyr.shapeSVGFile.trimmed() );
    map["size"] = QString::number( sizeOut );
    map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
                         tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );
    map["angle"] = QString::number( 0.0 ); // angle is handled by this local painter

    // offset is handled by this local painter
    // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
    //map["offset"] = QgsSymbolLayerV2Utils::encodePoint( tmpLyr.shapeOffset );
    //map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    map["fill"] = tmpLyr.shapeFillColor.name();
    map["outline"] = tmpLyr.shapeBorderColor.name();
    map["outline-width"] = QString::number( tmpLyr.shapeBorderWidth );

    // TODO: fix overriding SVG symbol's border width/units in QgsSvgCache
    // currently broken, fall back to symbol's
    //map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                              tmpLyr.shapeBorderWidthUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowShape )
    {
      // configure SVG shadow specs
      QgsStringMap shdwmap( map );
      shdwmap["fill"] = tmpLyr.shadowColor.name();
      shdwmap["outline"] = tmpLyr.shadowColor.name();
      shdwmap["size"] = QString::number( sizeOut * tmpLyr.rasterCompressFactor );

      // store SVG's drawing in QPicture for drop shadow call
      QPicture svgPict;
      QPainter svgp;
      svgp.begin( &svgPict );

      // draw shadow symbol

      // clone current render context map unit/mm conversion factors, but not
      // other map canvas parameters, then substitute this painter for use in symbology painting
      // NOTE: this is because the shadow needs to be scaled correctly for output to map canvas,
      //       but will be created relative to the SVG's computed size, not the current map canvas
      QgsRenderContext shdwContext;
      shdwContext.setMapToPixel( context.mapToPixel() );
      shdwContext.setScaleFactor( context.scaleFactor() );
      shdwContext.setPainter( &svgp );

      QgsSymbolLayerV2* symShdwL = QgsSvgMarkerSymbolLayerV2::create( shdwmap );
      QgsSvgMarkerSymbolLayerV2* svgShdwM = static_cast<QgsSvgMarkerSymbolLayerV2*>( symShdwL );
      QgsSymbolV2RenderContext svgShdwContext( shdwContext, QgsSymbolV2::Mixed,
          ( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );

      double svgSize = tmpLyr.scaleToPixelContext( sizeOut, context, tmpLyr.shapeSizeUnits, true, tmpLyr.shapeSizeMapUnitScale );
      svgShdwM->renderPoint( QPointF( svgSize / 2, -svgSize / 2 ), svgShdwContext );
      svgp.end();

      component.setPicture( &svgPict );
      // TODO: when SVG symbol's border width/units is fixed in QgsSvgCache, adjust for it here
      component.setPictureBuffer( 0.0 );

      component.setSize( QgsPoint( svgSize, svgSize ) );
      component.setOffset( QgsPoint( 0.0, 0.0 ) );

      // rotate about origin center of SVG
      p->save();
      p->translate( component.center().x(), component.center().y() );
      p->rotate( component.rotation() );
      p->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );
      double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits, true, tmpLyr.shapeOffsetMapUnitScale );
      double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits, true, tmpLyr.shapeOffsetMapUnitScale );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset() );
      p->translate( -svgSize / 2, svgSize / 2 );

      drawLabelShadow( context, component, tmpLyr );
      p->restore();

      delete svgShdwM;
      svgShdwM = 0;
    }

    // draw the actual symbol
    QgsSymbolLayerV2* symL = QgsSvgMarkerSymbolLayerV2::create( map );
    QgsSvgMarkerSymbolLayerV2* svgM = static_cast<QgsSvgMarkerSymbolLayerV2*>( symL );
    QgsSymbolV2RenderContext svgContext( context, QgsSymbolV2::Mixed,
                                         ( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );

    p->save();
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( tmpLyr.shapeBlendMode );
    }
    p->translate( component.center().x(), component.center().y() );
    p->rotate( component.rotation() );
    double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits, false, tmpLyr.shapeOffsetMapUnitScale );
    double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits, false, tmpLyr.shapeOffsetMapUnitScale );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );
    svgM->renderPoint( QPointF( 0, 0 ), svgContext );
    p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
    p->restore();

    delete svgM;
    svgM = 0;

  }
  else  // Generated Shapes
  {
    // all calculations done in shapeSizeUnits

    double w = labelWidth / ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM ? mmToMapUnits : 1 );
    double h = labelHeight / ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM ? mmToMapUnits : 1 );

    double xsize = tmpLyr.shapeSize.x();
    double ysize = tmpLyr.shapeSize.y();

    if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeFixed )
    {
      w = xsize;
      h = ysize;
    }
    else if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeBuffer )
    {
      if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeSquare )
      {
        if ( w > h )
          h = w;
        else if ( h > w )
          w = h;
      }
      else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeCircle )
      {
        // start with label bound by circle
        h = sqrt( pow( w, 2 ) + pow( h, 2 ) );
        w = h;
      }
      else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeEllipse )
      {
        // start with label bound by ellipse
        h = h / sqrt( 2.0 ) * 2;
        w = w / sqrt( 2.0 ) * 2;
      }

      w += xsize * 2;
      h += ysize * 2;
    }

    // convert everything over to map pixels from here on
    w = tmpLyr.scaleToPixelContext( w, context, tmpLyr.shapeSizeUnits, true, tmpLyr.shapeSizeMapUnitScale );
    h = tmpLyr.scaleToPixelContext( h, context, tmpLyr.shapeSizeUnits, true, tmpLyr.shapeSizeMapUnitScale );

    // offsets match those of symbology: -x = left, -y = up
    QRectF rect( -w / 2.0, - h / 2.0, w, h );

    if ( rect.isNull() )
      return;

    p->save();
    p->translate( QPointF( component.center().x(), component.center().y() ) );
    p->rotate( component.rotation() );
    double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits, false, tmpLyr.shapeOffsetMapUnitScale );
    double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits, false, tmpLyr.shapeOffsetMapUnitScale );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );

    double penSize = tmpLyr.scaleToPixelContext( tmpLyr.shapeBorderWidth, context, tmpLyr.shapeBorderWidthUnits, true, tmpLyr.shapeBorderWidthMapUnitScale );

    QPen pen;
    if ( tmpLyr.shapeBorderWidth > 0 )
    {
      pen.setColor( tmpLyr.shapeBorderColor );
      pen.setWidthF( penSize );
      if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeRectangle )
        pen.setJoinStyle( tmpLyr.shapeJoinStyle );
    }
    else
    {
      pen = Qt::NoPen;
    }

    // store painting in QPicture for shadow drawing
    QPicture shapePict;
    QPainter shapep;
    shapep.begin( &shapePict );
    shapep.setPen( pen );
    shapep.setBrush( tmpLyr.shapeFillColor );

    if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeRectangle
         || tmpLyr.shapeType == QgsPalLayerSettings::ShapeSquare )
    {
      if ( tmpLyr.shapeRadiiUnits == QgsPalLayerSettings::Percent )
      {
        shapep.drawRoundedRect( rect, tmpLyr.shapeRadii.x(), tmpLyr.shapeRadii.y(), Qt::RelativeSize );
      }
      else
      {
        double xRadius = tmpLyr.scaleToPixelContext( tmpLyr.shapeRadii.x(), context, tmpLyr.shapeRadiiUnits, true, tmpLyr.shapeRadiiMapUnitScale );
        double yRadius = tmpLyr.scaleToPixelContext( tmpLyr.shapeRadii.y(), context, tmpLyr.shapeRadiiUnits, true, tmpLyr.shapeRadiiMapUnitScale );
        shapep.drawRoundedRect( rect, xRadius, yRadius );
      }
    }
    else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeEllipse
              || tmpLyr.shapeType == QgsPalLayerSettings::ShapeCircle )
    {
      shapep.drawEllipse( rect );
    }
    shapep.end();

    p->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowShape )
    {
      component.setPicture( &shapePict );
      component.setPictureBuffer( penSize / 2.0 );

      component.setSize( QgsPoint( rect.width(), rect.height() ) );
      component.setOffset( QgsPoint( rect.width() / 2, -rect.height() / 2 ) );
      drawLabelShadow( context, component, tmpLyr );
    }

    p->setOpacity(( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( tmpLyr.shapeBlendMode );
    }

    // scale for any print output or image saving @ specific dpi
    p->scale( component.dpiRatio(), component.dpiRatio() );
    _fixQPictureDPI( p );
    p->drawPicture( 0, 0, shapePict );
    p->restore();
  }
}

void QgsPalLabeling::drawLabelShadow( QgsRenderContext& context,
                                      const QgsLabelComponent& component,
                                      const QgsPalLayerSettings& tmpLyr )
{
  // incoming component sizes should be multiplied by rasterCompressFactor, as
  // this allows shadows to be created at paint device dpi (e.g. high resolution),
  // then scale device painter by 1.0 / rasterCompressFactor for output

  QPainter* p = context.painter();
  double componentWidth = component.size().x(), componentHeight = component.size().y();
  double xOffset = component.offset().x(), yOffset = component.offset().y();
  double pictbuffer = component.pictureBuffer();

  // generate pixmap representation of label component drawing
  bool mapUnits = ( tmpLyr.shadowRadiusUnits == QgsPalLayerSettings::MapUnits );
  double radius = tmpLyr.scaleToPixelContext( tmpLyr.shadowRadius, context, tmpLyr.shadowRadiusUnits, !mapUnits, tmpLyr.shadowRadiusMapUnitScale );
  radius /= ( mapUnits ? tmpLyr.vectorScaleFactor / component.dpiRatio() : 1 );
  radius = ( int )( radius + 0.5 );

  // TODO: add labeling gui option to adjust blurBufferClippingScale to minimize pixels, or
  //       to ensure shadow isn't clipped too tight. (Or, find a better method of buffering)
  double blurBufferClippingScale = 3.75;
  int blurbuffer = ( radius > 17 ? 16 : radius ) * blurBufferClippingScale;

  QImage blurImg( componentWidth + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  componentHeight + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  QImage::Format_ARGB32_Premultiplied );

  // TODO: add labeling gui option to not show any shadows under/over a certian size
  // keep very small QImages from causing paint device issues, i.e. must be at least > 1
  int minBlurImgSize = 1;
  // max limitation on QgsSvgCache is 10,000 for screen, which will probably be reasonable for future caching here, too
  // 4 x QgsSvgCache limit for output to print/image at higher dpi
  // TODO: should it be higher, scale with dpi, or have no limit? Needs testing with very large labels rendered at high dpi output
  int maxBlurImgSize = 40000;
  if ( blurImg.isNull()
       || ( blurImg.width() < minBlurImgSize || blurImg.height() < minBlurImgSize )
       || ( blurImg.width() > maxBlurImgSize || blurImg.height() > maxBlurImgSize ) )
    return;

  blurImg.fill( QColor( Qt::transparent ).rgba() );
  QPainter pictp;
  if ( !pictp.begin( &blurImg ) )
    return;
  pictp.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPointF imgOffset( blurbuffer + pictbuffer + xOffset,
                     blurbuffer + pictbuffer + componentHeight + yOffset );

  pictp.drawPicture( imgOffset,
                     *component.picture() );

  // overlay shadow color
  pictp.setCompositionMode( QPainter::CompositionMode_SourceIn );
  pictp.fillRect( blurImg.rect(), tmpLyr.shadowColor );
  pictp.end();

  // blur the QImage in-place
  if ( tmpLyr.shadowRadius > 0.0 && radius > 0 )
  {
    QgsSymbolLayerV2Utils::blurImageInPlace( blurImg, blurImg.rect(), radius, tmpLyr.shadowRadiusAlphaOnly );
  }

  if ( tmpLyr.showingShadowRects ) // engine setting, not per layer
  {
    // debug rect for QImage shadow registration and clipping visualization
    QPainter picti;
    picti.begin( &blurImg );
    picti.setBrush( Qt::Dense7Pattern );
    QPen imgPen( QColor( 0, 0, 255, 255 ) );
    imgPen.setWidth( 1 );
    picti.setPen( imgPen );
    picti.setOpacity( 0.1 );
    picti.drawRect( 0, 0, blurImg.width(), blurImg.height() );
    picti.end();
  }

  double offsetDist = tmpLyr.scaleToPixelContext( tmpLyr.shadowOffsetDist, context, tmpLyr.shadowOffsetUnits, true, tmpLyr.shadowOffsetMapUnitScale );
  double angleRad = tmpLyr.shadowOffsetAngle * M_PI / 180; // to radians
  if ( tmpLyr.shadowOffsetGlobal )
  {
    // TODO: check for differences in rotation origin and cw/ccw direction,
    //       when this shadow function is used for something other than labels

    // it's 0-->cw-->360 for labels
    //QgsDebugMsgLevel( QString( "Shadow aggregated label rotation (degrees): %1" ).arg( component.rotation() + component.rotationOffset() ), 4 );
    angleRad -= ( component.rotation() * M_PI / 180 + component.rotationOffset() * M_PI / 180 );
  }

  QPointF transPt( -offsetDist * cos( angleRad + M_PI / 2 ),
                   -offsetDist * sin( angleRad + M_PI / 2 ) );

  p->save();
  p->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( tmpLyr.shadowBlendMode );
  }
  p->setOpacity(( 100.0 - ( double )( tmpLyr.shadowTransparency ) ) / 100.0 );

  double scale = ( double )tmpLyr.shadowScale / 100.0;
  // TODO: scale from center/center, left/center or left/top, instead of default left/bottom?
  p->scale( scale, scale );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawImage( 0, 0, blurImg );
  p->restore();

  // debug rects
  if ( tmpLyr.showingShadowRects ) // engine setting, not per layer
  {
    // draw debug rect for QImage painting registration
    p->save();
    p->setBrush( Qt::NoBrush );
    QPen imgPen( QColor( 255, 0, 0, 10 ) );
    imgPen.setWidth( 2 );
    imgPen.setStyle( Qt::DashLine );
    p->setPen( imgPen );
    p->scale( scale, scale );
    if ( component.useOrigin() )
    {
      p->translate( component.origin().x(), component.origin().y() );
    }
    p->translate( transPt );
    p->translate( -imgOffset.x(),
                  -imgOffset.y() );
    p->drawRect( 0, 0, blurImg.width(), blurImg.height() );
    p->restore();

    // draw debug rect for passed in component dimensions
    p->save();
    p->setBrush( Qt::NoBrush );
    QPen componentRectPen( QColor( 0, 255, 0, 70 ) );
    componentRectPen.setWidth( 1 );
    if ( component.useOrigin() )
    {
      p->translate( component.origin().x(), component.origin().y() );
    }
    p->setPen( componentRectPen );
    p->drawRect( QRect( -xOffset, -componentHeight - yOffset, componentWidth, componentHeight ) );
    p->restore();
  }
}

void QgsPalLabeling::loadEngineSettings()
{
  // start with engine defaults for new project, or project that has no saved settings
  Pal p;
  bool saved = false;
  mSearch = ( QgsPalLabeling::Search )( QgsProject::instance()->readNumEntry(
                                          "PAL", "/SearchMethod", ( int )p.getSearch(), &saved ) );
  mCandPoint = QgsProject::instance()->readNumEntry(
                 "PAL", "/CandidatesPoint", p.getPointP(), &saved );
  mCandLine = QgsProject::instance()->readNumEntry(
                "PAL", "/CandidatesLine", p.getLineP(), &saved );
  mCandPolygon = QgsProject::instance()->readNumEntry(
                   "PAL", "/CandidatesPolygon", p.getPolyP(), &saved );
  mShowingCandidates = QgsProject::instance()->readBoolEntry(
                         "PAL", "/ShowingCandidates", false, &saved );
  mShowingShadowRects = QgsProject::instance()->readBoolEntry(
                          "PAL", "/ShowingShadowRects", false, &saved );
  mShowingAllLabels = QgsProject::instance()->readBoolEntry(
                        "PAL", "/ShowingAllLabels", false, &saved );
  mShowingPartialsLabels = QgsProject::instance()->readBoolEntry(
                             "PAL", "/ShowingPartialsLabels", p.getShowPartial(), &saved );
  mDrawOutlineLabels = QgsProject::instance()->readBoolEntry(
                         "PAL", "/DrawOutlineLabels", true, &saved );
}

void QgsPalLabeling::saveEngineSettings()
{
  QgsProject::instance()->writeEntry( "PAL", "/SearchMethod", ( int )mSearch );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPoint", mCandPoint );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesLine", mCandLine );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPolygon", mCandPolygon );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingCandidates", mShowingCandidates );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingShadowRects", mShowingShadowRects );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingAllLabels", mShowingAllLabels );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingPartialsLabels", mShowingPartialsLabels );
  QgsProject::instance()->writeEntry( "PAL", "/DrawOutlineLabels", mDrawOutlineLabels );
}

void QgsPalLabeling::clearEngineSettings()
{
  QgsProject::instance()->removeEntry( "PAL", "/SearchMethod" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesPoint" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesLine" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesPolygon" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingCandidates" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingShadowRects" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingAllLabels" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingPartialsLabels" );
  QgsProject::instance()->removeEntry( "PAL", "/DrawOutlineLabels" );
}

QgsLabelingEngineInterface* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  lbl->mShowingShadowRects = mShowingShadowRects;
  lbl->mShowingPartialsLabels = mShowingPartialsLabels;
  lbl->mDrawOutlineLabels = mDrawOutlineLabels;
  return lbl;
}


QgsLabelingResults::QgsLabelingResults()
{
  mLabelSearchTree = new QgsLabelSearchTree();
}

QgsLabelingResults::~QgsLabelingResults()
{
  delete mLabelSearchTree;
  mLabelSearchTree = NULL;
}

QList<QgsLabelPosition> QgsLabelingResults::labelsAtPosition( const QgsPoint& p ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition*> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->label( p, positionPointers );
    QList<QgsLabelPosition*>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}

QList<QgsLabelPosition> QgsLabelingResults::labelsWithinRect( const QgsRectangle& r ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition*> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->labelsInRect( r, positionPointers );
    QList<QgsLabelPosition*>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}
