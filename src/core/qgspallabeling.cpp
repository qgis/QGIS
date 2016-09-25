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
#include "qgstextlabelfeature.h"
#include "qgsunittypes.h"
#include "qgscsexception.h"

#include <list>

#include <pal/pal.h>
#include <pal/feature.h>
#include <pal/layer.h>
#include <pal/palexception.h>
#include <pal/problem.h>
#include <pal/labelposition.h>

#include <cmath>

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>

#include "diagram/qgsdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsfontutils.h"
#include "qgslabelsearchtree.h"
#include "qgsexpression.h"
#include "qgsdatadefined.h"
#include "qgslabelingengine.h"
#include "qgsvectorlayerlabeling.h"

#include <qgslogger.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabelprovider.h>
#include <qgsgeometry.h>
#include <qgsmarkersymbollayer.h>
#include <qgspainting.h>
#include <qgsproject.h>
#include "qgssymbollayerutils.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include <QMessageBox>


Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter* p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale( static_cast< double >( qt_defaultDpiX() ) / p->device()->logicalDpiX(),
            static_cast< double >( qt_defaultDpiY() ) / p->device()->logicalDpiY() );
}


using namespace pal;

// -------------

/* ND: Default point label position priority. These are set to match variants of the ideal placement priority described
  in "Making Maps", Krygier & Wood (2011) (p216),
  "Elements of Cartography", Robinson et al (1995)
  and "Designing Better Maps", Brewer (2005) (p76)
  Note that while they agree on positions 1-4, 5-8 are more contentious so I've selected these placements
  based on my preferences, and to follow Krygier and Wood's placements more closer. (I'm not going to disagree
  with Denis Wood on anything cartography related...!)
*/
QVector< QgsPalLayerSettings::PredefinedPointPosition > QgsPalLayerSettings::DEFAULT_PLACEMENT_ORDER = QVector< QgsPalLayerSettings::PredefinedPointPosition >()
    << QgsPalLayerSettings::TopRight
    << QgsPalLayerSettings::TopLeft
    << QgsPalLayerSettings::BottomRight
    << QgsPalLayerSettings::BottomLeft
    << QgsPalLayerSettings::MiddleRight
    << QgsPalLayerSettings::MiddleLeft
    << QgsPalLayerSettings::TopSlightlyRight
    << QgsPalLayerSettings::BottomSlightlyRight;
//debugging only - don't use these placements by default
/* << QgsPalLayerSettings::TopSlightlyLeft
<< QgsPalLayerSettings::BottomSlightlyLeft;
<< QgsPalLayerSettings::TopMiddle
<< QgsPalLayerSettings::BottomMiddle;*/

QgsPalLayerSettings::QgsPalLayerSettings()
    : upsidedownLabels( Upright )
    , mCurFeat( nullptr )
    , xform( nullptr )
    , extentGeom( nullptr )
    , mFeaturesToLabel( 0 )
    , mFeatsSendingToPal( 0 )
    , mFeatsRegPal( 0 )
    , expression( nullptr )
{
  enabled = false;
  drawLabels = true;
  isExpression = false;
  fieldIndex = 0;

  previewBkgrdColor = Qt::white;
  useSubstitutions = false;

  // text formatting
  wrapChar = "";
  multilineAlign = MultiFollowPlacement;
  addDirectionSymbol = false;
  leftDirectionSymbol = QString( "<" );
  rightDirectionSymbol = QString( ">" );
  reverseDirectionSymbol = false;
  placeDirectionSymbol = SymbolLeftRight;
  formatNumbers = false;
  decimals = 3;
  plusSign = false;

  // placement
  placement = AroundPoint;
  placementFlags = AboveLine | MapOrientation;
  centroidWhole = false;
  centroidInside = false;
  predefinedPositionOrder = DEFAULT_PLACEMENT_ORDER;
  fitInPolygonOnly = false;
  quadOffset = QuadrantOver;
  xOffset = 0;
  yOffset = 0;
  labelOffsetInMapUnits = true;
  dist = 0;
  distInMapUnits = false;
  offsetType = FromPoint;
  angleOffset = 0;
  preserveRotation = true;
  maxCurvedCharAngleIn = 25.0;
  maxCurvedCharAngleOut = -25.0;
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
  obstacleFactor = 1.0;
  obstacleType = PolygonInterior;
  zIndex = 0.0;

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
  mDataDefinedNames.insert( Priority, QPair<QString, int>( "Priority", -1 ) );
  mDataDefinedNames.insert( IsObstacle, QPair<QString, int>( "IsObstacle", -1 ) );
  mDataDefinedNames.insert( ObstacleFactor, QPair<QString, int>( "ObstacleFactor", -1 ) );
  mDataDefinedNames.insert( PredefinedPositionOrder, QPair<QString, int>( "PredefinedPositionOrder", -1 ) );

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
  mDataDefinedNames.insert( ZIndex, QPair<QString, int>( "ZIndex", -1 ) );
  // (data defined only)
  mDataDefinedNames.insert( Show, QPair<QString, int>( "Show", 15 ) );
  mDataDefinedNames.insert( AlwaysShow, QPair<QString, int>( "AlwaysShow", 20 ) );

  // temp stuff for when drawing label components (don't copy)
  showingShadowRects = false;
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings& s )
    : mCurFeat( nullptr )
    , fieldIndex( 0 )
    , xform( nullptr )
    , extentGeom( nullptr )
    , mFeaturesToLabel( 0 )
    , mFeatsSendingToPal( 0 )
    , mFeatsRegPal( 0 )
    , showingShadowRects( false )
    , expression( nullptr )
{
  *this = s;
}

