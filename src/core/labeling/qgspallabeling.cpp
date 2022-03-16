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
#include "qgsapplication.h"
#include "qgsstyle.h"

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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#else
#include <QScreen>
#include <QWidget>
#endif
#include <QTextBoundaryFinder>

#include "diagram/qgsdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsfontutils.h"
#include "qgslabelsearchtree.h"
#include "qgsexpression.h"
#include "qgslabelingengine.h"
#include "qgsvectorlayerlabeling.h"
#include "qgstextrendererutils.h"
#include "qgstextfragment.h"
#include "qgsmultisurface.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayerdiagramprovider.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgsgeometry.h"
#include "qgsreferencedgeometry.h"
#include "qgsmarkersymbollayer.h"
#include "qgspainting.h"
#include "qgsproject.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgscurvepolygon.h"
#include "qgsmessagelog.h"
#include "qgsgeometrycollection.h"
#include "callouts/qgscallout.h"
#include "callouts/qgscalloutsregistry.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilebasiclabeling.h"

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
typedef QVector< QgsPalLayerSettings::PredefinedPointPosition > PredefinedPointPositionVector;
Q_GLOBAL_STATIC_WITH_ARGS( PredefinedPointPositionVector, DEFAULT_PLACEMENT_ORDER, (
{
  QgsPalLayerSettings::TopRight,
  QgsPalLayerSettings::TopLeft,
  QgsPalLayerSettings::BottomRight,
  QgsPalLayerSettings::BottomLeft,
  QgsPalLayerSettings::MiddleRight,
  QgsPalLayerSettings::MiddleLeft,
  QgsPalLayerSettings::TopSlightlyRight,
  QgsPalLayerSettings::BottomSlightlyRight
} ) )
//debugging only - don't use these placements by default
/* << QgsPalLayerSettings::TopSlightlyLeft
<< QgsPalLayerSettings::BottomSlightlyLeft;
<< QgsPalLayerSettings::TopMiddle
<< QgsPalLayerSettings::BottomMiddle;*/

Q_GLOBAL_STATIC( QgsPropertiesDefinition, sPropertyDefinitions )

