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
#include "qgsexception.h"

#include <list>

#include "pal/pal.h"
#include "pal/feature.h"
#include "pal/layer.h"
#include "pal/palexception.h"
#include "pal/problem.h"
#include "pal/labelposition.h"

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
#include "qgslabelingengine.h"
#include "qgsvectorlayerlabeling.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayerdiagramprovider.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgsgeometry.h"
#include "qgsmarkersymbollayer.h"
#include "qgspainting.h"
#include "qgsproject.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgscurvepolygon.h"
#include <QMessageBox>


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
const QVector< QgsPalLayerSettings::PredefinedPointPosition > QgsPalLayerSettings::DEFAULT_PLACEMENT_ORDER
{
  QgsPalLayerSettings::TopRight,
  QgsPalLayerSettings::TopLeft,
  QgsPalLayerSettings::BottomRight,
  QgsPalLayerSettings::BottomLeft,
  QgsPalLayerSettings::MiddleRight,
  QgsPalLayerSettings::MiddleLeft,
  QgsPalLayerSettings::TopSlightlyRight,
  QgsPalLayerSettings::BottomSlightlyRight
};
//debugging only - don't use these placements by default
/* << QgsPalLayerSettings::TopSlightlyLeft
<< QgsPalLayerSettings::BottomSlightlyLeft;
<< QgsPalLayerSettings::TopMiddle
<< QgsPalLayerSettings::BottomMiddle;*/

QgsPropertiesDefinition QgsPalLayerSettings::sPropertyDefinitions;