QgsPalLayerSettings& QgsPalLayerSettings::operator=( const QgsPalLayerSettings & s )
{
  if ( this == &s )
    return *this;

  // copy only permanent stuff

  enabled = s.enabled;
  drawLabels = s.drawLabels;

  // text style
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  previewBkgrdColor = s.previewBkgrdColor;
  substitutions = s.substitutions;
  useSubstitutions = s.useSubstitutions;

  // text formatting
  wrapChar = s.wrapChar;
  multilineAlign = s.multilineAlign;
  addDirectionSymbol = s.addDirectionSymbol;
  leftDirectionSymbol = s.leftDirectionSymbol;
  rightDirectionSymbol = s.rightDirectionSymbol;
  reverseDirectionSymbol = s.reverseDirectionSymbol;
  placeDirectionSymbol = s.placeDirectionSymbol;
  formatNumbers = s.formatNumbers;
  decimals = s.decimals;
  plusSign = s.plusSign;

  // placement
  placement = s.placement;
  placementFlags = s.placementFlags;
  centroidWhole = s.centroidWhole;
  centroidInside = s.centroidInside;
  predefinedPositionOrder = s.predefinedPositionOrder;
  fitInPolygonOnly = s.fitInPolygonOnly;
  quadOffset = s.quadOffset;
  xOffset = s.xOffset;
  yOffset = s.yOffset;
  labelOffsetInMapUnits = s.labelOffsetInMapUnits;
  labelOffsetMapUnitScale = s.labelOffsetMapUnitScale;
  dist = s.dist;
  offsetType = s.offsetType;
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
  obstacleFactor = s.obstacleFactor;
  obstacleType = s.obstacleType;
  zIndex = s.zIndex;

  mFormat = s.mFormat;

  // data defined
  removeAllDataDefinedProperties();
  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = s.dataDefinedProperties.constBegin();
  for ( ; it != s.dataDefinedProperties.constEnd(); ++it )
  {
    dataDefinedProperties.insert( it.key(), it.value() ? new QgsDataDefined( *it.value() ) : nullptr );
  }
  mDataDefinedNames = s.mDataDefinedNames;

  // scale factors
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;
  return *this;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
{
  // pal layer is deleted internally in PAL

  delete expression;

  // delete all QgsDataDefined objects (which also deletes their QgsExpression object)
  removeAllDataDefinedProperties();
}


QgsPalLayerSettings QgsPalLayerSettings::fromLayer( QgsVectorLayer* layer )
{
  QgsPalLayerSettings settings;
  settings.readFromLayer( layer );
  return settings;
}


QgsExpression* QgsPalLayerSettings::getLabelExpression()
{
  if ( !expression )
  {
    expression = new QgsExpression( fieldName );
  }
  return expression;
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

void QgsPalLayerSettings::readDataDefinedPropertyMap( QgsVectorLayer* layer, QDomElement* parentElem,
    QMap < QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > & propertyMap )
{
  if ( !layer && !parentElem )
  {
    return;
  }

  QMapIterator<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > i( mDataDefinedNames );
  while ( i.hasNext() )
  {
    i.next();
    if ( layer )
    {
      // reading from layer's custom properties (old way)
      readDataDefinedProperty( layer, i.key(), propertyMap );
    }
    else if ( parentElem )
    {
      // reading from XML (new way)
      QDomElement e = parentElem->firstChildElement( i.value().first );
      if ( !e.isNull() )
      {
        QgsDataDefined* dd = new QgsDataDefined();
        if ( dd->setFromXmlElement( e ) )
          propertyMap.insert( i.key(), dd );
        else
          delete dd;
      }
    }
  }
}

void QgsPalLayerSettings::writeDataDefinedPropertyMap( QgsVectorLayer* layer, QDomElement* parentElem,
    const QMap < QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > & propertyMap )
{
  if ( !layer && !parentElem )
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

        if ( parentElem )
        {
          // writing to XML document (instead of writing to layer)
          QDomDocument doc = parentElem->ownerDocument();
          QDomElement e = dd->toXmlElement( doc, i.value().first );
          parentElem->appendChild( e );
        }
      }
    }

    if ( layer )
    {
      // writing to layer's custom properties (old method)

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

  if ( !ddString.isEmpty() && ddString != QLatin1String( "0~~0~~~~" ) )
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
  if ( layer->customProperty( "labeling" ).toString() != QLatin1String( "pal" ) )
  {
    if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
      placement = OrderedPositionsAroundPoint;

    // for polygons the "over point" (over centroid) placement is better than the default
    // "around point" (around centroid) which is more suitable for points
    if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
      placement = OverPoint;

    return; // there's no information available
  }

  // NOTE: set defaults for newly added properties, for backwards compatibility

  enabled = layer->labelsEnabled();
  drawLabels = layer->customProperty( "labeling/drawLabels", true ).toBool();

  mFormat.readFromLayer( layer );

  // text style
  fieldName = layer->customProperty( "labeling/fieldName" ).toString();
  isExpression = layer->customProperty( "labeling/isExpression" ).toBool();
  previewBkgrdColor = QColor( layer->customProperty( "labeling/previewBkgrdColor", QVariant( "#ffffff" ) ).toString() );
  QDomDocument doc( "substitutions" );
  doc.setContent( layer->customProperty( "labeling/substitutions" ).toString() );
  QDomElement replacementElem = doc.firstChildElement( "substitutions" );
  substitutions.readXml( replacementElem );
  useSubstitutions = layer->customProperty( "labeling/useSubstitutions" ).toBool();

  // text formatting
  wrapChar = layer->customProperty( "labeling/wrapChar" ).toString();
  multilineAlign = static_cast< MultiLineAlign >( layer->customProperty( "labeling/multilineAlign", QVariant( MultiFollowPlacement ) ).toUInt() );
  addDirectionSymbol = layer->customProperty( "labeling/addDirectionSymbol" ).toBool();
  leftDirectionSymbol = layer->customProperty( "labeling/leftDirectionSymbol", QVariant( "<" ) ).toString();
  rightDirectionSymbol = layer->customProperty( "labeling/rightDirectionSymbol", QVariant( ">" ) ).toString();
  reverseDirectionSymbol = layer->customProperty( "labeling/reverseDirectionSymbol" ).toBool();
  placeDirectionSymbol = static_cast< DirectionSymbols >( layer->customProperty( "labeling/placeDirectionSymbol", QVariant( SymbolLeftRight ) ).toUInt() );
  formatNumbers = layer->customProperty( "labeling/formatNumbers" ).toBool();
  decimals = layer->customProperty( "labeling/decimals" ).toInt();
  plusSign = layer->customProperty( "labeling/plussign" ).toBool();

  // placement
  placement = static_cast< Placement >( layer->customProperty( "labeling/placement" ).toInt() );
  placementFlags = layer->customProperty( "labeling/placementFlags" ).toUInt();
  centroidWhole = layer->customProperty( "labeling/centroidWhole", QVariant( false ) ).toBool();
  centroidInside = layer->customProperty( "labeling/centroidInside", QVariant( false ) ).toBool();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( layer->customProperty( "labeling/predefinedPositionOrder" ).toString() );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = DEFAULT_PLACEMENT_ORDER;
  fitInPolygonOnly = layer->customProperty( "labeling/fitInPolygonOnly", QVariant( false ) ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  distInMapUnits = layer->customProperty( "labeling/distInMapUnits" ).toBool();
  if ( layer->customProperty( "labeling/distMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    distMapUnitScale.minScale = layer->customProperty( "labeling/distMapUnitMinScale", 0.0 ).toDouble();
    distMapUnitScale.maxScale = layer->customProperty( "labeling/distMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    distMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/distMapUnitScale" ).toString() );
  }
  offsetType = static_cast< OffsetType >( layer->customProperty( "labeling/offsetType", QVariant( FromPoint ) ).toUInt() );
  quadOffset = static_cast< QuadrantPosition >( layer->customProperty( "labeling/quadOffset", QVariant( QuadrantOver ) ).toUInt() );
  xOffset = layer->customProperty( "labeling/xOffset", QVariant( 0.0 ) ).toDouble();
  yOffset = layer->customProperty( "labeling/yOffset", QVariant( 0.0 ) ).toDouble();
  labelOffsetInMapUnits = layer->customProperty( "labeling/labelOffsetInMapUnits", QVariant( true ) ).toBool();
  if ( layer->customProperty( "labeling/labelOffsetMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    labelOffsetMapUnitScale.minScale = layer->customProperty( "labeling/labelOffsetMapUnitMinScale", 0.0 ).toDouble();
    labelOffsetMapUnitScale.maxScale = layer->customProperty( "labeling/labelOffsetMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    labelOffsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/labelOffsetMapUnitScale" ).toString() );
  }
  angleOffset = layer->customProperty( "labeling/angleOffset", QVariant( 0.0 ) ).toDouble();
  preserveRotation = layer->customProperty( "labeling/preserveRotation", QVariant( true ) ).toBool();
  maxCurvedCharAngleIn = layer->customProperty( "labeling/maxCurvedCharAngleIn", QVariant( 25.0 ) ).toDouble();
  maxCurvedCharAngleOut = layer->customProperty( "labeling/maxCurvedCharAngleOut", QVariant( -25.0 ) ).toDouble();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  repeatDistance = layer->customProperty( "labeling/repeatDistance", 0.0 ).toDouble();
  repeatDistanceUnit = static_cast< SizeUnit >( layer->customProperty( "labeling/repeatDistanceUnit", QVariant( MM ) ).toUInt() );
  if ( layer->customProperty( "labeling/repeatDistanceMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    repeatDistanceMapUnitScale.minScale = layer->customProperty( "labeling/repeatDistanceMapUnitMinScale", 0.0 ).toDouble();
    repeatDistanceMapUnitScale.maxScale = layer->customProperty( "labeling/repeatDistanceMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    repeatDistanceMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/repeatDistanceMapUnitScale" ).toString() );
  }

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
  upsidedownLabels = static_cast< UpsideDownLabels >( layer->customProperty( "labeling/upsidedownLabels", QVariant( Upright ) ).toUInt() );

  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
  minFeatureSize = layer->customProperty( "labeling/minFeatureSize" ).toDouble();
  limitNumLabels = layer->customProperty( "labeling/limitNumLabels", QVariant( false ) ).toBool();
  maxNumLabels = layer->customProperty( "labeling/maxNumLabels", QVariant( 2000 ) ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle", QVariant( true ) ).toBool();
  obstacleFactor = layer->customProperty( "labeling/obstacleFactor", QVariant( 1.0 ) ).toDouble();
  obstacleType = static_cast< ObstacleType >( layer->customProperty( "labeling/obstacleType", QVariant( PolygonInterior ) ).toUInt() );
  zIndex = layer->customProperty( "labeling/zIndex", QVariant( 0.0 ) ).toDouble();

  readDataDefinedPropertyMap( layer, nullptr, dataDefinedProperties );
}

void QgsPalLayerSettings::writeToLayer( QgsVectorLayer* layer )
{
  // this is a mark that labeling information is present
  layer->setCustomProperty( "labeling", "pal" );

  layer->setCustomProperty( "labeling/enabled", enabled );
  layer->setCustomProperty( "labeling/drawLabels", drawLabels );

  mFormat.writeToLayer( layer );

  // text style
  layer->setCustomProperty( "labeling/fieldName", fieldName );
  layer->setCustomProperty( "labeling/isExpression", isExpression );
  layer->setCustomProperty( "labeling/previewBkgrdColor", previewBkgrdColor.name() );
  QDomDocument doc( "substitutions" );
  QDomElement replacementElem = doc.createElement( "substitutions" );
  substitutions.writeXml( replacementElem, doc );
  QString replacementProps;
  QTextStream stream( &replacementProps );
  replacementElem.save( stream, -1 );
  layer->setCustomProperty( "labeling/substitutions", replacementProps );
  layer->setCustomProperty( "labeling/useSubstitutions", useSubstitutions );

  // text formatting
  layer->setCustomProperty( "labeling/wrapChar", wrapChar );
  layer->setCustomProperty( "labeling/multilineAlign", static_cast< unsigned int >( multilineAlign ) );
  layer->setCustomProperty( "labeling/addDirectionSymbol", addDirectionSymbol );
  layer->setCustomProperty( "labeling/leftDirectionSymbol", leftDirectionSymbol );
  layer->setCustomProperty( "labeling/rightDirectionSymbol", rightDirectionSymbol );
  layer->setCustomProperty( "labeling/reverseDirectionSymbol", reverseDirectionSymbol );
  layer->setCustomProperty( "labeling/placeDirectionSymbol", static_cast< unsigned int >( placeDirectionSymbol ) );
  layer->setCustomProperty( "labeling/formatNumbers", formatNumbers );
  layer->setCustomProperty( "labeling/decimals", decimals );
  layer->setCustomProperty( "labeling/plussign", plusSign );

  // placement
  layer->setCustomProperty( "labeling/placement", placement );
  layer->setCustomProperty( "labeling/placementFlags", static_cast< unsigned int >( placementFlags ) );
  layer->setCustomProperty( "labeling/centroidWhole", centroidWhole );
  layer->setCustomProperty( "labeling/centroidInside", centroidInside );
  layer->setCustomProperty( "labeling/predefinedPositionOrder", QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) );
  layer->setCustomProperty( "labeling/fitInPolygonOnly", fitInPolygonOnly );
  layer->setCustomProperty( "labeling/dist", dist );
  layer->setCustomProperty( "labeling/distInMapUnits", distInMapUnits );
  layer->setCustomProperty( "labeling/distMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( distMapUnitScale ) );
  layer->setCustomProperty( "labeling/offsetType", static_cast< unsigned int >( offsetType ) );
  layer->setCustomProperty( "labeling/quadOffset", static_cast< unsigned int >( quadOffset ) );
  layer->setCustomProperty( "labeling/xOffset", xOffset );
  layer->setCustomProperty( "labeling/yOffset", yOffset );
  layer->setCustomProperty( "labeling/labelOffsetInMapUnits", labelOffsetInMapUnits );
  layer->setCustomProperty( "labeling/labelOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( labelOffsetMapUnitScale ) );
  layer->setCustomProperty( "labeling/angleOffset", angleOffset );
  layer->setCustomProperty( "labeling/preserveRotation", preserveRotation );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleIn", maxCurvedCharAngleIn );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleOut", maxCurvedCharAngleOut );
  layer->setCustomProperty( "labeling/priority", priority );
  layer->setCustomProperty( "labeling/repeatDistance", repeatDistance );
  layer->setCustomProperty( "labeling/repeatDistanceUnit", repeatDistanceUnit );
  layer->setCustomProperty( "labeling/repeatDistanceMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( repeatDistanceMapUnitScale ) );

  // rendering
  layer->setCustomProperty( "labeling/scaleVisibility", scaleVisibility );
  layer->setCustomProperty( "labeling/scaleMin", scaleMin );
  layer->setCustomProperty( "labeling/scaleMax", scaleMax );
  layer->setCustomProperty( "labeling/fontLimitPixelSize", fontLimitPixelSize );
  layer->setCustomProperty( "labeling/fontMinPixelSize", fontMinPixelSize );
  layer->setCustomProperty( "labeling/fontMaxPixelSize", fontMaxPixelSize );
  layer->setCustomProperty( "labeling/displayAll", displayAll );
  layer->setCustomProperty( "labeling/upsidedownLabels", static_cast< unsigned int >( upsidedownLabels ) );

  layer->setCustomProperty( "labeling/labelPerPart", labelPerPart );
  layer->setCustomProperty( "labeling/mergeLines", mergeLines );
  layer->setCustomProperty( "labeling/minFeatureSize", minFeatureSize );
  layer->setCustomProperty( "labeling/limitNumLabels", limitNumLabels );
  layer->setCustomProperty( "labeling/maxNumLabels", maxNumLabels );
  layer->setCustomProperty( "labeling/obstacle", obstacle );
  layer->setCustomProperty( "labeling/obstacleFactor", obstacleFactor );
  layer->setCustomProperty( "labeling/obstacleType", static_cast< unsigned int >( obstacleType ) );
  layer->setCustomProperty( "labeling/zIndex", zIndex );

  writeDataDefinedPropertyMap( layer, nullptr, dataDefinedProperties );
  layer->emitStyleChanged();
}

void QgsPalLayerSettings::readXml( QDomElement& elem )
{
  enabled = true;
  drawLabels = true;

  // text style
  QDomElement textStyleElem = elem.firstChildElement( "text-style" );
  fieldName = textStyleElem.attribute( "fieldName" );
  isExpression = textStyleElem.attribute( "isExpression" ).toInt();

  mFormat.readXml( elem );
  previewBkgrdColor = QColor( textStyleElem.attribute( "previewBkgrdColor", "#ffffff" ) );
  substitutions.readXml( textStyleElem.firstChildElement( "substitutions" ) );
  useSubstitutions = textStyleElem.attribute( "useSubstitutions" ).toInt();

  // text formatting
  QDomElement textFormatElem = elem.firstChildElement( "text-format" );
  wrapChar = textFormatElem.attribute( "wrapChar" );
  multilineAlign = static_cast< MultiLineAlign >( textFormatElem.attribute( "multilineAlign", QString::number( MultiFollowPlacement ) ).toUInt() );
  addDirectionSymbol = textFormatElem.attribute( "addDirectionSymbol" ).toInt();
  leftDirectionSymbol = textFormatElem.attribute( "leftDirectionSymbol", "<" );
  rightDirectionSymbol = textFormatElem.attribute( "rightDirectionSymbol", ">" );
  reverseDirectionSymbol = textFormatElem.attribute( "reverseDirectionSymbol" ).toInt();
  placeDirectionSymbol = static_cast< DirectionSymbols >( textFormatElem.attribute( "placeDirectionSymbol", QString::number( SymbolLeftRight ) ).toUInt() );
  formatNumbers = textFormatElem.attribute( "formatNumbers" ).toInt();
  decimals = textFormatElem.attribute( "decimals" ).toInt();
  plusSign = textFormatElem.attribute( "plussign" ).toInt();

  // placement
  QDomElement placementElem = elem.firstChildElement( "placement" );
  placement = static_cast< Placement >( placementElem.attribute( "placement" ).toInt() );
  placementFlags = placementElem.attribute( "placementFlags" ).toUInt();
  centroidWhole = placementElem.attribute( "centroidWhole", "0" ).toInt();
  centroidInside = placementElem.attribute( "centroidInside", "0" ).toInt();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( placementElem.attribute( "predefinedPositionOrder" ) );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = DEFAULT_PLACEMENT_ORDER;
  fitInPolygonOnly = placementElem.attribute( "fitInPolygonOnly", "0" ).toInt();
  dist = placementElem.attribute( "dist" ).toDouble();
  distInMapUnits = placementElem.attribute( "distInMapUnits" ).toInt();
  if ( !placementElem.hasAttribute( "distMapUnitScale" ) )
  {
    //fallback to older property
    distMapUnitScale.minScale = placementElem.attribute( "distMapUnitMinScale", "0" ).toDouble();
    distMapUnitScale.maxScale = placementElem.attribute( "distMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    distMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( "distMapUnitScale" ) );
  }
  offsetType = static_cast< OffsetType >( placementElem.attribute( "offsetType", QString::number( FromPoint ) ).toUInt() );
  quadOffset = static_cast< QuadrantPosition >( placementElem.attribute( "quadOffset", QString::number( QuadrantOver ) ).toUInt() );
  xOffset = placementElem.attribute( "xOffset", "0" ).toDouble();
  yOffset = placementElem.attribute( "yOffset", "0" ).toDouble();
  labelOffsetInMapUnits = placementElem.attribute( "labelOffsetInMapUnits", "1" ).toInt();
  if ( !placementElem.hasAttribute( "labelOffsetMapUnitScale" ) )
  {
    //fallback to older property
    labelOffsetMapUnitScale.minScale = placementElem.attribute( "labelOffsetMapUnitMinScale", "0" ).toDouble();
    labelOffsetMapUnitScale.maxScale = placementElem.attribute( "labelOffsetMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    labelOffsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( "labelOffsetMapUnitScale" ) );
  }
  angleOffset = placementElem.attribute( "angleOffset", "0" ).toDouble();
  preserveRotation = placementElem.attribute( "preserveRotation", "1" ).toInt();
  maxCurvedCharAngleIn = placementElem.attribute( "maxCurvedCharAngleIn", "25" ).toDouble();
  maxCurvedCharAngleOut = placementElem.attribute( "maxCurvedCharAngleOut", "-25" ).toDouble();
  priority = placementElem.attribute( "priority" ).toInt();
  repeatDistance = placementElem.attribute( "repeatDistance", "0" ).toDouble();
  repeatDistanceUnit = static_cast< SizeUnit >( placementElem.attribute( "repeatDistanceUnit", QString::number( MM ) ).toUInt() );
  if ( !placementElem.hasAttribute( "repeatDistanceMapUnitScale" ) )
  {
    //fallback to older property
    repeatDistanceMapUnitScale.minScale = placementElem.attribute( "repeatDistanceMapUnitMinScale", "0" ).toDouble();
    repeatDistanceMapUnitScale.maxScale = placementElem.attribute( "repeatDistanceMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    repeatDistanceMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( "repeatDistanceMapUnitScale" ) );
  }

  // rendering
  QDomElement renderingElem = elem.firstChildElement( "rendering" );
  scaleMin = renderingElem.attribute( "scaleMin", "0" ).toInt();
  scaleMax = renderingElem.attribute( "scaleMax", "0" ).toInt();
  scaleVisibility = renderingElem.attribute( "scaleVisibility" ).toInt();

  fontLimitPixelSize = renderingElem.attribute( "fontLimitPixelSize", "0" ).toInt();
  fontMinPixelSize = renderingElem.attribute( "fontMinPixelSize", "0" ).toInt();
  fontMaxPixelSize = renderingElem.attribute( "fontMaxPixelSize", "10000" ).toInt();
  displayAll = renderingElem.attribute( "displayAll", "0" ).toInt();
  upsidedownLabels = static_cast< UpsideDownLabels >( renderingElem.attribute( "upsidedownLabels", QString::number( Upright ) ).toUInt() );

  labelPerPart = renderingElem.attribute( "labelPerPart" ).toInt();
  mergeLines = renderingElem.attribute( "mergeLines" ).toInt();
  minFeatureSize = renderingElem.attribute( "minFeatureSize" ).toDouble();
  limitNumLabels = renderingElem.attribute( "limitNumLabels", "0" ).toInt();
  maxNumLabels = renderingElem.attribute( "maxNumLabels", "2000" ).toInt();
  obstacle = renderingElem.attribute( "obstacle", "1" ).toInt();
  obstacleFactor = renderingElem.attribute( "obstacleFactor", "1" ).toDouble();
  obstacleType = static_cast< ObstacleType >( renderingElem.attribute( "obstacleType", QString::number( PolygonInterior ) ).toUInt() );
  zIndex = renderingElem.attribute( "zIndex", "0.0" ).toDouble();

  QDomElement ddElem = elem.firstChildElement( "data-defined" );
  readDataDefinedPropertyMap( nullptr, &ddElem, dataDefinedProperties );
}



QDomElement QgsPalLayerSettings::writeXml( QDomDocument& doc )
{
  // we assume (enabled == true && drawLabels == true) so those are not saved

  QDomElement textStyleElem = mFormat.writeXml( doc );

  // text style
  textStyleElem.setAttribute( "fieldName", fieldName );
  textStyleElem.setAttribute( "isExpression", isExpression );
  textStyleElem.setAttribute( "previewBkgrdColor", previewBkgrdColor.name() );
  QDomElement replacementElem = doc.createElement( "substitutions" );
  substitutions.writeXml( replacementElem, doc );
  textStyleElem.appendChild( replacementElem );
  textStyleElem.setAttribute( "useSubstitutions", useSubstitutions );

  // text formatting
  QDomElement textFormatElem = doc.createElement( "text-format" );
  textFormatElem.setAttribute( "wrapChar", wrapChar );
  textFormatElem.setAttribute( "multilineAlign", static_cast< unsigned int >( multilineAlign ) );
  textFormatElem.setAttribute( "addDirectionSymbol", addDirectionSymbol );
  textFormatElem.setAttribute( "leftDirectionSymbol", leftDirectionSymbol );
  textFormatElem.setAttribute( "rightDirectionSymbol", rightDirectionSymbol );
  textFormatElem.setAttribute( "reverseDirectionSymbol", reverseDirectionSymbol );
  textFormatElem.setAttribute( "placeDirectionSymbol", static_cast< unsigned int >( placeDirectionSymbol ) );
  textFormatElem.setAttribute( "formatNumbers", formatNumbers );
  textFormatElem.setAttribute( "decimals", decimals );
  textFormatElem.setAttribute( "plussign", plusSign );

  // placement
  QDomElement placementElem = doc.createElement( "placement" );
  placementElem.setAttribute( "placement", placement );
  placementElem.setAttribute( "placementFlags", static_cast< unsigned int >( placementFlags ) );
  placementElem.setAttribute( "centroidWhole", centroidWhole );
  placementElem.setAttribute( "centroidInside", centroidInside );
  placementElem.setAttribute( "predefinedPositionOrder", QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) );
  placementElem.setAttribute( "fitInPolygonOnly", fitInPolygonOnly );
  placementElem.setAttribute( "dist", dist );
  placementElem.setAttribute( "distInMapUnits", distInMapUnits );
  placementElem.setAttribute( "distMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( distMapUnitScale ) );
  placementElem.setAttribute( "offsetType", static_cast< unsigned int >( offsetType ) );
  placementElem.setAttribute( "quadOffset", static_cast< unsigned int >( quadOffset ) );
  placementElem.setAttribute( "xOffset", xOffset );
  placementElem.setAttribute( "yOffset", yOffset );
  placementElem.setAttribute( "labelOffsetInMapUnits", labelOffsetInMapUnits );
  placementElem.setAttribute( "labelOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( labelOffsetMapUnitScale ) );
  placementElem.setAttribute( "angleOffset", angleOffset );
  placementElem.setAttribute( "preserveRotation", preserveRotation );
  placementElem.setAttribute( "maxCurvedCharAngleIn", maxCurvedCharAngleIn );
  placementElem.setAttribute( "maxCurvedCharAngleOut", maxCurvedCharAngleOut );
  placementElem.setAttribute( "priority", priority );
  placementElem.setAttribute( "repeatDistance", repeatDistance );
  placementElem.setAttribute( "repeatDistanceUnit", repeatDistanceUnit );
  placementElem.setAttribute( "repeatDistanceMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( repeatDistanceMapUnitScale ) );

  // rendering
  QDomElement renderingElem = doc.createElement( "rendering" );
  renderingElem.setAttribute( "scaleVisibility", scaleVisibility );
  renderingElem.setAttribute( "scaleMin", scaleMin );
  renderingElem.setAttribute( "scaleMax", scaleMax );
  renderingElem.setAttribute( "fontLimitPixelSize", fontLimitPixelSize );
  renderingElem.setAttribute( "fontMinPixelSize", fontMinPixelSize );
  renderingElem.setAttribute( "fontMaxPixelSize", fontMaxPixelSize );
  renderingElem.setAttribute( "displayAll", displayAll );
  renderingElem.setAttribute( "upsidedownLabels", static_cast< unsigned int >( upsidedownLabels ) );

  renderingElem.setAttribute( "labelPerPart", labelPerPart );
  renderingElem.setAttribute( "mergeLines", mergeLines );
  renderingElem.setAttribute( "minFeatureSize", minFeatureSize );
  renderingElem.setAttribute( "limitNumLabels", limitNumLabels );
  renderingElem.setAttribute( "maxNumLabels", maxNumLabels );
  renderingElem.setAttribute( "obstacle", obstacle );
  renderingElem.setAttribute( "obstacleFactor", obstacleFactor );
  renderingElem.setAttribute( "obstacleType", static_cast< unsigned int >( obstacleType ) );
  renderingElem.setAttribute( "zIndex", zIndex );

  QDomElement ddElem = doc.createElement( "data-defined" );
  writeDataDefinedPropertyMap( nullptr, &ddElem, dataDefinedProperties );

  QDomElement elem = doc.createElement( "settings" );
  elem.appendChild( textStyleElem );
  elem.appendChild( textFormatElem );
  elem.appendChild( placementElem );
  elem.appendChild( renderingElem );
  elem.appendChild( ddElem );
  return elem;
}

void QgsPalLayerSettings::setDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p,
    bool active, bool useExpr, const QString& expr, const QString& field )
{
  bool defaultVals = ( !active && !useExpr && expr.isEmpty() && field.isEmpty() );

  if ( dataDefinedProperties.contains( p ) )
  {
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.constFind( p );
    if ( it != dataDefinedProperties.constEnd() )
    {
      QgsDataDefined* dd = it.value();
      dd->setActive( active );
      dd->setExpressionString( expr );
      dd->setField( field );
      dd->setUseExpression( useExpr );
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

void QgsPalLayerSettings::removeAllDataDefinedProperties()
{
  qDeleteAll( dataDefinedProperties );
  dataDefinedProperties.clear();
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
  if ( dataDefinedProperties.isEmpty() )
    return nullptr;

  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.constFind( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return nullptr;
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

QVariant QgsPalLayerSettings::dataDefinedValue( DataDefinedProperties p, QgsFeature& f, const QgsFields& fields, const QgsExpressionContext *context ) const
{
  if ( dataDefinedProperties.isEmpty() || !dataDefinedProperties.contains( p ) )
  {
    return QVariant();
  }

  //try to keep < 2.12 API - handle no passed expression context
  QScopedPointer< QgsExpressionContext > scopedEc;
  if ( !context )
  {
    scopedEc.reset( new QgsExpressionContext() );
    scopedEc->setFeature( f );
    scopedEc->setFields( fields );
  }
  const QgsExpressionContext* ec = context ? context : scopedEc.data();

  QgsDataDefined* dd = nullptr;
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

    result = expr->evaluate( ec );
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

bool QgsPalLayerSettings::dataDefinedEvaluate( DataDefinedProperties p, QVariant& exprVal, QgsExpressionContext *context, const QVariant& originalValue ) const
{
  // null passed-around QVariant
  exprVal.clear();
  if ( dataDefinedProperties.isEmpty() )
    return false;

  //try to keep < 2.12 API - handle no passed expression context
  QScopedPointer< QgsExpressionContext > scopedEc;
  if ( !context )
  {
    scopedEc.reset( new QgsExpressionContext() );
    scopedEc->setFeature( *mCurFeat );
    scopedEc->setFields( mCurFields );
  }
  QgsExpressionContext* ec = context ? context : scopedEc.data();

  ec->setOriginalValueVariable( originalValue );
  QVariant result = dataDefinedValue( p, *mCurFeat, mCurFields, ec );

  if ( result.isValid() && !result.isNull() )
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
  if ( dataDefinedProperties.isEmpty() )
    return false;

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
  if ( dataDefinedProperties.isEmpty() )
    return false;

  bool useExpression = false;
  QMap< DataDefinedProperties, QgsDataDefined* >::const_iterator it = dataDefinedProperties.find( p );
  if ( it != dataDefinedProperties.constEnd() )
  {
    useExpression = it.value()->useExpression();
  }

  return useExpression;
}

bool QgsPalLayerSettings::checkMinimumSizeMM( const QgsRenderContext& ct, const QgsGeometry& geom, double minSize ) const
{
  return QgsPalLabeling::checkMinimumSizeMM( ct, &geom, minSize );
}

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f, QgsRenderContext *context )
{
  if ( !fm || !f )
  {
    return;
  }

  //try to keep < 2.12 API - handle no passed render context
  QScopedPointer< QgsRenderContext > scopedRc;
  if ( !context )
  {
    scopedRc.reset( new QgsRenderContext() );
    if ( f )
      scopedRc->expressionContext().setFeature( *f );
  }
  QgsRenderContext* rc = context ? context : scopedRc.data();

  QString wrapchr = wrapChar;
  double multilineH = mFormat.lineHeight();

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
        placeDirSymb = static_cast< QgsPalLayerSettings::DirectionSymbols >( dataDefinedValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt() );
      }

    }

  }
  else // called externally with passed-in feature, evaluate data defined
  {
    rc->expressionContext().setOriginalValueVariable( wrapChar );
    QVariant exprVal = dataDefinedValue( QgsPalLayerSettings::MultiLineWrapChar, *f, mCurFields, &rc->expressionContext() );
    if ( exprVal.isValid() )
    {
      wrapchr = exprVal.toString();
    }
    exprVal.clear();
    rc->expressionContext().setOriginalValueVariable( multilineH );
    exprVal = dataDefinedValue( QgsPalLayerSettings::MultiLineHeight, *f, mCurFields, &rc->expressionContext() );
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
    rc->expressionContext().setOriginalValueVariable( addDirSymb );
    exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbDraw, *f, mCurFields, &rc->expressionContext() );
    if ( exprVal.isValid() )
    {
      addDirSymb = exprVal.toBool();
    }

    if ( addDirSymb ) // don't do extra evaluations if not adding a direction symbol
    {
      exprVal.clear();
      rc->expressionContext().setOriginalValueVariable( leftDirSymb );
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbLeft, *f, mCurFields, &rc->expressionContext() );
      if ( exprVal.isValid() )
      {
        leftDirSymb = exprVal.toString();
      }
      exprVal.clear();
      rc->expressionContext().setOriginalValueVariable( rightDirSymb );
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbRight, *f, mCurFields, &rc->expressionContext() );
      if ( exprVal.isValid() )
      {
        rightDirSymb = exprVal.toString();
      }
      exprVal.clear();
      rc->expressionContext().setOriginalValueVariable( static_cast< int >( placeDirSymb ) );
      exprVal = dataDefinedValue( QgsPalLayerSettings::DirSymbPlacement, *f, mCurFields, &rc->expressionContext() );
      if ( exprVal.isValid() )
      {
        bool ok;
        int enmint = exprVal.toInt( &ok );
        if ( ok )
        {
          placeDirSymb = static_cast< QgsPalLayerSettings::DirectionSymbols >( enmint );
        }
      }
    }

  }

  if ( wrapchr.isEmpty() )
  {
    wrapchr = QLatin1String( "\n" ); // default to new line delimiter
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
      text.prepend( dirSym + QLatin1String( "\n" ) ); // SymbolAbove or SymbolBelow
    }
  }

  double w = 0.0, h = 0.0;
  QStringList multiLineSplit = QgsPalLabeling::splitToLines( text, wrapchr );
  int lines = multiLineSplit.size();

  double labelHeight = fm->ascent() + fm->descent(); // ignore +1 for baseline

  h += fm->height() + static_cast< double >(( lines - 1 ) * labelHeight * multilineH );
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

void QgsPalLayerSettings::registerFeature( QgsFeature& f, QgsRenderContext &context, QgsLabelFeature** labelFeature , QgsGeometry* obstacleGeometry )
{
  // either used in QgsPalLabeling (palLayer is set) or in QgsLabelingEngine (labelFeature is set)
  Q_ASSERT( labelFeature );

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful
  mCurFeat = &f;

  // data defined is obstacle? calculate this first, to avoid wasting time working with obstacles we don't require
  bool isObstacle = obstacle; // start with layer default
  if ( dataDefinedEvaluate( QgsPalLayerSettings::IsObstacle, exprVal, &context.expressionContext(), obstacle ) )
  {
    isObstacle = exprVal.toBool();
  }

  if ( !drawLabels )
  {
    if ( isObstacle )
    {
      registerObstacleFeature( f, context, labelFeature, obstacleGeometry );
    }
    return;
  }

//  mCurFields = &layer->pendingFields();

  // store data defined-derived values for later adding to label feature for use during rendering
  dataDefinedValues.clear();

  // data defined show label? defaults to show label if not 0
  if ( dataDefinedIsActive( QgsPalLayerSettings::Show ) )
  {
    bool showLabel = dataDefinedEvaluate( QgsPalLayerSettings::Show, exprVal, &context.expressionContext(), true );
    showLabel &= exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Show:%1" ).arg( showLabel ? "true" : "false" ), 4 );
    if ( !showLabel )
    {
      return;
    }
  }

  // data defined scale visibility?
  bool useScaleVisibility = scaleVisibility;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ScaleVisibility, exprVal, &context.expressionContext(), scaleVisibility ) )
  {
    QgsDebugMsgLevel( QString( "exprVal ScaleVisibility:%1" ).arg( exprVal.toBool() ? "true" : "false" ), 4 );
    useScaleVisibility = exprVal.toBool();
  }

  if ( useScaleVisibility )
  {
    // data defined min scale?
    double minScale = scaleMin;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::MinScale, exprVal, &context.expressionContext(), scaleMin ) )
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

    if ( !qgsDoubleNear( minScale, 0.0 ) && context.rendererScale() < minScale )
    {
      return;
    }

    // data defined max scale?
    double maxScale = scaleMax;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::MaxScale, exprVal, &context.expressionContext(), scaleMax ) )
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

    if ( !qgsDoubleNear( maxScale, 0.0 ) && context.rendererScale() > maxScale )
    {
      return;
    }
  }

  QFont labelFont = mFormat.font();
  // labelFont will be added to label feature for use during label painting

  // data defined font units?
  QgsUnitTypes::RenderUnit fontunits = mFormat.sizeUnit();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontSizeUnit, exprVal, &context.expressionContext() ) )
  {
    QString units = exprVal.toString();
    QgsDebugMsgLevel( QString( "exprVal Font units:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        fontunits = res;
    }
  }

  //data defined label size?
  double fontSize = mFormat.size();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Size, exprVal, &context.expressionContext(), fontSize ) )
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

  int fontPixelSize = QgsTextRenderer::sizeToPixel( fontSize, context, fontunits, true, mFormat.sizeMapUnitScale() );
  // don't try to show font sizes less than 1 pixel (Qt complains)
  if ( fontPixelSize < 1 )
  {
    return;
  }
  labelFont.setPixelSize( fontPixelSize );

  // NOTE: labelFont now always has pixelSize set, so pointSize or pointSizeF might return -1

  // defined 'minimum/maximum pixel font size'?
  if ( fontunits == QgsUnitTypes::RenderMapUnits )
  {
    bool useFontLimitPixelSize = fontLimitPixelSize;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::FontLimitPixel, exprVal, &context.expressionContext(), fontLimitPixelSize ) )
    {
      QgsDebugMsgLevel( QString( "exprVal FontLimitPixel:%1" ).arg( exprVal.toBool() ? "true" : "false" ), 4 );
      useFontLimitPixelSize = exprVal.toBool();
    }

    if ( useFontLimitPixelSize )
    {
      int fontMinPixel = fontMinPixelSize;
      if ( dataDefinedEvaluate( QgsPalLayerSettings::FontMinPixel, exprVal, &context.expressionContext(), fontMinPixelSize ) )
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
      if ( dataDefinedEvaluate( QgsPalLayerSettings::FontMaxPixel, exprVal, &context.expressionContext(), fontMaxPixelSize ) )
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
  labelFont.setCapitalization( QFont::MixedCase ); // reset this - we don't use QFont's handling as it breaks with curved labels
  parseTextStyle( labelFont, fontunits, context );
  parseTextFormatting( context );
  parseTextBuffer( context );
  parseShapeBackground( context );
  parseDropShadow( context );

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

    QVariant result = exp->evaluate( &context.expressionContext() ); // expression prepared in QgsPalLabeling::prepareLayer()
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

  // apply text replacements
  if ( useSubstitutions )
  {
    labelText = substitutions.process( labelText );
  }

  // apply capitalization
  QgsStringUtils::Capitalization capitalization = QgsStringUtils::MixedCase;
  // maintain API - capitalization may have been set in textFont
  if ( mFormat.font().capitalization() != QFont::MixedCase )
  {
    capitalization = static_cast< QgsStringUtils::Capitalization >( mFormat.font().capitalization() );
  }
  // data defined font capitalization?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontCase, exprVal, &context.expressionContext() ) )
  {
    QString fcase = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal FontCase:%1" ).arg( fcase ), 4 );

    if ( !fcase.isEmpty() )
    {
      if ( fcase.compare( "NoChange", Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::MixedCase;
      }
      else if ( fcase.compare( "Upper", Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::AllUppercase;
      }
      else if ( fcase.compare( "Lower", Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::AllLowercase;
      }
      else if ( fcase.compare( "Capitalize", Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::ForceFirstLetterToCapital;
      }
    }
  }
  labelText = QgsStringUtils::capitalize( labelText, capitalization );

  // data defined format numbers?
  bool formatnum = formatNumbers;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::NumFormat, exprVal, &context.expressionContext(), formatNumbers ) )
  {
    formatnum = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal NumFormat:%1" ).arg( formatnum ? "true" : "false" ), 4 );
  }

  // format number if label text is coercible to a number
  if ( formatnum )
  {
    // data defined decimal places?
    int decimalPlaces = decimals;
    if ( dataDefinedEvaluate( QgsPalLayerSettings::NumDecimals, exprVal, &context.expressionContext(), decimals ) )
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
    if ( dataDefinedEvaluate( QgsPalLayerSettings::NumPlusSign, exprVal, &context.expressionContext(), plusSign ) )
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
        numberFormat.append( '+' );
      }
      numberFormat.append( "%1" );
      labelText = numberFormat.arg( d, 0, 'f', decimalPlaces );
    }
  }

  // NOTE: this should come AFTER any option that affects font metrics
  QScopedPointer<QFontMetricsF> labelFontMetrics( new QFontMetricsF( labelFont ) );
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelFontMetrics.data(), labelText, labelX, labelY, mCurFeat, &context );


  // maximum angle between curved label characters (hardcoded defaults used in QGIS <2.0)
  //
  double maxcharanglein = 20.0; // range 20.0-60.0
  double maxcharangleout = -20.0; // range 20.0-95.0

  if ( placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    maxcharanglein = maxCurvedCharAngleIn;
    maxcharangleout = maxCurvedCharAngleOut;

    //data defined maximum angle between curved label characters?
    if ( dataDefinedEvaluate( QgsPalLayerSettings::CurvedCharAngleInOut, exprVal, &context.expressionContext() ) )
    {
      QString ptstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QString( "exprVal CurvedCharAngleInOut:%1" ).arg( ptstr ), 4 );

      if ( !ptstr.isEmpty() )
      {
        QPointF maxcharanglePt = QgsSymbolLayerUtils::decodePoint( ptstr );
        maxcharanglein = qBound( 20.0, static_cast< double >( maxcharanglePt.x() ), 60.0 );
        maxcharangleout = qBound( 20.0, static_cast< double >( maxcharanglePt.y() ), 95.0 );
      }
    }
    // make sure maxcharangleout is always negative
    maxcharangleout = -( qAbs( maxcharangleout ) );
  }

  // data defined centroid whole or clipped?
  bool wholeCentroid = centroidWhole;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::CentroidWhole, exprVal, &context.expressionContext(), centroidWhole ) )
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

  QgsGeometry geom = f.geometry();
  if ( geom.isEmpty() )
  {
    return;
  }

  // simplify?
  const QgsVectorSimplifyMethod &simplifyMethod = context.vectorSimplifyMethod();
  QScopedPointer<QgsGeometry> scopedClonedGeom;
  if ( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( simplifyMethod.simplifyAlgorithm() );
    QgsGeometry g = geom;
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  // whether we're going to create a centroid for polygon
  bool centroidPoly = (( placement == QgsPalLayerSettings::AroundPoint
                         || placement == QgsPalLayerSettings::OverPoint )
                       && geom.type() == QgsWkbTypes::PolygonGeometry );

  // CLIP the geometry if it is bigger than the extent
  // don't clip if centroid is requested for whole feature
  bool doClip = false;
  if ( !centroidPoly || !wholeCentroid )
  {
    doClip = true;
  }

  // if using fitInPolygonOnly option, generate the permissible zone (must happen before geometry is modified - eg
  // as a result of using perimeter based labeling and the geometry is converted to a boundary)
  QgsGeometry permissibleZone;
  if ( geom.type() == QgsWkbTypes::PolygonGeometry && fitInPolygonOnly )
  {
    permissibleZone = geom;
    if ( QgsPalLabeling::geometryRequiresPreparation( permissibleZone, context, ct, doClip ? &extentGeom : nullptr ) )
    {
      permissibleZone = QgsPalLabeling::prepareGeometry( permissibleZone, context, ct, doClip ? &extentGeom : nullptr );
    }
  }

  // if using perimeter based labeling for polygons, get the polygon's
  // linear boundary and use that for the label geometry
  if (( geom.type() == QgsWkbTypes::PolygonGeometry )
      && ( placement == Line || placement == PerimeterCurved ) )
  {
    geom = QgsGeometry( geom.geometry()->boundary() );
  }

  const GEOSGeometry* geos_geom = nullptr;
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, doClip ? &extentGeom : nullptr ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, doClip ? &extentGeom : nullptr );

    if ( geom.isEmpty() )
      return;
  }
  geos_geom = geom.asGeos();

  const GEOSGeometry* geosObstacleGeom = nullptr;
  QScopedPointer<QgsGeometry> scopedObstacleGeom;
  if ( isObstacle )
  {
    if ( obstacleGeometry && QgsPalLabeling::geometryRequiresPreparation( *obstacleGeometry, context, ct, doClip ? &extentGeom : nullptr ) )
    {
      scopedObstacleGeom.reset( new QgsGeometry( QgsPalLabeling::prepareGeometry( *obstacleGeometry, context, ct, doClip ? &extentGeom : nullptr ) ) );
      obstacleGeometry = scopedObstacleGeom.data();
    }
    if ( obstacleGeometry )
    {
      geosObstacleGeom = obstacleGeometry->asGeos();
    }
  }

  if ( minFeatureSize > 0 && !checkMinimumSizeMM( context, geom, minFeatureSize ) )
    return;

  if ( !geos_geom )
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
    if ( mFeatsRegPal >= maxNumLabels )
    {
      return;
    }

    int divNum = static_cast< int >(( static_cast< double >( mFeaturesToLabel ) / maxNumLabels ) + 0.5 );
    if ( divNum && ( mFeatsRegPal == static_cast< int >( mFeatsSendingToPal / divNum ) ) )
    {
      mFeatsSendingToPal += 1;
      if ( divNum &&  mFeatsSendingToPal % divNum )
      {
        return;
      }
    }
  }

  GEOSGeometry* geos_geom_clone = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom );
  GEOSGeometry* geosObstacleGeomClone = nullptr;
  if ( geosObstacleGeom )
  {
    geosObstacleGeomClone = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geosObstacleGeom );
  }


  //data defined position / alignment / rotation?
  bool dataDefinedPosition = false;
  bool layerDefinedRotation = false;
  bool dataDefinedRotation = false;
  double xPos = 0.0, yPos = 0.0, angle = 0.0;
  bool ddXPos = false, ddYPos = false;
  double quadOffsetX = 0.0, quadOffsetY = 0.0;
  double offsetX = 0.0, offsetY = 0.0;

  //data defined quadrant offset?
  bool ddFixedQuad = false;
  QuadrantPosition quadOff = quadOffset;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetQuad, exprVal, &context.expressionContext(), static_cast< int >( quadOff ) ) )
  {
    bool ok;
    int quadInt = exprVal.toInt( &ok );
    QgsDebugMsgLevel( QString( "exprVal OffsetQuad:%1" ).arg( quadInt ), 4 );
    if ( ok && 0 <= quadInt && quadInt <= 8 )
    {
      quadOff = static_cast< QuadrantPosition >( quadInt );
      ddFixedQuad = true;
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetXY, exprVal, &context.expressionContext(), QgsSymbolLayerUtils::encodePoint( QPointF( xOffset, yOffset ) ) ) )
  {
    QString ptstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal OffsetXY:%1" ).arg( ptstr ), 4 );

    if ( !ptstr.isEmpty() )
    {
      QPointF ddOffPt = QgsSymbolLayerUtils::decodePoint( ptstr );
      xOff = ddOffPt.x();
      yOff = ddOffPt.y();
    }
  }

  // data defined label offset units?
  bool offinmapunits = labelOffsetInMapUnits;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::OffsetUnits, exprVal, &context.expressionContext() ) )
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
  if ( !qgsDoubleNear( xOff, 0.0 ) )
  {
    offsetX = xOff;  // must be positive to match symbology offset direction
    if ( !offinmapunits )
    {
      offsetX *= mapUntsPerMM; //convert offset from mm to map units
    }
  }
  if ( !qgsDoubleNear( yOff, 0.0 ) )
  {
    offsetY = -yOff; // must be negative to match symbology offset direction
    if ( !offinmapunits )
    {
      offsetY *= mapUntsPerMM; //convert offset from mm to map units
    }
  }

  // layer defined rotation?
  // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
  if ( placement == QgsPalLayerSettings::OverPoint && !qgsDoubleNear( angleOffset, 0.0 ) )
  {
    layerDefinedRotation = true;
    angle = angleOffset * M_PI / 180; // convert to radians
  }

  const QgsMapToPixel& m2p = context.mapToPixel();
  //data defined rotation?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Rotation, exprVal, &context.expressionContext(), angleOffset ) )
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

  if ( dataDefinedEvaluate( QgsPalLayerSettings::PositionX, exprVal, &context.expressionContext() ) )
  {
    if ( !exprVal.isNull() )
      xPos = exprVal.toDouble( &ddXPos );
    QgsDebugMsgLevel( QString( "exprVal PositionX:%1" ).arg( xPos ), 4 );

    if ( dataDefinedEvaluate( QgsPalLayerSettings::PositionY, exprVal, &context.expressionContext() ) )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      if ( !exprVal.isNull() )
        yPos = exprVal.toDouble( &ddYPos );
      QgsDebugMsgLevel( QString( "exprVal PositionY:%1" ).arg( yPos ), 4 );

      if ( ddXPos && ddYPos )
      {
        dataDefinedPosition = true;
        // layer rotation set, but don't rotate pinned labels unless data defined
        if ( layerDefinedRotation && !dataDefinedRotation )
        {
          angle = 0.0;
        }

        //x/y shift in case of alignment
        double xdiff = 0.0;
        double ydiff = 0.0;

        //horizontal alignment
        if ( dataDefinedEvaluate( QgsPalLayerSettings::Hali, exprVal, &context.expressionContext() ) )
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
        if ( dataDefinedEvaluate( QgsPalLayerSettings::Vali, exprVal, &context.expressionContext() ) )
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

        //project xPos and yPos from layer to map CRS, handle rotation
        QgsGeometry ddPoint( new QgsPointV2( xPos, yPos ) );
        if ( QgsPalLabeling::geometryRequiresPreparation( ddPoint, context, ct ) )
        {
          ddPoint = QgsPalLabeling::prepareGeometry( ddPoint, context, ct );
          xPos = static_cast< QgsPointV2* >( ddPoint.geometry() )->x();
          yPos = static_cast< QgsPointV2* >( ddPoint.geometry() )->y();
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::AlwaysShow, exprVal, &context.expressionContext() ) )
  {
    alwaysShow = exprVal.toBool();
  }

  // set repeat distance
  // data defined repeat distance?
  double repeatDist = repeatDistance;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::RepeatDistance, exprVal, &context.expressionContext(), repeatDistance ) )
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::RepeatDistanceUnit, exprVal, &context.expressionContext() ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal RepeatDistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      repeatdistinmapunit = ( _decodeUnits( units ) == QgsPalLayerSettings::MapUnits );
    }
  }

  if ( !qgsDoubleNear( repeatDist, 0.0 ) )
  {
    if ( !repeatdistinmapunit )
    {
      repeatDist *= mapUntsPerMM; //convert repeat distance from mm to map units
    }
  }

  //  feature to the layer
  QgsTextLabelFeature* lf = new QgsTextLabelFeature( f.id(), geos_geom_clone, QSizeF( labelX, labelY ) );
  mFeatsRegPal++;

  *labelFeature = lf;
  ( *labelFeature )->setHasFixedPosition( dataDefinedPosition );
  ( *labelFeature )->setFixedPosition( QgsPoint( xPos, yPos ) );
  // use layer-level defined rotation, but not if position fixed
  ( *labelFeature )->setHasFixedAngle( dataDefinedRotation || ( !dataDefinedPosition && !qgsDoubleNear( angle, 0.0 ) ) );
  ( *labelFeature )->setFixedAngle( angle );
  ( *labelFeature )->setQuadOffset( QPointF( quadOffsetX, quadOffsetY ) );
  ( *labelFeature )->setPositionOffset( QgsPoint( offsetX, offsetY ) );
  ( *labelFeature )->setOffsetType( offsetType );
  ( *labelFeature )->setAlwaysShow( alwaysShow );
  ( *labelFeature )->setRepeatDistance( repeatDist );
  ( *labelFeature )->setLabelText( labelText );
  ( *labelFeature )->setPermissibleZone( permissibleZone );
  if ( geosObstacleGeomClone )
  {
    ( *labelFeature )->setObstacleGeometry( geosObstacleGeomClone );

    if ( geom.type() == QgsWkbTypes::PointGeometry )
    {
      //register symbol size
      ( *labelFeature )->setSymbolSize( QSizeF( obstacleGeometry->boundingBox().width(),
                                        obstacleGeometry->boundingBox().height() ) );
    }
  }

  //set label's visual margin so that top visual margin is the leading, and bottom margin is the font's descent
  //this makes labels align to the font's baseline or highest character
  double topMargin = qMax( 0.25 * labelFontMetrics->ascent(), 0.0 );
  double bottomMargin = 1.0 + labelFontMetrics->descent();
  QgsLabelFeature::VisualMargin vm( topMargin, 0.0, bottomMargin, 0.0 );
  vm *= xform->mapUnitsPerPixel() / rasterCompressFactor;
  ( *labelFeature )->setVisualMargin( vm );

  // store the label's calculated font for later use during painting
  QgsDebugMsgLevel( QString( "PAL font stored definedFont: %1, Style: %2" ).arg( labelFont.toString(), labelFont.styleName() ), 4 );
  lf->setDefinedFont( labelFont );

  // TODO: only for placement which needs character info
  // account for any data defined font metrics adjustments
  lf->calculateInfo( placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved,
                     labelFontMetrics.data(), xform, rasterCompressFactor, maxcharanglein, maxcharangleout );
  // for labelFeature the LabelInfo is passed to feat when it is registered

  // TODO: allow layer-wide feature dist in PAL...?

  // data defined label-feature distance?
  double distance = dist;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::LabelDistance, exprVal, &context.expressionContext(), dist ) )
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::DistanceUnits, exprVal, &context.expressionContext() ) )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal DistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      distinmapunit = ( _decodeUnits( units ) == QgsPalLayerSettings::MapUnits );
    }
  }

  if ( distinmapunit ) //convert distance from mm/map units to pixels
  {
    distance /= distMapUnitScale.computeMapUnitsPerPixel( context );
  }
  else //mm
  {
    distance *= vectorScaleFactor;
  }

  // when using certain placement modes, we force a tiny minimum distance. This ensures that
  // candidates are created just offset from a border and avoids candidates being incorrectly flagged as colliding with neighbours
  if ( placement == QgsPalLayerSettings::Line || placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    distance = qMax( distance, 1.0 );
  }

  if ( !qgsDoubleNear( distance, 0.0 ) )
  {
    double d = ptOne.distance( ptZero ) * distance;
    ( *labelFeature )->setDistLabel( d );
  }

  if ( ddFixedQuad )
  {
    ( *labelFeature )->setHasFixedQuadrant( true );
  }

  // data defined z-index?
  double z = zIndex;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ZIndex, exprVal, &context.expressionContext(), zIndex ) )
  {
    bool ok;
    double zIndexD = exprVal.toDouble( &ok );
    if ( ok )
    {
      z = zIndexD;
    }
  }
  ( *labelFeature )->setZIndex( z );

  // data defined priority?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Priority, exprVal, &context.expressionContext(), priority ) )
  {
    bool ok;
    double priorityD = exprVal.toDouble( &ok );
    if ( ok )
    {
      priorityD = qBound( 0.0, priorityD, 10.0 );
      priorityD = 1 - priorityD / 10.0; // convert 0..10 --> 1..0
      ( *labelFeature )->setPriority( priorityD );
    }
  }

  ( *labelFeature )->setIsObstacle( isObstacle );

  double featObstacleFactor = obstacleFactor;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ObstacleFactor, exprVal, &context.expressionContext(), obstacleFactor ) )
  {
    bool ok;
    double factorD = exprVal.toDouble( &ok );
    if ( ok )
    {
      factorD = qBound( 0.0, factorD, 10.0 );
      factorD = factorD / 5.0 + 0.0001; // convert 0 -> 10 to 0.0001 -> 2.0
      featObstacleFactor = factorD;
    }
  }
  ( *labelFeature )->setObstacleFactor( featObstacleFactor );

  QVector< QgsPalLayerSettings::PredefinedPointPosition > positionOrder = predefinedPositionOrder;
  if ( positionOrder.isEmpty() )
    positionOrder = QgsPalLayerSettings::DEFAULT_PLACEMENT_ORDER;

  if ( dataDefinedEvaluate( QgsPalLayerSettings::PredefinedPositionOrder, exprVal, &context.expressionContext(), QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) ) )
  {
    QString orderD = exprVal.toString();
    positionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( orderD );
  }
  ( *labelFeature )->setPredefinedPositionOrder( positionOrder );

  // add parameters for data defined labeling to label feature
  lf->setDataDefinedValues( dataDefinedValues );
}