void QgsPalLayerSettings::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions()->isEmpty() )
    return;

  const QString origin = QStringLiteral( "labeling" );

  *sPropertyDefinitions() = QgsPropertiesDefinition
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
    { QgsPalLayerSettings::FontStretchFactor, QgsPropertyDefinition( "FontStretchFactor", QObject::tr( "Font stretch factor" ), QgsPropertyDefinition::IntegerPositiveGreaterZero, origin ) },
    { QgsPalLayerSettings::FontCase, QgsPropertyDefinition( "FontCase", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font case" ), QObject::tr( "string " ) + QStringLiteral( "[<b>NoChange</b>|<b>Upper</b>|<br><b>Lower</b>|<b>Title</b>|<b>Capitalize</b>|<b>SmallCaps</b>|<b>AllSmallCaps</b>]" ), origin ) },
    { QgsPalLayerSettings::FontLetterSpacing, QgsPropertyDefinition( "FontLetterSpacing", QObject::tr( "Letter spacing" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::FontWordSpacing, QgsPropertyDefinition( "FontWordSpacing", QObject::tr( "Word spacing" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::FontBlendMode, QgsPropertyDefinition( "FontBlendMode", QObject::tr( "Text blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { QgsPalLayerSettings::MultiLineWrapChar, QgsPropertyDefinition( "MultiLineWrapChar", QObject::tr( "Wrap character" ), QgsPropertyDefinition::String, origin ) },
    { QgsPalLayerSettings::AutoWrapLength, QgsPropertyDefinition( "AutoWrapLength", QObject::tr( "Automatic word wrap line length" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { QgsPalLayerSettings::MultiLineHeight, QgsPropertyDefinition( "MultiLineHeight", QObject::tr( "Line height" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::MultiLineAlignment, QgsPropertyDefinition( "MultiLineAlignment", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line alignment" ), QObject::tr( "string " ) + "[<b>Left</b>|<b>Center</b>|<b>Right</b>|<b>Follow</b>]", origin ) },
    { QgsPalLayerSettings::TextOrientation, QgsPropertyDefinition( "TextOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Text orientation" ), QObject::tr( "string " ) + "[<b>horizontal</b>|<b>vertical</b>]", origin ) },
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

    { QgsPalLayerSettings::MaskEnabled, QgsPropertyDefinition( "MaskEnabled", QObject::tr( "Enable mask" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::MaskBufferSize, QgsPropertyDefinition( "MaskBufferSize", QObject::tr( "Mask buffer size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::MaskBufferUnit, QgsPropertyDefinition( "MaskBufferUnit", QObject::tr( "Mask buffer unit" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::MaskOpacity, QgsPropertyDefinition( "MaskOpacity", QObject::tr( "Mask opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { QgsPalLayerSettings::MaskJoinStyle, QgsPropertyDefinition( "MaskJoinStyle", QObject::tr( "Mask join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },

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
    { QgsPalLayerSettings::LabelDistance, QgsPropertyDefinition( "LabelDistance", QObject::tr( "Label distance" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::DistanceUnits, QgsPropertyDefinition( "DistanceUnits", QObject::tr( "Label distance units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::OffsetRotation, QgsPropertyDefinition( "OffsetRotation", QObject::tr( "Offset rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { QgsPalLayerSettings::CurvedCharAngleInOut, QgsPropertyDefinition( "CurvedCharAngleInOut", QgsPropertyDefinition::DataTypeString, QObject::tr( "Curved character angles" ), QObject::tr( "double coord [<b>in,out</b> as 20.0-60.0,20.0-95.0]" ), origin ) },
    { QgsPalLayerSettings::RepeatDistance, QgsPropertyDefinition( "RepeatDistance", QObject::tr( "Repeat distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::RepeatDistanceUnit, QgsPropertyDefinition( "RepeatDistanceUnit", QObject::tr( "Repeat distance unit" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { QgsPalLayerSettings::OverrunDistance, QgsPropertyDefinition( "OverrunDistance", QObject::tr( "Overrun distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsPalLayerSettings::LineAnchorPercent, QgsPropertyDefinition( "LineAnchorPercent", QObject::tr( "Line anchor percentage, as fraction from 0.0 to 1.0" ), QgsPropertyDefinition::Double0To1, origin ) },
    { QgsPalLayerSettings::LineAnchorClipping, QgsPropertyDefinition( "LineAnchorClipping", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor clipping mode" ), QObject::tr( "string " ) + QStringLiteral( "[<b>visible</b>|<b>entire</b>]" ), origin ) },
    { QgsPalLayerSettings::LineAnchorType, QgsPropertyDefinition( "LineAnchorType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor type" ), QObject::tr( "string " ) + QStringLiteral( "[<b>hint</b>|<b>strict</b>]" ), origin ) },
    { QgsPalLayerSettings::LineAnchorTextPoint, QgsPropertyDefinition( "LineAnchorTextPoint", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor text point" ), QObject::tr( "string " ) + QStringLiteral( "[<b>follow</b>|<b>start</b>|<b>center</b>|<b>end</b>]" ), origin ) },
    { QgsPalLayerSettings::Priority, QgsPropertyDefinition( "Priority", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Label priority" ), QObject::tr( "double [0.0-10.0]" ), origin ) },
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
    {
      QgsPalLayerSettings::LinePlacementOptions, QgsPropertyDefinition( "LinePlacementFlags", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line placement options" ),  QObject::tr( "Comma separated list of placement options<br>" )
          + QStringLiteral( "[<b>OL</b>=On line|<b>AL</b>=Above line|<b>BL</b>=Below line|<br>"
                            "<b>LO</b>=Respect line orientation]" ), origin )
    },
    { QgsPalLayerSettings::PolygonLabelOutside, QgsPropertyDefinition( "PolygonLabelOutside", QgsPropertyDefinition::DataTypeString, QObject::tr( "Label outside polygons" ),  QObject::tr( "string " ) + "[<b>yes</b> (allow placing outside)|<b>no</b> (never place outside)|<b>force</b> (always place outside)]", origin ) },
    { QgsPalLayerSettings::PositionX, QgsPropertyDefinition( "PositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::PositionY, QgsPropertyDefinition( "PositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsPalLayerSettings::PositionPoint, QgsPropertyDefinition( "PositionPoint", QgsPropertyDefinition::DataTypeString, QObject::tr( "Position (point)" ), QObject::tr( "A point geometry" ), origin ) },
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
    { QgsPalLayerSettings::CalloutDraw, QgsPropertyDefinition( "CalloutDraw", QObject::tr( "Draw callout" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsPalLayerSettings::LabelAllParts, QgsPropertyDefinition( "LabelAllParts", QObject::tr( "Label all parts" ), QgsPropertyDefinition::Boolean, origin ) },
  };
}

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsPalLayerSettings::QgsPalLayerSettings()
  : predefinedPositionOrder( *DEFAULT_PLACEMENT_ORDER() )
  , mCallout( QgsCalloutRegistry::defaultCallout() )
{
  initPropertyDefinitions();

  mFormat = QgsStyle::defaultStyle()->defaultTextFormat( QgsStyle::TextFormatContext::Labeling );
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings &s )
  : fieldIndex( 0 )
  , mDataDefinedProperties( s.mDataDefinedProperties )
{
  *this = s;
}
Q_NOWARN_DEPRECATED_POP

QgsPalLayerSettings &QgsPalLayerSettings::operator=( const QgsPalLayerSettings &s )
{
  if ( this == &s )
    return *this;

  // copy only permanent stuff

  drawLabels = s.drawLabels;

  // text style
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  Q_NOWARN_DEPRECATED_PUSH
  previewBkgrdColor = s.previewBkgrdColor;
  Q_NOWARN_DEPRECATED_POP
  substitutions = s.substitutions;
  useSubstitutions = s.useSubstitutions;

  // text formatting
  wrapChar = s.wrapChar;
  autoWrapLength = s.autoWrapLength;
  useMaxLineLengthForAutoWrap = s.useMaxLineLengthForAutoWrap;
  multilineAlign = s.multilineAlign;
  formatNumbers = s.formatNumbers;
  decimals = s.decimals;
  plusSign = s.plusSign;

  // placement
  placement = s.placement;
  mPolygonPlacementFlags = s.mPolygonPlacementFlags;
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
  mRotationUnit = s.mRotationUnit;
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
  zIndex = s.zIndex;

  mFormat = s.mFormat;
  mDataDefinedProperties = s.mDataDefinedProperties;

  mCallout.reset( s.mCallout ? s.mCallout->clone() : nullptr );

  mLineSettings = s.mLineSettings;
  mObstacleSettings = s.mObstacleSettings;
  mThinningSettings = s.mThinningSettings;

  geometryGenerator = s.geometryGenerator;
  geometryGeneratorEnabled = s.geometryGeneratorEnabled;
  geometryGeneratorType = s.geometryGeneratorType;
  layerType = s.layerType;

  mLegendString = s.mLegendString;

  mUnplacedVisibility = s.mUnplacedVisibility;

  return *this;
}

bool QgsPalLayerSettings::prepare( QgsRenderContext &context, QSet<QString> &attributeNames, const QgsFields &fields, const QgsMapSettings &mapSettings, const QgsCoordinateReferenceSystem &crs )
{
  if ( drawLabels )
  {
    if ( fieldName.isEmpty() )
    {
      return false;
    }

    if ( isExpression )
    {
      QgsExpression exp( fieldName );
      if ( exp.hasEvalError() )
      {
        QgsDebugMsgLevel( "Prepare error:" + exp.evalErrorString(), 4 );
        return false;
      }
    }
    else
    {
      // If we aren't an expression, we check to see if we can find the column.
      if ( fields.lookupField( fieldName ) == -1 )
      {
        return false;
      }
    }
  }

  mCurFields = fields;

  if ( drawLabels || mObstacleSettings.isObstacle() )
  {
    if ( drawLabels )
    {
      // add field indices for label's text, from expression or field
      if ( isExpression )
      {
        // prepare expression for use in QgsPalLayerSettings::registerFeature()
        QgsExpression *exp = getLabelExpression();
        exp->prepare( &context.expressionContext() );
        if ( exp->hasEvalError() )
        {
          QgsDebugMsgLevel( "Prepare error:" + exp->evalErrorString(), 4 );
        }
        const auto referencedColumns = exp->referencedColumns();
        for ( const QString &name : referencedColumns )
        {
          attributeNames.insert( name );
        }
      }
      else
      {
        attributeNames.insert( fieldName );
      }
    }

    mDataDefinedProperties.prepare( context.expressionContext() );
    // add field indices of data defined expression or field
    attributeNames.unite( dataDefinedProperties().referencedFields( context.expressionContext() ) );
  }

  // NOW INITIALIZE QgsPalLayerSettings

  // TODO: ideally these (non-configuration) members should get out of QgsPalLayerSettings to QgsVectorLayerLabelProvider::prepare
  // (together with registerFeature() & related methods) and QgsPalLayerSettings just stores config

  // save the pal layer to our layer context (with some additional info)
  fieldIndex = fields.lookupField( fieldName );

  xform = &mapSettings.mapToPixel();
  ct = QgsCoordinateTransform();
  if ( context.coordinateTransform().isValid() )
    // this is context for layer rendering
    ct = context.coordinateTransform();
  else
  {
    // otherwise fall back to creating our own CT
    ct = QgsCoordinateTransform( crs, mapSettings.destinationCrs(), mapSettings.transformContext() );
  }
  ptZero = xform->toMapCoordinates( 0, 0 );
  ptOne = xform->toMapCoordinates( 1, 0 );

  // rect for clipping
  QgsRectangle r1 = mapSettings.visibleExtent();
  r1.grow( mapSettings.extentBuffer() );
  extentGeom = QgsGeometry::fromRect( r1 );

  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
  }

  mFeatsSendingToPal = 0;

  if ( geometryGeneratorEnabled )
  {
    mGeometryGeneratorExpression = QgsExpression( geometryGenerator );
    mGeometryGeneratorExpression.prepare( &context.expressionContext() );
    if ( mGeometryGeneratorExpression.hasParserError() )
    {
      QgsMessageLog::logMessage( mGeometryGeneratorExpression.parserErrorString(), QObject::tr( "Labeling" ) );
      return false;
    }

    const auto referencedColumns = mGeometryGeneratorExpression.referencedColumns();
    for ( const QString &name : referencedColumns )
    {
      attributeNames.insert( name );
    }
  }
  attributeNames.unite( mFormat.referencedFields( context ) );

  if ( mCallout )
  {
    const auto referencedColumns = mCallout->referencedFields( context );
    for ( const QString &name : referencedColumns )
    {
      attributeNames.insert( name );
    }
  }

  return true;
}

QSet<QString> QgsPalLayerSettings::referencedFields( const QgsRenderContext &context ) const
{
  QSet<QString> referenced;
  if ( drawLabels )
  {
    if ( isExpression )
    {
      referenced.unite( QgsExpression( fieldName ).referencedColumns() );
    }
    else
    {
      referenced.insert( fieldName );
    }
  }

  referenced.unite( mFormat.referencedFields( context ) );

  // calling referencedFields() with ignoreContext=true because in our expression context
  // we do not have valid QgsFields yet - because of that the field names from expressions
  // wouldn't get reported
  referenced.unite( mDataDefinedProperties.referencedFields( context.expressionContext(), true ) );

  if ( geometryGeneratorEnabled )
  {
    QgsExpression geomGeneratorExpr( geometryGenerator );
    referenced.unite( geomGeneratorExpr.referencedColumns() );
  }

  if ( mCallout )
  {
    referenced.unite( mCallout->referencedFields( context ) );
  }

  return referenced;
}

void QgsPalLayerSettings::startRender( QgsRenderContext &context )
{
  if ( mRenderStarted )
  {
    qWarning( "Start render called for when a previous render was already underway!!" );
    return;
  }

  if ( placement == QgsPalLayerSettings::Curved )
  {
    // force horizontal orientation, other orientation modes aren't unsupported for curved placement
    mFormat.setOrientation( QgsTextFormat::HorizontalOrientation );
    mDataDefinedProperties.property( QgsPalLayerSettings::TextOrientation ).setActive( false );
  }

  if ( mCallout )
  {
    mCallout->startRender( context );
  }

  mRenderStarted = true;
}

void QgsPalLayerSettings::stopRender( QgsRenderContext &context )
{
  if ( !mRenderStarted )
  {
    qWarning( "Stop render called for QgsPalLayerSettings without a startRender call!" );
    return;
  }

  if ( mCallout )
  {
    mCallout->stopRender( context );
  }

  mRenderStarted = false;
}

bool QgsPalLayerSettings::containsAdvancedEffects() const
{
  return mFormat.containsAdvancedEffects() || mCallout->containsAdvancedEffects();
}

QgsPalLayerSettings::~QgsPalLayerSettings()
{
  if ( mRenderStarted )
  {
    qWarning( "stopRender was not called on QgsPalLayerSettings object!" );
  }

  // pal layer is deleted internally in PAL

  delete expression;
}


const QgsPropertiesDefinition &QgsPalLayerSettings::propertyDefinitions()
{
  initPropertyDefinitions();
  return *sPropertyDefinitions();
}

QgsExpression *QgsPalLayerSettings::getLabelExpression()
{
  if ( !expression )
  {
    expression = new QgsExpression( fieldName );
  }
  return expression;
}

QgsUnitTypes::AngleUnit QgsPalLayerSettings::rotationUnit() const
{
  return mRotationUnit;
}

void QgsPalLayerSettings::setRotationUnit( QgsUnitTypes::AngleUnit angleUnit )
{
  mRotationUnit = angleUnit;
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
    newValue = values.join( QLatin1String( "~~" ) );
  }

  return newValue;
}

void QgsPalLayerSettings::readOldDataDefinedProperty( QgsVectorLayer *layer, QgsPalLayerSettings::Property p )
{
  QString newPropertyName = "labeling/dataDefined/" + sPropertyDefinitions()->value( p ).name();
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

  QgsPropertiesDefinition::const_iterator i = sPropertyDefinitions()->constBegin();
  for ( ; i != sPropertyDefinitions()->constEnd(); ++i )
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
  Q_NOWARN_DEPRECATED_PUSH
  previewBkgrdColor = QColor( layer->customProperty( QStringLiteral( "labeling/previewBkgrdColor" ), QVariant( "#ffffff" ) ).toString() );
  Q_NOWARN_DEPRECATED_POP
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
  mLineSettings.setAddDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/addDirectionSymbol" ) ).toBool() );
  mLineSettings.setLeftDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/leftDirectionSymbol" ), QVariant( "<" ) ).toString() );
  mLineSettings.setRightDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/rightDirectionSymbol" ), QVariant( ">" ) ).toString() );
  mLineSettings.setReverseDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/reverseDirectionSymbol" ) ).toBool() );
  mLineSettings.setDirectionSymbolPlacement( static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( layer->customProperty( QStringLiteral( "labeling/placeDirectionSymbol" ), QVariant( static_cast< int >( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight ) ) ).toUInt() ) );
  formatNumbers = layer->customProperty( QStringLiteral( "labeling/formatNumbers" ) ).toBool();
  decimals = layer->customProperty( QStringLiteral( "labeling/decimals" ) ).toInt();
  plusSign = layer->customProperty( QStringLiteral( "labeling/plussign" ) ).toBool();

  // placement
  placement = static_cast< Placement >( layer->customProperty( QStringLiteral( "labeling/placement" ) ).toInt() );
  mLineSettings.setPlacementFlags( static_cast< QgsLabeling::LinePlacementFlags >( layer->customProperty( QStringLiteral( "labeling/placementFlags" ) ).toUInt() ) );
  centroidWhole = layer->customProperty( QStringLiteral( "labeling/centroidWhole" ), QVariant( false ) ).toBool();
  centroidInside = layer->customProperty( QStringLiteral( "labeling/centroidInside" ), QVariant( false ) ).toBool();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( layer->customProperty( QStringLiteral( "labeling/predefinedPositionOrder" ) ).toString() );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = *DEFAULT_PLACEMENT_ORDER();
  fitInPolygonOnly = layer->customProperty( QStringLiteral( "labeling/fitInPolygonOnly" ), QVariant( false ) ).toBool();
  dist = layer->customProperty( QStringLiteral( "labeling/dist" ) ).toDouble();
  distUnits = layer->customProperty( QStringLiteral( "labeling/distInMapUnits" ) ).toBool() ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  if ( layer->customProperty( QStringLiteral( "labeling/distMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/distMapUnitMinScale" ), 0.0 ).toDouble();
    distMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0.0 ) ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/distMapUnitMaxScale" ), 0.0 ).toDouble();
    distMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0.0 ) ? 1.0 / oldMax : 0;
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
    labelOffsetMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0.0 ) ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/labelOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
    labelOffsetMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0 ) ? 1.0 / oldMax : 0;
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
  mRotationUnit = layer->customEnumProperty( QStringLiteral( "labeling/rotationUnit" ), QgsUnitTypes::AngleDegrees );
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
    repeatDistanceMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0 ) ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/repeatDistanceMapUnitMaxScale" ), 0.0 ).toDouble();
    repeatDistanceMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0 ) ? 1.0 / oldMax : 0;
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
  mLineSettings.setMergeLines( layer->customProperty( QStringLiteral( "labeling/mergeLines" ) ).toBool() );
  mThinningSettings.setMinimumFeatureSize( layer->customProperty( QStringLiteral( "labeling/minFeatureSize" ) ).toDouble() );
  mThinningSettings.setLimitNumberLabelsEnabled( layer->customProperty( QStringLiteral( "labeling/limitNumLabels" ), QVariant( false ) ).toBool() );
  mThinningSettings.setMaximumNumberLabels( layer->customProperty( QStringLiteral( "labeling/maxNumLabels" ), QVariant( 2000 ) ).toInt() );
  mObstacleSettings.setIsObstacle( layer->customProperty( QStringLiteral( "labeling/obstacle" ), QVariant( true ) ).toBool() );
  mObstacleSettings.setFactor( layer->customProperty( QStringLiteral( "labeling/obstacleFactor" ), QVariant( 1.0 ) ).toDouble() );
  mObstacleSettings.setType( static_cast< QgsLabelObstacleSettings::ObstacleType >( layer->customProperty( QStringLiteral( "labeling/obstacleType" ), QVariant( PolygonInterior ) ).toUInt() ) );
  zIndex = layer->customProperty( QStringLiteral( "labeling/zIndex" ), QVariant( 0.0 ) ).toDouble();

  mDataDefinedProperties.clear();
  if ( layer->customProperty( QStringLiteral( "labeling/ddProperties" ) ).isValid() )
  {
    QDomDocument doc( QStringLiteral( "dd" ) );
    doc.setContent( layer->customProperty( QStringLiteral( "labeling/ddProperties" ) ).toString() );
    QDomElement elem = doc.firstChildElement( QStringLiteral( "properties" ) );
    mDataDefinedProperties.readXml( elem, *sPropertyDefinitions() );
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

void QgsPalLayerSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  // text style
  QDomElement textStyleElem = elem.firstChildElement( QStringLiteral( "text-style" ) );
  fieldName = textStyleElem.attribute( QStringLiteral( "fieldName" ) );
  isExpression = textStyleElem.attribute( QStringLiteral( "isExpression" ) ).toInt();

  mFormat.readXml( elem, context );
  Q_NOWARN_DEPRECATED_PUSH
  previewBkgrdColor = QColor( textStyleElem.attribute( QStringLiteral( "previewBkgrdColor" ), QStringLiteral( "#ffffff" ) ) );
  Q_NOWARN_DEPRECATED_POP
  substitutions.readXml( textStyleElem.firstChildElement( QStringLiteral( "substitutions" ) ) );
  useSubstitutions = textStyleElem.attribute( QStringLiteral( "useSubstitutions" ) ).toInt();
  mLegendString = textStyleElem.attribute( QStringLiteral( "legendString" ), QObject::tr( "Aa" ) );

  // text formatting
  QDomElement textFormatElem = elem.firstChildElement( QStringLiteral( "text-format" ) );
  wrapChar = textFormatElem.attribute( QStringLiteral( "wrapChar" ) );
  autoWrapLength = textFormatElem.attribute( QStringLiteral( "autoWrapLength" ), QStringLiteral( "0" ) ).toInt();
  useMaxLineLengthForAutoWrap = textFormatElem.attribute( QStringLiteral( "useMaxLineLengthForAutoWrap" ), QStringLiteral( "1" ) ).toInt();
  multilineAlign = static_cast< MultiLineAlign >( textFormatElem.attribute( QStringLiteral( "multilineAlign" ), QString::number( MultiFollowPlacement ) ).toUInt() );
  mLineSettings.setAddDirectionSymbol( textFormatElem.attribute( QStringLiteral( "addDirectionSymbol" ) ).toInt() );
  mLineSettings.setLeftDirectionSymbol( textFormatElem.attribute( QStringLiteral( "leftDirectionSymbol" ), QStringLiteral( "<" ) ) );
  mLineSettings.setRightDirectionSymbol( textFormatElem.attribute( QStringLiteral( "rightDirectionSymbol" ), QStringLiteral( ">" ) ) );
  mLineSettings.setReverseDirectionSymbol( textFormatElem.attribute( QStringLiteral( "reverseDirectionSymbol" ) ).toInt() );
  mLineSettings.setDirectionSymbolPlacement( static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( textFormatElem.attribute( QStringLiteral( "placeDirectionSymbol" ), QString::number( static_cast< int >( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight ) ) ).toUInt() ) );
  formatNumbers = textFormatElem.attribute( QStringLiteral( "formatNumbers" ) ).toInt();
  decimals = textFormatElem.attribute( QStringLiteral( "decimals" ) ).toInt();
  plusSign = textFormatElem.attribute( QStringLiteral( "plussign" ) ).toInt();

  // placement
  QDomElement placementElem = elem.firstChildElement( QStringLiteral( "placement" ) );
  placement = static_cast< Placement >( placementElem.attribute( QStringLiteral( "placement" ) ).toInt() );
  mLineSettings.setPlacementFlags( static_cast< QgsLabeling::LinePlacementFlags >( placementElem.attribute( QStringLiteral( "placementFlags" ) ).toUInt() ) );
  mPolygonPlacementFlags = static_cast< QgsLabeling::PolygonPlacementFlags >( placementElem.attribute( QStringLiteral( "polygonPlacementFlags" ), QString::number( static_cast< int >( QgsLabeling::PolygonPlacementFlag::AllowPlacementInsideOfPolygon ) ) ).toInt() );

  centroidWhole = placementElem.attribute( QStringLiteral( "centroidWhole" ), QStringLiteral( "0" ) ).toInt();
  centroidInside = placementElem.attribute( QStringLiteral( "centroidInside" ), QStringLiteral( "0" ) ).toInt();
  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( placementElem.attribute( QStringLiteral( "predefinedPositionOrder" ) ) );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = *DEFAULT_PLACEMENT_ORDER();
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
    distMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0 ) ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "distMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    distMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0 ) ? 1.0 / oldMax : 0;
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
    labelOffsetMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0.0 ) ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "labelOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    labelOffsetMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0.0 ) ? 1.0 / oldMax : 0;
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
  mRotationUnit = qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "rotationUnit" ), qgsEnumValueToKey( QgsUnitTypes::AngleDegrees ) ), QgsUnitTypes::AngleDegrees );
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
    repeatDistanceMapUnitScale.minScale = !qgsDoubleNear( oldMin, 0.0 ) ? 1.0 / oldMin : 0;
    double oldMax = placementElem.attribute( QStringLiteral( "repeatDistanceMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    repeatDistanceMapUnitScale.maxScale = !qgsDoubleNear( oldMax, 0.0 ) ? 1.0 / oldMax : 0;
  }
  else
  {
    repeatDistanceMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "repeatDistanceMapUnitScale" ) ) );
  }

  mLineSettings.setOverrunDistance( placementElem.attribute( QStringLiteral( "overrunDistance" ), QStringLiteral( "0" ) ).toDouble() );
  mLineSettings.setOverrunDistanceUnit( QgsUnitTypes::decodeRenderUnit( placementElem.attribute( QStringLiteral( "overrunDistanceUnit" ) ) ) );
  mLineSettings.setOverrunDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "overrunDistanceMapUnitScale" ) ) ) );
  mLineSettings.setLineAnchorPercent( placementElem.attribute( QStringLiteral( "lineAnchorPercent" ), QStringLiteral( "0.5" ) ).toDouble() );
  mLineSettings.setAnchorType( static_cast< QgsLabelLineSettings::AnchorType >( placementElem.attribute( QStringLiteral( "lineAnchorType" ), QStringLiteral( "0" ) ).toInt() ) );
  mLineSettings.setAnchorClipping( static_cast< QgsLabelLineSettings::AnchorClipping >( placementElem.attribute( QStringLiteral( "lineAnchorClipping" ), QStringLiteral( "0" ) ).toInt() ) );
  // when reading the anchor text point we default to center mode, to keep same result as for proejcts created in < 3.26
  mLineSettings.setAnchorTextPoint( qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "lineAnchorTextPoint" ) ), QgsLabelLineSettings::AnchorTextPoint::CenterOfText ) );

  geometryGenerator = placementElem.attribute( QStringLiteral( "geometryGenerator" ) );
  geometryGeneratorEnabled = placementElem.attribute( QStringLiteral( "geometryGeneratorEnabled" ) ).toInt();
  geometryGeneratorType = qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "geometryGeneratorType" ) ), QgsWkbTypes::PointGeometry );

  layerType = qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "layerType" ) ), QgsWkbTypes::UnknownGeometry );

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
  mLineSettings.setMergeLines( renderingElem.attribute( QStringLiteral( "mergeLines" ) ).toInt() );
  mThinningSettings.setMinimumFeatureSize( renderingElem.attribute( QStringLiteral( "minFeatureSize" ) ).toDouble() );
  mThinningSettings.setLimitNumberLabelsEnabled( renderingElem.attribute( QStringLiteral( "limitNumLabels" ), QStringLiteral( "0" ) ).toInt() );
  mThinningSettings.setMaximumNumberLabels( renderingElem.attribute( QStringLiteral( "maxNumLabels" ), QStringLiteral( "2000" ) ).toInt() );
  mObstacleSettings.setIsObstacle( renderingElem.attribute( QStringLiteral( "obstacle" ), QStringLiteral( "1" ) ).toInt() );
  mObstacleSettings.setFactor( renderingElem.attribute( QStringLiteral( "obstacleFactor" ), QStringLiteral( "1" ) ).toDouble() );
  mObstacleSettings.setType( static_cast< QgsLabelObstacleSettings::ObstacleType >( renderingElem.attribute( QStringLiteral( "obstacleType" ), QString::number( PolygonInterior ) ).toUInt() ) );
  zIndex = renderingElem.attribute( QStringLiteral( "zIndex" ), QStringLiteral( "0.0" ) ).toDouble();
  mUnplacedVisibility = static_cast< Qgis::UnplacedLabelVisibility >( renderingElem.attribute( QStringLiteral( "unplacedVisibility" ), QString::number( static_cast< int >( Qgis::UnplacedLabelVisibility::FollowEngineSetting ) ) ).toInt() );

  QDomElement ddElem = elem.firstChildElement( QStringLiteral( "dd_properties" ) );
  if ( !ddElem.isNull() )
  {
    mDataDefinedProperties.readXml( ddElem, *sPropertyDefinitions() );
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

  // TODO - replace with registry when multiple callout styles exist
  const QString calloutType = elem.attribute( QStringLiteral( "calloutType" ) );
  if ( calloutType.isEmpty() )
    mCallout.reset( QgsCalloutRegistry::defaultCallout() );
  else
  {
    mCallout.reset( QgsApplication::calloutRegistry()->createCallout( calloutType, elem.firstChildElement( QStringLiteral( "callout" ) ), context ) );
    if ( !mCallout )
      mCallout.reset( QgsCalloutRegistry::defaultCallout() );
  }
}

QDomElement QgsPalLayerSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement textStyleElem = mFormat.writeXml( doc, context );

  // text style
  textStyleElem.setAttribute( QStringLiteral( "fieldName" ), fieldName );
  textStyleElem.setAttribute( QStringLiteral( "isExpression" ), isExpression );
  QDomElement replacementElem = doc.createElement( QStringLiteral( "substitutions" ) );
  substitutions.writeXml( replacementElem, doc );
  textStyleElem.appendChild( replacementElem );
  textStyleElem.setAttribute( QStringLiteral( "useSubstitutions" ), useSubstitutions );
  textStyleElem.setAttribute( QStringLiteral( "legendString" ), mLegendString );

  // text formatting
  QDomElement textFormatElem = doc.createElement( QStringLiteral( "text-format" ) );
  textFormatElem.setAttribute( QStringLiteral( "wrapChar" ), wrapChar );
  textFormatElem.setAttribute( QStringLiteral( "autoWrapLength" ), autoWrapLength );
  textFormatElem.setAttribute( QStringLiteral( "useMaxLineLengthForAutoWrap" ), useMaxLineLengthForAutoWrap );
  textFormatElem.setAttribute( QStringLiteral( "multilineAlign" ), static_cast< unsigned int >( multilineAlign ) );
  textFormatElem.setAttribute( QStringLiteral( "addDirectionSymbol" ), mLineSettings.addDirectionSymbol() );
  textFormatElem.setAttribute( QStringLiteral( "leftDirectionSymbol" ), mLineSettings.leftDirectionSymbol() );
  textFormatElem.setAttribute( QStringLiteral( "rightDirectionSymbol" ), mLineSettings.rightDirectionSymbol() );
  textFormatElem.setAttribute( QStringLiteral( "reverseDirectionSymbol" ), mLineSettings.reverseDirectionSymbol() );
  textFormatElem.setAttribute( QStringLiteral( "placeDirectionSymbol" ), static_cast< unsigned int >( mLineSettings.directionSymbolPlacement() ) );
  textFormatElem.setAttribute( QStringLiteral( "formatNumbers" ), formatNumbers );
  textFormatElem.setAttribute( QStringLiteral( "decimals" ), decimals );
  textFormatElem.setAttribute( QStringLiteral( "plussign" ), plusSign );

  // placement
  QDomElement placementElem = doc.createElement( QStringLiteral( "placement" ) );
  placementElem.setAttribute( QStringLiteral( "placement" ), placement );
  placementElem.setAttribute( QStringLiteral( "polygonPlacementFlags" ), static_cast< int >( mPolygonPlacementFlags ) );
  placementElem.setAttribute( QStringLiteral( "placementFlags" ), static_cast< unsigned int >( mLineSettings.placementFlags() ) );
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
  placementElem.setAttribute( QStringLiteral( "rotationUnit" ), qgsEnumValueToKey( mRotationUnit ) );
  placementElem.setAttribute( QStringLiteral( "maxCurvedCharAngleIn" ), maxCurvedCharAngleIn );
  placementElem.setAttribute( QStringLiteral( "maxCurvedCharAngleOut" ), maxCurvedCharAngleOut );
  placementElem.setAttribute( QStringLiteral( "priority" ), priority );
  placementElem.setAttribute( QStringLiteral( "repeatDistance" ), repeatDistance );
  placementElem.setAttribute( QStringLiteral( "repeatDistanceUnits" ), QgsUnitTypes::encodeUnit( repeatDistanceUnit ) );
  placementElem.setAttribute( QStringLiteral( "repeatDistanceMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( repeatDistanceMapUnitScale ) );
  placementElem.setAttribute( QStringLiteral( "overrunDistance" ), mLineSettings.overrunDistance() );
  placementElem.setAttribute( QStringLiteral( "overrunDistanceUnit" ), QgsUnitTypes::encodeUnit( mLineSettings.overrunDistanceUnit() ) );
  placementElem.setAttribute( QStringLiteral( "overrunDistanceMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mLineSettings.overrunDistanceMapUnitScale() ) );
  placementElem.setAttribute( QStringLiteral( "lineAnchorPercent" ), mLineSettings.lineAnchorPercent() );
  placementElem.setAttribute( QStringLiteral( "lineAnchorType" ), static_cast< int >( mLineSettings.anchorType() ) );
  placementElem.setAttribute( QStringLiteral( "lineAnchorClipping" ), static_cast< int >( mLineSettings.anchorClipping() ) );
  placementElem.setAttribute( QStringLiteral( "lineAnchorTextPoint" ), qgsEnumValueToKey( mLineSettings.anchorTextPoint() ) );

  placementElem.setAttribute( QStringLiteral( "geometryGenerator" ), geometryGenerator );
  placementElem.setAttribute( QStringLiteral( "geometryGeneratorEnabled" ), geometryGeneratorEnabled );
  const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWkbTypes::GeometryType>() );
  placementElem.setAttribute( QStringLiteral( "geometryGeneratorType" ), metaEnum.valueToKey( geometryGeneratorType ) );

  placementElem.setAttribute( QStringLiteral( "layerType" ), metaEnum.valueToKey( layerType ) );

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
  renderingElem.setAttribute( QStringLiteral( "mergeLines" ), mLineSettings.mergeLines() );
  renderingElem.setAttribute( QStringLiteral( "minFeatureSize" ), mThinningSettings.minimumFeatureSize() );
  renderingElem.setAttribute( QStringLiteral( "limitNumLabels" ), mThinningSettings.limitNumberOfLabelsEnabled() );
  renderingElem.setAttribute( QStringLiteral( "maxNumLabels" ), mThinningSettings.maximumNumberLabels() );
  renderingElem.setAttribute( QStringLiteral( "obstacle" ), mObstacleSettings.isObstacle() );
  renderingElem.setAttribute( QStringLiteral( "obstacleFactor" ), mObstacleSettings.factor() );
  renderingElem.setAttribute( QStringLiteral( "obstacleType" ), static_cast< unsigned int >( mObstacleSettings.type() ) );
  renderingElem.setAttribute( QStringLiteral( "zIndex" ), zIndex );
  renderingElem.setAttribute( QStringLiteral( "unplacedVisibility" ), static_cast< int >( mUnplacedVisibility ) );

  QDomElement ddElem = doc.createElement( QStringLiteral( "dd_properties" ) );
  mDataDefinedProperties.writeXml( ddElem, *sPropertyDefinitions() );

  QDomElement elem = doc.createElement( QStringLiteral( "settings" ) );
  elem.appendChild( textStyleElem );
  elem.appendChild( textFormatElem );
  elem.appendChild( placementElem );
  elem.appendChild( renderingElem );
  elem.appendChild( ddElem );

  if ( mCallout )
  {
    elem.setAttribute( QStringLiteral( "calloutType" ), mCallout->type() );
    mCallout->saveProperties( doc, elem, context );
  }

  return elem;
}

void QgsPalLayerSettings::setCallout( QgsCallout *callout )
{
  mCallout.reset( callout );
}

QPixmap QgsPalLayerSettings::labelSettingsPreviewPixmap( const QgsPalLayerSettings &settings, QSize size, const QString &previewText, int padding )
{
  // for now, just use format
  QgsTextFormat tempFormat = settings.format();
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );

  painter.setRenderHint( QPainter::Antialiasing );

  QRect rect( 0, 0, size.width(), size.height() );

  // shameless eye candy - use a subtle gradient when drawing background
  painter.setPen( Qt::NoPen );
  QColor background1 = tempFormat.previewBackgroundColor();
  if ( ( background1.lightnessF() < 0.7 ) )
  {
    background1 = background1.darker( 125 );
  }
  else
  {
    background1 = background1.lighter( 125 );
  }
  QColor background2 = tempFormat.previewBackgroundColor();
  QLinearGradient linearGrad( QPointF( 0, 0 ), QPointF( 0, rect.height() ) );
  linearGrad.setColorAt( 0, background1 );
  linearGrad.setColorAt( 1, background2 );
  painter.setBrush( QBrush( linearGrad ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }
  painter.setBrush( Qt::NoBrush );
  painter.setPen( Qt::NoPen );
  padding += 1; // move text away from background border

  QgsRenderContext context;
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
  context.setMapToPixel( newCoordXForm );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  const double logicalDpiX = QgsApplication::desktop()->logicalDpiX();
#else
  QWidget *activeWindow = QApplication::activeWindow();
  const double logicalDpiX = activeWindow && activeWindow->screen() ? activeWindow->screen()->logicalDotsPerInchX() : 96.0;
#endif
  context.setScaleFactor( logicalDpiX / 25.4 );

  context.setUseAdvancedEffects( true );
  context.setPainter( &painter );

  // slightly inset text to account for buffer/background
  const double fontSize = context.convertToPainterUnits( tempFormat.size(), tempFormat.sizeUnit(), tempFormat.sizeMapUnitScale() );
  double xtrans = 0;
  if ( tempFormat.buffer().enabled() )
    xtrans = tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
             ? fontSize * tempFormat.buffer().size() / 100
             : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() );
  if ( tempFormat.background().enabled() && tempFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat.background().size().width(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  double ytrans = 0.0;
  if ( tempFormat.buffer().enabled() )
    ytrans = std::max( ytrans, tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                       ? fontSize * tempFormat.buffer().size() / 100
                       : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() ) );
  if ( tempFormat.background().enabled() )
    ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.background().size().height(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  const QStringList text = QStringList() << ( previewText.isEmpty() ? settings.legendString() : previewText );
  const double textHeight = QgsTextRenderer::textHeight( context, tempFormat, text, QgsTextRenderer::Rect );
  QRectF textRect = rect;
  textRect.setLeft( xtrans + padding );
  textRect.setWidth( rect.width() - xtrans - 2 * padding );

  if ( textRect.width() > 2000 )
    textRect.setWidth( 2000 - 2 * padding );

  const double bottom = textRect.height() / 2 + textHeight / 2;
  textRect.setTop( bottom - textHeight );
  textRect.setBottom( bottom );

  const double iconWidth = QFontMetricsF( QFont() ).horizontalAdvance( 'X' ) * Qgis::UI_SCALE_FACTOR;

  if ( settings.callout() && settings.callout()->enabled() )
  {
    // draw callout preview
    const double textWidth = QgsTextRenderer::textWidth( context, tempFormat, text );
    QgsCallout *callout = settings.callout();
    callout->startRender( context );
    QgsCallout::QgsCalloutContext calloutContext;
    QRectF labelRect( textRect.left() + ( textRect.width() - textWidth ) / 2.0, textRect.top(), textWidth, textRect.height() );
    callout->render( context, labelRect, 0, QgsGeometry::fromPointXY( QgsPointXY( labelRect.left() - iconWidth * 1.5, labelRect.bottom() + iconWidth ) ), calloutContext );
    callout->stopRender( context );
  }

  QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignCenter, text, context, tempFormat );

  if ( size.width() > 30 )
  {
    // draw a label icon

    QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ).paint( &painter, QRect(
          rect.width() - iconWidth * 3, rect.height() - iconWidth * 3,
          iconWidth * 2, iconWidth * 2 ), Qt::AlignRight | Qt::AlignBottom );
  }

  // draw border on top of text
  painter.setBrush( Qt::NoBrush );
  painter.setPen( QPen( tempFormat.previewBackgroundColor().darker( 150 ), 0 ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }

  painter.end();
  return pixmap;
}

Qgis::UnplacedLabelVisibility QgsPalLayerSettings::unplacedVisibility() const
{
  return mUnplacedVisibility;
}

void QgsPalLayerSettings::setUnplacedVisibility( Qgis::UnplacedLabelVisibility visibility )
{
  mUnplacedVisibility = visibility;
}

bool QgsPalLayerSettings::checkMinimumSizeMM( const QgsRenderContext &ct, const QgsGeometry &geom, double minSize ) const
{
  return QgsPalLabeling::checkMinimumSizeMM( ct, geom, minSize );
}

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF *fm, const QString &text, double &labelX, double &labelY, const QgsFeature *f, QgsRenderContext *context, double *rotatedLabelX, double *rotatedLabelY, QgsTextDocument *document )
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
  QgsTextFormat::TextOrientation orientation = mFormat.orientation();

  bool addDirSymb = mLineSettings.addDirectionSymbol();
  QString leftDirSymb = mLineSettings.leftDirectionSymbol();
  QString rightDirSymb = mLineSettings.rightDirectionSymbol();
  QgsLabelLineSettings::DirectionSymbolPlacement placeDirSymb = mLineSettings.directionSymbolPlacement();

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

    if ( dataDefinedValues.contains( QgsPalLayerSettings::TextOrientation ) )
    {
      orientation = QgsTextRendererUtils::decodeTextOrientation( dataDefinedValues.value( QgsPalLayerSettings::TextOrientation ).toString() );
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
        placeDirSymb = static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( dataDefinedValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt() );
      }

    }

  }
  else // called externally with passed-in feature, evaluate data defined
  {
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::MultiLineWrapChar ) )
    {
      rc->expressionContext().setOriginalValueVariable( wrapChar );
      wrapchr = mDataDefinedProperties.value( QgsPalLayerSettings::MultiLineWrapChar, rc->expressionContext(), wrapchr ).toString();
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::AutoWrapLength ) )
    {
      rc->expressionContext().setOriginalValueVariable( evalAutoWrapLength );
      evalAutoWrapLength = mDataDefinedProperties.value( QgsPalLayerSettings::AutoWrapLength, rc->expressionContext(), evalAutoWrapLength ).toInt();
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::MultiLineHeight ) )
    {
      rc->expressionContext().setOriginalValueVariable( multilineH );
      multilineH = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MultiLineHeight, rc->expressionContext(), multilineH );
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::TextOrientation ) )
    {
      QString encoded = QgsTextRendererUtils::encodeTextOrientation( orientation );
      rc->expressionContext().setOriginalValueVariable( encoded );
      orientation = QgsTextRendererUtils::decodeTextOrientation( mDataDefinedProperties.valueAsString( QgsPalLayerSettings::TextOrientation, rc->expressionContext(), encoded ) );
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::DirSymbDraw ) )
    {
      rc->expressionContext().setOriginalValueVariable( addDirSymb );
      addDirSymb = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::DirSymbDraw, rc->expressionContext(), addDirSymb );
    }

    if ( addDirSymb ) // don't do extra evaluations if not adding a direction symbol
    {
      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::DirSymbLeft ) )
      {
        rc->expressionContext().setOriginalValueVariable( leftDirSymb );
        leftDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbLeft, rc->expressionContext(), leftDirSymb ).toString();
      }

      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::DirSymbRight ) )
      {
        rc->expressionContext().setOriginalValueVariable( rightDirSymb );
        rightDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbRight, rc->expressionContext(), rightDirSymb ).toString();
      }

      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::DirSymbPlacement ) )
      {
        rc->expressionContext().setOriginalValueVariable( static_cast< int >( placeDirSymb ) );
        placeDirSymb = static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::DirSymbPlacement, rc->expressionContext(), static_cast< int >( placeDirSymb ) ) );
      }
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

    if ( fm->horizontalAdvance( rightDirSymb ) > fm->horizontalAdvance( dirSym ) )
      dirSym = rightDirSymb;

    switch ( placeDirSymb )
    {
      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight:
        textCopy.append( dirSym );
        break;

      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove:
      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow:
        textCopy.prepend( dirSym + QStringLiteral( "\n" ) );
        break;
    }
  }

  double w = 0.0, h = 0.0, rw = 0.0, rh = 0.0;
  double labelHeight = fm->ascent() + fm->descent(); // ignore +1 for baseline

  QStringList multiLineSplit;

  if ( document )
  {
    document->splitLines( wrapchr, evalAutoWrapLength, useMaxLineLengthForAutoWrap );
    multiLineSplit = document->toPlainText();
  }
  else
  {
    multiLineSplit = QgsPalLabeling::splitToLines( textCopy, wrapchr, evalAutoWrapLength, useMaxLineLengthForAutoWrap );
  }

  int lines = multiLineSplit.size();

  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      h += fm->height() + static_cast< double >( ( lines - 1 ) * labelHeight * multilineH );

      for ( const auto &line : multiLineSplit )
      {
        w = std::max( w, fm->horizontalAdvance( line ) );
      }
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    {
      double letterSpacing = mFormat.scaledFont( *context ).letterSpacing();
      double labelWidth = fm->maxWidth();
      w = labelWidth + ( lines - 1 ) * labelWidth * multilineH;

      int maxLineLength = 0;
      for ( const auto &line : multiLineSplit )
      {
        maxLineLength = std::max( maxLineLength, static_cast<int>( line.length() ) );
      }
      h = fm->ascent() * maxLineLength + ( maxLineLength - 1 ) * letterSpacing;
      break;
    }

    case QgsTextFormat::RotationBasedOrientation:
    {
      double widthHorizontal = 0.0;
      for ( const auto &line : multiLineSplit )
      {
        widthHorizontal = std::max( w, fm->horizontalAdvance( line ) );
      }

      double widthVertical = 0.0;
      double letterSpacing = mFormat.scaledFont( *context ).letterSpacing();
      double labelWidth = fm->maxWidth();
      widthVertical = labelWidth + ( lines - 1 ) * labelWidth * multilineH;

      double heightHorizontal = 0.0;
      heightHorizontal += fm->height() + static_cast< double >( ( lines - 1 ) * labelHeight * multilineH );

      double heightVertical = 0.0;
      int maxLineLength = 0;
      for ( const auto &line : multiLineSplit )
      {
        maxLineLength = std::max( maxLineLength, static_cast<int>( line.length() ) );
      }
      heightVertical = fm->ascent() * maxLineLength + ( maxLineLength - 1 ) * letterSpacing;

      w = widthHorizontal;
      rw = heightVertical;
      h = heightHorizontal;
      rh = widthVertical;
      break;
    }
  }