void QgsPalLayerSettings::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  const QString origin = QStringLiteral( "labeling" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsPalLayerSettings::Size, QgsPropertyDefinition( "Size", QObject::tr( "Font size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::Bold, QgsPropertyDefinition( "Bold", QObject::tr( "Bold style" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::Italic, QgsPropertyDefinition( "Italic", QObject::tr( "Italic style" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::Underline, QgsPropertyDefinition( "Underline", QObject::tr( "Draw underline" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::Color, QgsPropertyDefinition( "Color", QObject::tr( "Text color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { QgsPalLayerSettings::Strikeout, QgsPropertyDefinition( "Strikeout", QObject::tr( "Draw strikeout" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      QgsPalLayerSettings::Family, QgsPropertyDefinition( "Family", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font family" ), QObject::tr( "string " ) + QObject::tr( "[<b>family</b>|<b>family[foundry]</b>],<br>"
          "e.g. Helvetica or Helvetica [Cronyx]" ), origin )
    },
    {
      QgsPalLayerSettings::FontStyle, QgsPropertyDefinition( "FontStyle", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font style" ), QObject::tr( "string " ) + QObject::tr( "[<b>font style name</b>|<b>Ignore</b>],<br>"
          "e.g. Bold Condensed or Light Italic" ), origin )
    },
    { QgsPalLayerSettings::FontSizeUnit, QgsPropertyDefinition( "FontSizeUnit", QObject::tr( "Font size units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::FontTransp, QgsPropertyDefinition( "FontTransp", QObject::tr( "Text transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::FontOpacity, QgsPropertyDefinition( "FontOpacity", QObject::tr( "Text opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::FontCase, QgsPropertyDefinition( "FontCase", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font case" ), QObject::tr( "string " ) + QStringLiteral( "[<b>NoChange</b>|<b>Upper</b>|<br><b>Lower</b>|<b>Capitalize</b>]" ), origin ) },
    { QgsPalLayerSettings::FontLetterSpacing, QgsPropertyDefinition( "FontLetterSpacing", QObject::tr( "Letter spacing" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::FontWordSpacing, QgsPropertyDefinition( "FontWordSpacing", QObject::tr( "Word spacing" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::FontBlendMode, QgsPropertyDefinition( "FontBlendMode", QObject::tr( "Text blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { QgsPalLayerSettings::MultiLineWrapChar, QgsPropertyDefinition( "MultiLineWrapChar", QObject::tr( "Wrap character" ), QgsPropertyDefinition::String, origin ) },
    { QgsPalLayerSettings::AutoWrapLength, QgsPropertyDefinition( "AutoWrapLength", QObject::tr( "Automatic word wrap line length" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::MultiLineHeight, QgsPropertyDefinition( "MultiLineHeight", QObject::tr( "Line height" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::MultiLineAlignment, QgsPropertyDefinition( "MultiLineAlignment", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line alignment" ), QObject::tr( "string " ) + "[<b>Left</b>|<b>Center</b>|<b>Right</b>|<b>Follow</b>]", origin ) },
    { QgsPalLayerSettings::DirSymbDraw, QgsPropertyDefinition( "DirSymbDraw", QObject::tr( "Draw direction symbol" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::DirSymbLeft, QgsPropertyDefinition( "DirSymbLeft", QObject::tr( "Left direction symbol" ), QgsPropertyDefinition::String, origin ) },
    { QgsPalLayerSettings::DirSymbRight, QgsPropertyDefinition( "DirSymbRight", QObject::tr( "Right direction symbol" ), QgsPropertyDefinition::String, origin ) },
    { QgsPalLayerSettings::DirSymbPlacement, QgsPropertyDefinition( "DirSymbPlacement", QgsPropertyDefinition::DataTypeString, QObject::tr( "Direction symbol placement" ), QObject::tr( "string " ) + "[<b>LeftRight</b>|<b>Above</b>|<b>Below</b>]", origin ) },
    { QgsPalLayerSettings::DirSymbReverse, QgsPropertyDefinition( "DirSymbReverse", QObject::tr( "Reverse direction symbol" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::NumFormat, QgsPropertyDefinition( "NumFormat", QObject::tr( "Format as number" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::NumDecimals, QgsPropertyDefinition( "NumDecimals", QObject::tr( "Number of decimal places" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::NumPlusSign, QgsPropertyDefinition( "NumPlusSign", QObject::tr( "Draw + sign" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::BufferDraw, QgsPropertyDefinition( "BufferDraw", QObject::tr( "Draw buffer" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::BufferSize, QgsPropertyDefinition( "BufferSize", QObject::tr( "Symbol size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::BufferUnit, QgsPropertyDefinition( "BufferUnit", QObject::tr( "Buffer units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::BufferColor, QgsPropertyDefinition( "BufferColor", QObject::tr( "Buffer color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { QgsPalLayerSettings::BufferTransp, QgsPropertyDefinition( "BufferTransp", QObject::tr( "Buffer transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::BufferOpacity, QgsPropertyDefinition( "BufferOpacity", QObject::tr( "Buffer opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::BufferJoinStyle, QgsPropertyDefinition( "BufferJoinStyle", QObject::tr( "Buffer join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },
    { QgsPalLayerSettings::BufferBlendMode, QgsPropertyDefinition( "BufferBlendMode", QObject::tr( "Buffer blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { QgsPalLayerSettings::ShapeDraw, QgsPropertyDefinition( "ShapeDraw", QObject::tr( "Draw shape" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      QgsPalLayerSettings::ShapeKind, QgsPropertyDefinition( "ShapeKind", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape type" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Rectangle</b>|<b>Square</b>|<br>"
          "<b>Ellipse</b>|<b>Circle</b>|<b>SVG</b>]" ), origin )
    },
    { QgsPalLayerSettings::ShapeSVGFile, QgsPropertyDefinition( "ShapeSVGFile", QObject::tr( "Shape SVG path" ), QgsPropertyDefinition::SvgPath, origin ) },
    { QgsPalLayerSettings::ShapeSizeType, QgsPropertyDefinition( "ShapeSizeType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape size type" ), QObject::tr( "string " ) + "[<b>Buffer</b>|<b>Fixed</b>]", origin ) },
    { QgsPalLayerSettings::ShapeSizeX, QgsPropertyDefinition( "ShapeSizeX", QObject::tr( "Shape size (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::ShapeSizeY, QgsPropertyDefinition( "ShapeSizeY", QObject::tr( "Shape size (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::ShapeSizeUnits, QgsPropertyDefinition( "ShapeSizeUnits", QObject::tr( "Shape size units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShapeRotationType, QgsPropertyDefinition( "ShapeRotationType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape rotation type" ), QObject::tr( "string " ) + "[<b>Sync</b>|<b>Offset</b>|<b>Fixed</b>]", origin ) },
    { QgsPalLayerSettings::ShapeRotation, QgsPropertyDefinition( "ShapeRotation", QObject::tr( "Shape rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::ShapeOffset, QgsPropertyDefinition( "ShapeOffset", QObject::tr( "Shape offset" ), QgsPropertyDefinition::Offset, origin ) },
    { QgsPalLayerSettings::ShapeOffsetUnits, QgsPropertyDefinition( "ShapeOffsetUnits", QObject::tr( "Shape offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShapeRadii, QgsPropertyDefinition( "ShapeRadii", QObject::tr( "Shape radii" ), QgsPropertyDefinition::Size2D, origin ) },
    { QgsPalLayerSettings::ShapeRadiiUnits, QgsPropertyDefinition( "ShapeRadiiUnits", QObject::tr( "Symbol radii units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShapeTransparency, QgsPropertyDefinition( "ShapeTransparency", QObject::tr( "Shape transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::ShapeOpacity, QgsPropertyDefinition( "ShapeOpacity", QObject::tr( "Shape opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::ShapeBlendMode, QgsPropertyDefinition( "ShapeBlendMode", QObject::tr( "Shape blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { QgsPalLayerSettings::ShapeFillColor, QgsPropertyDefinition( "ShapeFillColor", QObject::tr( "Shape fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsPalLayerSettings::ShapeStrokeColor, QgsPropertyDefinition( "ShapeBorderColor", QObject::tr( "Shape stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsPalLayerSettings::ShapeStrokeWidth, QgsPropertyDefinition( "ShapeBorderWidth", QObject::tr( "Shape stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { QgsPalLayerSettings::ShapeStrokeWidthUnits, QgsPropertyDefinition( "ShapeBorderWidthUnits", QObject::tr( "Shape stroke width units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShapeJoinStyle, QgsPropertyDefinition( "ShapeJoinStyle", QObject::tr( "Shape join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },
    { QgsPalLayerSettings::ShadowDraw, QgsPropertyDefinition( "ShadowDraw", QObject::tr( "Draw shadow" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      QgsPalLayerSettings::ShadowUnder, QgsPropertyDefinition( "ShadowUnder", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Lowest</b>|<b>Text</b>|<br>"
          "<b>Buffer</b>|<b>Background</b>]" ), origin )
    },
    { QgsPalLayerSettings::ShadowOffsetAngle, QgsPropertyDefinition( "ShadowOffsetAngle", QObject::tr( "Shadow offset angle" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::ShadowOffsetDist, QgsPropertyDefinition( "ShadowOffsetDist", QObject::tr( "Shadow offset distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::ShadowOffsetUnits, QgsPropertyDefinition( "ShadowOffsetUnits", QObject::tr( "Shadow offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShadowRadius, QgsPropertyDefinition( "ShadowRadius", QObject::tr( "Shadow blur radius" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::ShadowRadiusUnits, QgsPropertyDefinition( "ShadowRadiusUnits", QObject::tr( "Shadow blur units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::ShadowTransparency, QgsPropertyDefinition( "ShadowTransparency", QObject::tr( "Shadow transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::ShadowOpacity, QgsPropertyDefinition( "ShadowOpacity", QObject::tr( "Shadow opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::ShadowScale, QgsPropertyDefinition( "ShadowScale", QObject::tr( "Shadow scale" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::ShadowColor, QgsPropertyDefinition( "ShadowColor", QObject::tr( "Shadow color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { QgsPalLayerSettings::ShadowBlendMode, QgsPropertyDefinition( "ShadowBlendMode", QObject::tr( "Shadow blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },

    { QgsPalLayerSettings::CentroidWhole, QgsPropertyDefinition( "CentroidWhole", QgsPropertyDefinition::DataTypeString, QObject::tr( "Centroid of whole shape" ), QObject::tr( "string " ) + "[<b>Visible</b>|<b>Whole</b>]", origin ) },
    {
      QgsPalLayerSettings::OffsetQuad, QgsPropertyDefinition( "OffsetQuad", QgsPropertyDefinition::DataTypeString, QObject::tr( "Offset quadrant" ), QObject::tr( "int<br>" ) + QStringLiteral( "[<b>0</b>=Above Left|<b>1</b>=Above|<b>2</b>=Above Right|<br>"
          "<b>3</b>=Left|<b>4</b>=Over|<b>5</b>=Right|<br>"
          "<b>6</b>=Below Left|<b>7</b>=Below|<b>8</b>=Below Right]" ), origin )
    },
    { QgsPalLayerSettings::OffsetXY, QgsPropertyDefinition( "OffsetXY", QObject::tr( "Offset" ), QgsPropertyDefinition::Offset, origin ) },
    { QgsPalLayerSettings::OffsetUnits, QgsPropertyDefinition( "OffsetUnits", QObject::tr( "Offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::LabelDistance, QgsPropertyDefinition( "LabelDistance", QObject::tr( "Label distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::DistanceUnits, QgsPropertyDefinition( "DistanceUnits", QObject::tr( "Label distance units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::OffsetRotation, QgsPropertyDefinition( "OffsetRotation", QObject::tr( "Offset rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::CurvedCharAngleInOut, QgsPropertyDefinition( "CurvedCharAngleInOut", QgsPropertyDefinition::DataTypeString, QObject::tr( "Curved character angles" ), QObject::tr( "double coord [<b>in,out</b> as 20.0-60.0,20.0-95.0]" ), origin ) },
    { QgsPalLayerSettings::RepeatDistance, QgsPropertyDefinition( "RepeatDistance", QObject::tr( "Repeat distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::RepeatDistanceUnit, QgsPropertyDefinition( "RepeatDistanceUnit", QObject::tr( "Repeat distance unit" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::Priority, QgsPropertyDefinition( "Priority", QgsPropertyDefinition::DataTypeString, QObject::tr( "Label priority" ), QObject::tr( "double [0.0-10.0]" ), origin ) },
    { QgsPalLayerSettings::IsObstacle, QgsPropertyDefinition( "IsObstacle", QObject::tr( "Feature is a label obstacle" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::ObstacleFactor, QgsPropertyDefinition( "ObstacleFactor", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Obstacle factor" ), QObject::tr( "double [0.0-10.0]" ), origin ) },
    {
      QgsPalLayerSettings::PredefinedPositionOrder, QgsPropertyDefinition( "PredefinedPositionOrder", QgsPropertyDefinition::DataTypeString, QObject::tr( "Predefined position order" ),  QObject::tr( "Comma separated list of placements in order of priority<br>" )
          + QStringLiteral( "[<b>TL</b>=Top left|<b>TSL</b>=Top, slightly left|<b>T</b>=Top middle|<br>"
                            "<b>TSR</b>=Top, slightly right|<b>TR</b>=Top right|<br>"
                            "<b>L</b>=Left|<b>R</b>=Right|<br>"
                            "<b>BL</b>=Bottom left|<b>BSL</b>=Bottom, slightly left|<b>B</b>=Bottom middle|<br>"
                            "<b>BSR</b>=Bottom, slightly right|<b>BR</b>=Bottom right]" ), origin )
    },
    { QgsPalLayerSettings::PositionX, QgsPropertyDefinition( "PositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::PositionY, QgsPropertyDefinition( "PositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::Hali, QgsPropertyDefinition( "Hali", QgsPropertyDefinition::DataTypeString, QObject::tr( "Horizontal alignment" ), QObject::tr( "string " ) + "[<b>Left</b>|<b>Center</b>|<b>Right</b>]", origin ) },
    {
      QgsPalLayerSettings::Vali, QgsPropertyDefinition( "Vali", QgsPropertyDefinition::DataTypeString, QObject::tr( "Vertical alignment" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Bottom</b>|<b>Base</b>|<br>"
          "<b>Half</b>|<b>Cap</b>|<b>Top</b>]" ), origin )
    },
    { QgsPalLayerSettings::Rotation, QgsPropertyDefinition( "Rotation", QObject::tr( "Label rotation (deprecated)" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::LabelRotation, QgsPropertyDefinition( "LabelRotation", QObject::tr( "Label rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::ScaleVisibility, QgsPropertyDefinition( "ScaleVisibility", QObject::tr( "Scale based visibility" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::MinScale, QgsPropertyDefinition( "MinScale", QObject::tr( "Minimum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::MaxScale, QgsPropertyDefinition( "MaxScale", QObject::tr( "Maximum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::MinimumScale, QgsPropertyDefinition( "MinimumScale", QObject::tr( "Minimum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::MaximumScale, QgsPropertyDefinition( "MaximumScale", QObject::tr( "Maximum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },

    { QgsPalLayerSettings::FontLimitPixel, QgsPropertyDefinition( "FontLimitPixel", QObject::tr( "Limit font pixel size" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::FontMinPixel, QgsPropertyDefinition( "FontMinPixel", QObject::tr( "Minimum pixel size" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::FontMaxPixel, QgsPropertyDefinition( "FontMaxPixel", QObject::tr( "Maximum pixel size" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::ZIndex, QgsPropertyDefinition( "ZIndex", QObject::tr( "Label z-index" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::Show, QgsPropertyDefinition( "Show", QObject::tr( "Show label" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::AlwaysShow, QgsPropertyDefinition( "AlwaysShow", QObject::tr( "Always show label" ), QgsPropertyDefinition::Boolean, origin ) },
  };
}

QgsPalLayerSettings::QgsPalLayerSettings()
{
  initPropertyDefinitions();

  drawLabels = true;
  isExpression = false;
  fieldIndex = 0;

  previewBkgrdColor = Qt::white;
  useSubstitutions = false;

  // text formatting
  multilineAlign = MultiFollowPlacement;
  addDirectionSymbol = false;
  leftDirectionSymbol = QStringLiteral( "<" );
  rightDirectionSymbol = QStringLiteral( ">" );
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
  offsetUnits = QgsUnitTypes::RenderMillimeters;
  dist = 0;
  distUnits = QgsUnitTypes::RenderMillimeters;
  offsetType = FromPoint;
  angleOffset = 0;
  preserveRotation = true;
  maxCurvedCharAngleIn = 25.0;
  maxCurvedCharAngleOut = -25.0;
  priority = 5;
  repeatDistance = 0;
  repeatDistanceUnit = QgsUnitTypes::RenderMillimeters;

  // rendering
  scaleVisibility = false;
  maximumScale = 0.0;
  minimumScale = 0.0;
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
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings &s )
  : fieldIndex( 0 )
  , mDataDefinedProperties( s.mDataDefinedProperties )
{
  *this = s;
}

QgsPalLayerSettings &QgsPalLayerSettings::operator=( const QgsPalLayerSettings &s )
{
  if ( this == &s )
    return *this;

  // copy only permanent stuff

  drawLabels = s.drawLabels;

  // text style
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  previewBkgrdColor = s.previewBkgrdColor;
  substitutions = s.substitutions;
  useSubstitutions = s.useSubstitutions;

  // text formatting
  wrapChar = s.wrapChar;
  autoWrapLength = s.autoWrapLength;
  useMaxLineLengthForAutoWrap = s.useMaxLineLengthForAutoWrap;
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
  offsetUnits = s.offsetUnits;
  labelOffsetMapUnitScale = s.labelOffsetMapUnitScale;
  dist = s.dist;
  offsetType = s.offsetType;
  distUnits = s.distUnits;
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
  maximumScale = s.maximumScale;
  minimumScale = s.minimumScale;
  fontLimitPixelSize = s.fontLimitPixelSize;
  fontMinPixelSize = s.fontMinPixelSize;
  fontMaxPixelSize = s.fontMaxPixelSize;
  displayAll = s.displayAll;
  upsidedownLabels = s.upsidedownLabels;

  labelPerPart = s.labelPerPart;
  mergeLines = s.mergeLines && !s.addDirectionSymbol;
  minFeatureSize = s.minFeatureSize;
  limitNumLabels = s.limitNumLabels;
  maxNumLabels = s.maxNumLabels;
  obstacle = s.obstacle;
  obstacleFactor = s.obstacleFactor;
  obstacleType = s.obstacleType;
  zIndex = s.zIndex;

  mFormat = s.mFormat;
  mDataDefinedProperties = s.mDataDefinedProperties;

  return *this;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
{
  // pal layer is deleted internally in PAL

  delete expression;
}


const QgsPropertiesDefinition &QgsPalLayerSettings::propertyDefinitions()
{
  initPropertyDefinitions();
  return sPropertyDefinitions;
}

QgsExpression *QgsPalLayerSettings::getLabelExpression()
{
  if ( !expression )
  {
    expression = new QgsExpression( fieldName );
  }
  return expression;
}

static Qt::PenJoinStyle _decodePenJoinStyle( const QString &str )
{
  if ( str.compare( QLatin1String( "Miter" ), Qt::CaseInsensitive ) == 0 ) return Qt::MiterJoin;
  if ( str.compare( QLatin1String( "Round" ), Qt::CaseInsensitive ) == 0 ) return Qt::RoundJoin;
  return Qt::BevelJoin; // "Bevel"
}

QString updateDataDefinedString( const QString &value )
{
  // TODO: update or remove this when project settings for labeling are migrated to better XML layout
  QString newValue = value;
  if ( !value.isEmpty() && !value.contains( QLatin1String( "~~" ) ) )
  {
    QStringList values;
    values << QStringLiteral( "1" ); // all old-style values are active if not empty
    values << QStringLiteral( "0" );
    values << QString();
    values << value; // all old-style values are only field names
    newValue = values.join( QStringLiteral( "~~" ) );
  }

  return newValue;
}

void QgsPalLayerSettings::readOldDataDefinedProperty( QgsVectorLayer *layer, QgsPalLayerSettings::Property p )
{
  QString newPropertyName = "labeling/dataDefined/" + sPropertyDefinitions.value( p ).name();
  QVariant newPropertyField = layer->customProperty( newPropertyName, QVariant() );

  if ( !newPropertyField.isValid() )
    return;

  QString ddString = newPropertyField.toString();

  if ( !ddString.isEmpty() && ddString != QLatin1String( "0~~0~~~~" ) )
  {
    // TODO: update this when project settings for labeling are migrated to better XML layout
    QString newStyleString = updateDataDefinedString( ddString );
    QStringList ddv = newStyleString.split( QStringLiteral( "~~" ) );

    bool active = ddv.at( 0 ).toInt();
    if ( ddv.at( 1 ).toInt() )
    {
      mDataDefinedProperties.setProperty( p, QgsProperty::fromExpression( ddv.at( 2 ), active ) );
    }
    else
    {
      mDataDefinedProperties.setProperty( p, QgsProperty::fromField( ddv.at( 3 ), active ) );
    }
  }
  else
  {
    // remove unused properties
    layer->removeCustomProperty( newPropertyName );
  }
}

void QgsPalLayerSettings::readOldDataDefinedPropertyMap( QgsVectorLayer *layer, QDomElement *parentElem )
{
  if ( !layer && !parentElem )
  {
    return;
  }

  QgsPropertiesDefinition::const_iterator i = sPropertyDefinitions.constBegin();
  for ( ; i != sPropertyDefinitions.constEnd(); ++i )
  {
    if ( layer )
    {
      // reading from layer's custom properties
      readOldDataDefinedProperty( layer, static_cast< Property >( i.key() ) );
    }
    else if ( parentElem )
    {
      // reading from XML
      QDomElement e = parentElem->firstChildElement( i.value().name() );
      if ( !e.isNull() )
      {
        bool active = e.attribute( QStringLiteral( "active" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
        bool isExpression = e.attribute( QStringLiteral( "useExpr" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
        if ( isExpression )
        {
          mDataDefinedProperties.setProperty( i.key(), QgsProperty::fromExpression( e.attribute( QStringLiteral( "expr" ) ), active ) );
        }
        else
        {
          mDataDefinedProperties.setProperty( i.key(), QgsProperty::fromField( e.attribute( QStringLiteral( "field" ) ), active ) );
        }
      }
    }
  }
}

void QgsPalLayerSettings::readFromLayerCustomProperties( QgsVectorLayer *layer )
{
  if ( layer->customProperty( QStringLiteral( "labeling" ) ).toString() != QLatin1String( "pal" ) )
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

  drawLabels = layer->customProperty( QStringLiteral( "labeling/drawLabels" ), true ).toBool();

  mFormat.readFromLayer( layer );

  // text style
  fieldName = layer->customProperty( QStringLiteral( "labeling/fieldName" ) ).toString();
  isExpression = layer->customProperty( QStringLiteral( "labeling/isExpression" ) ).toBool();
  previewBkgrdColor = QColor( layer->customProperty( QStringLiteral( "labeling/previewBkgrdColor" ), QVariant( "#ffffff" ) ).toString() );
  QDomDocument doc( QStringLiteral( "substitutions" ) );
  doc.setContent( layer->customProperty( QStringLiteral( "labeling/substitutions" ) ).toString() );
  QDomElement replacementElem = doc.firstChildElement( QStringLiteral( "substitutions" ) );
  substitutions.readXml( replacementElem );
  useSubstitutions = layer->customProperty( QStringLiteral( "labeling/useSubstitutions" ) ).toBool();

  // text formatting
  wrapChar = layer->customProperty( QStringLiteral( "labeling/wrapChar" ) ).toString();
  autoWrapLength = layer->customProperty( QStringLiteral( "labeling/autoWrapLength" ) ).toInt();
  useMaxLineLengthForAutoWrap = layer->customProperty( QStringLiteral( "labeling/useMaxLineLengthForAutoWrap" ), QStringLiteral( "1" ) ).toBool();

  multilineAlign = static_cast< MultiLineAlign >( layer->customProperty( QStringLiteral( "labeling/multilineAlign" ), QVariant( MultiFollowPlacement ) ).toUInt() );
  addDirectionSymbol = layer->customProperty( QStringLiteral( "labeling/addDirectionSymbol" ) ).toBool();
  leftDirectionSymbol = layer->customProperty( QStringLiteral( "labeling/leftDirectionSymbol" ), QVariant( "<" ) ).toString();
  rightDirectionSymbol = layer->customProperty( QStringLiteral( "labeling/rightDirectionSymbol" ), QVariant( ">" ) ).toString();
  reverseDirectionSymbol = layer->customProperty( QStringLiteral( "labeling/reverseDirectionSymbol" ) ).toBool();
  placeDirectionSymbol = static_cast< DirectionSymbols >( layer->customProperty( QStringLiteral( "labeling/placeDirectionSymbol" ), QVariant( SymbolLeftRight ) ).toUInt() );
  formatNumbers = layer->customProperty( QStringLiteral( "labeling/formatNumbers" ) ).toBool();
  decimals = layer->customProperty( QStringLiteral( "labeling/decimals" ) ).toInt();
  plusSign = layer->customProperty( QStringLiteral( "labeling/plussign" ) ).toBool();

  // placement
  placement = static_cast< Placement >( layer->customProperty( QStringLiteral( "labeling/placement" ) ).toInt() );
  placementFlags = layer->customProperty( QStringLiteral( "labeling/placementFlags" ) ).toUInt();
  centroidWhole = layer->customProperty( QStringLiteral( "labeling/centroidWhole" ), QVariant( false ) ).toBool();
  centroidInside = layer->customProperty( QStringLiteral( "labeling/centroidInside" ), QVariant( false ) ).toBool();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( layer->customProperty( QStringLiteral( "labeling/predefinedPositionOrder" ) ).toString() );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = DEFAULT_PLACEMENT_ORDER;
  fitInPolygonOnly = layer->customProperty( QStringLiteral( "labeling/fitInPolygonOnly" ), QVariant( false ) ).toBool();
  dist = layer->customProperty( QStringLiteral( "labeling/dist" ) ).toDouble();
  distUnits = layer->customProperty( QStringLiteral( "labeling/distInMapUnits" ) ).toBool() ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  if ( layer->customProperty( QStringLiteral( "labeling/distMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/distMapUnitMinScale" ), 0.0 ).toDouble();
    distMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/distMapUnitMaxScale" ), 0.0 ).toDouble();
    distMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    distMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/distMapUnitScale" ) ).toString() );
  }
  offsetType = static_cast< OffsetType >( layer->customProperty( QStringLiteral( "labeling/offsetType" ), QVariant( FromPoint ) ).toUInt() );
  quadOffset = static_cast< QuadrantPosition >( layer->customProperty( QStringLiteral( "labeling/quadOffset" ), QVariant( QuadrantOver ) ).toUInt() );
  xOffset = layer->customProperty( QStringLiteral( "labeling/xOffset" ), QVariant( 0.0 ) ).toDouble();
  yOffset = layer->customProperty( QStringLiteral( "labeling/yOffset" ), QVariant( 0.0 ) ).toDouble();
  if ( layer->customProperty( QStringLiteral( "labeling/labelOffsetInMapUnits" ), QVariant( true ) ).toBool() )
    offsetUnits = QgsUnitTypes::RenderMapUnits;
  else
    offsetUnits = QgsUnitTypes::RenderMillimeters;

  if ( layer->customProperty( QStringLiteral( "labeling/labelOffsetMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/labelOffsetMapUnitMinScale" ), 0.0 ).toDouble();
    labelOffsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/labelOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
    labelOffsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    labelOffsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/labelOffsetMapUnitScale" ) ).toString() );
  }

  QVariant tempAngle = layer->customProperty( QStringLiteral( "labeling/angleOffset" ), QVariant() );
  if ( tempAngle.isValid() )
  {
    double oldAngle = layer->customProperty( QStringLiteral( "labeling/angleOffset" ), QVariant( 0.0 ) ).toDouble();
    angleOffset = std::fmod( 360 - oldAngle, 360.0 );
  }
  else
  {
    angleOffset = layer->customProperty( QStringLiteral( "labeling/rotationAngle" ), QVariant( 0.0 ) ).toDouble();
  }

  preserveRotation = layer->customProperty( QStringLiteral( "labeling/preserveRotation" ), QVariant( true ) ).toBool();
  maxCurvedCharAngleIn = layer->customProperty( QStringLiteral( "labeling/maxCurvedCharAngleIn" ), QVariant( 25.0 ) ).toDouble();
  maxCurvedCharAngleOut = layer->customProperty( QStringLiteral( "labeling/maxCurvedCharAngleOut" ), QVariant( -25.0 ) ).toDouble();
  priority = layer->customProperty( QStringLiteral( "labeling/priority" ) ).toInt();
  repeatDistance = layer->customProperty( QStringLiteral( "labeling/repeatDistance" ), 0.0 ).toDouble();
  switch ( layer->customProperty( QStringLiteral( "labeling/repeatDistanceUnit" ), QVariant( 1 ) ).toUInt() )
  {
    case 0:
      repeatDistanceUnit = QgsUnitTypes::RenderPoints;
      break;
    case 1:
      repeatDistanceUnit = QgsUnitTypes::RenderMillimeters;
      break;
    case 2:
      repeatDistanceUnit = QgsUnitTypes::RenderMapUnits;
      break;
    case 3:
      repeatDistanceUnit = QgsUnitTypes::RenderPercentage;
      break;
  }
  if ( layer->customProperty( QStringLiteral( "labeling/repeatDistanceMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/repeatDistanceMapUnitMinScale" ), 0.0 ).toDouble();
    repeatDistanceMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/repeatDistanceMapUnitMaxScale" ), 0.0 ).toDouble();
    repeatDistanceMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    repeatDistanceMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/repeatDistanceMapUnitScale" ) ).toString() );
  }

  // rendering
  double scalemn = layer->customProperty( QStringLiteral( "labeling/scaleMin" ), QVariant( 0 ) ).toDouble();
  double scalemx = layer->customProperty( QStringLiteral( "labeling/scaleMax" ), QVariant( 0 ) ).toDouble();

  // fix for scale visibility limits being keyed off of just its values in the past (<2.0)
  QVariant scalevis = layer->customProperty( QStringLiteral( "labeling/scaleVisibility" ), QVariant() );
  if ( scalevis.isValid() )
  {
    scaleVisibility = scalevis.toBool();
    maximumScale = scalemn;
    minimumScale = scalemx;
  }
  else if ( scalemn > 0 || scalemx > 0 )
  {
    scaleVisibility = true;
    maximumScale = scalemn;
    minimumScale = scalemx;
  }
  else
  {
    // keep scaleMin and scaleMax at new 1.0 defaults (1 and 10000000, were 0 and 0)
    scaleVisibility = false;
  }


  fontLimitPixelSize = layer->customProperty( QStringLiteral( "labeling/fontLimitPixelSize" ), QVariant( false ) ).toBool();
  fontMinPixelSize = layer->customProperty( QStringLiteral( "labeling/fontMinPixelSize" ), QVariant( 0 ) ).toInt();
  fontMaxPixelSize = layer->customProperty( QStringLiteral( "labeling/fontMaxPixelSize" ), QVariant( 10000 ) ).toInt();
  displayAll = layer->customProperty( QStringLiteral( "labeling/displayAll" ), QVariant( false ) ).toBool();
  upsidedownLabels = static_cast< UpsideDownLabels >( layer->customProperty( QStringLiteral( "labeling/upsidedownLabels" ), QVariant( Upright ) ).toUInt() );

  labelPerPart = layer->customProperty( QStringLiteral( "labeling/labelPerPart" ) ).toBool();
  mergeLines = layer->customProperty( QStringLiteral( "labeling/mergeLines" ) ).toBool();
  minFeatureSize = layer->customProperty( QStringLiteral( "labeling/minFeatureSize" ) ).toDouble();
  limitNumLabels = layer->customProperty( QStringLiteral( "labeling/limitNumLabels" ), QVariant( false ) ).toBool();
  maxNumLabels = layer->customProperty( QStringLiteral( "labeling/maxNumLabels" ), QVariant( 2000 ) ).toInt();
  obstacle = layer->customProperty( QStringLiteral( "labeling/obstacle" ), QVariant( true ) ).toBool();
  obstacleFactor = layer->customProperty( QStringLiteral( "labeling/obstacleFactor" ), QVariant( 1.0 ) ).toDouble();
  obstacleType = static_cast< ObstacleType >( layer->customProperty( QStringLiteral( "labeling/obstacleType" ), QVariant( PolygonInterior ) ).toUInt() );
  zIndex = layer->customProperty( QStringLiteral( "labeling/zIndex" ), QVariant( 0.0 ) ).toDouble();

  mDataDefinedProperties.clear();
  if ( layer->customProperty( QStringLiteral( "labeling/ddProperties" ) ).isValid() )
  {
    QDomDocument doc( QStringLiteral( "dd" ) );
    doc.setContent( layer->customProperty( QStringLiteral( "labeling/ddProperties" ) ).toString() );
    QDomElement elem = doc.firstChildElement( QStringLiteral( "properties" ) );
    mDataDefinedProperties.readXml( elem, sPropertyDefinitions );
  }
  else
  {
    // read QGIS 2.x style data defined properties
    readOldDataDefinedPropertyMap( layer, nullptr );
  }
  // upgrade older data defined settings
  if ( mDataDefinedProperties.isActive( FontTransp ) )
  {
    mDataDefinedProperties.setProperty( FontOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( FontTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( FontTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( BufferTransp ) )
  {
    mDataDefinedProperties.setProperty( BufferOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( BufferTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( BufferTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( ShapeTransparency ) )
  {
    mDataDefinedProperties.setProperty( ShapeOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( ShapeTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( ShapeTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( ShadowTransparency ) )
  {
    mDataDefinedProperties.setProperty( ShadowOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( ShadowTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( ShadowTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( Rotation ) )
  {
    mDataDefinedProperties.setProperty( LabelRotation, QgsProperty::fromExpression( QStringLiteral( "360 - (%1)" ).arg( mDataDefinedProperties.property( Rotation ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( Rotation, QgsProperty() );
  }
  // older 2.x projects had min/max scale flipped - so change them here.
  if ( mDataDefinedProperties.isActive( MinScale ) )
  {
    mDataDefinedProperties.setProperty( MaximumScale, mDataDefinedProperties.property( MinScale ) );
    mDataDefinedProperties.setProperty( MinScale, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( MaxScale ) )
  {
    mDataDefinedProperties.setProperty( MinimumScale, mDataDefinedProperties.property( MaxScale ) );
    mDataDefinedProperties.setProperty( MaxScale, QgsProperty() );
  }
}

void QgsPalLayerSettings::readXml( QDomElement &elem, const QgsReadWriteContext &context )
{
  // text style
  QDomElement textStyleElem = elem.firstChildElement( QStringLiteral( "text-style" ) );
  fieldName = textStyleElem.attribute( QStringLiteral( "fieldName" ) );
  isExpression = textStyleElem.attribute( QStringLiteral( "isExpression" ) ).toInt();

  mFormat.readXml( elem, context );
  previewBkgrdColor = QColor( textStyleElem.attribute( QStringLiteral( "previewBkgrdColor" ), QStringLiteral( "#ffffff" ) ) );
  substitutions.readXml( textStyleElem.firstChildElement( QStringLiteral( "substitutions" ) ) );
  useSubstitutions = textStyleElem.attribute( QStringLiteral( "useSubstitutions" ) ).toInt();

  // text formatting
  QDomElement textFormatElem = elem.firstChildElement( QStringLiteral( "text-format" ) );
  wrapChar = textFormatElem.attribute( QStringLiteral( "wrapChar" ) );
  autoWrapLength = textFormatElem.attribute( QStringLiteral( "autoWrapLength" ), QStringLiteral( "0" ) ).toInt();
  useMaxLineLengthForAutoWrap = textFormatElem.attribute( QStringLiteral( "useMaxLineLengthForAutoWrap" ), QStringLiteral( "1" ) ).toInt();
  multilineAlign = static_cast< MultiLineAlign >( textFormatElem.attribute( QStringLiteral( "multilineAlign" ), QString::number( MultiFollowPlacement ) ).toUInt() );
  addDirectionSymbol = textFormatElem.attribute( QStringLiteral( "addDirectionSymbol" ) ).toInt();
  leftDirectionSymbol = textFormatElem.attribute( QStringLiteral( "leftDirectionSymbol" ), QStringLiteral( "<" ) );
  rightDirectionSymbol = textFormatElem.attribute( QStringLiteral( "rightDirectionSymbol" ), QStringLiteral( ">" ) );
  reverseDirectionSymbol = textFormatElem.attribute( QStringLiteral( "reverseDirectionSymbol" ) ).toInt();
  placeDirectionSymbol = static_cast< DirectionSymbols >( textFormatElem.attribute( QStringLiteral( "placeDirectionSymbol" ), QString::number( SymbolLeftRight ) ).toUInt() );
  formatNumbers = textFormatElem.attribute( QStringLiteral( "formatNumbers" ) ).toInt();
  decimals = textFormatElem.attribute( QStringLiteral( "decimals" ) ).toInt();
  plusSign = textFormatElem.attribute( QStringLiteral( "plussign" ) ).toInt();

  // placement
  QDomElement placementElem = elem.firstChildElement( QStringLiteral( "placement" ) );
  placement = static_cast< Placement >( placementElem.attribute( QStringLiteral( "placement" ) ).toInt() );
  placementFlags = placementElem.attribute( QStringLiteral( "placementFlags" ) ).toUInt();
  centroidWhole = placementElem.attribute( QStringLiteral( "centroidWhole" ), QStringLiteral( "0" ) ).toInt();
  centroidInside = placementElem.attribute( QStringLiteral( "centroidInside" ), QStringLiteral( "0" ) ).toInt();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( placementElem.attribute( QStringLiteral( "predefinedPositionOrder" ) ) );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = DEFAULT_PLACEMENT_ORDER;
  fitInPolygonOnly = placementElem.attribute( QStringLiteral( "fitInPolygonOnly" ), QStringLiteral( "0" ) ).toInt();
  dist = placementElem.attribute( QStringLiteral( "dist" ) ).toDouble();
  if ( !placementElem.hasAttribute( QStringLiteral( "distUnits" ) ) )
  {
    if ( placementElem.attribute( QStringLiteral( "distInMapUnits" ) ).toInt() )
      distUnits = QgsUnitTypes::RenderMapUnits;
    else
      distUnits = QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    distUnits = QgsUnitTypes::decodeRenderUnit( placementElem.attribute( QStringLiteral( "distUnits" ) ) );
  }
  if ( !placementElem.hasAttribute( QStringLiteral( "distMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = placementElem.attribute( QStringLiteral( "distMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    distMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "distMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    distMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    distMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "distMapUnitScale" ) ) );
  }
  offsetType = static_cast< OffsetType >( placementElem.attribute( QStringLiteral( "offsetType" ), QString::number( FromPoint ) ).toUInt() );
  quadOffset = static_cast< QuadrantPosition >( placementElem.attribute( QStringLiteral( "quadOffset" ), QString::number( QuadrantOver ) ).toUInt() );
  xOffset = placementElem.attribute( QStringLiteral( "xOffset" ), QStringLiteral( "0" ) ).toDouble();
  yOffset = placementElem.attribute( QStringLiteral( "yOffset" ), QStringLiteral( "0" ) ).toDouble();
  if ( !placementElem.hasAttribute( QStringLiteral( "offsetUnits" ) ) )
  {
    offsetUnits = placementElem.attribute( QStringLiteral( "labelOffsetInMapUnits" ), QStringLiteral( "1" ) ).toInt() ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    offsetUnits = QgsUnitTypes::decodeRenderUnit( placementElem.attribute( QStringLiteral( "offsetUnits" ) ) );
  }
  if ( !placementElem.hasAttribute( QStringLiteral( "labelOffsetMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = placementElem.attribute( QStringLiteral( "labelOffsetMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    labelOffsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "labelOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    labelOffsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    labelOffsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "labelOffsetMapUnitScale" ) ) );
  }

  if ( placementElem.hasAttribute( QStringLiteral( "angleOffset" ) ) )
  {
    double oldAngle = placementElem.attribute( QStringLiteral( "angleOffset" ), QStringLiteral( "0" ) ).toDouble();
    angleOffset = std::fmod( 360 - oldAngle, 360.0 );
  }
  else
  {
    angleOffset = placementElem.attribute( QStringLiteral( "rotationAngle" ), QStringLiteral( "0" ) ).toDouble();
  }

  preserveRotation = placementElem.attribute( QStringLiteral( "preserveRotation" ), QStringLiteral( "1" ) ).toInt();
  maxCurvedCharAngleIn = placementElem.attribute( QStringLiteral( "maxCurvedCharAngleIn" ), QStringLiteral( "25" ) ).toDouble();
  maxCurvedCharAngleOut = placementElem.attribute( QStringLiteral( "maxCurvedCharAngleOut" ), QStringLiteral( "-25" ) ).toDouble();
  priority = placementElem.attribute( QStringLiteral( "priority" ) ).toInt();
  repeatDistance = placementElem.attribute( QStringLiteral( "repeatDistance" ), QStringLiteral( "0" ) ).toDouble();
  if ( !placementElem.hasAttribute( QStringLiteral( "repeatDistanceUnits" ) ) )
  {
    // upgrade old setting
    switch ( placementElem.attribute( QStringLiteral( "repeatDistanceUnit" ), QString::number( 1 ) ).toUInt() )
    {
      case 0:
        repeatDistanceUnit = QgsUnitTypes::RenderPoints;
        break;
      case 1:
        repeatDistanceUnit = QgsUnitTypes::RenderMillimeters;
        break;
      case 2:
        repeatDistanceUnit = QgsUnitTypes::RenderMapUnits;
        break;
      case 3:
        repeatDistanceUnit = QgsUnitTypes::RenderPercentage;
        break;
    }
  }
  else
  {
    repeatDistanceUnit = QgsUnitTypes::decodeRenderUnit( placementElem.attribute( QStringLiteral( "repeatDistanceUnits" ) ) );
  }
  if ( !placementElem.hasAttribute( QStringLiteral( "repeatDistanceMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = placementElem.attribute( QStringLiteral( "repeatDistanceMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    repeatDistanceMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "repeatDistanceMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    repeatDistanceMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    repeatDistanceMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "repeatDistanceMapUnitScale" ) ) );
  }

  // rendering
  QDomElement renderingElem = elem.firstChildElement( QStringLiteral( "rendering" ) );

  drawLabels = renderingElem.attribute( QStringLiteral( "drawLabels" ), QStringLiteral( "1" ) ).toInt();

  maximumScale = renderingElem.attribute( QStringLiteral( "scaleMin" ), QStringLiteral( "0" ) ).toDouble();
  minimumScale = renderingElem.attribute( QStringLiteral( "scaleMax" ), QStringLiteral( "0" ) ).toDouble();
  scaleVisibility = renderingElem.attribute( QStringLiteral( "scaleVisibility" ) ).toInt();

  fontLimitPixelSize = renderingElem.attribute( QStringLiteral( "fontLimitPixelSize" ), QStringLiteral( "0" ) ).toInt();
  fontMinPixelSize = renderingElem.attribute( QStringLiteral( "fontMinPixelSize" ), QStringLiteral( "0" ) ).toInt();
  fontMaxPixelSize = renderingElem.attribute( QStringLiteral( "fontMaxPixelSize" ), QStringLiteral( "10000" ) ).toInt();
  displayAll = renderingElem.attribute( QStringLiteral( "displayAll" ), QStringLiteral( "0" ) ).toInt();
  upsidedownLabels = static_cast< UpsideDownLabels >( renderingElem.attribute( QStringLiteral( "upsidedownLabels" ), QString::number( Upright ) ).toUInt() );

  labelPerPart = renderingElem.attribute( QStringLiteral( "labelPerPart" ) ).toInt();
  mergeLines = renderingElem.attribute( QStringLiteral( "mergeLines" ) ).toInt();
  minFeatureSize = renderingElem.attribute( QStringLiteral( "minFeatureSize" ) ).toDouble();
  limitNumLabels = renderingElem.attribute( QStringLiteral( "limitNumLabels" ), QStringLiteral( "0" ) ).toInt();
  maxNumLabels = renderingElem.attribute( QStringLiteral( "maxNumLabels" ), QStringLiteral( "2000" ) ).toInt();
  obstacle = renderingElem.attribute( QStringLiteral( "obstacle" ), QStringLiteral( "1" ) ).toInt();
  obstacleFactor = renderingElem.attribute( QStringLiteral( "obstacleFactor" ), QStringLiteral( "1" ) ).toDouble();
  obstacleType = static_cast< ObstacleType >( renderingElem.attribute( QStringLiteral( "obstacleType" ), QString::number( PolygonInterior ) ).toUInt() );
  zIndex = renderingElem.attribute( QStringLiteral( "zIndex" ), QStringLiteral( "0.0" ) ).toDouble();

  QDomElement ddElem = elem.firstChildElement( QStringLiteral( "dd_properties" ) );
  if ( !ddElem.isNull() )
  {
    mDataDefinedProperties.readXml( ddElem, sPropertyDefinitions );
  }
  else
  {
    // upgrade 2.x style dd project
    mDataDefinedProperties.clear();
    QDomElement ddElem = elem.firstChildElement( QStringLiteral( "data-defined" ) );
    readOldDataDefinedPropertyMap( nullptr, &ddElem );
  }
  // upgrade older data defined settings
  if ( mDataDefinedProperties.isActive( FontTransp ) )
  {
    mDataDefinedProperties.setProperty( FontOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( FontTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( FontTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( BufferTransp ) )
  {
    mDataDefinedProperties.setProperty( BufferOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( BufferTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( BufferTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( ShapeTransparency ) )
  {
    mDataDefinedProperties.setProperty( ShapeOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( ShapeTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( ShapeTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( ShadowTransparency ) )
  {
    mDataDefinedProperties.setProperty( ShadowOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( ShadowTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( ShadowTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( Rotation ) )
  {
    mDataDefinedProperties.setProperty( LabelRotation, QgsProperty::fromExpression( QStringLiteral( "360 - (%1)" ).arg( mDataDefinedProperties.property( Rotation ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( Rotation, QgsProperty() );
  }
  // older 2.x projects had min/max scale flipped - so change them here.
  if ( mDataDefinedProperties.isActive( MinScale ) )
  {
    mDataDefinedProperties.setProperty( MaximumScale, mDataDefinedProperties.property( MinScale ) );
    mDataDefinedProperties.setProperty( MinScale, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( MaxScale ) )
  {
    mDataDefinedProperties.setProperty( MinimumScale, mDataDefinedProperties.property( MaxScale ) );
    mDataDefinedProperties.setProperty( MaxScale, QgsProperty() );
  }
}



QDomElement QgsPalLayerSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement textStyleElem = mFormat.writeXml( doc, context );

  // text style
  textStyleElem.setAttribute( QStringLiteral( "fieldName" ), fieldName );
  textStyleElem.setAttribute( QStringLiteral( "isExpression" ), isExpression );
  textStyleElem.setAttribute( QStringLiteral( "previewBkgrdColor" ), previewBkgrdColor.name() );
  QDomElement replacementElem = doc.createElement( QStringLiteral( "substitutions" ) );
  substitutions.writeXml( replacementElem, doc );
  textStyleElem.appendChild( replacementElem );
  textStyleElem.setAttribute( QStringLiteral( "useSubstitutions" ), useSubstitutions );

  // text formatting
  QDomElement textFormatElem = doc.createElement( QStringLiteral( "text-format" ) );
  textFormatElem.setAttribute( QStringLiteral( "wrapChar" ), wrapChar );
  textFormatElem.setAttribute( QStringLiteral( "autoWrapLength" ), autoWrapLength );
  textFormatElem.setAttribute( QStringLiteral( "useMaxLineLengthForAutoWrap" ), useMaxLineLengthForAutoWrap );
  textFormatElem.setAttribute( QStringLiteral( "multilineAlign" ), static_cast< unsigned int >( multilineAlign ) );
  textFormatElem.setAttribute( QStringLiteral( "addDirectionSymbol" ), addDirectionSymbol );
  textFormatElem.setAttribute( QStringLiteral( "leftDirectionSymbol" ), leftDirectionSymbol );
  textFormatElem.setAttribute( QStringLiteral( "rightDirectionSymbol" ), rightDirectionSymbol );
  textFormatElem.setAttribute( QStringLiteral( "reverseDirectionSymbol" ), reverseDirectionSymbol );
  textFormatElem.setAttribute( QStringLiteral( "placeDirectionSymbol" ), static_cast< unsigned int >( placeDirectionSymbol ) );
  textFormatElem.setAttribute( QStringLiteral( "formatNumbers" ), formatNumbers );
  textFormatElem.setAttribute( QStringLiteral( "decimals" ), decimals );
  textFormatElem.setAttribute( QStringLiteral( "plussign" ), plusSign );

  // placement
  QDomElement placementElem = doc.createElement( QStringLiteral( "placement" ) );
  placementElem.setAttribute( QStringLiteral( "placement" ), placement );
  placementElem.setAttribute( QStringLiteral( "placementFlags" ), static_cast< unsigned int >( placementFlags ) );
  placementElem.setAttribute( QStringLiteral( "centroidWhole" ), centroidWhole );
  placementElem.setAttribute( QStringLiteral( "centroidInside" ), centroidInside );
  placementElem.setAttribute( QStringLiteral( "predefinedPositionOrder" ), QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) );
  placementElem.setAttribute( QStringLiteral( "fitInPolygonOnly" ), fitInPolygonOnly );
  placementElem.setAttribute( QStringLiteral( "dist" ), dist );
  placementElem.setAttribute( QStringLiteral( "distUnits" ), QgsUnitTypes::encodeUnit( distUnits ) );
  placementElem.setAttribute( QStringLiteral( "distMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( distMapUnitScale ) );
  placementElem.setAttribute( QStringLiteral( "offsetType" ), static_cast< unsigned int >( offsetType ) );
  placementElem.setAttribute( QStringLiteral( "quadOffset" ), static_cast< unsigned int >( quadOffset ) );
  placementElem.setAttribute( QStringLiteral( "xOffset" ), xOffset );
  placementElem.setAttribute( QStringLiteral( "yOffset" ), yOffset );
  placementElem.setAttribute( QStringLiteral( "offsetUnits" ), QgsUnitTypes::encodeUnit( offsetUnits ) );
  placementElem.setAttribute( QStringLiteral( "labelOffsetMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( labelOffsetMapUnitScale ) );
  placementElem.setAttribute( QStringLiteral( "rotationAngle" ), angleOffset );
  placementElem.setAttribute( QStringLiteral( "preserveRotation" ), preserveRotation );
  placementElem.setAttribute( QStringLiteral( "maxCurvedCharAngleIn" ), maxCurvedCharAngleIn );
  placementElem.setAttribute( QStringLiteral( "maxCurvedCharAngleOut" ), maxCurvedCharAngleOut );
  placementElem.setAttribute( QStringLiteral( "priority" ), priority );
  placementElem.setAttribute( QStringLiteral( "repeatDistance" ), repeatDistance );
  placementElem.setAttribute( QStringLiteral( "repeatDistanceUnits" ), QgsUnitTypes::encodeUnit( repeatDistanceUnit ) );
  placementElem.setAttribute( QStringLiteral( "repeatDistanceMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( repeatDistanceMapUnitScale ) );

  // rendering
  QDomElement renderingElem = doc.createElement( QStringLiteral( "rendering" ) );
  renderingElem.setAttribute( QStringLiteral( "drawLabels" ), drawLabels );
  renderingElem.setAttribute( QStringLiteral( "scaleVisibility" ), scaleVisibility );
  renderingElem.setAttribute( QStringLiteral( "scaleMin" ), maximumScale );
  renderingElem.setAttribute( QStringLiteral( "scaleMax" ), minimumScale );
  renderingElem.setAttribute( QStringLiteral( "fontLimitPixelSize" ), fontLimitPixelSize );
  renderingElem.setAttribute( QStringLiteral( "fontMinPixelSize" ), fontMinPixelSize );
  renderingElem.setAttribute( QStringLiteral( "fontMaxPixelSize" ), fontMaxPixelSize );
  renderingElem.setAttribute( QStringLiteral( "displayAll" ), displayAll );
  renderingElem.setAttribute( QStringLiteral( "upsidedownLabels" ), static_cast< unsigned int >( upsidedownLabels ) );

  renderingElem.setAttribute( QStringLiteral( "labelPerPart" ), labelPerPart );
  renderingElem.setAttribute( QStringLiteral( "mergeLines" ), mergeLines );
  renderingElem.setAttribute( QStringLiteral( "minFeatureSize" ), minFeatureSize );
  renderingElem.setAttribute( QStringLiteral( "limitNumLabels" ), limitNumLabels );
  renderingElem.setAttribute( QStringLiteral( "maxNumLabels" ), maxNumLabels );
  renderingElem.setAttribute( QStringLiteral( "obstacle" ), obstacle );
  renderingElem.setAttribute( QStringLiteral( "obstacleFactor" ), obstacleFactor );
  renderingElem.setAttribute( QStringLiteral( "obstacleType" ), static_cast< unsigned int >( obstacleType ) );
  renderingElem.setAttribute( QStringLiteral( "zIndex" ), zIndex );

  QDomElement ddElem = doc.createElement( QStringLiteral( "dd_properties" ) );
  mDataDefinedProperties.writeXml( ddElem, sPropertyDefinitions );

  QDomElement elem = doc.createElement( QStringLiteral( "settings" ) );
  elem.appendChild( textStyleElem );
  elem.appendChild( textFormatElem );
  elem.appendChild( placementElem );
  elem.appendChild( renderingElem );
  elem.appendChild( ddElem );
  return elem;
}

bool QgsPalLayerSettings::checkMinimumSizeMM( const QgsRenderContext &ct, const QgsGeometry &geom, double minSize ) const
{
  return QgsPalLabeling::checkMinimumSizeMM( ct, geom, minSize );
}

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF *fm, const QString &text, double &labelX, double &labelY, const QgsFeature *f, QgsRenderContext *context )
{
  if ( !fm || !f )
  {
    return;
  }

  QString textCopy( text );

  //try to keep < 2.12 API - handle no passed render context
  std::unique_ptr< QgsRenderContext > scopedRc;
  if ( !context )
  {
    scopedRc.reset( new QgsRenderContext() );
    if ( f )
      scopedRc->expressionContext().setFeature( *f );
  }
  QgsRenderContext *rc = context ? context : scopedRc.get();

  QString wrapchr = wrapChar;
  int evalAutoWrapLength = autoWrapLength;
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

    if ( dataDefinedValues.contains( QgsPalLayerSettings::AutoWrapLength ) )
    {
      evalAutoWrapLength = dataDefinedValues.value( QgsPalLayerSettings::AutoWrapLength, evalAutoWrapLength ).toInt();
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
    wrapchr = mDataDefinedProperties.value( QgsPalLayerSettings::MultiLineWrapChar, rc->expressionContext(), wrapchr ).toString();

    rc->expressionContext().setOriginalValueVariable( evalAutoWrapLength );
    evalAutoWrapLength = mDataDefinedProperties.value( QgsPalLayerSettings::AutoWrapLength, rc->expressionContext(), evalAutoWrapLength ).toInt();

    rc->expressionContext().setOriginalValueVariable( multilineH );
    multilineH = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MultiLineHeight, rc->expressionContext(), multilineH );

    rc->expressionContext().setOriginalValueVariable( addDirSymb );
    addDirSymb = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::DirSymbDraw, rc->expressionContext(), addDirSymb );

    if ( addDirSymb ) // don't do extra evaluations if not adding a direction symbol
    {
      rc->expressionContext().setOriginalValueVariable( leftDirSymb );
      leftDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbLeft, rc->expressionContext(), leftDirSymb ).toString();

      rc->expressionContext().setOriginalValueVariable( rightDirSymb );
      rightDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbLeft, rc->expressionContext(), rightDirSymb ).toString();

      rc->expressionContext().setOriginalValueVariable( static_cast< int >( placeDirSymb ) );
      placeDirSymb = static_cast< QgsPalLayerSettings::DirectionSymbols >( mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::DirSymbPlacement, rc->expressionContext(), placeDirSymb ) );
    }

  }

  if ( wrapchr.isEmpty() )
  {
    wrapchr = QStringLiteral( "\n" ); // default to new line delimiter
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
      textCopy.append( dirSym );
    }
    else
    {
      textCopy.prepend( dirSym + QStringLiteral( "\n" ) ); // SymbolAbove or SymbolBelow
    }
  }

  double w = 0.0, h = 0.0;
  const QStringList multiLineSplit = QgsPalLabeling::splitToLines( textCopy, wrapchr, evalAutoWrapLength, useMaxLineLengthForAutoWrap );
  int lines = multiLineSplit.size();

  double labelHeight = fm->ascent() + fm->descent(); // ignore +1 for baseline

  h += fm->height() + static_cast< double >( ( lines - 1 ) * labelHeight * multilineH );

  for ( const QString &line : multiLineSplit )
  {
    w = std::max( w, fm->width( line ) );
  }

#if 0 // XXX strk
  QgsPointXY ptSize = xform->toMapCoordinatesF( w, h );
  labelX = std::fabs( ptSize.x() - ptZero.x() );
  labelY = std::fabs( ptSize.y() - ptZero.y() );
#else
  double uPP = xform->mapUnitsPerPixel();
  labelX = w * uPP;
  labelY = h * uPP;
#endif
}

void QgsPalLayerSettings::registerFeature( const QgsFeature &f, QgsRenderContext &context, QgsLabelFeature **labelFeature, QgsGeometry obstacleGeometry )
{
  // either used in QgsPalLabeling (palLayer is set) or in QgsLabelingEngine (labelFeature is set)
  Q_ASSERT( labelFeature );

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful
  mCurFeat = &f;

  // data defined is obstacle? calculate this first, to avoid wasting time working with obstacles we don't require
  bool isObstacle = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::IsObstacle, context.expressionContext(), obstacle ); // default to layer default

  if ( !drawLabels )
  {
    if ( isObstacle )
    {
      registerObstacleFeature( f, context, labelFeature, obstacleGeometry );
    }
    return;
  }

//  mCurFields = &layer->fields();

  // store data defined-derived values for later adding to label feature for use during rendering
  dataDefinedValues.clear();

  // data defined show label? defaults to show label if not set
  context.expressionContext().setOriginalValueVariable( true );
  if ( !mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Show, context.expressionContext(), true ) )
  {
    return;
  }

  // data defined scale visibility?
  bool useScaleVisibility = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::ScaleVisibility, context.expressionContext(), scaleVisibility );

  if ( useScaleVisibility )
  {
    // data defined min scale?
    context.expressionContext().setOriginalValueVariable( maximumScale );
    double maxScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MaximumScale, context.expressionContext(), maximumScale );

    // scales closer than 1:1
    if ( maxScale < 0 )
    {
      maxScale = 1 / std::fabs( maxScale );
    }

    if ( !qgsDoubleNear( maxScale, 0.0 ) && context.rendererScale() < maxScale )
    {
      return;
    }

    // data defined max scale?
    context.expressionContext().setOriginalValueVariable( minimumScale );
    double minScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MinimumScale, context.expressionContext(), minimumScale );

    // scales closer than 1:1
    if ( minScale < 0 )
    {
      minScale = 1 / std::fabs( minScale );
    }

    if ( !qgsDoubleNear( minScale, 0.0 ) && context.rendererScale() > minScale )
    {
      return;
    }
  }

  QFont labelFont = mFormat.font();
  // labelFont will be added to label feature for use during label painting

  // data defined font units?
  QgsUnitTypes::RenderUnit fontunits = mFormat.sizeUnit();
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontSizeUnit, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        fontunits = res;
    }
  }

  //data defined label size?
  context.expressionContext().setOriginalValueVariable( mFormat.size() );
  double fontSize = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Size, context.expressionContext(), mFormat.size() );
  if ( fontSize <= 0.0 )
  {
    return;
  }

  int fontPixelSize = QgsTextRenderer::sizeToPixel( fontSize, context, fontunits, mFormat.sizeMapUnitScale() );
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
    if ( mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::FontLimitPixel, context.expressionContext(), fontLimitPixelSize ) )
    {
      int fontMinPixel = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::FontMinPixel, context.expressionContext(), fontMinPixelSize );
      int fontMaxPixel = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::FontMaxPixel, context.expressionContext(), fontMaxPixelSize );

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
    QgsExpression *exp = getLabelExpression();
    if ( exp->hasParserError() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Expression parser error:%1" ).arg( exp->parserErrorString() ), 4 );
      return;
    }

    QVariant result = exp->evaluate( &context.expressionContext() ); // expression prepared in QgsPalLabeling::prepareLayer()
    if ( exp->hasEvalError() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Expression parser eval error:%1" ).arg( exp->evalErrorString() ), 4 );
      return;
    }
    labelText = result.isNull() ? QString() : result.toString();
  }
  else
  {
    const QVariant &v = f.attribute( fieldIndex );
    labelText = v.isNull() ? QString() : v.toString();
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
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontCase, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString fcase = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal FontCase:%1" ).arg( fcase ), 4 );

    if ( !fcase.isEmpty() )
    {
      if ( fcase.compare( QLatin1String( "NoChange" ), Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::MixedCase;
      }
      else if ( fcase.compare( QLatin1String( "Upper" ), Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::AllUppercase;
      }
      else if ( fcase.compare( QLatin1String( "Lower" ), Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::AllLowercase;
      }
      else if ( fcase.compare( QLatin1String( "Capitalize" ), Qt::CaseInsensitive ) == 0 )
      {
        capitalization = QgsStringUtils::ForceFirstLetterToCapital;
      }
    }
  }
  labelText = QgsStringUtils::capitalize( labelText, capitalization );

  // format number if label text is coercible to a number
  if ( mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::NumFormat, context.expressionContext(), formatNumbers ) )
  {
    // data defined decimal places?
    int decimalPlaces = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::NumDecimals, context.expressionContext(), decimals );
    if ( decimalPlaces <= 0 ) // needs to be positive
      decimalPlaces = decimals;

    // data defined plus sign?
    bool signPlus = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::NumPlusSign, context.expressionContext(), plusSign );

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
  std::unique_ptr<QFontMetricsF> labelFontMetrics( new QFontMetricsF( labelFont ) );
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelFontMetrics.get(), labelText, labelX, labelY, mCurFeat, &context );


  // maximum angle between curved label characters (hardcoded defaults used in QGIS <2.0)
  //
  double maxcharanglein = 20.0; // range 20.0-60.0
  double maxcharangleout = -20.0; // range 20.0-95.0

  if ( placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    maxcharanglein = maxCurvedCharAngleIn;
    maxcharangleout = maxCurvedCharAngleOut;

    //data defined maximum angle between curved label characters?
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::CurvedCharAngleInOut, context.expressionContext() );
    if ( exprVal.isValid() )
    {
      QString ptstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal CurvedCharAngleInOut:%1" ).arg( ptstr ), 4 );

      if ( !ptstr.isEmpty() )
      {
        QPointF maxcharanglePt = QgsSymbolLayerUtils::decodePoint( ptstr );
        maxcharanglein = qBound( 20.0, static_cast< double >( maxcharanglePt.x() ), 60.0 );
        maxcharangleout = qBound( 20.0, static_cast< double >( maxcharanglePt.y() ), 95.0 );
      }
    }
    // make sure maxcharangleout is always negative
    maxcharangleout = -( std::fabs( maxcharangleout ) );
  }

  // data defined centroid whole or clipped?
  bool wholeCentroid = centroidWhole;
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::CentroidWhole, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal CentroidWhole:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      if ( str.compare( QLatin1String( "Visible" ), Qt::CaseInsensitive ) == 0 )
      {
        wholeCentroid = false;
      }
      else if ( str.compare( QLatin1String( "Whole" ), Qt::CaseInsensitive ) == 0 )
      {
        wholeCentroid = true;
      }
    }
  }

  QgsGeometry geom = f.geometry();
  if ( geom.isNull() )
  {
    return;
  }

  // simplify?
  const QgsVectorSimplifyMethod &simplifyMethod = context.vectorSimplifyMethod();
  std::unique_ptr<QgsGeometry> scopedClonedGeom;
  if ( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( simplifyMethod.simplifyAlgorithm() );
    QgsGeometry g = geom;
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  // whether we're going to create a centroid for polygon
  bool centroidPoly = ( ( placement == QgsPalLayerSettings::AroundPoint
                          || placement == QgsPalLayerSettings::OverPoint )
                        && geom.type() == QgsWkbTypes::PolygonGeometry );

  // CLIP the geometry if it is bigger than the extent
  // don't clip if centroid is requested for whole feature
  bool doClip = false;
  if ( !centroidPoly || !wholeCentroid )
  {
    doClip = true;
  }

  // if using fitInPolygonOnly option, generate the permissible zone (must happen before geometry is modified - e.g.,
  // as a result of using perimeter based labeling and the geometry is converted to a boundary)
  QgsGeometry permissibleZone;
  if ( geom.type() == QgsWkbTypes::PolygonGeometry && fitInPolygonOnly )
  {
    permissibleZone = geom;
    if ( QgsPalLabeling::geometryRequiresPreparation( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry() ) )
    {
      permissibleZone = QgsPalLabeling::prepareGeometry( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry() );
    }
  }

  // if using perimeter based labeling for polygons, get the polygon's
  // linear boundary and use that for the label geometry
  if ( ( geom.type() == QgsWkbTypes::PolygonGeometry )
       && ( placement == Line || placement == PerimeterCurved ) )
  {
    geom = QgsGeometry( geom.constGet()->boundary() );
  }

  geos::unique_ptr geos_geom_clone;
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, doClip ? extentGeom : QgsGeometry() ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, doClip ? extentGeom : QgsGeometry() );

    if ( geom.isNull() )
      return;
  }
  geos_geom_clone = QgsGeos::asGeos( geom );

  if ( isObstacle )
  {
    if ( !obstacleGeometry.isNull() && QgsPalLabeling::geometryRequiresPreparation( obstacleGeometry, context, ct, doClip ? extentGeom : QgsGeometry() ) )
    {
      obstacleGeometry = QgsGeometry( QgsPalLabeling::prepareGeometry( obstacleGeometry, context, ct, doClip ? extentGeom : QgsGeometry() ) );
    }
  }

  if ( minFeatureSize > 0 && !checkMinimumSizeMM( context, geom, minFeatureSize ) )
    return;

  if ( !geos_geom_clone )
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

    int divNum = static_cast< int >( ( static_cast< double >( mFeaturesToLabel ) / maxNumLabels ) + 0.5 ); // NOLINT
    if ( divNum && ( mFeatsRegPal == static_cast< int >( mFeatsSendingToPal / divNum ) ) )
    {
      mFeatsSendingToPal += 1;
      if ( divNum &&  mFeatsSendingToPal % divNum )
      {
        return;
      }
    }
  }

  geos::unique_ptr geosObstacleGeomClone;
  if ( !obstacleGeometry.isNull() )
  {
    geosObstacleGeomClone = QgsGeos::asGeos( obstacleGeometry );
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
  context.expressionContext().setOriginalValueVariable( static_cast< int >( quadOff ) );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetQuad, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    bool ok;
    int quadInt = exprVal.toInt( &ok );
    QgsDebugMsgLevel( QStringLiteral( "exprVal OffsetQuad:%1" ).arg( quadInt ), 4 );
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
  context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( QPointF( xOffset, yOffset ) ) );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetXY, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString ptstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal OffsetXY:%1" ).arg( ptstr ), 4 );

    if ( !ptstr.isEmpty() )
    {
      QPointF ddOffPt = QgsSymbolLayerUtils::decodePoint( ptstr );
      xOff = ddOffPt.x();
      yOff = ddOffPt.y();
    }
  }

  // data defined label offset units?
  QgsUnitTypes::RenderUnit offUnit = offsetUnits;
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetUnits, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal OffsetUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      bool ok = false;
      QgsUnitTypes::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
      {
        offUnit = decodedUnits;
      }
    }
  }

  // adjust offset of labels to match chosen unit and map scale
  // offsets match those of symbology: -x = left, -y = up
  offsetX = context.convertToMapUnits( xOff, offUnit, labelOffsetMapUnitScale );
  // must be negative to match symbology offset direction
  offsetY = context.convertToMapUnits( -yOff, offUnit, labelOffsetMapUnitScale );

  // layer defined rotation?
  // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
  if ( placement == QgsPalLayerSettings::OverPoint && !qgsDoubleNear( angleOffset, 0.0 ) )
  {
    layerDefinedRotation = true;
    angle = ( 360 - angleOffset ) * M_PI / 180; // convert to radians counterclockwise
  }

  const QgsMapToPixel &m2p = context.mapToPixel();
  //data defined rotation?
  context.expressionContext().setOriginalValueVariable( angleOffset );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::LabelRotation, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    bool ok;
    double rotD = exprVal.toDouble( &ok );
    QgsDebugMsgLevel( QStringLiteral( "exprVal Rotation:%1" ).arg( rotD ), 4 );
    if ( ok )
    {
      dataDefinedRotation = true;
      // TODO: add setting to disable having data defined rotation follow
      //       map rotation ?
      rotD += m2p.mapRotation();
      angle = ( 360 - rotD ) * M_PI / 180.0;
    }
  }

  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::PositionX, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    if ( !exprVal.isNull() )
      xPos = exprVal.toDouble( &ddXPos );
    QgsDebugMsgLevel( QStringLiteral( "exprVal PositionX:%1" ).arg( xPos ), 4 );

    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::PositionY, context.expressionContext() );
    if ( exprVal.isValid() )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      if ( !exprVal.isNull() )
        yPos = exprVal.toDouble( &ddYPos );
      QgsDebugMsgLevel( QStringLiteral( "exprVal PositionY:%1" ).arg( yPos ), 4 );

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
        exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Hali, context.expressionContext() );
        if ( exprVal.isValid() )
        {
          QString haliString = exprVal.toString();
          QgsDebugMsgLevel( QStringLiteral( "exprVal Hali:%1" ).arg( haliString ), 4 );
          if ( haliString.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX / 2.0;
          }
          else if ( haliString.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX;
          }
        }

        //vertical alignment
        exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Vali, context.expressionContext() );
        if ( exprVal.isValid() )
        {
          QString valiString = exprVal.toString();
          QgsDebugMsgLevel( QStringLiteral( "exprVal Vali:%1" ).arg( valiString ), 4 );

          if ( valiString.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) != 0 )
          {
            if ( valiString.compare( QLatin1String( "Top" ), Qt::CaseInsensitive ) == 0 )
            {
              ydiff -= labelY;
            }
            else
            {
              double descentRatio = labelFontMetrics->descent() / labelFontMetrics->height();
              if ( valiString.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
              {
                ydiff -= labelY * descentRatio;
              }
              else //'Cap' or 'Half'
              {
                double capHeightRatio = ( labelFontMetrics->boundingRect( 'H' ).height() + 1 + labelFontMetrics->descent() ) / labelFontMetrics->height();
                ydiff -= labelY * capHeightRatio;
                if ( valiString.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
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
          double xd = xdiff * std::cos( angle ) - ydiff * std::sin( angle );
          double yd = xdiff * std::sin( angle ) + ydiff * std::cos( angle );
          xdiff = xd;
          ydiff = yd;
        }

        //project xPos and yPos from layer to map CRS, handle rotation
        QgsGeometry ddPoint( new QgsPoint( xPos, yPos ) );
        if ( QgsPalLabeling::geometryRequiresPreparation( ddPoint, context, ct ) )
        {
          ddPoint = QgsPalLabeling::prepareGeometry( ddPoint, context, ct );
          xPos = static_cast< const QgsPoint * >( ddPoint.constGet() )->x();
          yPos = static_cast< const QgsPoint * >( ddPoint.constGet() )->y();
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
  bool alwaysShow = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::AlwaysShow, context.expressionContext(), false );

  // set repeat distance
  // data defined repeat distance?
  context.expressionContext().setOriginalValueVariable( repeatDistance );
  double repeatDist = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::RepeatDistance, context.expressionContext(), repeatDistance );

  // data defined label-repeat distance units?
  QgsUnitTypes::RenderUnit repeatUnits = repeatDistanceUnit;
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::RepeatDistanceUnit, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal RepeatDistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      bool ok = false;
      QgsUnitTypes::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
      {
        repeatUnits = decodedUnits;
      }
    }
  }

  if ( !qgsDoubleNear( repeatDist, 0.0 ) )
  {
    if ( repeatUnits != QgsUnitTypes::RenderMapUnits )
    {
      repeatDist = context.convertToMapUnits( repeatDist, repeatUnits, repeatDistanceMapUnitScale );
    }
  }

  //  feature to the layer
  QgsTextLabelFeature *lf = new QgsTextLabelFeature( f.id(), std::move( geos_geom_clone ), QSizeF( labelX, labelY ) );
  mFeatsRegPal++;

  *labelFeature = lf;
  ( *labelFeature )->setHasFixedPosition( dataDefinedPosition );
  ( *labelFeature )->setFixedPosition( QgsPointXY( xPos, yPos ) );
  // use layer-level defined rotation, but not if position fixed
  ( *labelFeature )->setHasFixedAngle( dataDefinedRotation || ( !dataDefinedPosition && !qgsDoubleNear( angle, 0.0 ) ) );
  ( *labelFeature )->setFixedAngle( angle );
  ( *labelFeature )->setQuadOffset( QPointF( quadOffsetX, quadOffsetY ) );
  ( *labelFeature )->setPositionOffset( QgsPointXY( offsetX, offsetY ) );
  ( *labelFeature )->setOffsetType( offsetType );
  ( *labelFeature )->setAlwaysShow( alwaysShow );
  ( *labelFeature )->setRepeatDistance( repeatDist );
  ( *labelFeature )->setLabelText( labelText );
  ( *labelFeature )->setPermissibleZone( permissibleZone );
  if ( geosObstacleGeomClone )
  {
    ( *labelFeature )->setObstacleGeometry( std::move( geosObstacleGeomClone ) );

    if ( geom.type() == QgsWkbTypes::PointGeometry )
    {
      //register symbol size
      ( *labelFeature )->setSymbolSize( QSizeF( obstacleGeometry.boundingBox().width(),
                                        obstacleGeometry.boundingBox().height() ) );
    }
  }

  //set label's visual margin so that top visual margin is the leading, and bottom margin is the font's descent
  //this makes labels align to the font's baseline or highest character
  double topMargin = std::max( 0.25 * labelFontMetrics->ascent(), 0.0 );
  double bottomMargin = 1.0 + labelFontMetrics->descent();
  QgsMargins vm( 0.0, topMargin, 0.0, bottomMargin );
  vm *= xform->mapUnitsPerPixel();
  ( *labelFeature )->setVisualMargin( vm );

  // store the label's calculated font for later use during painting
  QgsDebugMsgLevel( QStringLiteral( "PAL font stored definedFont: %1, Style: %2" ).arg( labelFont.toString(), labelFont.styleName() ), 4 );
  lf->setDefinedFont( labelFont );

  // TODO: only for placement which needs character info
  // account for any data defined font metrics adjustments
  lf->calculateInfo( placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved,
                     labelFontMetrics.get(), xform, maxcharanglein, maxcharangleout );
  // for labelFeature the LabelInfo is passed to feat when it is registered

  // TODO: allow layer-wide feature dist in PAL...?

  // data defined label-feature distance?
  context.expressionContext().setOriginalValueVariable( dist );
  double distance = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::LabelDistance, context.expressionContext(), dist );

  // data defined label-feature distance units?
  QgsUnitTypes::RenderUnit distUnit = distUnits;
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::DistanceUnits, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal DistanceUnits:%1" ).arg( units ), 4 );
    if ( !units.isEmpty() )
    {
      bool ok = false;
      QgsUnitTypes::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
      {
        distUnit = decodedUnits;
      }
    }
  }
  distance = context.convertToPainterUnits( distance, distUnit, distMapUnitScale );

  // when using certain placement modes, we force a tiny minimum distance. This ensures that
  // candidates are created just offset from a border and avoids candidates being incorrectly flagged as colliding with neighbours
  if ( placement == QgsPalLayerSettings::Line || placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    distance = std::max( distance, 1.0 );
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
  context.expressionContext().setOriginalValueVariable( zIndex );
  double z = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::ZIndex, context.expressionContext(), zIndex );
  ( *labelFeature )->setZIndex( z );

  // data defined priority?
  context.expressionContext().setOriginalValueVariable( priority );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Priority, context.expressionContext() );
  if ( exprVal.isValid() )
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
  context.expressionContext().setOriginalValueVariable( obstacleFactor );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ObstacleFactor, context.expressionContext() );
  if ( exprVal.isValid() )
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

  context.expressionContext().setOriginalValueVariable( QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) );
  QString dataDefinedOrder = mDataDefinedProperties.valueAsString( QgsPalLayerSettings::PredefinedPositionOrder, context.expressionContext() );
  if ( !dataDefinedOrder.isEmpty() )
  {
    positionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( dataDefinedOrder );
  }
  ( *labelFeature )->setPredefinedPositionOrder( positionOrder );

  // add parameters for data defined labeling to label feature
  lf->setDataDefinedValues( dataDefinedValues );
}

void QgsPalLayerSettings::registerObstacleFeature( const QgsFeature &f, QgsRenderContext &context, QgsLabelFeature **obstacleFeature, const QgsGeometry &obstacleGeometry )
{
  mCurFeat = &f;

  QgsGeometry geom;
  if ( !obstacleGeometry.isNull() )
  {
    geom = obstacleGeometry;
  }
  else
  {
    geom = f.geometry();
  }

  if ( geom.isNull() )
  {
    return;
  }

  // simplify?
  const QgsVectorSimplifyMethod &simplifyMethod = context.vectorSimplifyMethod();
  std::unique_ptr<QgsGeometry> scopedClonedGeom;
  if ( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( simplifyMethod.simplifyAlgorithm() );
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  geos::unique_ptr geos_geom_clone;
  std::unique_ptr<QgsGeometry> scopedPreparedGeom;

  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, extentGeom ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, extentGeom );
  }
  geos_geom_clone = QgsGeos::asGeos( geom );

  if ( !geos_geom_clone )
    return; // invalid geometry

  //  feature to the layer
  *obstacleFeature = new QgsLabelFeature( f.id(), std::move( geos_geom_clone ), QSizeF( 0, 0 ) );
  ( *obstacleFeature )->setIsObstacle( true );
  mFeatsRegPal++;
}

bool QgsPalLayerSettings::dataDefinedValEval( DataDefinedValueType valType,
    QgsPalLayerSettings::Property p,
    QVariant &exprVal, QgsExpressionContext &context, const QVariant &originalValue )
{
  if ( !mDataDefinedProperties.isActive( p ) )
    return false;

  context.setOriginalValueVariable( originalValue );
  exprVal = mDataDefinedProperties.value( p, context );
  if ( exprVal.isValid() )
  {
    switch ( valType )
    {
      case DDBool:
      {
        bool bol = exprVal.toBool();
        dataDefinedValues.insert( p, QVariant( bol ) );
        return true;
      }
      case DDInt:
      {
        bool ok;
        int size = exprVal.toInt( &ok );

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
      case DDOpacity:
      {
        bool ok;
        int size = exprVal.toDouble( &ok );
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

        dataDefinedValues.insert( p, QVariant( str ) ); // let it stay empty if it is
        return true;
      }
      case DDUnits:
      {
        QString unitstr = exprVal.toString().trimmed();

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

        if ( !ptstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( QgsSymbolLayerUtils::decodePoint( ptstr ) ) );
          return true;
        }
        return false;
      }
      case DDSizeF:
      {
        QString ptstr = exprVal.toString().trimmed();

        if ( !ptstr.isEmpty() )
        {
          dataDefinedValues.insert( p, QVariant( QgsSymbolLayerUtils::decodeSize( ptstr ) ) );
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

void QgsPalLayerSettings::parseTextStyle( QFont &labelFont,
    QgsUnitTypes::RenderUnit fontunits,
    QgsRenderContext &context )
{
  // NOTE: labelFont already has pixelSize set, so pointSize or pointSizeF might return -1

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // Two ways to generate new data defined font:
  // 1) Family + [bold] + [italic] (named style is ignored and font is built off of base family)
  // 2) Family + named style  (bold or italic is ignored)

  // data defined font family?
  QString ddFontFamily;
  context.expressionContext().setOriginalValueVariable( labelFont.family() );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Family, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString family = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal Font family:%1" ).arg( family ), 4 );

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
  QString ddFontStyle;
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontStyle, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString fontstyle = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal Font style:%1" ).arg( fontstyle ), 4 );
    ddFontStyle = fontstyle;
  }

  // data defined bold font style?
  context.expressionContext().setOriginalValueVariable( labelFont.bold() );
  bool ddBold = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Bold, context.expressionContext(), false );

  // data defined italic font style?
  context.expressionContext().setOriginalValueVariable( labelFont.italic() );
  bool ddItalic = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Italic, context.expressionContext(), false );

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
            && ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
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
    if ( ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
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
  context.expressionContext().setOriginalValueVariable( wordspace );
  wordspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::FontWordSpacing, context.expressionContext(), wordspace );
  labelFont.setWordSpacing( context.convertToPainterUnits( wordspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined letter spacing?
  double letterspace = labelFont.letterSpacing();
  context.expressionContext().setOriginalValueVariable( letterspace );
  letterspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::FontLetterSpacing, context.expressionContext(), letterspace );
  labelFont.setLetterSpacing( QFont::AbsoluteSpacing, context.convertToPainterUnits( letterspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined strikeout font style?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Strikeout ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.strikeOut() );
    bool strikeout = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Strikeout, context.expressionContext(), false );
    labelFont.setStrikeOut( strikeout );
  }

  // data defined underline font style?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Underline ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.underline() );
    bool underline = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Underline, context.expressionContext(), false );
    labelFont.setUnderline( underline );
  }

  // pass the rest on to QgsPalLabeling::drawLabeling

  // data defined font color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Color, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( mFormat.color() ) );

  // data defined font opacity?
  dataDefinedValEval( DDOpacity, QgsPalLayerSettings::FontOpacity, exprVal, context.expressionContext(), mFormat.opacity() * 100 );

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
  double bufferOpacity = buffer.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::BufferOpacity, exprVal, context.expressionContext(), bufferOpacity ) )
  {
    bufferOpacity = exprVal.toDouble();
  }

  drawBuffer = ( drawBuffer && bufrSize > 0.0 && bufferOpacity > 0 );

  if ( !drawBuffer )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::BufferDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::BufferSize );
    dataDefinedValues.remove( QgsPalLayerSettings::BufferOpacity );
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

  int evalAutoWrapLength = autoWrapLength;
  if ( dataDefinedValEval( DDInt, QgsPalLayerSettings::AutoWrapLength, exprVal, context.expressionContext(), evalAutoWrapLength ) )
  {
    evalAutoWrapLength = exprVal.toInt();
  }

  // data defined multiline height?
  dataDefinedValEval( DDDouble, QgsPalLayerSettings::MultiLineHeight, exprVal, context.expressionContext() );

  // data defined multiline text align?
  context.expressionContext().setOriginalValueVariable( mFormat.lineHeight() );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::MultiLineAlignment, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal MultiLineAlignment:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      // "Left"
      QgsPalLayerSettings::MultiLineAlign aligntype = QgsPalLayerSettings::MultiLeft;

      if ( str.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
      {
        aligntype = QgsPalLayerSettings::MultiCenter;
      }
      else if ( str.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
      {
        aligntype = QgsPalLayerSettings::MultiRight;
      }
      else if ( str.compare( QLatin1String( "Follow" ), Qt::CaseInsensitive ) == 0 )
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
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbPlacement, context.expressionContext() );
    if ( exprVal.isValid() )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal DirSymbPlacement:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        // "LeftRight"
        QgsPalLayerSettings::DirectionSymbols placetype = QgsPalLayerSettings::SymbolLeftRight;

        if ( str.compare( QLatin1String( "Above" ), Qt::CaseInsensitive ) == 0 )
        {
          placetype = QgsPalLayerSettings::SymbolAbove;
        }
        else if ( str.compare( QLatin1String( "Below" ), Qt::CaseInsensitive ) == 0 )
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
  double shapeOpacity = background.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::ShapeOpacity, exprVal, context.expressionContext(), shapeOpacity ) )
  {
    shapeOpacity = 100.0 * exprVal.toDouble();
  }

  drawShape = ( drawShape && shapeOpacity > 0 ); // size is not taken into account (could be)

  if ( !drawShape )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeOpacity );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape kind?
  QgsTextBackgroundSettings::ShapeType shapeKind = background.type();
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeKind, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString skind = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeKind:%1" ).arg( skind ), 4 );

    if ( !skind.isEmpty() )
    {
      // "Rectangle"
      QgsTextBackgroundSettings::ShapeType shpkind = QgsTextBackgroundSettings::ShapeRectangle;

      if ( skind.compare( QLatin1String( "Square" ), Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeSquare;
      }
      else if ( skind.compare( QLatin1String( "Ellipse" ), Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeEllipse;
      }
      else if ( skind.compare( QLatin1String( "Circle" ), Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeCircle;
      }
      else if ( skind.compare( QLatin1String( "SVG" ), Qt::CaseInsensitive ) == 0 )
      {
        shpkind = QgsTextBackgroundSettings::ShapeSVG;
      }
      shapeKind = shpkind;
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeKind, QVariant( static_cast< int >( shpkind ) ) );
    }
  }

  // data defined shape SVG path?
  QString svgPath = background.svgFile();
  context.expressionContext().setOriginalValueVariable( svgPath );
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeSVGFile, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString svgfile = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSVGFile:%1" ).arg( svgfile ), 4 );

    // '' empty paths are allowed
    svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
    dataDefinedValues.insert( QgsPalLayerSettings::ShapeSVGFile, QVariant( svgPath ) );
  }

  // data defined shape size type?
  QgsTextBackgroundSettings::SizeType shpSizeType = background.sizeType();
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeSizeType, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString stype = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSizeType:%1" ).arg( stype ), 4 );

    if ( !stype.isEmpty() )
    {
      // "Buffer"
      QgsTextBackgroundSettings::SizeType sizType = QgsTextBackgroundSettings::SizeBuffer;

      if ( stype.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
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
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeOpacity );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeKind );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSVGFile );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSizeX );
    dataDefinedValues.remove( QgsPalLayerSettings::ShapeSizeY );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape size units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeSizeUnits, exprVal, context.expressionContext() );

  // data defined shape rotation type?
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeRotationType, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString rotstr = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeRotationType:%1" ).arg( rotstr ), 4 );

    if ( !rotstr.isEmpty() )
    {
      // "Sync"
      QgsTextBackgroundSettings::RotationType rottype = QgsTextBackgroundSettings::RotationSync;

      if ( rotstr.compare( QLatin1String( "Offset" ), Qt::CaseInsensitive ) == 0 )
      {
        rottype = QgsTextBackgroundSettings::RotationOffset;
      }
      else if ( rotstr.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
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
  dataDefinedValEval( DDSizeF, QgsPalLayerSettings::ShapeRadii, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeSize( background.radii() ) );

  // data defined shape radii units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeRadiiUnits, exprVal, context.expressionContext() );

  // data defined shape blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::ShapeBlendMode, exprVal, context.expressionContext() );

  // data defined shape fill color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::ShapeFillColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( background.fillColor() ) );

  // data defined shape stroke color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::ShapeStrokeColor, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeColor( background.strokeColor() ) );

  // data defined shape stroke width?
  dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::ShapeStrokeWidth, exprVal, context.expressionContext(), background.strokeWidth() );

  // data defined shape stroke width units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::ShapeStrokeWidthUnits, exprVal, context.expressionContext() );

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
  double shadowOpacity = shadow.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::ShadowOpacity, exprVal, context.expressionContext(), shadowOpacity ) )
  {
    shadowOpacity = exprVal.toDouble();
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

  drawShadow = ( drawShadow && shadowOpacity > 0 && !( shadowOffDist == 0.0 && shadowRad == 0.0 ) );

  if ( !drawShadow )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::ShadowDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowOpacity );
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowOffsetDist );
    dataDefinedValues.remove( QgsPalLayerSettings::ShadowRadius );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shadow under type?
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShadowUnder, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString str = exprVal.toString().trimmed();
    QgsDebugMsgLevel( QStringLiteral( "exprVal ShadowUnder:%1" ).arg( str ), 4 );

    if ( !str.isEmpty() )
    {
      // "Lowest"
      QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextShadowSettings::ShadowLowest;

      if ( str.compare( QLatin1String( "Text" ), Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsTextShadowSettings::ShadowText;
      }
      else if ( str.compare( QLatin1String( "Buffer" ), Qt::CaseInsensitive ) == 0 )
      {
        shdwtype = QgsTextShadowSettings::ShadowBuffer;
      }
      else if ( str.compare( QLatin1String( "Background" ), Qt::CaseInsensitive ) == 0 )
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


bool QgsPalLabeling::staticWillUseLayer( QgsVectorLayer *layer )
{
  return layer->labelsEnabled() || layer->diagramsEnabled();
}


bool QgsPalLabeling::geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry )
{
  if ( geometry.isNull() )
  {
    return false;
  }

  //requires reprojection
  if ( ct.isValid() && !ct.isShortCircuited() )
    return true;

  //requires rotation
  const QgsMapToPixel &m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
    return true;

  //requires clip
  if ( !clipGeometry.isNull() && !clipGeometry.boundingBox().contains( geometry.boundingBox() ) )
    return true;

  //requires fixing
  if ( geometry.type() == QgsWkbTypes::PolygonGeometry && !geometry.isGeosValid() )
    return true;

  return false;
}

QStringList QgsPalLabeling::splitToLines( const QString &text, const QString &wrapCharacter, const int autoWrapLength, const bool useMaxLineLengthWhenAutoWrapping )
{
  QStringList multiLineSplit;
  if ( !wrapCharacter.isEmpty() && wrapCharacter != QLatin1String( "\n" ) )
  {
    //wrap on both the wrapchr and new line characters
    const QStringList lines = text.split( wrapCharacter );
    for ( const QString &line : lines )
    {
      multiLineSplit.append( line.split( '\n' ) );
    }
  }
  else
  {
    multiLineSplit = text.split( '\n' );
  }

  // apply auto wrapping to each manually created line
  if ( autoWrapLength != 0 )
  {
    QStringList autoWrappedLines;
    autoWrappedLines.reserve( multiLineSplit.count() );
    for ( const QString &line : qgis::as_const( multiLineSplit ) )
    {
      autoWrappedLines.append( QgsStringUtils::wordWrap( line, autoWrapLength, useMaxLineLengthWhenAutoWrapping ).split( '\n' ) );
    }
    multiLineSplit = autoWrappedLines;
  }
  return multiLineSplit;
}

QStringList QgsPalLabeling::splitToGraphemes( const QString &text )
{
  QStringList graphemes;
  QTextBoundaryFinder boundaryFinder( QTextBoundaryFinder::Grapheme, text );
  int currentBoundary = -1;
  int previousBoundary = 0;
  while ( ( currentBoundary = boundaryFinder.toNextBoundary() ) > 0 )
  {
    graphemes << text.mid( previousBoundary, currentBoundary - previousBoundary );
    previousBoundary = currentBoundary;
  }
  return graphemes;
}

QgsGeometry QgsPalLabeling::prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry )
{
  if ( geometry.isNull() )
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
      QgsDebugMsgLevel( QStringLiteral( "Ignoring feature due to transformation exception" ), 4 );
      return QgsGeometry();
    }
    // geometry transforms may result in nan points, remove these
    geom.filterVertices( []( const QgsPoint & point )->bool
    {
      return std::isfinite( point.x() ) && std::isfinite( point.y() );
    } );
    if ( QgsCurvePolygon *cp = qgsgeometry_cast< QgsCurvePolygon * >( geom.get() ) )
      cp->removeInvalidRings();
  }

  // Rotate the geometry if needed, before clipping
  const QgsMapToPixel &m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPointXY center = context.extent().center();

    if ( ct.isValid() && !ct.isShortCircuited() )
    {
      try
      {
        center = ct.transform( center );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsgLevel( QStringLiteral( "Ignoring feature due to transformation exception" ), 4 );
        return QgsGeometry();
      }
    }

    if ( geom.rotate( m2p.mapRotation(), center ) )
    {
      QgsDebugMsg( QStringLiteral( "Error rotating geometry" ).arg( geom.asWkt() ) );
      return QgsGeometry();
    }
  }

  // fix invalid polygons
  if ( geom.type() == QgsWkbTypes::PolygonGeometry && !geom.isGeosValid() )
  {
    QgsGeometry bufferGeom = geom.buffer( 0, 0 );
    if ( bufferGeom.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Could not repair geometry: %1" ).arg( bufferGeom.lastError() ) );
      return QgsGeometry();
    }
    geom = bufferGeom;
  }

  if ( !clipGeometry.isNull() &&
       ( ( qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.boundingBox().contains( geom.boundingBox() ) )
         || ( !qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.contains( geom ) ) ) )
  {
    QgsGeometry clipGeom = geom.intersection( clipGeometry ); // creates new geometry
    if ( clipGeom.isNull() )
    {
      return QgsGeometry();
    }
    geom = clipGeom;
  }

  return geom;
}

bool QgsPalLabeling::checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry &geom, double minSize )
{
  if ( minSize <= 0 )
  {
    return true;
  }

  if ( geom.isNull() )
  {
    return false;
  }

  QgsWkbTypes::GeometryType featureType = geom.type();
  if ( featureType == QgsWkbTypes::PointGeometry ) //minimum size does not apply to point features
  {
    return true;
  }

  double mapUnitsPerMM = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();
  if ( featureType == QgsWkbTypes::LineGeometry )
  {
    double length = geom.length();
    if ( length >= 0.0 )
    {
      return ( length >= ( minSize * mapUnitsPerMM ) );
    }
  }
  else if ( featureType == QgsWkbTypes::PolygonGeometry )
  {
    double area = geom.area();
    if ( area >= 0.0 )
    {
      return ( std::sqrt( area ) >= ( minSize * mapUnitsPerMM ) );
    }
  }
  return true; //should never be reached. Return true in this case to label such geometries anyway.
}


void QgsPalLabeling::dataDefinedTextStyle( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
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
  if ( ddValues.contains( QgsPalLayerSettings::FontOpacity ) )
  {
    format.setOpacity( ddValues.value( QgsPalLayerSettings::FontOpacity ).toDouble() / 100.0 );
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

void QgsPalLabeling::dataDefinedTextFormatting( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
{
  if ( ddValues.contains( QgsPalLayerSettings::MultiLineWrapChar ) )
  {
    tmpLyr.wrapChar = ddValues.value( QgsPalLayerSettings::MultiLineWrapChar ).toString();
  }

  if ( ddValues.contains( QgsPalLayerSettings::AutoWrapLength ) )
  {
    tmpLyr.autoWrapLength = ddValues.value( QgsPalLayerSettings::AutoWrapLength ).toInt();
  }

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

void QgsPalLabeling::dataDefinedTextBuffer( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
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

  //buffer opacity
  if ( ddValues.contains( QgsPalLayerSettings::BufferOpacity ) )
  {
    buffer.setOpacity( ddValues.value( QgsPalLayerSettings::BufferOpacity ).toDouble() / 100.0 );
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

void QgsPalLabeling::dataDefinedShapeBackground( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
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

  if ( ddValues.contains( QgsPalLayerSettings::ShapeStrokeColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::ShapeStrokeColor );
    background.setStrokeColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeOpacity ) )
  {
    background.setOpacity( ddValues.value( QgsPalLayerSettings::ShapeOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeStrokeWidth ) )
  {
    background.setStrokeWidth( ddValues.value( QgsPalLayerSettings::ShapeStrokeWidth ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::ShapeStrokeWidthUnits ) )
  {
    background.setStrokeWidthUnit( static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::ShapeStrokeWidthUnits ).toInt() ) );
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

void QgsPalLabeling::dataDefinedDropShadow( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
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

  if ( ddValues.contains( QgsPalLayerSettings::ShadowOpacity ) )
  {
    shadow.setOpacity( ddValues.value( QgsPalLayerSettings::ShadowOpacity ).toDouble() / 100.0 );
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


void QgsPalLabeling::drawLabelCandidateRect( pal::LabelPosition *lp, QPainter *painter, const QgsMapToPixel *xform, QList<QgsLabelCandidate> *candidates )
{
  QgsPointXY outPt = xform->transform( lp->getX(), lp->getY() );

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
  QgsPointXY outPt2 = xform->transform( lp->getX() + lp->getWidth(), lp->getY() + lp->getHeight() );
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

QgsLabelingResults::QgsLabelingResults()
{
  mLabelSearchTree = new QgsLabelSearchTree();
}

QgsLabelingResults::~QgsLabelingResults()
{
  delete mLabelSearchTree;
  mLabelSearchTree = nullptr;
}

QList<QgsLabelPosition> QgsLabelingResults::labelsAtPosition( const QgsPointXY &p ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition *> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->label( p, positionPointers );
    QList<QgsLabelPosition *>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}

QList<QgsLabelPosition> QgsLabelingResults::labelsWithinRect( const QgsRectangle &r ) const
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition *> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->labelsInRect( r, positionPointers );
    QList<QgsLabelPosition *>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}