void QgsPalLayerSettings::registerObstacleFeature( QgsFeature& f, QgsRenderContext &context, QgsLabelFeature** obstacleFeature, QgsGeometry* obstacleGeometry )
{
  mCurFeat = &f;

  QgsGeometry geom;
  if ( obstacleGeometry )
  {
    geom = *obstacleGeometry;
  }
  else
  {
    geom = f.geometry();
  }

  if ( geom.isEmpty() )
  {
    return;
  }

  // simplify?
  const QgsVectorSimplifyMethod &simplifyMethod = context.vectorSimplifyMethod();
  QScopedPointer<QgsGeometry> scopedClonedGeom;
  if ( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( simplifyMethod.simplifyAlgorithm() );
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  const GEOSGeometry* geos_geom = nullptr;
  QScopedPointer<QgsGeometry> scopedPreparedGeom;

  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, &extentGeom ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, &extentGeom );
  }
  geos_geom = geom.asGeos();

  if ( !geos_geom )
    return; // invalid geometry

  GEOSGeometry* geos_geom_clone;
  geos_geom_clone = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom );

  //  feature to the layer
  *obstacleFeature = new QgsLabelFeature( f.id(), geos_geom_clone, QSizeF( 0, 0 ) );
  ( *obstacleFeature )->setIsObstacle( true );
  mFeatsRegPal++;
}