#if 0 // XXX strk
  QgsPointXY ptSize = xform->toMapCoordinatesF( w, h );
  labelX = std::fabs( ptSize.x() - ptZero.x() );
  labelY = std::fabs( ptSize.y() - ptZero.y() );
#else
  double uPP = xform->mapUnitsPerPixel();
  labelX = w * uPP;
  labelY = h * uPP;
  if ( rotatedLabelX && rotatedLabelY )
  {
    *rotatedLabelX = rw * uPP;
    *rotatedLabelY = rh * uPP;
  }
#endif
}

void QgsPalLayerSettings::registerFeature( const QgsFeature &f, QgsRenderContext &context )
{
  registerFeatureWithDetails( f, context, QgsGeometry(), nullptr );
}

std::unique_ptr<QgsLabelFeature> QgsPalLayerSettings::registerFeatureWithDetails( const QgsFeature &f, QgsRenderContext &context, QgsGeometry obstacleGeometry, const QgsSymbol *symbol )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful
  mCurFeat = &f;

  // data defined is obstacle? calculate this first, to avoid wasting time working with obstacles we don't require
  bool isObstacle = mObstacleSettings.isObstacle();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::IsObstacle ) )
    isObstacle = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::IsObstacle, context.expressionContext(), isObstacle ); // default to layer default

  if ( !drawLabels )
  {
    if ( isObstacle )
    {
      return registerObstacleFeature( f, context, obstacleGeometry );
    }
    else
    {
      return nullptr;
    }
  }

  QgsFeature feature = f;
  if ( geometryGeneratorEnabled )
  {
    const QgsGeometry geometry = mGeometryGeneratorExpression.evaluate( &context.expressionContext() ).value<QgsGeometry>();
    if ( mGeometryGeneratorExpression.hasEvalError() )
      QgsMessageLog::logMessage( mGeometryGeneratorExpression.evalErrorString(), QObject::tr( "Labeling" ) );

    if ( obstacleGeometry.isNull() )
    {
      // if an explicit obstacle geometry hasn't been set, we must always use the original feature geometry
      // as the obstacle -- because we want to use the geometry which was used to render the symbology
      // for the feature as the obstacle for other layers' labels, NOT the generated geometry which is used
      // only to place labels for this layer.
      obstacleGeometry = f.geometry();
    }

    feature.setGeometry( geometry );
  }

  // store data defined-derived values for later adding to label feature for use during rendering
  dataDefinedValues.clear();

  // data defined show label? defaults to show label if not set
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Show ) )
  {
    context.expressionContext().setOriginalValueVariable( true );
    if ( !mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Show, context.expressionContext(), true ) )
    {
      return nullptr;
    }
  }

  // data defined scale visibility?
  bool useScaleVisibility = scaleVisibility;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ScaleVisibility ) )
    useScaleVisibility = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::ScaleVisibility, context.expressionContext(), scaleVisibility );

  if ( useScaleVisibility )
  {
    // data defined min scale?
    double maxScale = maximumScale;
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::MaximumScale ) )
    {
      context.expressionContext().setOriginalValueVariable( maximumScale );
      maxScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MaximumScale, context.expressionContext(), maxScale );
    }

    // scales closer than 1:1
    if ( maxScale < 0 )
    {
      maxScale = 1 / std::fabs( maxScale );
    }

    if ( !qgsDoubleNear( maxScale, 0.0 ) && context.rendererScale() < maxScale )
    {
      return nullptr;
    }

    // data defined min scale?
    double minScale = minimumScale;
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::MinimumScale ) )
    {
      context.expressionContext().setOriginalValueVariable( minimumScale );
      minScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::MinimumScale, context.expressionContext(), minScale );
    }

    // scales closer than 1:1
    if ( minScale < 0 )
    {
      minScale = 1 / std::fabs( minScale );
    }

    if ( !qgsDoubleNear( minScale, 0.0 ) && context.rendererScale() > minScale )
    {
      return nullptr;
    }
  }

  QFont labelFont = mFormat.font();
  // labelFont will be added to label feature for use during label painting

  // data defined font units?
  QgsUnitTypes::RenderUnit fontunits = mFormat.sizeUnit();
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontSizeUnit, context.expressionContext() );
  if ( !exprVal.isNull() )
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
  double fontSize = mFormat.size();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Size ) )
  {
    context.expressionContext().setOriginalValueVariable( fontSize );
    fontSize = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Size, context.expressionContext(), fontSize );
  }
  if ( fontSize <= 0.0 )
  {
    return nullptr;
  }

  int fontPixelSize = QgsTextRenderer::sizeToPixel( fontSize, context, fontunits, mFormat.sizeMapUnitScale() );
  // don't try to show font sizes less than 1 pixel (Qt complains)
  if ( fontPixelSize < 1 )
  {
    return nullptr;
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
        return nullptr;
      }
    }
  }

  // NOTE: the following parsing functions calculate and store any data defined values for later use in QgsPalLabeling::drawLabeling
  // this is done to provide clarity, and because such parsing is not directly related to PAL feature registration calculations

  // calculate rest of font attributes and store any data defined values
  // this is done here for later use in making label backgrounds part of collision management (when implemented)
  labelFont.setCapitalization( QFont::MixedCase ); // reset this - we don't use QFont's handling as it breaks with curved labels

  parseTextStyle( labelFont, fontunits, context );
  if ( mDataDefinedProperties.hasActiveProperties() )
  {
    parseTextFormatting( context );
    parseTextBuffer( context );
    parseTextMask( context );
    parseShapeBackground( context );
    parseDropShadow( context );
  }

  QString labelText;

  // Check to see if we are a expression string.
  if ( isExpression )
  {
    QgsExpression *exp = getLabelExpression();
    if ( exp->hasParserError() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Expression parser error:%1" ).arg( exp->parserErrorString() ), 4 );
      return nullptr;
    }

    QVariant result = exp->evaluate( &context.expressionContext() ); // expression prepared in QgsPalLabeling::prepareLayer()
    if ( exp->hasEvalError() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Expression parser eval error:%1" ).arg( exp->evalErrorString() ), 4 );
      return nullptr;
    }
    labelText = result.isNull() ? QString() : result.toString();
  }
  else
  {
    const QVariant &v = feature.attribute( fieldIndex );
    labelText = v.isNull() ? QString() : v.toString();
  }

  // apply text replacements
  if ( useSubstitutions )
  {
    labelText = substitutions.process( labelText );
  }

  // apply capitalization
  Qgis::Capitalization capitalization = mFormat.capitalization();
  // maintain API - capitalization may have been set in textFont
  if ( capitalization == Qgis::Capitalization::MixedCase && mFormat.font().capitalization() != QFont::MixedCase )
  {
    capitalization = static_cast< Qgis::Capitalization >( mFormat.font().capitalization() );
  }
  // data defined font capitalization?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::FontCase ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontCase, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString fcase = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal FontCase:%1" ).arg( fcase ), 4 );

      if ( !fcase.isEmpty() )
      {
        if ( fcase.compare( QLatin1String( "NoChange" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::MixedCase;
        }
        else if ( fcase.compare( QLatin1String( "Upper" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::AllUppercase;
        }
        else if ( fcase.compare( QLatin1String( "Lower" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::AllLowercase;
        }
        else if ( fcase.compare( QLatin1String( "Capitalize" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::ForceFirstLetterToCapital;
        }
        else if ( fcase.compare( QLatin1String( "Title" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::TitleCase;
        }
#if defined(HAS_KDE_QT5_SMALL_CAPS_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
        else if ( fcase.compare( QLatin1String( "SmallCaps" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::SmallCaps;
        }
        else if ( fcase.compare( QLatin1String( "AllSmallCaps" ), Qt::CaseInsensitive ) == 0 )
        {
          capitalization = Qgis::Capitalization::AllSmallCaps;
        }
#endif
      }
    }
  }
  labelText = QgsStringUtils::capitalize( labelText, capitalization );

  // format number if label text is coercible to a number
  bool evalFormatNumbers = formatNumbers;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::NumFormat ) )
  {
    evalFormatNumbers = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::NumFormat, context.expressionContext(), evalFormatNumbers );
  }
  if ( evalFormatNumbers )
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
  double labelX, labelY, rotatedLabelX, rotatedLabelY; // will receive label size

  QgsTextDocument doc;
  if ( format().allowHtmlFormatting() )
    doc = QgsTextDocument::fromHtml( QStringList() << labelText );

  // also applies the line split to doc!
  calculateLabelSize( labelFontMetrics.get(), labelText, labelX, labelY, mCurFeat, &context, &rotatedLabelX, &rotatedLabelY, format().allowHtmlFormatting() ? &doc : nullptr );

  // maximum angle between curved label characters (hardcoded defaults used in QGIS <2.0)
  //
  double maxcharanglein = 20.0; // range 20.0-60.0
  double maxcharangleout = -20.0; // range 20.0-95.0

  if ( placement == QgsPalLayerSettings::Curved || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    maxcharanglein = maxCurvedCharAngleIn;
    maxcharangleout = maxCurvedCharAngleOut;

    //data defined maximum angle between curved label characters?
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::CurvedCharAngleInOut ) )
    {
      exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::CurvedCharAngleInOut, context.expressionContext() );
      bool ok = false;
      const QPointF maxcharanglePt = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
      if ( ok )
      {
        maxcharanglein = std::clamp( static_cast< double >( maxcharanglePt.x() ), 20.0, 60.0 );
        maxcharangleout = std::clamp( static_cast< double >( maxcharanglePt.y() ), 20.0, 95.0 );
      }
    }
    // make sure maxcharangleout is always negative
    maxcharangleout = -( std::fabs( maxcharangleout ) );
  }

  // data defined centroid whole or clipped?
  bool wholeCentroid = centroidWhole;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::CentroidWhole ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::CentroidWhole, context.expressionContext() );
    if ( !exprVal.isNull() )
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
  }

  QgsGeometry geom = feature.geometry();
  if ( geom.isNull() )
  {
    return nullptr;
  }

  // simplify?
  const QgsVectorSimplifyMethod &simplifyMethod = context.vectorSimplifyMethod();
  std::unique_ptr<QgsGeometry> scopedClonedGeom;
  if ( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() )
  {
    unsigned int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( simplifyMethod.simplifyAlgorithm() );
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  if ( !context.featureClipGeometry().isEmpty() )
  {
    const QgsWkbTypes::GeometryType expectedType = geom.type();
    geom = geom.intersection( context.featureClipGeometry() );
    geom.convertGeometryCollectionToSubclass( expectedType );
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


  QgsLabeling::PolygonPlacementFlags polygonPlacement = mPolygonPlacementFlags;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::PolygonLabelOutside ) )
  {
    const QVariant dataDefinedOutside = mDataDefinedProperties.value( QgsPalLayerSettings::PolygonLabelOutside, context.expressionContext() );
    if ( !dataDefinedOutside.isNull() )
    {
      if ( dataDefinedOutside.type() == QVariant::String )
      {
        const QString value = dataDefinedOutside.toString().trimmed();
        if ( value.compare( QLatin1String( "force" ), Qt::CaseInsensitive ) == 0 )
        {
          // forced outside placement -- remove inside flag, add outside flag
          polygonPlacement &= ~static_cast< int >( QgsLabeling::PolygonPlacementFlag::AllowPlacementInsideOfPolygon );
          polygonPlacement |= QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else if ( value.compare( QLatin1String( "yes" ), Qt::CaseInsensitive ) == 0 )
        {
          // permit outside placement
          polygonPlacement |= QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else if ( value.compare( QLatin1String( "no" ), Qt::CaseInsensitive ) == 0 )
        {
          // block outside placement
          polygonPlacement &= ~static_cast< int >( QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon );
        }
      }
      else
      {
        if ( dataDefinedOutside.toBool() )
        {
          // permit outside placement
          polygonPlacement |= QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else
        {
          // block outside placement
          polygonPlacement &= ~static_cast< int >( QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon );
        }
      }
    }
  }

  QgsLabelLineSettings lineSettings = mLineSettings;
  lineSettings.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );

  if ( geom.type() == QgsWkbTypes::LineGeometry )
  {
    switch ( lineSettings.anchorClipping() )
    {
      case QgsLabelLineSettings::AnchorClipping::UseVisiblePartsOfLine:
        break;

      case QgsLabelLineSettings::AnchorClipping::UseEntireLine:
        doClip = false;
        break;
    }
  }

  // if using fitInPolygonOnly option, generate the permissible zone (must happen before geometry is modified - e.g.,
  // as a result of using perimeter based labeling and the geometry is converted to a boundary)
  // note that we also force this if we are permitting labels to be placed outside of polygons too!
  QgsGeometry permissibleZone;
  if ( geom.type() == QgsWkbTypes::PolygonGeometry && ( fitInPolygonOnly || polygonPlacement & QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon ) )
  {
    permissibleZone = geom;
    if ( QgsPalLabeling::geometryRequiresPreparation( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() ) )
    {
      permissibleZone = QgsPalLabeling::prepareGeometry( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() );
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
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() );

    if ( geom.isEmpty() )
      return nullptr;
  }
  geos_geom_clone = QgsGeos::asGeos( geom );

  if ( isObstacle || ( geom.type() == QgsWkbTypes::PointGeometry && offsetType == FromSymbolBounds ) )
  {
    if ( !obstacleGeometry.isNull() && QgsPalLabeling::geometryRequiresPreparation( obstacleGeometry, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() ) )
    {
      obstacleGeometry = QgsGeometry( QgsPalLabeling::prepareGeometry( obstacleGeometry, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() ) );
    }
  }

  QgsLabelThinningSettings featureThinningSettings = mThinningSettings;
  featureThinningSettings.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );

  double minimumSize = 0.0;
  if ( featureThinningSettings.minimumFeatureSize() > 0 )
  {
    // for minimum feature size on merged lines, we need to delay the filtering after the merging occurred in PAL
    if ( geom.type() == QgsWkbTypes::LineGeometry && mLineSettings.mergeLines() )
    {
      minimumSize = context.convertToMapUnits( featureThinningSettings.minimumFeatureSize(), QgsUnitTypes::RenderMillimeters );
    }
    else
    {
      if ( !checkMinimumSizeMM( context, geom, featureThinningSettings.minimumFeatureSize() ) )
        return nullptr;
    }
  }

  if ( !geos_geom_clone )
    return nullptr; // invalid geometry

  // likelihood exists label will be registered with PAL and may be drawn
  // check if max number of features to label (already registered with PAL) has been reached
  // Debug output at end of QgsPalLabeling::drawLabeling(), when deleting temp geometries
  if ( featureThinningSettings.limitNumberOfLabelsEnabled() )
  {
    if ( !featureThinningSettings.maximumNumberLabels() )
    {
      return nullptr;
    }
    if ( mFeatsRegPal >= featureThinningSettings.maximumNumberLabels() )
    {
      return nullptr;
    }

    int divNum = static_cast< int >( ( static_cast< double >( mFeaturesToLabel ) / featureThinningSettings.maximumNumberLabels() ) + 0.5 ); // NOLINT
    if ( divNum && ( mFeatsRegPal == static_cast< int >( mFeatsSendingToPal / divNum ) ) )
    {
      mFeatsSendingToPal += 1;
      if ( divNum &&  mFeatsSendingToPal % divNum )
      {
        return nullptr;
      }
    }
  }

  //data defined position / alignment / rotation?
  bool layerDefinedRotation = false;
  bool dataDefinedRotation = false;
  double xPos = 0.0, yPos = 0.0, angle = 0.0;
  double quadOffsetX = 0.0, quadOffsetY = 0.0;
  double offsetX = 0.0, offsetY = 0.0;
  QgsPointXY anchorPosition;

  if ( placement == QgsPalLayerSettings::OverPoint )
  {
    anchorPosition = geom.centroid().asPoint();
  }
  //x/y shift in case of alignment
  double xdiff = 0.0;
  double ydiff = 0.0;

  //data defined quadrant offset?
  bool ddFixedQuad = false;
  QuadrantPosition quadOff = quadOffset;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::OffsetQuad ) )
  {
    context.expressionContext().setOriginalValueVariable( static_cast< int >( quadOff ) );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetQuad, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      bool ok;
      int quadInt = exprVal.toInt( &ok );
      if ( ok && 0 <= quadInt && quadInt <= 8 )
      {
        quadOff = static_cast< QuadrantPosition >( quadInt );
        ddFixedQuad = true;
      }
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
      break;
  }

  //data defined label offset?
  double xOff = xOffset;
  double yOff = yOffset;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::OffsetXY ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( QPointF( xOffset, yOffset ) ) );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetXY, context.expressionContext() );
    bool ok = false;
    const QPointF ddOffPt = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      xOff = ddOffPt.x();
      yOff = ddOffPt.y();
    }
  }

  // data defined label offset units?
  QgsUnitTypes::RenderUnit offUnit = offsetUnits;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::OffsetUnits ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::OffsetUnits, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString units = exprVal.toString().trimmed();
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
  }

  // adjust offset of labels to match chosen unit and map scale
  // offsets match those of symbology: -x = left, -y = up
  offsetX = context.convertToMapUnits( xOff, offUnit, labelOffsetMapUnitScale );
  // must be negative to match symbology offset direction
  offsetY = context.convertToMapUnits( -yOff, offUnit, labelOffsetMapUnitScale );

  // layer defined rotation?
  if ( !qgsDoubleNear( angleOffset, 0.0 ) )
  {
    layerDefinedRotation = true;
    angle = ( 360 - angleOffset ) * M_PI / 180; // convert to radians counterclockwise
  }

  const QgsMapToPixel &m2p = context.mapToPixel();
  //data defined rotation?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::LabelRotation ) )
  {
    context.expressionContext().setOriginalValueVariable( angleOffset );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::LabelRotation, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      bool ok;
      const double rotation = exprVal.toDouble( &ok );
      if ( ok )
      {
        dataDefinedRotation = true;

        double rotationDegrees = rotation * QgsUnitTypes::fromUnitToUnitFactor( mRotationUnit,
                                 QgsUnitTypes::AngleDegrees );

        // TODO: add setting to disable having data defined rotation follow
        //       map rotation ?
        rotationDegrees += m2p.mapRotation();
        angle = ( 360 - rotationDegrees ) * M_PI / 180.0;
      }
    }
  }

  bool hasDataDefinedPosition = false;
  {
    bool ddPosition = false;

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::PositionX )
         && mDataDefinedProperties.isActive( QgsPalLayerSettings::PositionY ) )
    {
      const QVariant xPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::PositionX, context.expressionContext() );
      const QVariant yPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::PositionY, context.expressionContext() );
      if ( !xPosProperty.isNull()
           && !yPosProperty.isNull() )
      {
        ddPosition = true;

        bool ddXPos = false, ddYPos = false;
        xPos = xPosProperty.toDouble( &ddXPos );
        yPos = yPosProperty.toDouble( &ddYPos );
        if ( ddXPos && ddYPos )
          hasDataDefinedPosition = true;
      }
    }
    else if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::PositionPoint ) )
    {
      const QVariant pointPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::PositionPoint, context.expressionContext() );
      if ( !pointPosProperty.isNull() )
      {
        ddPosition = true;

        QgsPoint point;
        if ( pointPosProperty.canConvert<QgsReferencedGeometry>() )
        {
          QgsReferencedGeometry referencedGeometryPoint = pointPosProperty.value<QgsReferencedGeometry>();
          point = QgsPoint( referencedGeometryPoint.asPoint() );

          if ( !referencedGeometryPoint.isNull()
               && ct.sourceCrs() != referencedGeometryPoint.crs() )
            QgsMessageLog::logMessage( QObject::tr( "Label position geometry is not in layer coordinates reference system. Layer CRS: '%1', Geometry CRS: '%2'" ).arg( ct.sourceCrs().userFriendlyIdentifier(), referencedGeometryPoint.crs().userFriendlyIdentifier() ), QObject::tr( "Labeling" ), Qgis::Warning );
        }
        else if ( pointPosProperty.canConvert<QgsGeometry>() )
        {
          point = QgsPoint( pointPosProperty.value<QgsGeometry>().asPoint() );
        }

        if ( !point.isEmpty() )
        {
          hasDataDefinedPosition = true;

          xPos = point.x();
          yPos = point.y();
        }
      }
    }

    if ( ddPosition )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      if ( hasDataDefinedPosition )
      {
        // layer rotation set, but don't rotate pinned labels unless data defined
        if ( layerDefinedRotation && !dataDefinedRotation )
        {
          angle = 0.0;
        }

        //horizontal alignment
        if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Hali ) )
        {
          exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Hali, context.expressionContext() );
          if ( !exprVal.isNull() )
          {
            QString haliString = exprVal.toString();
            if ( haliString.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
            {
              xdiff -= labelX / 2.0;
            }
            else if ( haliString.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
            {
              xdiff -= labelX;
            }
          }
        }

        //vertical alignment
        if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Vali ) )
        {
          exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Vali, context.expressionContext() );
          if ( !exprVal.isNull() )
          {
            QString valiString = exprVal.toString();
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
          if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( ddPoint.constGet() ) )
          {
            xPos = point->x();
            yPos = point->y();
            anchorPosition = QgsPointXY( xPos, yPos );
          }
          else
          {
            QgsMessageLog::logMessage( QObject::tr( "Invalid data defined label position (%1, %2)" ).arg( xPos ).arg( yPos ), QObject::tr( "Labeling" ) );
            hasDataDefinedPosition = false;
          }
        }
        else
        {
          anchorPosition = QgsPointXY( xPos, yPos );
        }

        xPos += xdiff;
        yPos += ydiff;
      }
      else
      {
        anchorPosition = QgsPointXY( xPos, yPos );
      }
    }
  }

  // data defined always show?
  bool alwaysShow = false;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::AlwaysShow ) )
  {
    alwaysShow = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::AlwaysShow, context.expressionContext(), false );
  }

  // set repeat distance
  // data defined repeat distance?
  double repeatDist = repeatDistance;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::RepeatDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( repeatDist );
    repeatDist = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::RepeatDistance, context.expressionContext(), repeatDist );
  }

  // data defined label-repeat distance units?
  QgsUnitTypes::RenderUnit repeatUnits = repeatDistanceUnit;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::RepeatDistanceUnit ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::RepeatDistanceUnit, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString units = exprVal.toString().trimmed();
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
  }

  if ( !qgsDoubleNear( repeatDist, 0.0 ) )
  {
    if ( repeatUnits != QgsUnitTypes::RenderMapUnits )
    {
      repeatDist = context.convertToMapUnits( repeatDist, repeatUnits, repeatDistanceMapUnitScale );
    }
  }

  //  overrun distance
  double overrunDistanceEval = lineSettings.overrunDistance();
  if ( !qgsDoubleNear( overrunDistanceEval, 0.0 ) )
  {
    overrunDistanceEval = context.convertToMapUnits( overrunDistanceEval, lineSettings.overrunDistanceUnit(), lineSettings.overrunDistanceMapUnitScale() );
  }

  // we smooth out the overrun label extensions by 1 mm, to avoid little jaggies right at the start or end of the lines
  // causing the overrun extension to extend out in an undesirable direction. This is hard coded, we don't want to overload
  // users with options they likely don't need to see...
  const double overrunSmoothDist = context.convertToMapUnits( 1, QgsUnitTypes::RenderMillimeters );

  bool labelAll = labelPerPart && !hasDataDefinedPosition;
  if ( !hasDataDefinedPosition )
  {
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::LabelAllParts ) )
    {
      context.expressionContext().setOriginalValueVariable( labelPerPart );
      labelAll = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::LabelAllParts, context.expressionContext(), labelPerPart );
    }
  }

  //  feature to the layer
  std::unique_ptr< QgsTextLabelFeature > labelFeature = std::make_unique< QgsTextLabelFeature>( feature.id(), std::move( geos_geom_clone ), QSizeF( labelX, labelY ) );
  labelFeature->setAnchorPosition( anchorPosition );
  labelFeature->setFeature( feature );
  labelFeature->setSymbol( symbol );
  labelFeature->setDocument( doc );
  if ( !qgsDoubleNear( rotatedLabelX, 0.0 ) && !qgsDoubleNear( rotatedLabelY, 0.0 ) )
    labelFeature->setRotatedSize( QSizeF( rotatedLabelX, rotatedLabelY ) );
  mFeatsRegPal++;

  labelFeature->setHasFixedPosition( hasDataDefinedPosition );
  labelFeature->setFixedPosition( QgsPointXY( xPos, yPos ) );
  // use layer-level defined rotation, but not if position fixed
  labelFeature->setHasFixedAngle( dataDefinedRotation || ( !hasDataDefinedPosition && !qgsDoubleNear( angle, 0.0 ) ) );
  labelFeature->setFixedAngle( angle );
  labelFeature->setQuadOffset( QPointF( quadOffsetX, quadOffsetY ) );
  labelFeature->setPositionOffset( QgsPointXY( offsetX, offsetY ) );
  labelFeature->setOffsetType( offsetType );
  labelFeature->setAlwaysShow( alwaysShow );
  labelFeature->setRepeatDistance( repeatDist );
  labelFeature->setLabelText( labelText );
  labelFeature->setPermissibleZone( permissibleZone );
  labelFeature->setOverrunDistance( overrunDistanceEval );
  labelFeature->setOverrunSmoothDistance( overrunSmoothDist );
  labelFeature->setLineAnchorPercent( lineSettings.lineAnchorPercent() );
  labelFeature->setLineAnchorType( lineSettings.anchorType() );
  labelFeature->setLineAnchorTextPoint( lineSettings.anchorTextPoint() );
  labelFeature->setLabelAllParts( labelAll );
  labelFeature->setOriginalFeatureCrs( context.coordinateTransform().sourceCrs() );
  labelFeature->setMinimumSize( minimumSize );
  if ( geom.type() == QgsWkbTypes::PointGeometry && !obstacleGeometry.isNull() )
  {
    //register symbol size
    labelFeature->setSymbolSize( QSizeF( obstacleGeometry.boundingBox().width(),
                                         obstacleGeometry.boundingBox().height() ) );
  }

  //set label's visual margin so that top visual margin is the leading, and bottom margin is the font's descent
  //this makes labels align to the font's baseline or highest character
  double topMargin = std::max( 0.25 * labelFontMetrics->ascent(), 0.0 );
  double bottomMargin = 1.0 + labelFontMetrics->descent();
  QgsMargins vm( 0.0, topMargin, 0.0, bottomMargin );
  vm *= xform->mapUnitsPerPixel();
  labelFeature->setVisualMargin( vm );

  // store the label's calculated font for later use during painting
  QgsDebugMsgLevel( QStringLiteral( "PAL font stored definedFont: %1, Style: %2" ).arg( labelFont.toString(), labelFont.styleName() ), 4 );
  labelFeature->setDefinedFont( labelFont );
  labelFeature->setFontMetrics( *labelFontMetrics );

  labelFeature->setMaximumCharacterAngleInside( std::clamp( maxcharanglein, 20.0, 60.0 ) * M_PI / 180 );
  labelFeature->setMaximumCharacterAngleOutside( std::clamp( maxcharangleout, -95.0, -20.0 ) * M_PI / 180 );
  switch ( placement )
  {
    case QgsPalLayerSettings::AroundPoint:
    case QgsPalLayerSettings::OverPoint:
    case QgsPalLayerSettings::Line:
    case QgsPalLayerSettings::Horizontal:
    case QgsPalLayerSettings::Free:
    case QgsPalLayerSettings::OrderedPositionsAroundPoint:
    case QgsPalLayerSettings::OutsidePolygons:
      // these placements don't require text metrics
      break;

    case QgsPalLayerSettings::Curved:
    case QgsPalLayerSettings::PerimeterCurved:
      labelFeature->setTextMetrics( QgsTextLabelFeature::calculateTextMetrics( xform, *labelFontMetrics, labelFont.letterSpacing(), labelFont.wordSpacing(), labelText, format().allowHtmlFormatting() ? &doc : nullptr ) );
      break;
  }

  // for labelFeature the LabelInfo is passed to feat when it is registered

  // TODO: allow layer-wide feature dist in PAL...?

  // data defined label-feature distance?
  double distance = dist;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::LabelDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( distance );
    distance = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::LabelDistance, context.expressionContext(), distance );
  }

  // data defined label-feature distance units?
  QgsUnitTypes::RenderUnit distUnit = distUnits;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::DistanceUnits ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::DistanceUnits, context.expressionContext() );
    if ( !exprVal.isNull() )
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
  }
  distance = context.convertToPainterUnits( distance, distUnit, distMapUnitScale );

  // when using certain placement modes, we force a tiny minimum distance. This ensures that
  // candidates are created just offset from a border and avoids candidates being incorrectly flagged as colliding with neighbours
  if ( placement == QgsPalLayerSettings::Line
       || placement == QgsPalLayerSettings::Curved
       || placement == QgsPalLayerSettings::PerimeterCurved )
  {
    distance = ( distance < 0 ? -1 : 1 ) * std::max( std::fabs( distance ), 1.0 );
  }
  else if ( placement == QgsPalLayerSettings::OutsidePolygons
            || ( ( placement == QgsPalLayerSettings::Horizontal
                   || placement == QgsPalLayerSettings::AroundPoint
                   || placement == QgsPalLayerSettings::OverPoint ||
                   placement == QgsPalLayerSettings::Free ) && polygonPlacement & QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon ) )
  {
    distance = std::max( distance, 2.0 );
  }

  if ( !qgsDoubleNear( distance, 0.0 ) )
  {
    double d = ptOne.distance( ptZero ) * distance;
    labelFeature->setDistLabel( d );
  }

  if ( ddFixedQuad )
  {
    labelFeature->setHasFixedQuadrant( true );
  }

  labelFeature->setArrangementFlags( lineSettings.placementFlags() );

  labelFeature->setPolygonPlacementFlags( polygonPlacement );

  // data defined z-index?
  double z = zIndex;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ZIndex ) )
  {
    context.expressionContext().setOriginalValueVariable( z );
    z = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::ZIndex, context.expressionContext(), z );
  }
  labelFeature->setZIndex( z );

  // data defined priority?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Priority ) )
  {
    context.expressionContext().setOriginalValueVariable( priority );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Priority, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      bool ok;
      double priorityD = exprVal.toDouble( &ok );
      if ( ok )
      {
        priorityD = std::clamp( priorityD, 0.0, 10.0 );
        priorityD = 1 - priorityD / 10.0; // convert 0..10 --> 1..0
        labelFeature->setPriority( priorityD );
      }
    }
  }

  QgsLabelObstacleSettings os = mObstacleSettings;
  os.setIsObstacle( isObstacle );
  os.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );
  os.setObstacleGeometry( obstacleGeometry );
  labelFeature->setObstacleSettings( os );

  QVector< QgsPalLayerSettings::PredefinedPointPosition > positionOrder = predefinedPositionOrder;
  if ( positionOrder.isEmpty() )
    positionOrder = *DEFAULT_PLACEMENT_ORDER();

  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::PredefinedPositionOrder ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsLabelingUtils::encodePredefinedPositionOrder( predefinedPositionOrder ) );
    QString dataDefinedOrder = mDataDefinedProperties.valueAsString( QgsPalLayerSettings::PredefinedPositionOrder, context.expressionContext() );
    if ( !dataDefinedOrder.isEmpty() )
    {
      positionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( dataDefinedOrder );
    }
  }
  labelFeature->setPredefinedPositionOrder( positionOrder );

  // add parameters for data defined labeling to label feature
  labelFeature->setDataDefinedValues( dataDefinedValues );

  return labelFeature;
}