bool QgsPalLayerSettings::dataDefinedValEval( DataDefinedValueType valType,
    QgsPalLayerSettings::DataDefinedProperties p,
    QVariant& exprVal, QgsExpressionContext& context, const QVariant& originalValue )
{
  if ( dataDefinedEvaluate( p, exprVal, &context, originalValue ) )
  {
#ifdef QGISDEBUG
    QString dbgStr = QString( "exprVal %1:" ).arg( mDataDefinedNames.value( p ).first ) + "%1"; // clazy:exclude=unused-non-trivial-variable
#endif

    switch ( valType )
    {
      case DDBool:
      {
        bool bol = exprVal.toBool();
        QgsDebugMsgLevel( dbgStr.arg( bol ? "true" : "false" ), 4 );
        dataDefinedValues.insert( p, QVariant( bol ) );
        return true;
      }
      case DDInt:
      {
        bool ok;
        int size = exprVal.toInt( &ok );
        QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

        if ( ok )
        {
          dataDefinedValues.insert( p, QVariant( size ) );
          return true;
        }
        return false;
      }
      case DDIntPos:
      {
        bool ok;
        int size = exprVal.toInt( &ok );
        QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

        if ( ok && size > 0 )
        {
          dataDefinedValues.insert( p, QVariant( size ) );
          return true;
        }
        return false;
      }
      case DDDouble:
      {
        bool ok;
        double size = exprVal.toDouble( &ok );
        QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

        if ( ok )
        {
          dataDefinedValues.insert( p, QVariant( size ) );
          return true;
        }
        return false;
      }
      case DDDoublePos:
      {
        bool ok;
        double size = exprVal.toDouble( &ok );
        QgsDebugMsgLevel( dbgStr.arg( size ), 4 );

        if ( ok && size > 0.0 )
        {
          dataDefinedValues.insert( p, QVariant( size ) );
          return true;
        }
        return false;
      }
      case DDRotation180:
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
        return false;
      }
      case DDTransparency:
      {
        bool ok;
        int size = exprVal.toInt( &ok );
        QgsDebugMsgLevel( dbgStr.arg( size ), 4 );
        if ( ok && size >= 0 && size <= 100 )
        {
          dataDefinedValues.insert( p, QVariant( size ) );
          return true;
        }
        return false;
      }
      case DDString:
      {
        QString str = exprVal.toString(); // don't trim whitespace
        QgsDebugMsgLevel( dbgStr.arg( str ), 4 );

        dataDefinedValues.insert( p, QVariant( str ) ); // let it stay empty if it is
        return true;
      }
      case DDUnits:
      {
        QString unitstr = exprVal.toString().trimmed();
        QgsDebugMsgLevel( dbgStr.arg( unitstr ), 4 );

        if ( !unitstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( static_cast< int >( QgsUnitTypes::decodeRenderUnit( unitstr ) ) ) );
          return true;
        }
        return false;
      }
      case DDColor:
      {
        QString colorstr = exprVal.toString().trimmed();
        QgsDebugMsgLevel( dbgStr.arg( colorstr ), 4 );
        QColor color = QgsSymbolLayerUtils::decodeColor( colorstr );

        if ( color.isValid() )
        {
          dataDefinedValues.insert( p, QVariant( color ) );
          return true;
        }
        return false;
      }
      case DDJoinStyle:
      {
        QString joinstr = exprVal.toString().trimmed();
        QgsDebugMsgLevel( dbgStr.arg( joinstr ), 4 );

        if ( !joinstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( static_cast< int >( _decodePenJoinStyle( joinstr ) ) ) );
          return true;
        }
        return false;
      }
      case DDBlendMode:
      {
        QString blendstr = exprVal.toString().trimmed();
        QgsDebugMsgLevel( dbgStr.arg( blendstr ), 4 );

        if ( !blendstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( static_cast< int >( QgsSymbolLayerUtils::decodeBlendMode( blendstr ) ) ) );
          return true;
        }
        return false;
      }
      case DDPointF:
      {
        QString ptstr = exprVal.toString().trimmed();
        QgsDebugMsgLevel( dbgStr.arg( ptstr ), 4 );

        if ( !ptstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( QgsSymbolLayerUtils::decodePoint( ptstr ) ) );
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

void QgsPalLayerSettings::parseTextStyle( QFont& labelFont,
    QgsUnitTypes::RenderUnit fontunits,
    QgsRenderContext &context )
{
  // NOTE: labelFont already has pixelSize set, so pointSize or pointSizeF might return -1

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // Two ways to generate new data defined font:
  // 1) Family + [bold] + [italic] (named style is ignored and font is built off of base family)
  // 2) Family + named style  (bold or italic is ignored)

  // data defined font family?
  QString ddFontFamily( "" );
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Family, exprVal, &context.expressionContext(), labelFont.family() ) )
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontStyle, exprVal, &context.expressionContext() ) )
  {
    QString fontstyle = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal Font style:%1" ).arg( fontstyle ), 4 );
    ddFontStyle = fontstyle;
  }

  // data defined bold font style?
  bool ddBold = false;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Bold, exprVal, &context.expressionContext(), labelFont.bold() ) )
  {
    bool bold = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font bold:%1" ).arg( bold ? "true" : "false" ), 4 );
    ddBold = bold;
  }

  // data defined italic font style?
  bool ddItalic = false;
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Italic, exprVal, &context.expressionContext(), labelFont.italic() ) )
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
      QFont styledfont = mFontDB.font( ddFontFamily, mFormat.namedStyle(), appFont.pointSize() );
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
    newFont.setUnderline( labelFont.underline() );
    newFont.setStrikeOut( labelFont.strikeOut() );
    newFont.setWordSpacing( labelFont.wordSpacing() );
    newFont.setLetterSpacing( QFont::AbsoluteSpacing, labelFont.letterSpacing() );

    labelFont = newFont;
  }

  // data defined word spacing?
  double wordspace = labelFont.wordSpacing();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontWordSpacing, exprVal, &context.expressionContext(), wordspace ) )
  {
    bool ok;
    double wspacing = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QString( "exprVal FontWordSpacing:%1" ).arg( wspacing ), 4 );
    if ( ok )
    {
      wordspace = wspacing;
    }
  }
  labelFont.setWordSpacing( QgsTextRenderer::scaleToPixelContext( wordspace, context, fontunits, false, mFormat.sizeMapUnitScale() ) );

  // data defined letter spacing?
  double letterspace = labelFont.letterSpacing();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::FontLetterSpacing, exprVal, &context.expressionContext(), letterspace ) )
  {
    bool ok;
    double lspacing = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QString( "exprVal FontLetterSpacing:%1" ).arg( lspacing ), 4 );
    if ( ok )
    {
      letterspace = lspacing;
    }
  }
  labelFont.setLetterSpacing( QFont::AbsoluteSpacing, QgsTextRenderer::scaleToPixelContext( letterspace, context, fontunits, false, mFormat.sizeMapUnitScale() ) );

  // data defined strikeout font style?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Strikeout, exprVal, &context.expressionContext(), labelFont.strikeOut() ) )
  {
    bool strikeout = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font strikeout:%1" ).arg( strikeout ? "true" : "false" ), 4 );
    labelFont.setStrikeOut( strikeout );
  }

  // data defined underline font style?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::Underline, exprVal, &context.expressionContext(), labelFont.underline() ) )
  {
    bool underline = exprVal.toBool();
    QgsDebugMsgLevel( QString( "exprVal Font underline:%1" ).arg( underline ? "true" : "false" ), 4 );
    labelFont.setUnderline( underline );
  }

  // pass the rest on to QgsPalLabeling::drawLabeling

  // data defined font color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Color, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( mFormat.color() ) );

  // data defined font transparency?
  dataDefinedValEval( DDTransparency, QgsPalLayerSettings::FontTransp, exprVal, context.expressionContext(), 100 - mFormat.opacity() * 100 );

  // data defined font blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::FontBlendMode, exprVal, context.expressionContext() );

}

void QgsPalLayerSettings::parseTextBuffer( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextBufferSettings buffer = mFormat.buffer();

  // data defined draw buffer?
  bool drawBuffer = mFormat.buffer().enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::BufferDraw, exprVal, context.expressionContext(), buffer.enabled() ) )
  {
    drawBuffer = exprVal.toBool();
  }

  if ( !drawBuffer )
  {
    return;
  }

  // data defined buffer size?
  double bufrSize = buffer.size();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::BufferSize, exprVal, context.expressionContext(), buffer.size() ) )
  {
    bufrSize = exprVal.toDouble();
  }

  // data defined buffer transparency?
  int bufTransp = 100 - buffer.opacity() * 100;
  if ( dataDefinedValEval( DDTransparency, QgsPalLayerSettings::BufferTransp, exprVal, context.expressionContext(), bufTransp ) )
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
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::BufferUnit, exprVal, context.expressionContext() );

  // data defined buffer color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::BufferColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( buffer.color() ) );

  // data defined buffer pen join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::BufferJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( buffer.joinStyle() ) );

  // data defined buffer blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::BufferBlendMode, exprVal, context.expressionContext() );
}

void QgsPalLayerSettings::parseTextFormatting( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined multiline wrap character?
  QString wrapchr = wrapChar;
  if ( dataDefinedValEval( DDString, QgsPalLayerSettings::MultiLineWrapChar, exprVal, context.expressionContext(), wrapChar ) )
  {
    wrapchr = exprVal.toString();
  }

  // data defined multiline height?
  dataDefinedValEval( DDDouble, QgsPalLayerSettings::MultiLineHeight, exprVal, context.expressionContext() );

  // data defined multiline text align?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::MultiLineAlignment, exprVal, &context.expressionContext(), mFormat.lineHeight() ) )
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
      else if ( str.compare( "Follow", Qt::CaseInsensitive ) == 0 )
      {
        aligntype = QgsPalLayerSettings::MultiFollowPlacement;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::MultiLineAlignment, QVariant( static_cast< int >( aligntype ) ) );
    }
  }

  // data defined direction symbol?
  bool drawDirSymb = addDirectionSymbol;
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::DirSymbDraw, exprVal, context.expressionContext(), addDirectionSymbol ) )
  {
    drawDirSymb = exprVal.toBool();
  }

  if ( drawDirSymb )
  {
    // data defined direction left symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::DirSymbLeft, exprVal, context.expressionContext(), leftDirectionSymbol );

    // data defined direction right symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::DirSymbRight, exprVal, context.expressionContext(), rightDirectionSymbol );

    // data defined direction symbol placement?
    if ( dataDefinedEvaluate( QgsPalLayerSettings::DirSymbPlacement, exprVal, &context.expressionContext() ) )
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
        dataDefinedValues.insert( QgsPalLayerSettings::DirSymbPlacement, QVariant( static_cast< int >( placetype ) ) );
      }
    }

    // data defined direction symbol reversed?
    dataDefinedValEval( DDBool, QgsPalLayerSettings::DirSymbReverse, exprVal, context.expressionContext(), reverseDirectionSymbol );
  }

  // formatting for numbers is inline with generation of base label text and not passed to label painting
}