std::unique_ptr<QgsLabelFeature> QgsPalLayerSettings::registerObstacleFeature( const QgsFeature &f, QgsRenderContext &context, const QgsGeometry &obstacleGeometry )
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
    return nullptr;
  }

  // don't even try to register linestrings with only one vertex as an obstacle
  if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( geom.constGet() ) )
  {
    if ( ls->numPoints() < 2 )
      return nullptr;
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

  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, ct, extentGeom, mLineSettings.mergeLines() ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, ct, extentGeom, mLineSettings.mergeLines() );
  }
  geos_geom_clone = QgsGeos::asGeos( geom );

  if ( !geos_geom_clone )
    return nullptr; // invalid geometry

  //  feature to the layer
  std::unique_ptr< QgsLabelFeature > obstacleFeature = std::make_unique< QgsLabelFeature >( f.id(), std::move( geos_geom_clone ), QSizeF( 0, 0 ) );
  obstacleFeature->setFeature( f );

  QgsLabelObstacleSettings os = mObstacleSettings;
  os.setIsObstacle( true );
  os.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );
  obstacleFeature->setObstacleSettings( os );

  mFeatsRegPal++;
  return obstacleFeature;
}

bool QgsPalLayerSettings::dataDefinedValEval( DataDefinedValueType valType,
    QgsPalLayerSettings::Property p,
    QVariant &exprVal, QgsExpressionContext &context, const QVariant &originalValue )
{
  if ( !mDataDefinedProperties.isActive( p ) )
    return false;

  context.setOriginalValueVariable( originalValue );
  exprVal = mDataDefinedProperties.value( p, context );
  if ( !exprVal.isNull() )
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
        int size = exprVal.toInt( &ok );
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
          dataDefinedValues.insert( p, QVariant( static_cast< int >( QgsSymbolLayerUtils::decodePenJoinStyle( joinstr ) ) ) );
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
        bool ok = false;
        const QPointF res = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
        if ( ok )
        {
          dataDefinedValues.insert( p, res );
          return true;
        }
        return false;
      }
      case DDSizeF:
      {
        bool ok = false;
        const QSizeF res = QgsSymbolLayerUtils::toSize( exprVal, &ok );
        if ( ok )
        {
          dataDefinedValues.insert( p, res );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Family ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.family() );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Family, context.expressionContext() );
    if ( !exprVal.isNull() )
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
  }

  // data defined named font style?
  QString ddFontStyle;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::FontStyle ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::FontStyle, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString fontstyle = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal Font style:%1" ).arg( fontstyle ), 4 );
      ddFontStyle = fontstyle;
    }
  }

  // data defined bold font style?
  bool ddBold = false;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Bold ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.bold() );
    ddBold = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Bold, context.expressionContext(), false );
  }

  // data defined italic font style?
  bool ddItalic = false;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Italic ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.italic() );
    ddItalic = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Italic, context.expressionContext(), false );
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
            && ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
  {
    if ( !ddFontFamily.isEmpty() )
    {
      // both family and style are different, build font from database
      if ( !mFontDB )
        mFontDB = std::make_unique< QFontDatabase >();

      QFont styledfont = mFontDB->font( ddFontFamily, ddFontStyle, appFont.pointSize() );
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
      if ( !mFontDB )
        mFontDB = std::make_unique< QFontDatabase >();
      QFont styledfont = mFontDB->font( ddFontFamily, mFormat.namedStyle(), appFont.pointSize() );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::FontWordSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( wordspace );
    wordspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::FontWordSpacing, context.expressionContext(), wordspace );
  }
  labelFont.setWordSpacing( context.convertToPainterUnits( wordspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined letter spacing?
  double letterspace = labelFont.letterSpacing();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::FontLetterSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( letterspace );
    letterspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::FontLetterSpacing, context.expressionContext(), letterspace );
  }
  labelFont.setLetterSpacing( QFont::AbsoluteSpacing, context.convertToPainterUnits( letterspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined strikeout font style?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Strikeout ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.strikeOut() );
    bool strikeout = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Strikeout, context.expressionContext(), false );
    labelFont.setStrikeOut( strikeout );
  }

  // data defined stretch
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::FontStretchFactor ) )
  {
    context.expressionContext().setOriginalValueVariable( mFormat.stretchFactor() );
    labelFont.setStretch( mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::FontStretchFactor, context.expressionContext(),  mFormat.stretchFactor() ) );
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
  else if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::BufferDraw ) && exprVal.isNull() )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::BufferDraw, QVariant( drawBuffer ) );
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

void QgsPalLayerSettings::parseTextMask( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextMaskSettings mask = mFormat.mask();

  // data defined enabled mask?
  bool maskEnabled = mask.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::MaskEnabled, exprVal, context.expressionContext(), mask.enabled() ) )
  {
    maskEnabled = exprVal.toBool();
  }

  if ( !maskEnabled )
  {
    return;
  }

  // data defined buffer size?
  double bufrSize = mask.size();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::MaskBufferSize, exprVal, context.expressionContext(), mask.size() ) )
  {
    bufrSize = exprVal.toDouble();
  }

  // data defined opacity?
  double opacity = mask.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::MaskOpacity, exprVal, context.expressionContext(), opacity ) )
  {
    opacity = exprVal.toDouble();
  }

  maskEnabled = ( maskEnabled && bufrSize > 0.0 && opacity > 0 );

  if ( !maskEnabled )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::MaskEnabled, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::MaskBufferSize );
    dataDefinedValues.remove( QgsPalLayerSettings::MaskOpacity );
    return; // don't bother evaluating values that won't be used
  }

  // data defined buffer units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::MaskBufferUnit, exprVal, context.expressionContext() );

  // data defined buffer pen join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::MaskJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( mask.joinStyle() ) );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::MultiLineAlignment ) )
  {
    context.expressionContext().setOriginalValueVariable( mFormat.lineHeight() );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::MultiLineAlignment, context.expressionContext() );
    if ( !exprVal.isNull() )
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
        else if ( str.compare( QLatin1String( "Justify" ), Qt::CaseInsensitive ) == 0 )
        {
          aligntype = QgsPalLayerSettings::MultiJustify;
        }
        dataDefinedValues.insert( QgsPalLayerSettings::MultiLineAlignment, QVariant( static_cast< int >( aligntype ) ) );
      }
    }
  }

  // text orientation
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::TextOrientation ) )
  {
    const QString encoded = QgsTextRendererUtils::encodeTextOrientation( mFormat.orientation() );
    context.expressionContext().setOriginalValueVariable( encoded );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::TextOrientation, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString str = exprVal.toString().trimmed();
      if ( !str.isEmpty() )
        dataDefinedValues.insert( QgsPalLayerSettings::TextOrientation, str );
    }
  }

  // data defined direction symbol?
  bool drawDirSymb = mLineSettings.addDirectionSymbol();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::DirSymbDraw, exprVal, context.expressionContext(), drawDirSymb ) )
  {
    drawDirSymb = exprVal.toBool();
  }

  if ( drawDirSymb )
  {
    // data defined direction left symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::DirSymbLeft, exprVal, context.expressionContext(), mLineSettings.leftDirectionSymbol() );

    // data defined direction right symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::DirSymbRight, exprVal, context.expressionContext(), mLineSettings.rightDirectionSymbol() );

    // data defined direction symbol placement?
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::DirSymbPlacement, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal DirSymbPlacement:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        // "LeftRight"
        QgsLabelLineSettings::DirectionSymbolPlacement placetype = QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight;

        if ( str.compare( QLatin1String( "Above" ), Qt::CaseInsensitive ) == 0 )
        {
          placetype = QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove;
        }
        else if ( str.compare( QLatin1String( "Below" ), Qt::CaseInsensitive ) == 0 )
        {
          placetype = QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow;
        }
        dataDefinedValues.insert( QgsPalLayerSettings::DirSymbPlacement, QVariant( static_cast< int >( placetype ) ) );
      }
    }

    // data defined direction symbol reversed?
    dataDefinedValEval( DDBool, QgsPalLayerSettings::DirSymbReverse, exprVal, context.expressionContext(), mLineSettings.reverseDirectionSymbol() );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ShapeKind ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeKind, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString skind = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeKind:%1" ).arg( skind ), 4 );

      if ( !skind.isEmpty() )
      {
        shapeKind = QgsTextRendererUtils::decodeShapeType( skind );
        dataDefinedValues.insert( QgsPalLayerSettings::ShapeKind, QVariant( static_cast< int >( shapeKind ) ) );
      }
    }
  }

  // data defined shape SVG path?
  QString svgPath = background.svgFile();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ShapeSVGFile ) )
  {
    context.expressionContext().setOriginalValueVariable( svgPath );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeSVGFile, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString svgfile = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSVGFile:%1" ).arg( svgfile ), 4 );

      // '' empty paths are allowed
      svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
      dataDefinedValues.insert( QgsPalLayerSettings::ShapeSVGFile, QVariant( svgPath ) );
    }
  }

  // data defined shape size type?
  QgsTextBackgroundSettings::SizeType shpSizeType = background.sizeType();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ShapeSizeType ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeSizeType, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString stype = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSizeType:%1" ).arg( stype ), 4 );

      if ( !stype.isEmpty() )
      {
        shpSizeType = QgsTextRendererUtils::decodeBackgroundSizeType( stype );
        dataDefinedValues.insert( QgsPalLayerSettings::ShapeSizeType, QVariant( static_cast< int >( shpSizeType ) ) );
      }
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
  if ( shapeKind == QgsTextBackgroundSettings::ShapeMarkerSymbol
       && ( !background.markerSymbol()
            || ( background.markerSymbol()
                 && shpSizeType == QgsTextBackgroundSettings::SizeFixed
                 && ddShpSizeX == 0.0 ) ) )
  {
    skip = true;
  }
  if ( shapeKind != QgsTextBackgroundSettings::ShapeSVG
       && shapeKind != QgsTextBackgroundSettings::ShapeMarkerSymbol
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ShapeRotationType ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShapeRotationType, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString rotstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeRotationType:%1" ).arg( rotstr ), 4 );

      if ( !rotstr.isEmpty() )
      {
        // "Sync"
        QgsTextBackgroundSettings::RotationType rottype = QgsTextRendererUtils::decodeBackgroundRotationType( rotstr );
        dataDefinedValues.insert( QgsPalLayerSettings::ShapeRotationType, QVariant( static_cast< int >( rottype ) ) );
      }
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::ShadowUnder ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::ShadowUnder, context.expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShadowUnder:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextRendererUtils::decodeShadowPlacementType( str );
        dataDefinedValues.insert( QgsPalLayerSettings::ShadowUnder, QVariant( static_cast< int >( shdwtype ) ) );
      }
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