void QgsPalLayerSettings::parseShapeBackground( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextBackgroundSettings background = mFormat.background();

  // data defined draw shape?
  bool drawShape = background.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::ShapeDraw, exprVal, context.expressionContext(), drawShape ) )
  {
    drawShape = exprVal.toBool();
  }

  if ( !drawShape )
  {
    return;
  }

  // data defined shape transparency?
  int shapeTransp = 100 - background.opacity() * 100;
  if ( dataDefinedValEval( DDTransparency, QgsPalLayerSettings::ShapeTransparency, exprVal, context.expressionContext(), shapeTransp ) )
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
  QgsTextBackgroundSettings::ShapeType shapeKind = background.type();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeKind, exprVal, &context.expressionContext() ) )
  {
    QString skind = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeKind:%1" ).arg( skind ), 4 );

    if ( !skind.isEmpty() )
    {
      // "Rectangle"
      QgsTextBackgroundSettings::ShapeType shpkind = QgsTextBackgroundSettings::ShapeRectangle;

      if ( skind.compare( "Square", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeSquare;
      }
      else if ( skind.compare( "Ellipse", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeEllipse;
      }
      else if ( skind.compare( "Circle", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeCircle;
      }
      else if ( skind.compare( "SVG", Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeSVG;
      }
      shapeKind = shpkind;
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeKind, QVariant( static_cast< int >( shpkind ) ) );
    }
  }

  // data defined shape SVG path?
  QString svgPath = background.svgFile();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeSVGFile, exprVal, &context.expressionContext(), svgPath ) )
  {
    QString svgfile = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeSVGFile:%1" ).arg( svgfile ), 4 );

    // '' empty paths are allowed
    svgPath = svgfile;
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeSVGFile, QVariant( svgfile ) );
  }

  // data defined shape size type?
  QgsTextBackgroundSettings::SizeType shpSizeType = background.sizeType();
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeSizeType, exprVal, &context.expressionContext() ) )
  {
    QString stype = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeSizeType:%1" ).arg( stype ), 4 );

    if ( !stype.isEmpty() )
    {
      // "Buffer"
      QgsTextBackgroundSettings::SizeType sizType = QgsTextBackgroundSettings::SizeBuffer;

      if ( stype.compare( "Fixed", Qt::CaseInsensitive ) == 0 )
      {
        sizType = QgsTextBackgroundSettings::SizeFixed;
      }
      shpSizeType = sizType;
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeSizeType, QVariant( static_cast< int >( sizType ) ) );
    }
  }

  // data defined shape size X? (SVGs only use X for sizing)
  double ddShpSizeX = background.size().width();
  if ( dataDefinedValEval( DDDouble, QgsPalLayerSettings::ShapeSizeX, exprVal, context.expressionContext(), ddShpSizeX ) )
  {
    ddShpSizeX = exprVal.toDouble();
  }

  // data defined shape size Y?
  double ddShpSizeY = background.size().height();
  if ( dataDefinedValEval( DDDouble, QgsPalLayerSettings::ShapeSizeY, exprVal, context.expressionContext(), ddShpSizeY ) )
  {
    ddShpSizeY = exprVal.toDouble();
  }

  // don't continue under certain circumstances (e.g. size is fixed)
  bool skip = false;
  if ( shapeKind == QgsTextBackgroundSettings::ShapeSVG
       && ( svgPath.isEmpty()
            || ( !svgPath.isEmpty()
                 && shpSizeType == QgsTextBackgroundSettings::SizeFixed
                 && ddShpSizeX == 0.0 ) ) )
  {
    skip = true;
  }
  if ( shapeKind != QgsTextBackgroundSettings::ShapeSVG
       && shpSizeType == QgsTextBackgroundSettings::SizeFixed
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
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeSizeUnits, exprVal, context.expressionContext() );

  // data defined shape rotation type?
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShapeRotationType, exprVal, &context.expressionContext() ) )
  {
    QString rotstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShapeRotationType:%1" ).arg( rotstr ), 4 );

    if ( !rotstr.isEmpty() )
    {
      // "Sync"
      QgsTextBackgroundSettings::RotationType rottype = QgsTextBackgroundSettings::RotationSync;

      if ( rotstr.compare( "Offset", Qt::CaseInsensitive ) == 0 )
      {
        rottype = QgsTextBackgroundSettings::RotationOffset;
      }
      else if ( rotstr.compare( "Fixed", Qt::CaseInsensitive ) == 0 )
      {
        rottype = QgsTextBackgroundSettings::RotationFixed;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeRotationType, QVariant( static_cast< int >( rottype ) ) );
    }
  }

  // data defined shape rotation?
  dataDefinedValEval( DDRotation180, QgsPalLayerSettings::ShapeRotation, exprVal, context.expressionContext(), background.rotation() );

  // data defined shape offset?
  dataDefinedValEval( DDPointF, QgsPalLayerSettings::ShapeOffset, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePoint( background.offset() ) );

  // data defined shape offset units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeOffsetUnits, exprVal, context.expressionContext() );

  // data defined shape radii?
  dataDefinedValEval( DDPointF, QgsPalLayerSettings::ShapeRadii, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeSize( background.radii() ) );

  // data defined shape radii units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeRadiiUnits, exprVal, context.expressionContext() );

  // data defined shape blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::ShapeBlendMode, exprVal, context.expressionContext() );

  // data defined shape fill color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::ShapeFillColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( background.fillColor() ) );

  // data defined shape border color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::ShapeBorderColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( background.borderColor() ) );

  // data defined shape border width?
  dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::ShapeBorderWidth, exprVal, context.expressionContext(), background.borderWidth() );

  // data defined shape border width units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeBorderWidthUnits, exprVal, context.expressionContext() );

  // data defined shape join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::ShapeJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( background.joinStyle() ) );

}

void QgsPalLayerSettings::parseDropShadow( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextShadowSettings shadow = mFormat.shadow();

  // data defined draw shadow?
  bool drawShadow = shadow.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::ShadowDraw, exprVal, context.expressionContext(), drawShadow ) )
  {
    drawShadow = exprVal.toBool();
  }

  if ( !drawShadow )
  {
    return;
  }

  // data defined shadow transparency?
  int shadowTransp = 100 - shadow.opacity() * 100;
  if ( dataDefinedValEval( DDTransparency, QgsPalLayerSettings::ShadowTransparency, exprVal, context.expressionContext(), shadowTransp ) )
  {
    shadowTransp = exprVal.toInt();
  }

  // data defined shadow offset distance?
  double shadowOffDist = shadow.offsetDistance();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::ShadowOffsetDist, exprVal, context.expressionContext(), shadowOffDist ) )
  {
    shadowOffDist = exprVal.toDouble();
  }

  // data defined shadow offset distance?
  double shadowRad = shadow.blurRadius();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::ShadowRadius, exprVal, context.expressionContext(), shadowRad ) )
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
  if ( dataDefinedEvaluate( QgsPalLayerSettings::ShadowUnder, exprVal, &context.expressionContext() ) )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QString( "exprVal ShadowUnder:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      // "Lowest"
      QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextShadowSettings::ShadowLowest;

      if ( str.compare( "Text", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsTextShadowSettings::ShadowText;
      }
      else if ( str.compare( "Buffer", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsTextShadowSettings::ShadowBuffer;
      }
      else if ( str.compare( "Background", Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsTextShadowSettings::ShadowShape;
      }
      dataDefinedValues.insert( QgsPalLayerSettings::ShadowUnder, QVariant( static_cast< int >( shdwtype ) ) );
    }
  }

  // data defined shadow offset angle?
  dataDefinedValEval( DDRotation180, QgsPalLayerSettings::ShadowOffsetAngle, exprVal, context.expressionContext(), shadow.offsetAngle() );

  // data defined shadow offset units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShadowOffsetUnits, exprVal, context.expressionContext() );

  // data defined shadow radius?
  dataDefinedValEval( DDDouble, QgsPalLayerSettings::ShadowRadius, exprVal, context.expressionContext(), shadow.blurRadius() );

  // data defined shadow radius units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShadowRadiusUnits, exprVal, context.expressionContext() );

  // data defined shadow scale?  ( gui bounds to 0-2000, no upper bound here )
  dataDefinedValEval( DDIntPos, QgsPalLayerSettings::ShadowScale, exprVal, context.expressionContext(), shadow.scale() );

  // data defined shadow color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::ShadowColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( shadow.color() ) );

  // data defined shadow blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::ShadowBlendMode, exprVal, context.expressionContext() );
}

// -------------

QgsPalLabeling::QgsPalLabeling()
    : mEngine( new QgsLabelingEngine() )
{
}

QgsPalLabeling::~QgsPalLabeling()
{
  delete mEngine;
  mEngine = nullptr;
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
  if ( layer->customProperty( "labeling" ).toString() == "pal" )
    enabled = layer->labelsEnabled() || layer->diagramsEnabled();
  else if ( layer->labeling()->type() == "rule-based" )
    return true;

  return enabled;
}


void QgsPalLabeling::clearActiveLayers()
{
}

void QgsPalLabeling::clearActiveLayer( const QString &layerID )
{
  Q_UNUSED( layerID );
}


int QgsPalLabeling::prepareLayer( QgsVectorLayer* layer, QSet<QString>& attrNames, QgsRenderContext& ctx )
{
  if ( !willUseLayer( layer ) )
  {
    return 0;
  }

  if ( !layer->labeling() )
    return 0;

  QgsVectorLayerLabelProvider* lp = layer->labeling()->provider( layer );
  if ( !lp )
    return 0;

  //QgsVectorLayerLabelProvider* lp = new QgsVectorLayerLabelProvider( layer, false );
  // need to be added before calling prepare() - uses map settings from engine
  mEngine->addProvider( lp );
  mLabelProviders[layer->id()] = lp; // fast lookup table by layer ID

  if ( !lp->prepare( ctx, attrNames ) )
  {
    mEngine->removeProvider( lp );
    return 0;
  }

  return 1; // init successful
}

int QgsPalLabeling::prepareDiagramLayer( QgsVectorLayer* layer, QSet<QString>& attrNames, QgsRenderContext& ctx )
{
  QgsVectorLayerDiagramProvider* dp = new QgsVectorLayerDiagramProvider( layer, false );
  // need to be added before calling prepare() - uses map settings from engine
  mEngine->addProvider( dp );
  mDiagramProviders[layer->id()] = dp; // fast lookup table by layer ID

  if ( !dp->prepare( ctx, attrNames ) )
  {
    mEngine->removeProvider( dp );
    return 0;
  }

  return 1;
}

void QgsPalLabeling::registerFeature( const QString& layerID, QgsFeature& f, QgsRenderContext &context )
{
  if ( QgsVectorLayerLabelProvider* provider = mLabelProviders.value( layerID, nullptr ) )
    provider->registerFeature( f, context );
}

bool QgsPalLabeling::geometryRequiresPreparation( const QgsGeometry& geometry, QgsRenderContext &context, const QgsCoordinateTransform& ct, QgsGeometry* clipGeometry )
{
  if ( geometry.isEmpty() )
  {
    return false;
  }

  //requires reprojection
  if ( ct.isValid() && !ct.isShortCircuited() )
    return true;

  //requires rotation
  const QgsMapToPixel& m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
    return true;

  //requires clip
  if ( clipGeometry && !clipGeometry->boundingBox().contains( geometry.boundingBox() ) )
    return true;

  //requires fixing
  if ( geometry.type() == QgsWkbTypes::PolygonGeometry && !geometry.isGeosValid() )
    return true;

  return false;
}

QStringList QgsPalLabeling::splitToLines( const QString &text, const QString &wrapCharacter )
{
  QStringList multiLineSplit;
  if ( !wrapCharacter.isEmpty() && wrapCharacter != QLatin1String( "\n" ) )
  {
    //wrap on both the wrapchr and new line characters
    Q_FOREACH ( const QString& line, text.split( wrapCharacter ) )
    {
      multiLineSplit.append( line.split( '\n' ) );
    }
  }
  else
  {
    multiLineSplit = text.split( '\n' );
  }

  return multiLineSplit;
}

QStringList QgsPalLabeling::splitToGraphemes( const QString &text )
{
  QStringList graphemes;
  QTextBoundaryFinder boundaryFinder( QTextBoundaryFinder::Grapheme, text );
  int currentBoundary = -1;
  int previousBoundary = 0;
  while (( currentBoundary = boundaryFinder.toNextBoundary() ) > 0 )
  {
    graphemes << text.mid( previousBoundary, currentBoundary - previousBoundary );
    previousBoundary = currentBoundary;
  }
  return graphemes;
}

QgsGeometry QgsPalLabeling::prepareGeometry( const QgsGeometry& geometry, QgsRenderContext &context, const QgsCoordinateTransform& ct, QgsGeometry* clipGeometry )
{
  if ( geometry.isEmpty() )
  {
    return QgsGeometry();
  }

  //don't modify the feature's geometry so that geometry based expressions keep working
  QgsGeometry geom = geometry;

  //reproject the geometry if necessary
  if ( ct.isValid() && !ct.isShortCircuited() )
  {
    try
    {
      geom.transform( ct );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsgLevel( QString( "Ignoring feature due to transformation exception" ), 4 );
      return QgsGeometry();
    }
  }

  // Rotate the geometry if needed, before clipping
  const QgsMapToPixel& m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPoint center = context.extent().center();

    if ( ct.isValid() && !ct.isShortCircuited() )
    {
      try
      {
        center = ct.transform( center );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsgLevel( QString( "Ignoring feature due to transformation exception" ), 4 );
        return QgsGeometry();
      }
    }

    if ( geom.rotate( m2p.mapRotation(), center ) )
    {
      QgsDebugMsg( QString( "Error rotating geometry" ).arg( geom.exportToWkt() ) );
      return QgsGeometry();
    }
  }

  if ( !geom.asGeos() )
    return QgsGeometry();  // there is something really wrong with the geometry

  // fix invalid polygons
  if ( geom.type() == QgsWkbTypes::PolygonGeometry && !geom.isGeosValid() )
  {
    QgsGeometry bufferGeom = geom.buffer( 0, 0 );
    if ( bufferGeom.isEmpty() )
    {
      return QgsGeometry();
    }
    geom = bufferGeom;
  }

  if ( clipGeometry &&
       (( qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry->boundingBox().contains( geom.boundingBox() ) )
        || ( !qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry->contains( geom ) ) ) )
  {
    QgsGeometry clipGeom = geom.intersection( *clipGeometry ); // creates new geometry
    if ( clipGeom.isEmpty() )
    {
      return QgsGeometry();
    }
    geom = clipGeom;
  }

  return geom;
}

bool QgsPalLabeling::checkMinimumSizeMM( const QgsRenderContext& context, const QgsGeometry* geom, double minSize )
{
  if ( minSize <= 0 )
  {
    return true;
  }

  if ( !geom )
  {
    return false;
  }

  QgsWkbTypes::GeometryType featureType = geom->type();
  if ( featureType == QgsWkbTypes::PointGeometry ) //minimum size does not apply to point features
  {
    return true;
  }

  double mapUnitsPerMM = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();
  if ( featureType == QgsWkbTypes::LineGeometry )
  {
    double length = geom->length();
    if ( length >= 0.0 )
    {
      return ( length >= ( minSize * mapUnitsPerMM ) );
    }
  }
  else if ( featureType == QgsWkbTypes::PolygonGeometry )
  {
    double area = geom->area();
    if ( area >= 0.0 )
    {
      return ( sqrt( area ) >= ( minSize * mapUnitsPerMM ) );
    }
  }
  return true; //should never be reached. Return true in this case to label such geometries anyway.
}

void QgsPalLabeling::registerDiagramFeature( const QString& layerID, QgsFeature& feat, QgsRenderContext &context )
{
  if ( QgsVectorLayerDiagramProvider* provider = mDiagramProviders.value( layerID, nullptr ) )
    provider->registerFeature( feat, context );
}


void QgsPalLabeling::init( const QgsMapSettings& mapSettings )
{
  mEngine->setMapSettings( mapSettings );
}

void QgsPalLabeling::exit()
{
  delete mEngine;
  mEngine = new QgsLabelingEngine();
}

void QgsPalLabeling::dataDefinedTextStyle( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  QgsTextFormat format = tmpLyr.format();
  bool changed = false;

  //font color
  if ( ddValues.contains( QgsPalLayerSettings::Color ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Color );
    format.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  //font transparency
  if ( ddValues.contains( QgsPalLayerSettings::FontTransp ) )
  {
    format.setOpacity( 1.0 - ddValues.value( QgsPalLayerSettings::FontTransp ).toInt() / 100.0 );
    changed = true;
  }

  //font blend mode
  if ( ddValues.contains( QgsPalLayerSettings::FontBlendMode ) )
  {
    format.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::FontBlendMode ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    tmpLyr.setFormat( format );
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
      QgsTextFormat format = tmpLyr.format();
      format.setLineHeight( ddValues.value( QgsPalLayerSettings::MultiLineHeight ).toDouble() );
      tmpLyr.setFormat( format );
    }

    if ( ddValues.contains( QgsPalLayerSettings::MultiLineAlignment ) )
    {
      tmpLyr.multilineAlign = static_cast< QgsPalLayerSettings::MultiLineAlign >( ddValues.value( QgsPalLayerSettings::MultiLineAlignment ).toInt() );
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
      tmpLyr.placeDirectionSymbol = static_cast< QgsPalLayerSettings::DirectionSymbols >( ddValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt() );
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
  QgsTextBufferSettings buffer = tmpLyr.format().buffer();
  bool changed = false;

  //buffer draw
  if ( ddValues.contains( QgsPalLayerSettings::BufferDraw ) )
  {
    buffer.setEnabled( ddValues.value( QgsPalLayerSettings::BufferDraw ).toBool() );
    changed = true;
  }

  if ( !buffer.enabled() )
  {
    if ( changed )
    {
      QgsTextFormat format = tmpLyr.format();
      format.setBuffer( buffer );
      tmpLyr.setFormat( format );
    }

    // tmpLyr.bufferSize > 0.0 && tmpLyr.bufferTransp < 100 figured in during evaluation
    return; // don't continue looking for unused values
  }

  //buffer size
  if ( ddValues.contains( QgsPalLayerSettings::BufferSize ) )
  {
    buffer.setSize( ddValues.value( QgsPalLayerSettings::BufferSize ).toDouble() );
    changed = true;
  }

  //buffer transparency
  if ( ddValues.contains( QgsPalLayerSettings::BufferTransp ) )
  {
    buffer.setOpacity( 1.0 - ddValues.value( QgsPalLayerSettings::BufferTransp ).toInt() / 100.0 );
    changed = true;
  }

  //buffer size units
  if ( ddValues.contains( QgsPalLayerSettings::BufferUnit ) )
  {
    QgsUnitTypes::RenderUnit bufunit = static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::BufferUnit ).toInt() );
    buffer.setSizeUnit( bufunit );
    changed = true;
  }

  //buffer color
  if ( ddValues.contains( QgsPalLayerSettings::BufferColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::BufferColor );
    buffer.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  //buffer pen join style
  if ( ddValues.contains( QgsPalLayerSettings::BufferJoinStyle ) )
  {
    buffer.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::BufferJoinStyle ).toInt() ) );
    changed = true;
  }

  //buffer blend mode
  if ( ddValues.contains( QgsPalLayerSettings::BufferBlendMode ) )
  {
    buffer.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::BufferBlendMode ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setBuffer( buffer );
    tmpLyr.setFormat( format );
  }
}

void QgsPalLabeling::dataDefinedShapeBackground( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  QgsTextBackgroundSettings background = tmpLyr.format().background();
  bool changed = false;

  //shape draw
  if ( ddValues.contains( QgsPalLayerSettings::ShapeDraw ) )
  {
    background.setEnabled( ddValues.value( QgsPalLayerSettings::ShapeDraw ).toBool() );
    changed = true;
  }

  if ( !background.enabled() )
  {
    if ( changed )
    {
      QgsTextFormat format = tmpLyr.format();
      format.setBackground( background );
      tmpLyr.setFormat( format );
    }
    return; // don't continue looking for unused values
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeKind ) )
  {
    background.setType( static_cast< QgsTextBackgroundSettings::ShapeType >( ddValues.value( QgsPalLayerSettings::ShapeKind ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSVGFile ) )
  {
    background.setSvgFile( ddValues.value( QgsPalLayerSettings::ShapeSVGFile ).toString() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeType ) )
  {
    background.setSizeType( static_cast< QgsTextBackgroundSettings::SizeType >( ddValues.value( QgsPalLayerSettings::ShapeSizeType ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeX ) )
  {
    QSizeF size = background.size();
    size.setWidth( ddValues.value( QgsPalLayerSettings::ShapeSizeX ).toDouble() );
    background.setSize( size );
    changed = true;
  }
  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeY ) )
  {
    QSizeF size = background.size();
    size.setHeight( ddValues.value( QgsPalLayerSettings::ShapeSizeY ).toDouble() );
    background.setSize( size );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeSizeUnits ) )
  {
    background.setSizeUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShapeSizeUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRotationType ) )
  {
    background.setRotationType( static_cast< QgsTextBackgroundSettings::RotationType >( ddValues.value( QgsPalLayerSettings::ShapeRotationType ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRotation ) )
  {
    background.setRotation( ddValues.value( QgsPalLayerSettings::ShapeRotation ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeOffset ) )
  {
    background.setOffset( ddValues.value( QgsPalLayerSettings::ShapeOffset ).toPointF() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeOffsetUnits ) )
  {
    background.setOffsetUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShapeOffsetUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRadii ) )
  {
    background.setRadii( ddValues.value( QgsPalLayerSettings::ShapeRadii ).toSizeF() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeRadiiUnits ) )
  {
    background.setRadiiUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShapeRadiiUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBlendMode ) )
  {
    background.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::ShapeBlendMode ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeFillColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShapeFillColor );
    background.setFillColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShapeBorderColor );
    background.setBorderColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeTransparency ) )
  {
    background.setOpacity( 1.0 - ddValues.value( QgsPalLayerSettings::ShapeTransparency ).toInt() / 100.0 );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderWidth ) )
  {
    background.setBorderWidth( ddValues.value( QgsPalLayerSettings::ShapeBorderWidth ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeBorderWidthUnits ) )
  {
    background.setBorderWidthUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShapeBorderWidthUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeJoinStyle ) )
  {
    background.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::ShapeJoinStyle ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setBackground( background );
    tmpLyr.setFormat( format );
  }
}

void QgsPalLabeling::dataDefinedDropShadow( QgsPalLayerSettings& tmpLyr,
    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues )
{
  QgsTextShadowSettings shadow = tmpLyr.format().shadow();
  bool changed = false;

  //shadow draw
  if ( ddValues.contains( QgsPalLayerSettings::ShadowDraw ) )
  {
    shadow.setEnabled( ddValues.value( QgsPalLayerSettings::ShadowDraw ).toBool() );
    changed = true;
  }

  if ( !shadow.enabled() )
  {
    if ( changed )
    {
      QgsTextFormat format = tmpLyr.format();
      format.setShadow( shadow );
      tmpLyr.setFormat( format );
    }
    return; // don't continue looking for unused values
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowUnder ) )
  {
    shadow.setShadowPlacement( static_cast< QgsTextShadowSettings::ShadowPlacement >( ddValues.value( QgsPalLayerSettings::ShadowUnder ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetAngle ) )
  {
    shadow.setOffsetAngle( ddValues.value( QgsPalLayerSettings::ShadowOffsetAngle ).toInt() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetDist ) )
  {
    shadow.setOffsetDistance( ddValues.value( QgsPalLayerSettings::ShadowOffsetDist ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOffsetUnits ) )
  {
    shadow.setOffsetUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShadowOffsetUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowRadius ) )
  {
    shadow.setBlurRadius( ddValues.value( QgsPalLayerSettings::ShadowRadius ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowRadiusUnits ) )
  {
    shadow.setBlurRadiusUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShadowRadiusUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShadowColor );
    shadow.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowTransparency ) )
  {
    shadow.setOpacity( 1.0 - ddValues.value( QgsPalLayerSettings::ShadowTransparency ).toInt() / 100.0 );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShadowScale ) )
  {
    shadow.setScale( ddValues.value( QgsPalLayerSettings::ShadowScale ).toInt() );
    changed = true;
  }


  if ( ddValues.contains( QgsPalLayerSettings::ShadowBlendMode ) )
  {
    shadow.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::ShadowBlendMode ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setShadow( shadow );
    tmpLyr.setFormat( format );
  }
}



void QgsPalLabeling::drawLabeling( QgsRenderContext& context )
{
  mEngine->run( context );
}

void QgsPalLabeling::deleteTemporaryData()
{
}

QgsLabelingResults *QgsPalLabeling::takeResults()
{
  return mEngine->takeResults();
}

void QgsPalLabeling::numCandidatePositions( int& candPoint, int& candLine, int& candPolygon )
{
  mEngine->numCandidatePositions( candPoint, candLine, candPolygon );
}

void QgsPalLabeling::setNumCandidatePositions( int candPoint, int candLine, int candPolygon )
{
  mEngine->setNumCandidatePositions( candPoint, candLine, candPolygon );
}

void QgsPalLabeling::setSearchMethod( QgsPalLabeling::Search s )
{
  mEngine->setSearchMethod( s );
}

QgsPalLabeling::Search QgsPalLabeling::searchMethod() const
{
  return mEngine->searchMethod();
}

bool QgsPalLabeling::isShowingCandidates() const
{
  return mEngine->testFlag( QgsLabelingEngine::DrawCandidates );
}

void QgsPalLabeling::setShowingCandidates( bool showing )
{
  mEngine->setFlag( QgsLabelingEngine::DrawCandidates, showing );
}

bool QgsPalLabeling::isShowingShadowRectangles() const
{
  return mEngine->testFlag( QgsLabelingEngine::DrawShadowRects );
}

void QgsPalLabeling::setShowingShadowRectangles( bool showing )
{
  mEngine->setFlag( QgsLabelingEngine::DrawShadowRects, showing );
}

bool QgsPalLabeling::isShowingAllLabels() const
{
  return mEngine->testFlag( QgsLabelingEngine::UseAllLabels );
}