bool QgsPalLabeling::staticWillUseLayer( const QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer );
      return vl->labelsEnabled() || vl->diagramsEnabled();
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      const QgsVectorTileLayer *vl = qobject_cast< const QgsVectorTileLayer * >( layer );
      if ( !vl->labeling() )
        return false;

      if ( const QgsVectorTileBasicLabeling *labeling = dynamic_cast< const QgsVectorTileBasicLabeling *>( vl->labeling() ) )
        return !labeling->styles().empty();

      return false;
    }

    case QgsMapLayerType::RasterLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
      return false;
  }
  return false;
}


bool QgsPalLabeling::geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry, bool mergeLines )
{
  if ( geometry.isNull() )
  {
    return false;
  }

  if ( geometry.type() == QgsWkbTypes::LineGeometry && geometry.isMultipart() && mergeLines )
  {
    return true;
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
    for ( const QString &line : std::as_const( multiLineSplit ) )
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

QgsGeometry QgsPalLabeling::prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry, bool mergeLines )
{
  if ( geometry.isNull() )
  {
    return QgsGeometry();
  }

  //don't modify the feature's geometry so that geometry based expressions keep working
  QgsGeometry geom = geometry;

  if ( geom.type() == QgsWkbTypes::LineGeometry && geom.isMultipart() && mergeLines )
  {
    geom = geom.mergeLines();
  }

  //reproject the geometry if necessary
  if ( ct.isValid() && !ct.isShortCircuited() )
  {
    try
    {
      geom.transform( ct );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse )
      QgsDebugMsgLevel( QStringLiteral( "Ignoring feature due to transformation exception" ), 4 );
      return QgsGeometry();
    }
    // geometry transforms may result in nan points, remove these
    geom.filterVertices( []( const QgsPoint & point )->bool
    {
      return std::isfinite( point.x() ) && std::isfinite( point.y() );
    } );
    if ( QgsCurvePolygon *cp = qgsgeometry_cast< QgsCurvePolygon * >( geom.get() ) )
    {
      cp->removeInvalidRings();
    }
    else if ( QgsMultiSurface *ms = qgsgeometry_cast< QgsMultiSurface * >( geom.get() ) )
    {
      for ( int i = 0; i < ms->numGeometries(); ++i )
      {
        if ( QgsCurvePolygon *cp = qgsgeometry_cast< QgsCurvePolygon * >( ms->geometryN( i ) ) )
          cp->removeInvalidRings();
      }
    }
  }

  // Rotate the geometry if needed, before clipping
  const QgsMapToPixel &m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPointXY center = context.mapExtent().center();
    if ( geom.rotate( m2p.mapRotation(), center ) != Qgis::GeometryOperationResult::Success )
    {
      QgsDebugMsg( QStringLiteral( "Error rotating geometry" ).arg( geom.asWkt() ) );
      return QgsGeometry();
    }
  }

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  // much faster code path for GEOS 3.9+
  const bool mustClip = ( !clipGeometry.isNull() &&
                          ( ( qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.boundingBox().contains( geom.boundingBox() ) )
                            || ( !qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.contains( geom ) ) ) );

  bool mustClipExact = false;
  if ( mustClip )
  {
    // nice and fast, but can result in invalid geometries. At least it will potentially strip out a bunch of unwanted vertices upfront!
    QgsGeometry clipGeom = geom.clipped( clipGeometry.boundingBox() );
    if ( clipGeom.isEmpty() )
      return QgsGeometry();

    geom = clipGeom;

    // we've now clipped against the BOUNDING BOX of clipGeometry. But if clipGeometry is an axis parallel rectangle, then there's no
    // need to do an exact (potentially costly) intersection clip as well!
    mustClipExact = !clipGeometry.isAxisParallelRectangle( 0.001 );
  }

  // fix invalid polygons
  if ( geom.type() == QgsWkbTypes::PolygonGeometry )
  {
    if ( geom.isMultipart() )
    {
      // important -- we need to treat ever part in isolation here. We can't test the validity of the whole geometry
      // at once, because touching parts would result in an invalid geometry, and buffering this "dissolves" the parts.
      // because the actual label engine treats parts as separate entities, we aren't bound by the usual "touching parts are invalid" rule
      // see https://github.com/qgis/QGIS/issues/26763
      QVector< QgsGeometry> parts;
      parts.reserve( qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() )->numGeometries() );
      for ( auto it = geom.const_parts_begin(); it != geom.const_parts_end(); ++it )
      {
        QgsGeometry partGeom( ( *it )->clone() );
        if ( !partGeom.isGeosValid() )
        {

          partGeom = partGeom.makeValid();
        }
        parts.append( partGeom );
      }
      geom = QgsGeometry::collectGeometry( parts );
    }
    else if ( !geom.isGeosValid() )
    {

      QgsGeometry bufferGeom = geom.makeValid();
      if ( bufferGeom.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Could not repair geometry: %1" ).arg( bufferGeom.lastError() ) );
        return QgsGeometry();
      }
      geom = bufferGeom;
    }
  }

  if ( mustClipExact )
  {
    // now do the real intersection against the actual clip geometry
    QgsGeometry clipGeom = geom.intersection( clipGeometry );
    if ( clipGeom.isEmpty() )
    {
      return QgsGeometry();
    }
    geom = clipGeom;
  }