void QgsPalLabeling::setShowingAllLabels( bool showing )
{
  mEngine->setFlag( QgsLabelingEngine::UseAllLabels, showing );
}

bool QgsPalLabeling::isShowingPartialsLabels() const
{
  return mEngine->testFlag( QgsLabelingEngine::UsePartialCandidates );
}

void QgsPalLabeling::setShowingPartialsLabels( bool showing )
{
  mEngine->setFlag( QgsLabelingEngine::UsePartialCandidates, showing );
}

bool QgsPalLabeling::isDrawingOutlineLabels() const
{
  return mEngine->testFlag( QgsLabelingEngine::RenderOutlineLabels );
}

void QgsPalLabeling::setDrawingOutlineLabels( bool outline )
{
  mEngine->setFlag( QgsLabelingEngine::RenderOutlineLabels, outline );
}

bool QgsPalLabeling::drawLabelRectOnly() const
{
  return mEngine->testFlag( QgsLabelingEngine::DrawLabelRectOnly );
}

void QgsPalLabeling::setDrawLabelRectOnly( bool drawRect )
{
  mEngine->setFlag( QgsLabelingEngine::DrawLabelRectOnly, drawRect );
}

void QgsPalLabeling::drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform, QList<QgsLabelCandidate>* candidates )
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

  if ( lp->conflictsWithObstacle() )
  {
    painter->setPen( QColor( 255, 0, 0, 64 ) );
  }
  else
  {
    painter->setPen( QColor( 0, 0, 0, 64 ) );
  }
  painter->drawRect( rect );
  painter->restore();

  // save the rect
  rect.moveTo( outPt.x(), outPt.y() );
  if ( candidates )
    candidates->append( QgsLabelCandidate( rect, lp->cost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->getNextPart() )
    drawLabelCandidateRect( lp->getNextPart(), painter, xform, candidates );
}


void QgsPalLabeling::drawLabelBuffer( QgsRenderContext& context,
                                      const QgsLabelComponent& component,
                                      const QgsPalLayerSettings& tmpLyr )
{
  QPainter* p = context.painter();

  QgsTextFormat format = tmpLyr.format();
  QgsTextBufferSettings buffer = format.buffer();

  double penSize = QgsTextRenderer::scaleToPixelContext( buffer.size(), context,
                   buffer.sizeUnit(), true, buffer.sizeMapUnitScale() );

  QPainterPath path;
  path.setFillRule( Qt::WindingFill );
  path.addText( 0, 0, format.scaledFont( context ), component.text() );
  QPen pen( buffer.color() );
  pen.setWidthF( penSize );
  pen.setJoinStyle( buffer.joinStyle() );
  QColor tmpColor( buffer.color() );
  // honor pref for whether to fill buffer interior
  if ( !buffer.fillBufferInterior() )
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

  if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowBuffer )
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
    p->setCompositionMode( buffer.blendMode() );
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
  QgsTextFormat format = tmpLyr.format();
  QgsTextBackgroundSettings background = format.background();

  QPainter* p = context.painter();
  double labelWidth = component.size().x(), labelHeight = component.size().y();
  //QgsDebugMsgLevel( QString( "Background label rotation: %1" ).arg( component.rotation() ), 4 );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  if ( background.rotationType() != QgsTextBackgroundSettings::RotationFixed )
  {
    component.setRotation( -( component.rotation() * 180 / M_PI ) ); // RotationSync
    component.setRotationOffset(
      background.rotationType() == QgsTextBackgroundSettings::RotationOffset ? background.rotation() : 0.0 );
  }
  else // RotationFixed
  {
    component.setRotation( 0.0 ); // don't use label's rotation
    component.setRotationOffset( background.rotation() );
  }

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  if ( background.type() == QgsTextBackgroundSettings::ShapeSVG )
  {
    // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

    if ( background.svgFile().isEmpty() )
      return;

    double sizeOut = 0.0;
    // only one size used for SVG sizing/scaling (no use of shapeSize.y() or Y field in gui)
    if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
    {
      sizeOut = background.size().width();
    }
    else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
    {
      // add buffer to greatest dimension of label
      if ( labelWidth >= labelHeight )
        sizeOut = labelWidth;
      else
        sizeOut = labelHeight;

      // label size in map units, convert to shapeSizeUnits, if different
      if ( background.sizeUnit() != QgsUnitTypes::RenderMapUnits )
      {
        sizeOut = QgsSymbolLayerUtils::convertToMapUnits( context, sizeOut,
                  background.sizeUnit(), background.sizeMapUnitScale() );
      }

      // add buffer
      sizeOut += background.size().width() * 2;
    }

    // don't bother rendering symbols smaller than 1x1 pixels in size
    // TODO: add option to not show any svgs under/over a certain size
    if ( QgsTextRenderer::scaleToPixelContext( sizeOut, context, background.sizeUnit(),
         false, background.sizeMapUnitScale() ) < 1.0 )
      return;

    QgsStringMap map; // for SVG symbology marker
    map["name"] = QgsSymbolLayerUtils::symbolNameToPath( background.svgFile().trimmed() );
    map["size"] = QString::number( sizeOut );
    map["size_unit"] = QgsUnitTypes::encodeUnit( background.sizeUnit() );
    map["angle"] = QString::number( 0.0 ); // angle is handled by this local painter

    // offset is handled by this local painter
    // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
    //map["offset"] = QgsSymbolLayerUtils::encodePoint( tmpLyr.shapeOffset );
    //map["offset_unit"] = QgsUnitTypes::encodeUnit(
    //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsUnitTypes::MapUnit : QgsUnitTypes::MM );

    map["fill"] = background.fillColor().name();
    map["outline"] = background.borderColor().name();
    map["outline-width"] = QString::number( background.borderWidth() );
    map["outline_width_unit"] = QgsUnitTypes::encodeUnit( background.borderWidthUnit() );

    if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
    {
      QgsTextShadowSettings shadow = format.shadow();
      // configure SVG shadow specs
      QgsStringMap shdwmap( map );
      shdwmap["fill"] = shadow.color().name();
      shdwmap["outline"] = shadow.color().name();
      shdwmap["size"] = QString::number( sizeOut * context.rasterScaleFactor() );

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

      QgsSymbolLayer* symShdwL = QgsSvgMarkerSymbolLayer::create( shdwmap );
      QgsSvgMarkerSymbolLayer* svgShdwM = static_cast<QgsSvgMarkerSymbolLayer*>( symShdwL );
      QgsSymbolRenderContext svgShdwContext( shdwContext, QgsUnitTypes::RenderUnknownUnit, shadow.opacity() );

      double svgSize = QgsTextRenderer::scaleToPixelContext( sizeOut, context, background.sizeUnit(),
                       true, background.sizeMapUnitScale() );
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
      p->scale( 1.0 / context.rasterScaleFactor(), 1.0 / context.rasterScaleFactor() );
      double xoff = QgsTextRenderer::scaleToPixelContext( background.offset().x(), context, background.offsetUnit(), true, background.offsetMapUnitScale() );
      double yoff = QgsTextRenderer::scaleToPixelContext( background.offset().y(), context, background.offsetUnit(), true, background.offsetMapUnitScale() );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset() );
      p->translate( -svgSize / 2, svgSize / 2 );

      drawLabelShadow( context, component, tmpLyr );
      p->restore();

      delete svgShdwM;
      svgShdwM = nullptr;
    }

    // draw the actual symbol
    QgsSymbolLayer* symL = QgsSvgMarkerSymbolLayer::create( map );
    QgsSvgMarkerSymbolLayer* svgM = static_cast<QgsSvgMarkerSymbolLayer*>( symL );
    QgsSymbolRenderContext svgContext( context, QgsUnitTypes::RenderUnknownUnit, background.opacity() );

    p->save();
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( background.blendMode() );
    }
    p->translate( component.center().x(), component.center().y() );
    p->rotate( component.rotation() );
    double xoff = QgsTextRenderer::scaleToPixelContext( background.offset().x(), context, background.offsetUnit(), false, background.offsetMapUnitScale() );
    double yoff = QgsTextRenderer::scaleToPixelContext( background.offset().y(), context, background.offsetUnit(), false, background.offsetMapUnitScale() );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );
    svgM->renderPoint( QPointF( 0, 0 ), svgContext );
    p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
    p->restore();

    delete svgM;
    svgM = nullptr;

  }
  else  // Generated Shapes
  {
    // all calculations done in background size units
    double w = QgsSymbolLayerUtils::convertFromMapUnits( context, labelWidth, background.sizeUnit() );
    double h = QgsSymbolLayerUtils::convertFromMapUnits( context, labelHeight, background.sizeUnit() );

    double xsize = background.size().width();
    double ysize = background.size().height();

    if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
    {
      w = xsize;
      h = ysize;
    }
    else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
    {
      if ( background.type() == QgsTextBackgroundSettings::ShapeSquare )
      {
        if ( w > h )
          h = w;
        else if ( h > w )
          w = h;
      }
      else if ( background.type() == QgsTextBackgroundSettings::ShapeCircle )
      {
        // start with label bound by circle
        h = sqrt( pow( w, 2 ) + pow( h, 2 ) );
        w = h;
      }
      else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse )
      {
        // start with label bound by ellipse
        h = h / sqrt( 2.0 ) * 2;
        w = w / sqrt( 2.0 ) * 2;
      }

      w += xsize * 2;
      h += ysize * 2;
    }

    // convert everything over to map pixels from here on
    w = QgsTextRenderer::scaleToPixelContext( w, context, background.sizeUnit(), true, background.sizeMapUnitScale() );
    h = QgsTextRenderer::scaleToPixelContext( h, context, background.sizeUnit(), true, background.sizeMapUnitScale() );

    // offsets match those of symbology: -x = left, -y = up
    QRectF rect( -w / 2.0, - h / 2.0, w, h );

    if ( rect.isNull() )
      return;

    p->save();
    p->translate( QPointF( component.center().x(), component.center().y() ) );
    p->rotate( component.rotation() );
    double xoff = QgsTextRenderer::scaleToPixelContext( background.offset().x(), context, background.offsetUnit(), false, background.offsetMapUnitScale() );
    double yoff = QgsTextRenderer::scaleToPixelContext( background.offset().y(), context, background.offsetUnit(), false, background.offsetMapUnitScale() );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );

    double penSize = QgsTextRenderer::scaleToPixelContext( background.borderWidth(), context, background.borderWidthUnit(), true, background.borderWidthMapUnitScale() );

    QPen pen;
    if ( background.borderWidth() > 0 )
    {
      pen.setColor( background.borderColor() );
      pen.setWidthF( penSize );
      if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle )
        pen.setJoinStyle( background.joinStyle() );
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
    shapep.setBrush( background.fillColor() );

    if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle
         || background.type() == QgsTextBackgroundSettings::ShapeSquare )
    {
      if ( background.radiiUnit() == QgsUnitTypes::RenderPercentage )
      {
        shapep.drawRoundedRect( rect, background.radii().width(), background.radii().height(), Qt::RelativeSize );
      }
      else
      {
        double xRadius = QgsTextRenderer::scaleToPixelContext( background.radii().width(), context, background.radiiUnit(), true, background.radiiMapUnitScale() );
        double yRadius = QgsTextRenderer::scaleToPixelContext( background.radii().height(), context, background.radiiUnit(), true, background.radiiMapUnitScale() );
        shapep.drawRoundedRect( rect, xRadius, yRadius );
      }
    }
    else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse
              || background.type() == QgsTextBackgroundSettings::ShapeCircle )
    {
      shapep.drawEllipse( rect );
    }
    shapep.end();

    p->scale( 1.0 / context.rasterScaleFactor(), 1.0 / context.rasterScaleFactor() );

    if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
    {
      component.setPicture( &shapePict );
      component.setPictureBuffer( penSize / 2.0 );

      component.setSize( QgsPoint( rect.width(), rect.height() ) );
      component.setOffset( QgsPoint( rect.width() / 2, -rect.height() / 2 ) );
      drawLabelShadow( context, component, tmpLyr );
    }

    p->setOpacity( background.opacity() );
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( background.blendMode() );
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

  QgsTextFormat format = tmpLyr.format();
  QgsTextShadowSettings shadow = format.shadow();

  QPainter* p = context.painter();
  double componentWidth = component.size().x(), componentHeight = component.size().y();
  double xOffset = component.offset().x(), yOffset = component.offset().y();
  double pictbuffer = component.pictureBuffer();

  // generate pixmap representation of label component drawing
  bool mapUnits = shadow.blurRadiusUnit() == QgsUnitTypes::RenderMapUnits;
  double radius = QgsTextRenderer::scaleToPixelContext( shadow.blurRadius(), context, shadow.blurRadiusUnit(), !mapUnits, shadow.blurRadiusMapUnitScale() );
  radius /= ( mapUnits ? tmpLyr.vectorScaleFactor / component.dpiRatio() : 1 );
  radius = static_cast< int >( radius + 0.5 );

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
  pictp.fillRect( blurImg.rect(), shadow.color() );
  pictp.end();

  // blur the QImage in-place
  if ( shadow.blurRadius() > 0.0 && radius > 0 )
  {
    QgsSymbolLayerUtils::blurImageInPlace( blurImg, blurImg.rect(), radius, shadow.blurAlphaOnly() );
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

  double offsetDist = QgsTextRenderer::scaleToPixelContext( shadow.offsetDistance(), context,
                      shadow.offsetUnit(), true, shadow.offsetMapUnitScale() );
  double angleRad = shadow.offsetAngle() * M_PI / 180; // to radians
  if ( shadow.offsetGlobal() )
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
    p->setCompositionMode( shadow.blendMode() );
  }
  p->setOpacity( shadow.opacity() );

  double scale = shadow.scale() / 100.0;
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
  mEngine->readSettingsFromProject();
}

void QgsPalLabeling::saveEngineSettings()
{
  mEngine->writeSettingsToProject();
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

QgsPalLabeling* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->setShowingAllLabels( isShowingAllLabels() );
  lbl->setShowingCandidates( isShowingCandidates() );
  lbl->setDrawLabelRectOnly( drawLabelRectOnly() );
  lbl->setShowingShadowRectangles( isShowingShadowRectangles() );
  lbl->setShowingPartialsLabels( isShowingPartialsLabels() );
  lbl->setDrawingOutlineLabels( isDrawingOutlineLabels() );
  return lbl;
}


QgsLabelingResults::QgsLabelingResults()
{
  mLabelSearchTree = new QgsLabelSearchTree();
}

QgsLabelingResults::~QgsLabelingResults()
{
  delete mLabelSearchTree;
  mLabelSearchTree = nullptr;
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