#else
  // fix invalid polygons
  if ( geom.type() == QgsWkbTypes::PolygonGeometry )
  {
    if ( geom.isMultipart() )
    {
      // important -- we need to treat ever part in isolation here. We can't test the validity of the whole geometry
      // at once, because touching parts would result in an invalid geometry, and buffering this "dissolves" the parts.
      // because the actual label engine treats parts as separate entities, we aren't bound by the usual "touching parts are invalid" rule
      // see https://github.com/qgis/QGIS/issues/26763
      QVector< QgsGeometry> parts;
      parts.reserve( qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() )->numGeometries() );
      for ( auto it = geom.const_parts_begin(); it != geom.const_parts_end(); ++it )
      {
        QgsGeometry partGeom( ( *it )->clone() );
        if ( !partGeom.isGeosValid() )
        {
          partGeom = partGeom.buffer( 0, 0 );
        }
        parts.append( partGeom );
      }
      geom = QgsGeometry::collectGeometry( parts );
    }
    else if ( !geom.isGeosValid() )
    {
      QgsGeometry bufferGeom = geom.buffer( 0, 0 );
      if ( bufferGeom.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Could not repair geometry: %1" ).arg( bufferGeom.lastError() ) );
        return QgsGeometry();
      }
      geom = bufferGeom;
    }
  }

  if ( !clipGeometry.isNull() &&
       ( ( qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.boundingBox().contains( geom.boundingBox() ) )
         || ( !qgsDoubleNear( m2p.mapRotation(), 0 ) && !clipGeometry.contains( geom ) ) ) )
  {
    QgsGeometry clipGeom = geom.intersection( clipGeometry ); // creates new geometry
    if ( clipGeom.isEmpty() )
    {
      return QgsGeometry();
    }
    geom = clipGeom;
  }
#endif

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

  if ( ddValues.contains( QgsPalLayerSettings::TextOrientation ) )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setOrientation( QgsTextRendererUtils::decodeTextOrientation( ddValues.value( QgsPalLayerSettings::TextOrientation ).toString() ) );
    tmpLyr.setFormat( format );
  }

  if ( ddValues.contains( QgsPalLayerSettings::DirSymbDraw ) )
  {
    tmpLyr.lineSettings().setAddDirectionSymbol( ddValues.value( QgsPalLayerSettings::DirSymbDraw ).toBool() );
  }

  if ( tmpLyr.lineSettings().addDirectionSymbol() )
  {

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbLeft ) )
    {
      tmpLyr.lineSettings().setLeftDirectionSymbol( ddValues.value( QgsPalLayerSettings::DirSymbLeft ).toString() );
    }
    if ( ddValues.contains( QgsPalLayerSettings::DirSymbRight ) )
    {
      tmpLyr.lineSettings().setRightDirectionSymbol( ddValues.value( QgsPalLayerSettings::DirSymbRight ).toString() );
    }

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbPlacement ) )
    {
      tmpLyr.lineSettings().setDirectionSymbolPlacement( static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( ddValues.value( QgsPalLayerSettings::DirSymbPlacement ).toInt() ) );
    }

    if ( ddValues.contains( QgsPalLayerSettings::DirSymbReverse ) )
    {
      tmpLyr.lineSettings().setReverseDirectionSymbol( ddValues.value( QgsPalLayerSettings::DirSymbReverse ).toBool() );
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

void QgsPalLabeling::dataDefinedTextMask( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
{
  if ( ddValues.isEmpty() )
    return;

  QgsTextMaskSettings mask = tmpLyr.format().mask();
  bool changed = false;

  // enabled ?
  if ( ddValues.contains( QgsPalLayerSettings::MaskEnabled ) )
  {
    mask.setEnabled( ddValues.value( QgsPalLayerSettings::MaskEnabled ).toBool() );
    changed = true;
  }

  if ( !mask.enabled() )
  {
    if ( changed )
    {
      QgsTextFormat format = tmpLyr.format();
      format.setMask( mask );
      tmpLyr.setFormat( format );
    }

    // tmpLyr.bufferSize > 0.0 && tmpLyr.bufferTransp < 100 figured in during evaluation
    return; // don't continue looking for unused values
  }

  // buffer size
  if ( ddValues.contains( QgsPalLayerSettings::MaskBufferSize ) )
  {
    mask.setSize( ddValues.value( QgsPalLayerSettings::MaskBufferSize ).toDouble() );
    changed = true;
  }

  // opacity
  if ( ddValues.contains( QgsPalLayerSettings::MaskOpacity ) )
  {
    mask.setOpacity( ddValues.value( QgsPalLayerSettings::MaskOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  // buffer size units
  if ( ddValues.contains( QgsPalLayerSettings::MaskBufferUnit ) )
  {
    QgsUnitTypes::RenderUnit bufunit = static_cast< QgsUnitTypes::RenderUnit >( ddValues.value( QgsPalLayerSettings::MaskBufferUnit ).toInt() );
    mask.setSizeUnit( bufunit );
    changed = true;
  }

  // pen join style
  if ( ddValues.contains( QgsPalLayerSettings::MaskJoinStyle ) )
  {
    mask.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::MaskJoinStyle ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setMask( mask );
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
  if ( lp->nextPart() )
    drawLabelCandidateRect( lp->nextPart(), painter, xform, candidates );
}
