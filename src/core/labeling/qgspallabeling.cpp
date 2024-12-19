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
#include "qgscolorutils.h"
#include "qgstextlabelfeature.h"
#include "qgsunittypes.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsstyle.h"
#include "qgstextrenderer.h"
#include "qgsscaleutils.h"

#include <cmath>

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>
#include <QScreen>
#include <QWidget>
#include <QTextBoundaryFinder>

#include "qgsfontutils.h"
#include "qgsexpression.h"
#include "qgslabelingengine.h"
#include "qgstextrendererutils.h"
#include "qgsmultisurface.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsreferencedgeometry.h"
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
#include "qgsfontmanager.h"
#include "qgsvariantutils.h"
#include "qgsmeshlayer.h"
#include "qgsrasterlayer.h"

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
typedef QVector< Qgis::LabelPredefinedPointPosition > PredefinedPointPositionVector;
Q_GLOBAL_STATIC_WITH_ARGS( PredefinedPointPositionVector, DEFAULT_PLACEMENT_ORDER, (
{
  Qgis::LabelPredefinedPointPosition::TopRight,
  Qgis::LabelPredefinedPointPosition::TopLeft,
  Qgis::LabelPredefinedPointPosition::BottomRight,
  Qgis::LabelPredefinedPointPosition::BottomLeft,
  Qgis::LabelPredefinedPointPosition::MiddleRight,
  Qgis::LabelPredefinedPointPosition::MiddleLeft,
  Qgis::LabelPredefinedPointPosition::TopSlightlyRight,
  Qgis::LabelPredefinedPointPosition::BottomSlightlyRight
} ) )
//debugging only - don't use these placements by default
/* << static_cast< int >( QgsPalLayerSettings::Property::TopSlightlyLeft )
<< static_cast< int >( QgsPalLayerSettings::Property::BottomSlightlyLeft );
<< static_cast< int >( QgsPalLayerSettings::Property::TopMiddle )
<< static_cast< int >( QgsPalLayerSettings::Property::BottomMiddle );*/

Q_GLOBAL_STATIC( QgsPropertiesDefinition, sPropertyDefinitions )

void QgsPalLayerSettings::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions()->isEmpty() )
    return;

  const QString origin = QStringLiteral( "labeling" );

  *sPropertyDefinitions() = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsPalLayerSettings::Property::Size ), QgsPropertyDefinition( "Size", QObject::tr( "Font size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Bold ), QgsPropertyDefinition( "Bold", QObject::tr( "Bold style" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Italic ), QgsPropertyDefinition( "Italic", QObject::tr( "Italic style" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Underline ), QgsPropertyDefinition( "Underline", QObject::tr( "Draw underline" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Color ), QgsPropertyDefinition( "Color", QObject::tr( "Text color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Strikeout ), QgsPropertyDefinition( "Strikeout", QObject::tr( "Draw strikeout" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::Family ), QgsPropertyDefinition( "Family", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font family" ), QObject::tr( "string " ) + QObject::tr( "[<b>family</b>|<b>family[foundry]</b>],<br>"
          "e.g. Helvetica or Helvetica [Cronyx]" ), origin )
    },
    {
      static_cast< int >( QgsPalLayerSettings::Property::FontStyle ), QgsPropertyDefinition( "FontStyle", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font style" ), QObject::tr( "string " ) + QObject::tr( "[<b>font style name</b>|<b>Ignore</b>],<br>"
          "e.g. Bold Condensed or Light Italic" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::FontSizeUnit ), QgsPropertyDefinition( "FontSizeUnit", QObject::tr( "Font size units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontTransp ), QgsPropertyDefinition( "FontTransp", QObject::tr( "Text transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontOpacity ), QgsPropertyDefinition( "FontOpacity", QObject::tr( "Text opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontStretchFactor ), QgsPropertyDefinition( "FontStretchFactor", QObject::tr( "Font stretch factor" ), QgsPropertyDefinition::IntegerPositiveGreaterZero, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontCase ), QgsPropertyDefinition( "FontCase", QgsPropertyDefinition::DataTypeString, QObject::tr( "Font case" ), QObject::tr( "string " ) + QStringLiteral( "[<b>NoChange</b>|<b>Upper</b>|<br><b>Lower</b>|<b>Title</b>|<b>Capitalize</b>|<b>SmallCaps</b>|<b>AllSmallCaps</b>]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontLetterSpacing ), QgsPropertyDefinition( "FontLetterSpacing", QObject::tr( "Letter spacing" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontWordSpacing ), QgsPropertyDefinition( "FontWordSpacing", QObject::tr( "Word spacing" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontBlendMode ), QgsPropertyDefinition( "FontBlendMode", QObject::tr( "Text blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MultiLineWrapChar ), QgsPropertyDefinition( "MultiLineWrapChar", QObject::tr( "Wrap character" ), QgsPropertyDefinition::String, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::AutoWrapLength ), QgsPropertyDefinition( "AutoWrapLength", QObject::tr( "Automatic word wrap line length" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MultiLineHeight ), QgsPropertyDefinition( "MultiLineHeight", QObject::tr( "Line height" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MultiLineAlignment ), QgsPropertyDefinition( "MultiLineAlignment", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line alignment" ), QObject::tr( "string " ) + "[<b>Left</b>|<b>Center</b>|<b>Right</b>|<b>Follow</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::TabStopDistance ), QgsPropertyDefinition( "TabStopDistance", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Tab stop distance(s)" ), QObject::tr( "Numeric distance or array of distances" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::TextOrientation ), QgsPropertyDefinition( "TextOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Text orientation" ), QObject::tr( "string " ) + "[<b>horizontal</b>|<b>vertical</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DirSymbDraw ), QgsPropertyDefinition( "DirSymbDraw", QObject::tr( "Draw direction symbol" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DirSymbLeft ), QgsPropertyDefinition( "DirSymbLeft", QObject::tr( "Left direction symbol" ), QgsPropertyDefinition::String, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DirSymbRight ), QgsPropertyDefinition( "DirSymbRight", QObject::tr( "Right direction symbol" ), QgsPropertyDefinition::String, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DirSymbPlacement ), QgsPropertyDefinition( "DirSymbPlacement", QgsPropertyDefinition::DataTypeString, QObject::tr( "Direction symbol placement" ), QObject::tr( "string " ) + "[<b>LeftRight</b>|<b>Above</b>|<b>Below</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DirSymbReverse ), QgsPropertyDefinition( "DirSymbReverse", QObject::tr( "Reverse direction symbol" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::NumFormat ), QgsPropertyDefinition( "NumFormat", QObject::tr( "Format as number" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::NumDecimals ), QgsPropertyDefinition( "NumDecimals", QObject::tr( "Number of decimal places" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::NumPlusSign ), QgsPropertyDefinition( "NumPlusSign", QObject::tr( "Draw + sign" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferDraw ), QgsPropertyDefinition( "BufferDraw", QObject::tr( "Draw buffer" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferSize ), QgsPropertyDefinition( "BufferSize", QObject::tr( "Symbol size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferUnit ), QgsPropertyDefinition( "BufferUnit", QObject::tr( "Buffer units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferColor ), QgsPropertyDefinition( "BufferColor", QObject::tr( "Buffer color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferTransp ), QgsPropertyDefinition( "BufferTransp", QObject::tr( "Buffer transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferOpacity ), QgsPropertyDefinition( "BufferOpacity", QObject::tr( "Buffer opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferJoinStyle ), QgsPropertyDefinition( "BufferJoinStyle", QObject::tr( "Buffer join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::BufferBlendMode ), QgsPropertyDefinition( "BufferBlendMode", QObject::tr( "Buffer blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },

    { static_cast< int >( QgsPalLayerSettings::Property::MaskEnabled ), QgsPropertyDefinition( "MaskEnabled", QObject::tr( "Enable mask" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaskBufferSize ), QgsPropertyDefinition( "MaskBufferSize", QObject::tr( "Mask buffer size" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaskBufferUnit ), QgsPropertyDefinition( "MaskBufferUnit", QObject::tr( "Mask buffer unit" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaskOpacity ), QgsPropertyDefinition( "MaskOpacity", QObject::tr( "Mask opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaskJoinStyle ), QgsPropertyDefinition( "MaskJoinStyle", QObject::tr( "Mask join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },

    { static_cast< int >( QgsPalLayerSettings::Property::ShapeDraw ), QgsPropertyDefinition( "ShapeDraw", QObject::tr( "Draw shape" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::ShapeKind ), QgsPropertyDefinition( "ShapeKind", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape type" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Rectangle</b>|<b>Square</b>|<br>"
          "<b>Ellipse</b>|<b>Circle</b>|<b>SVG</b>]" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeSVGFile ), QgsPropertyDefinition( "ShapeSVGFile", QObject::tr( "Shape SVG path" ), QgsPropertyDefinition::SvgPath, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeSizeType ), QgsPropertyDefinition( "ShapeSizeType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape size type" ), QObject::tr( "string " ) + "[<b>Buffer</b>|<b>Fixed</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeSizeX ), QgsPropertyDefinition( "ShapeSizeX", QObject::tr( "Shape size (X)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeSizeY ), QgsPropertyDefinition( "ShapeSizeY", QObject::tr( "Shape size (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeSizeUnits ), QgsPropertyDefinition( "ShapeSizeUnits", QObject::tr( "Shape size units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeRotationType ), QgsPropertyDefinition( "ShapeRotationType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Shape rotation type" ), QObject::tr( "string " ) + "[<b>Sync</b>|<b>Offset</b>|<b>Fixed</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeRotation ), QgsPropertyDefinition( "ShapeRotation", QObject::tr( "Shape rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeOffset ), QgsPropertyDefinition( "ShapeOffset", QObject::tr( "Shape offset" ), QgsPropertyDefinition::Offset, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeOffsetUnits ), QgsPropertyDefinition( "ShapeOffsetUnits", QObject::tr( "Shape offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeRadii ), QgsPropertyDefinition( "ShapeRadii", QObject::tr( "Shape radii" ), QgsPropertyDefinition::Size2D, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeRadiiUnits ), QgsPropertyDefinition( "ShapeRadiiUnits", QObject::tr( "Symbol radii units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeTransparency ), QgsPropertyDefinition( "ShapeTransparency", QObject::tr( "Shape transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeOpacity ), QgsPropertyDefinition( "ShapeOpacity", QObject::tr( "Shape opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeBlendMode ), QgsPropertyDefinition( "ShapeBlendMode", QObject::tr( "Shape blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeFillColor ), QgsPropertyDefinition( "ShapeFillColor", QObject::tr( "Shape fill color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeStrokeColor ), QgsPropertyDefinition( "ShapeBorderColor", QObject::tr( "Shape stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeStrokeWidth ), QgsPropertyDefinition( "ShapeBorderWidth", QObject::tr( "Shape stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeStrokeWidthUnits ), QgsPropertyDefinition( "ShapeBorderWidthUnits", QObject::tr( "Shape stroke width units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShapeJoinStyle ), QgsPropertyDefinition( "ShapeJoinStyle", QObject::tr( "Shape join style" ), QgsPropertyDefinition::PenJoinStyle, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowDraw ), QgsPropertyDefinition( "ShadowDraw", QObject::tr( "Draw shadow" ), QgsPropertyDefinition::Boolean, origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::ShadowUnder ), QgsPropertyDefinition( "ShadowUnder", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Lowest</b>|<b>Text</b>|<br>"
          "<b>Buffer</b>|<b>Background</b>]" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowOffsetAngle ), QgsPropertyDefinition( "ShadowOffsetAngle", QObject::tr( "Shadow offset angle" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowOffsetDist ), QgsPropertyDefinition( "ShadowOffsetDist", QObject::tr( "Shadow offset distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowOffsetUnits ), QgsPropertyDefinition( "ShadowOffsetUnits", QObject::tr( "Shadow offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowRadius ), QgsPropertyDefinition( "ShadowRadius", QObject::tr( "Shadow blur radius" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowRadiusUnits ), QgsPropertyDefinition( "ShadowRadiusUnits", QObject::tr( "Shadow blur units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowTransparency ), QgsPropertyDefinition( "ShadowTransparency", QObject::tr( "Shadow transparency" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowOpacity ), QgsPropertyDefinition( "ShadowOpacity", QObject::tr( "Shadow opacity" ), QgsPropertyDefinition::Opacity, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowScale ), QgsPropertyDefinition( "ShadowScale", QObject::tr( "Shadow scale" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowColor ), QgsPropertyDefinition( "ShadowColor", QObject::tr( "Shadow color" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ShadowBlendMode ), QgsPropertyDefinition( "ShadowBlendMode", QObject::tr( "Shadow blend mode" ), QgsPropertyDefinition::BlendMode, origin ) },

    { static_cast< int >( QgsPalLayerSettings::Property::CentroidWhole ), QgsPropertyDefinition( "CentroidWhole", QgsPropertyDefinition::DataTypeString, QObject::tr( "Centroid of whole shape" ), QObject::tr( "string " ) + "[<b>Visible</b>|<b>Whole</b>]", origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::OffsetQuad ), QgsPropertyDefinition( "OffsetQuad", QgsPropertyDefinition::DataTypeString, QObject::tr( "Offset quadrant" ), QObject::tr( "int<br>" ) + QStringLiteral( "[<b>0</b>=Above Left|<b>1</b>=Above|<b>2</b>=Above Right|<br>"
          "<b>3</b>=Left|<b>4</b>=Over|<b>5</b>=Right|<br>"
          "<b>6</b>=Below Left|<b>7</b>=Below|<b>8</b>=Below Right]" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::OffsetXY ), QgsPropertyDefinition( "OffsetXY", QObject::tr( "Offset" ), QgsPropertyDefinition::Offset, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::OffsetUnits ), QgsPropertyDefinition( "OffsetUnits", QObject::tr( "Offset units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LabelDistance ), QgsPropertyDefinition( "LabelDistance", QObject::tr( "Label distance" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::DistanceUnits ), QgsPropertyDefinition( "DistanceUnits", QObject::tr( "Label distance units" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::OffsetRotation ), QgsPropertyDefinition( "OffsetRotation", QObject::tr( "Offset rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::CurvedCharAngleInOut ), QgsPropertyDefinition( "CurvedCharAngleInOut", QgsPropertyDefinition::DataTypeString, QObject::tr( "Curved character angles" ), QObject::tr( "double coord [<b>in,out</b> as 20.0-60.0,20.0-95.0]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::RepeatDistance ), QgsPropertyDefinition( "RepeatDistance", QObject::tr( "Repeat distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::RepeatDistanceUnit ), QgsPropertyDefinition( "RepeatDistanceUnit", QObject::tr( "Repeat distance unit" ), QgsPropertyDefinition::RenderUnits, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::OverrunDistance ), QgsPropertyDefinition( "OverrunDistance", QObject::tr( "Overrun distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LineAnchorPercent ), QgsPropertyDefinition( "LineAnchorPercent", QObject::tr( "Line anchor percentage, as fraction from 0.0 to 1.0" ), QgsPropertyDefinition::Double0To1, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LineAnchorClipping ), QgsPropertyDefinition( "LineAnchorClipping", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor clipping mode" ), QObject::tr( "string " ) + QStringLiteral( "[<b>visible</b>|<b>entire</b>]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LineAnchorType ), QgsPropertyDefinition( "LineAnchorType", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor type" ), QObject::tr( "string " ) + QStringLiteral( "[<b>hint</b>|<b>strict</b>]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LineAnchorTextPoint ), QgsPropertyDefinition( "LineAnchorTextPoint", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line anchor text point" ), QObject::tr( "string " ) + QStringLiteral( "[<b>follow</b>|<b>start</b>|<b>center</b>|<b>end</b>]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Priority ), QgsPropertyDefinition( "Priority", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Label priority" ), QObject::tr( "double [0.0-10.0]" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::IsObstacle ), QgsPropertyDefinition( "IsObstacle", QObject::tr( "Feature is a label obstacle" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ObstacleFactor ), QgsPropertyDefinition( "ObstacleFactor", QgsPropertyDefinition::DataTypeNumeric, QObject::tr( "Obstacle factor" ), QObject::tr( "double [0.0-10.0]" ), origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::PredefinedPositionOrder ), QgsPropertyDefinition( "PredefinedPositionOrder", QgsPropertyDefinition::DataTypeString, QObject::tr( "Predefined position order" ),  QObject::tr( "Comma separated list of placements in order of priority<br>" )
          + QStringLiteral( "[<b>TL</b>=Top left|<b>TSL</b>=Top, slightly left|<b>T</b>=Top middle|<br>"
                            "<b>TSR</b>=Top, slightly right|<b>TR</b>=Top right|<br>"
                            "<b>L</b>=Left|<b>R</b>=Right|<br>"
                            "<b>BL</b>=Bottom left|<b>BSL</b>=Bottom, slightly left|<b>B</b>=Bottom middle|<br>"
                            "<b>BSR</b>=Bottom, slightly right|<b>BR</b>=Bottom right|<b>O</b>=Over point]" ), origin )
    },
    {
      static_cast< int >( QgsPalLayerSettings::Property::LinePlacementOptions ), QgsPropertyDefinition( "LinePlacementFlags", QgsPropertyDefinition::DataTypeString, QObject::tr( "Line placement options" ),  QObject::tr( "Comma separated list of placement options<br>" )
          + QStringLiteral( "[<b>OL</b>=On line|<b>AL</b>=Above line|<b>BL</b>=Below line|<br>"
                            "<b>LO</b>=Respect line orientation]" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::PolygonLabelOutside ), QgsPropertyDefinition( "PolygonLabelOutside", QgsPropertyDefinition::DataTypeString, QObject::tr( "Label outside polygons" ),  QObject::tr( "string " ) + "[<b>yes</b> (allow placing outside)|<b>no</b> (never place outside)|<b>force</b> (always place outside)]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::PositionX ), QgsPropertyDefinition( "PositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::PositionY ), QgsPropertyDefinition( "PositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::PositionPoint ), QgsPropertyDefinition( "PositionPoint", QgsPropertyDefinition::DataTypeString, QObject::tr( "Position (point)" ), QObject::tr( "A point geometry" ), origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Hali ), QgsPropertyDefinition( "Hali", QgsPropertyDefinition::DataTypeString, QObject::tr( "Horizontal alignment" ), QObject::tr( "string " ) + "[<b>Left</b>|<b>Center</b>|<b>Right</b>]", origin ) },
    {
      static_cast< int >( QgsPalLayerSettings::Property::Vali ), QgsPropertyDefinition( "Vali", QgsPropertyDefinition::DataTypeString, QObject::tr( "Vertical alignment" ), QObject::tr( "string " ) + QStringLiteral( "[<b>Bottom</b>|<b>Base</b>|<br>"
          "<b>Half</b>|<b>Cap</b>|<b>Top</b>]" ), origin )
    },
    { static_cast< int >( QgsPalLayerSettings::Property::Rotation ), QgsPropertyDefinition( "Rotation", QObject::tr( "Label rotation (deprecated)" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LabelRotation ), QgsPropertyDefinition( "LabelRotation", QObject::tr( "Label rotation" ), QgsPropertyDefinition::Rotation, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ScaleVisibility ), QgsPropertyDefinition( "ScaleVisibility", QObject::tr( "Scale based visibility" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MinScale ), QgsPropertyDefinition( "MinScale", QObject::tr( "Minimum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaxScale ), QgsPropertyDefinition( "MaxScale", QObject::tr( "Maximum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MinimumScale ), QgsPropertyDefinition( "MinimumScale", QObject::tr( "Minimum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaximumScale ), QgsPropertyDefinition( "MaximumScale", QObject::tr( "Maximum scale (denominator)" ), QgsPropertyDefinition::Double, origin ) },

    { static_cast< int >( QgsPalLayerSettings::Property::FontLimitPixel ), QgsPropertyDefinition( "FontLimitPixel", QObject::tr( "Limit font pixel size" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontMinPixel ), QgsPropertyDefinition( "FontMinPixel", QObject::tr( "Minimum pixel size" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::FontMaxPixel ), QgsPropertyDefinition( "FontMaxPixel", QObject::tr( "Maximum pixel size" ), QgsPropertyDefinition::IntegerPositive, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::ZIndex ), QgsPropertyDefinition( "ZIndex", QObject::tr( "Label z-index" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::Show ), QgsPropertyDefinition( "Show", QObject::tr( "Show label" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::AlwaysShow ), QgsPropertyDefinition( "AlwaysShow", QObject::tr( "Always show label" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::CalloutDraw ), QgsPropertyDefinition( "CalloutDraw", QObject::tr( "Draw callout" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::LabelAllParts ), QgsPropertyDefinition( "LabelAllParts", QObject::tr( "Label all parts" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::AllowDegradedPlacement ), QgsPropertyDefinition( "AllowDegradedPlacement", QObject::tr( "Allow inferior fallback placements" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::OverlapHandling ), QgsPropertyDefinition( "OverlapHandling", QgsPropertyDefinition::DataTypeString, QObject::tr( "Overlap handing" ), QObject::tr( "string " ) + "[<b>Prevent</b>|<b>AllowIfNeeded</b>|<b>AlwaysAllow</b>]", origin ) },
    { static_cast< int >( QgsPalLayerSettings::Property::MaximumDistance ), QgsPropertyDefinition( "MaximumDistance", QObject::tr( "Maximum distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
  };
}

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsPalLayerSettings::QgsPalLayerSettings()
  : mCallout( QgsCalloutRegistry::defaultCallout() )
{
  mPointSettings.setPredefinedPositionOrder( *DEFAULT_PLACEMENT_ORDER() );

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
  fitInPolygonOnly = s.fitInPolygonOnly;
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
  upsidedownLabels = s.upsidedownLabels;

  labelPerPart = s.labelPerPart;
  zIndex = s.zIndex;

  mFormat = s.mFormat;
  mDataDefinedProperties = s.mDataDefinedProperties;

  mCallout.reset( s.mCallout ? s.mCallout->clone() : nullptr );

  mPlacementSettings = s.mPlacementSettings;
  mLineSettings = s.mLineSettings;
  mPointSettings = s.mPointSettings;
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
        // prepare expression for use in static_cast< int >( QgsPalLayerSettings::Property::registerFeature )()
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
  QgsDebugMsgLevel( QStringLiteral( "Visible extent: %1" ).arg( r1.toString() ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "mapSetting extentBuffer: %1" ).arg( mapSettings.extentBuffer() ), 2 );
  r1.grow( mapSettings.extentBuffer() );
  QgsDebugMsgLevel( QStringLiteral( "Grown visible extent: %1" ).arg( r1.toString() ), 2 );
  extentGeom = QgsGeometry::fromRect( r1 );
  QgsDebugMsgLevel( QStringLiteral( "Extent geom  %1" ).arg( extentGeom.asWkt() ), 2 );

  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
    QgsDebugMsgLevel( QStringLiteral( "Rotated extent geom  %1" ).arg( extentGeom.asWkt() ), 2 );
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

  switch ( placement )
  {
    case Qgis::LabelPlacement::PerimeterCurved:
    case Qgis::LabelPlacement::Curved:
    {
      // force horizontal orientation, other orientation modes aren't unsupported for curved placement
      mFormat.setOrientation( Qgis::TextOrientation::Horizontal );
      mDataDefinedProperties.property( QgsPalLayerSettings::Property::TextOrientation ).setActive( false );
      break;
    }

    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    case Qgis::LabelPlacement::OutsidePolygons:
      break;
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

Qgis::AngleUnit QgsPalLayerSettings::rotationUnit() const
{
  return mRotationUnit;
}

void QgsPalLayerSettings::setRotationUnit( Qgis::AngleUnit angleUnit )
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
  QString newPropertyName = "labeling/dataDefined/" + sPropertyDefinitions()->value( static_cast< int >( p ) ).name();
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
    if ( layer->geometryType() == Qgis::GeometryType::Point )
      placement = Qgis::LabelPlacement::OrderedPositionsAroundPoint;

    // for polygons the "over point" (over centroid) placement is better than the default
    // "around point" (around centroid) which is more suitable for points
    if ( layer->geometryType() == Qgis::GeometryType::Polygon )
      placement = Qgis::LabelPlacement::OverPoint;

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

  multilineAlign = static_cast< Qgis::LabelMultiLineAlignment >( layer->customProperty( QStringLiteral( "labeling/multilineAlign" ), QVariant( static_cast< int >( Qgis::LabelMultiLineAlignment::FollowPlacement ) ) ).toUInt() );
  mLineSettings.setAddDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/addDirectionSymbol" ) ).toBool() );
  mLineSettings.setLeftDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/leftDirectionSymbol" ), QVariant( "<" ) ).toString() );
  mLineSettings.setRightDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/rightDirectionSymbol" ), QVariant( ">" ) ).toString() );
  mLineSettings.setReverseDirectionSymbol( layer->customProperty( QStringLiteral( "labeling/reverseDirectionSymbol" ) ).toBool() );
  mLineSettings.setDirectionSymbolPlacement( static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( layer->customProperty( QStringLiteral( "labeling/placeDirectionSymbol" ), QVariant( static_cast< int >( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight ) ) ).toUInt() ) );
  formatNumbers = layer->customProperty( QStringLiteral( "labeling/formatNumbers" ) ).toBool();
  decimals = layer->customProperty( QStringLiteral( "labeling/decimals" ) ).toInt();
  plusSign = layer->customProperty( QStringLiteral( "labeling/plussign" ) ).toBool();

  // placement
  placement = static_cast< Qgis::LabelPlacement >( layer->customProperty( QStringLiteral( "labeling/placement" ) ).toInt() );
  mLineSettings.setPlacementFlags( static_cast< Qgis::LabelLinePlacementFlags >( layer->customProperty( QStringLiteral( "labeling/placementFlags" ) ).toUInt() ) );
  centroidWhole = layer->customProperty( QStringLiteral( "labeling/centroidWhole" ), QVariant( false ) ).toBool();
  centroidInside = layer->customProperty( QStringLiteral( "labeling/centroidInside" ), QVariant( false ) ).toBool();

  QVector<Qgis::LabelPredefinedPointPosition> predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( layer->customProperty( QStringLiteral( "labeling/predefinedPositionOrder" ) ).toString() );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = *DEFAULT_PLACEMENT_ORDER();
  mPointSettings.setPredefinedPositionOrder( predefinedPositionOrder );

  fitInPolygonOnly = layer->customProperty( QStringLiteral( "labeling/fitInPolygonOnly" ), QVariant( false ) ).toBool();
  dist = layer->customProperty( QStringLiteral( "labeling/dist" ) ).toDouble();
  distUnits = layer->customProperty( QStringLiteral( "labeling/distInMapUnits" ) ).toBool() ? Qgis::RenderUnit::MapUnits : Qgis::RenderUnit::Millimeters;
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
  offsetType = static_cast< Qgis::LabelOffsetType >( layer->customProperty( QStringLiteral( "labeling/offsetType" ), QVariant( static_cast< int >( Qgis::LabelOffsetType::FromPoint ) ) ).toUInt() );
  mPointSettings.setQuadrant( static_cast< Qgis::LabelQuadrantPosition >( layer->customProperty( QStringLiteral( "labeling/quadOffset" ), QVariant( static_cast< int >( Qgis::LabelQuadrantPosition::Over ) ) ).toUInt() ) );
  xOffset = layer->customProperty( QStringLiteral( "labeling/xOffset" ), QVariant( 0.0 ) ).toDouble();
  yOffset = layer->customProperty( QStringLiteral( "labeling/yOffset" ), QVariant( 0.0 ) ).toDouble();
  if ( layer->customProperty( QStringLiteral( "labeling/labelOffsetInMapUnits" ), QVariant( true ) ).toBool() )
    offsetUnits = Qgis::RenderUnit::MapUnits;
  else
    offsetUnits = Qgis::RenderUnit::Millimeters;

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
  mRotationUnit = layer->customEnumProperty( QStringLiteral( "labeling/rotationUnit" ), Qgis::AngleUnit::Degrees );
  maxCurvedCharAngleIn = layer->customProperty( QStringLiteral( "labeling/maxCurvedCharAngleIn" ), QVariant( 25.0 ) ).toDouble();
  maxCurvedCharAngleOut = layer->customProperty( QStringLiteral( "labeling/maxCurvedCharAngleOut" ), QVariant( -25.0 ) ).toDouble();
  priority = layer->customProperty( QStringLiteral( "labeling/priority" ) ).toInt();
  repeatDistance = layer->customProperty( QStringLiteral( "labeling/repeatDistance" ), 0.0 ).toDouble();
  switch ( layer->customProperty( QStringLiteral( "labeling/repeatDistanceUnit" ), QVariant( 1 ) ).toUInt() )
  {
    case 0:
      repeatDistanceUnit = Qgis::RenderUnit::Points;
      break;
    case 1:
      repeatDistanceUnit = Qgis::RenderUnit::Millimeters;
      break;
    case 2:
      repeatDistanceUnit = Qgis::RenderUnit::MapUnits;
      break;
    case 3:
      repeatDistanceUnit = Qgis::RenderUnit::Percentage;
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
  if ( layer->customProperty( QStringLiteral( "labeling/displayAll" ), QVariant( false ) ).toBool() )
  {
    mPlacementSettings.setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );
    mPlacementSettings.setAllowDegradedPlacement( true );
  }
  else
  {
    mPlacementSettings.setOverlapHandling( Qgis::LabelOverlapHandling::PreventOverlap );
    mPlacementSettings.setAllowDegradedPlacement( false );
  }
  upsidedownLabels = static_cast< Qgis::UpsideDownLabelHandling >( layer->customProperty( QStringLiteral( "labeling/upsidedownLabels" ), QVariant( static_cast< int >( Qgis::UpsideDownLabelHandling::FlipUpsideDownLabels ) ) ).toUInt() );

  labelPerPart = layer->customProperty( QStringLiteral( "labeling/labelPerPart" ) ).toBool();
  mLineSettings.setMergeLines( layer->customProperty( QStringLiteral( "labeling/mergeLines" ) ).toBool() );
  mThinningSettings.setMinimumFeatureSize( layer->customProperty( QStringLiteral( "labeling/minFeatureSize" ) ).toDouble() );
  mThinningSettings.setLimitNumberLabelsEnabled( layer->customProperty( QStringLiteral( "labeling/limitNumLabels" ), QVariant( false ) ).toBool() );
  mThinningSettings.setMaximumNumberLabels( layer->customProperty( QStringLiteral( "labeling/maxNumLabels" ), QVariant( 2000 ) ).toInt() );
  mObstacleSettings.setIsObstacle( layer->customProperty( QStringLiteral( "labeling/obstacle" ), QVariant( true ) ).toBool() );
  mObstacleSettings.setFactor( layer->customProperty( QStringLiteral( "labeling/obstacleFactor" ), QVariant( 1.0 ) ).toDouble() );
  mObstacleSettings.setType( static_cast< QgsLabelObstacleSettings::ObstacleType >( layer->customProperty( QStringLiteral( "labeling/obstacleType" ), QVariant( static_cast< int >( QgsLabelObstacleSettings::ObstacleType::PolygonInterior ) ) ).toUInt() ) );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontTransp ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::FontOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::FontTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::FontTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::BufferTransp ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::BufferOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::BufferTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::BufferTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeTransparency ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShapeOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::ShapeTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShapeTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShadowTransparency ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShadowOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::ShadowTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShadowTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Rotation ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::LabelRotation, QgsProperty::fromExpression( QStringLiteral( "360 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::Rotation ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::Rotation, QgsProperty() );
  }
  // older 2.x projects had min/max scale flipped - so change them here.
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MinScale ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MaximumScale, mDataDefinedProperties.property( QgsPalLayerSettings::Property::MinScale ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MinScale, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MaxScale ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MinimumScale, mDataDefinedProperties.property( QgsPalLayerSettings::Property::MaxScale ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MaxScale, QgsProperty() );
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
  multilineAlign = static_cast< Qgis::LabelMultiLineAlignment >( textFormatElem.attribute( QStringLiteral( "multilineAlign" ), QString::number( static_cast< int >( Qgis::LabelMultiLineAlignment::FollowPlacement ) ) ).toUInt() );
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
  placement = static_cast< Qgis::LabelPlacement >( placementElem.attribute( QStringLiteral( "placement" ) ).toInt() );
  mLineSettings.setPlacementFlags( static_cast< Qgis::LabelLinePlacementFlags >( placementElem.attribute( QStringLiteral( "placementFlags" ) ).toUInt() ) );
  mPolygonPlacementFlags = static_cast< Qgis::LabelPolygonPlacementFlags >( placementElem.attribute( QStringLiteral( "polygonPlacementFlags" ), QString::number( static_cast< int >( Qgis::LabelPolygonPlacementFlag::AllowPlacementInsideOfPolygon ) ) ).toInt() );

  centroidWhole = placementElem.attribute( QStringLiteral( "centroidWhole" ), QStringLiteral( "0" ) ).toInt();
  centroidInside = placementElem.attribute( QStringLiteral( "centroidInside" ), QStringLiteral( "0" ) ).toInt();

  QVector<Qgis::LabelPredefinedPointPosition>  predefinedPositionOrder = QgsLabelingUtils::decodePredefinedPositionOrder( placementElem.attribute( QStringLiteral( "predefinedPositionOrder" ) ) );
  if ( predefinedPositionOrder.isEmpty() )
    predefinedPositionOrder = *DEFAULT_PLACEMENT_ORDER();
  mPointSettings.setPredefinedPositionOrder( predefinedPositionOrder );

  fitInPolygonOnly = placementElem.attribute( QStringLiteral( "fitInPolygonOnly" ), QStringLiteral( "0" ) ).toInt();
  dist = placementElem.attribute( QStringLiteral( "dist" ) ).toDouble();
  if ( !placementElem.hasAttribute( QStringLiteral( "distUnits" ) ) )
  {
    if ( placementElem.attribute( QStringLiteral( "distInMapUnits" ) ).toInt() )
      distUnits = Qgis::RenderUnit::MapUnits;
    else
      distUnits = Qgis::RenderUnit::Millimeters;
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
  offsetType = static_cast< Qgis::LabelOffsetType >( placementElem.attribute( QStringLiteral( "offsetType" ), QString::number( static_cast< int >( Qgis::LabelOffsetType::FromPoint ) ) ).toUInt() );
  mPointSettings.setQuadrant( static_cast< Qgis::LabelQuadrantPosition >( placementElem.attribute( QStringLiteral( "quadOffset" ), QString::number( static_cast< int >( Qgis::LabelQuadrantPosition::Over ) ) ).toUInt() ) );
  xOffset = placementElem.attribute( QStringLiteral( "xOffset" ), QStringLiteral( "0" ) ).toDouble();
  yOffset = placementElem.attribute( QStringLiteral( "yOffset" ), QStringLiteral( "0" ) ).toDouble();
  if ( !placementElem.hasAttribute( QStringLiteral( "offsetUnits" ) ) )
  {
    offsetUnits = placementElem.attribute( QStringLiteral( "labelOffsetInMapUnits" ), QStringLiteral( "1" ) ).toInt() ? Qgis::RenderUnit::MapUnits : Qgis::RenderUnit::Millimeters;
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
  {
    QString rotationUnitString = placementElem.attribute( QStringLiteral( "rotationUnit" ), qgsEnumValueToKey( Qgis::AngleUnit::Degrees ) );
    if ( rotationUnitString.startsWith( QLatin1String( "Angle" ) ) )
    {
      // compatibility with QGIS < 3.30
      rotationUnitString = rotationUnitString.mid( 5 );
    }

    mRotationUnit = qgsEnumKeyToValue( rotationUnitString, Qgis::AngleUnit::Degrees );
  }
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
        repeatDistanceUnit = Qgis::RenderUnit::Points;
        break;
      case 1:
        repeatDistanceUnit = Qgis::RenderUnit::Millimeters;
        break;
      case 2:
        repeatDistanceUnit = Qgis::RenderUnit::MapUnits;
        break;
      case 3:
        repeatDistanceUnit = Qgis::RenderUnit::Percentage;
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

  mPointSettings.setMaximumDistance( placementElem.attribute( QStringLiteral( "maximumDistance" ), QStringLiteral( "0" ) ).toDouble() );
  mPointSettings.setMaximumDistanceUnit( QgsUnitTypes::decodeRenderUnit( placementElem.attribute( QStringLiteral( "maximumDistanceUnit" ) ) ) );
  mPointSettings.setMaximumDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( placementElem.attribute( QStringLiteral( "maximumDistanceMapUnitScale" ) ) ) );

  geometryGenerator = placementElem.attribute( QStringLiteral( "geometryGenerator" ) );
  geometryGeneratorEnabled = placementElem.attribute( QStringLiteral( "geometryGeneratorEnabled" ) ).toInt();
  {
    QString geometryTypeKey = placementElem.attribute( QStringLiteral( "geometryGeneratorType" ) );
    // maintain compatibility with < 3.3.0
    if ( geometryTypeKey.endsWith( QLatin1String( "Geometry" ) ) )
      geometryTypeKey.chop( 8 );

    geometryGeneratorType = qgsEnumKeyToValue( geometryTypeKey, Qgis::GeometryType::Point );
  }
  {
    QString layerTypeKey = placementElem.attribute( QStringLiteral( "layerType" ) );
    // maintain compatibility with < 3.3.0
    if ( layerTypeKey.endsWith( QLatin1String( "Geometry" ) ) )
      layerTypeKey.chop( 8 );

    layerType = qgsEnumKeyToValue( layerTypeKey, Qgis::GeometryType::Unknown );
  }

  mPlacementSettings.setAllowDegradedPlacement( placementElem.attribute( QStringLiteral( "allowDegraded" ), QStringLiteral( "0" ) ).toInt() );

  // rendering
  QDomElement renderingElem = elem.firstChildElement( QStringLiteral( "rendering" ) );

  drawLabels = renderingElem.attribute( QStringLiteral( "drawLabels" ), QStringLiteral( "1" ) ).toInt();

  maximumScale = renderingElem.attribute( QStringLiteral( "scaleMin" ), QStringLiteral( "0" ) ).toDouble();
  minimumScale = renderingElem.attribute( QStringLiteral( "scaleMax" ), QStringLiteral( "0" ) ).toDouble();
  scaleVisibility = renderingElem.attribute( QStringLiteral( "scaleVisibility" ) ).toInt();

  fontLimitPixelSize = renderingElem.attribute( QStringLiteral( "fontLimitPixelSize" ), QStringLiteral( "0" ) ).toInt();
  fontMinPixelSize = renderingElem.attribute( QStringLiteral( "fontMinPixelSize" ), QStringLiteral( "0" ) ).toInt();
  fontMaxPixelSize = renderingElem.attribute( QStringLiteral( "fontMaxPixelSize" ), QStringLiteral( "10000" ) ).toInt();

  if ( placementElem.hasAttribute( QStringLiteral( "overlapHandling" ) ) )
  {
    mPlacementSettings.setOverlapHandling( qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "overlapHandling" ) ), Qgis::LabelOverlapHandling::PreventOverlap ) );
  }
  else
  {
    // legacy setting
    if ( renderingElem.attribute( QStringLiteral( "displayAll" ), QStringLiteral( "0" ) ).toInt() )
    {
      mPlacementSettings.setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );
      mPlacementSettings.setAllowDegradedPlacement( true );
    }
    else
    {
      mPlacementSettings.setOverlapHandling( Qgis::LabelOverlapHandling::PreventOverlap );
      mPlacementSettings.setAllowDegradedPlacement( false );
    }
  }

  mPlacementSettings.setPrioritization( qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "prioritization" ) ), Qgis::LabelPrioritization::PreferCloser ) );

  upsidedownLabels = static_cast< Qgis::UpsideDownLabelHandling >( renderingElem.attribute( QStringLiteral( "upsidedownLabels" ), QString::number( static_cast< int >( Qgis::UpsideDownLabelHandling::FlipUpsideDownLabels ) ) ).toUInt() );

  labelPerPart = renderingElem.attribute( QStringLiteral( "labelPerPart" ) ).toInt();
  mLineSettings.setMergeLines( renderingElem.attribute( QStringLiteral( "mergeLines" ) ).toInt() );
  mThinningSettings.setMinimumFeatureSize( renderingElem.attribute( QStringLiteral( "minFeatureSize" ) ).toDouble() );
  mThinningSettings.setLimitNumberLabelsEnabled( renderingElem.attribute( QStringLiteral( "limitNumLabels" ), QStringLiteral( "0" ) ).toInt() );
  mThinningSettings.setMaximumNumberLabels( renderingElem.attribute( QStringLiteral( "maxNumLabels" ), QStringLiteral( "2000" ) ).toInt() );
  mObstacleSettings.setIsObstacle( renderingElem.attribute( QStringLiteral( "obstacle" ), QStringLiteral( "1" ) ).toInt() );
  mObstacleSettings.setFactor( renderingElem.attribute( QStringLiteral( "obstacleFactor" ), QStringLiteral( "1" ) ).toDouble() );
  mObstacleSettings.setType( static_cast< QgsLabelObstacleSettings::ObstacleType >( renderingElem.attribute( QStringLiteral( "obstacleType" ), QString::number( static_cast< int >( QgsLabelObstacleSettings::ObstacleType::PolygonInterior ) ) ).toUInt() ) );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontTransp ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::FontOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::FontTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::FontTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::BufferTransp ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::BufferOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::BufferTransp ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::BufferTransp, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeTransparency ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShapeOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::ShapeTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShapeTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShadowTransparency ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShadowOpacity, QgsProperty::fromExpression( QStringLiteral( "100 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::ShadowTransparency ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::ShadowTransparency, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Rotation ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::LabelRotation, QgsProperty::fromExpression( QStringLiteral( "360 - (%1)" ).arg( mDataDefinedProperties.property( QgsPalLayerSettings::Property::Rotation ).asExpression() ) ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::Rotation, QgsProperty() );
  }
  // older 2.x projects had min/max scale flipped - so change them here.
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MinScale ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MaximumScale, mDataDefinedProperties.property( QgsPalLayerSettings::Property::MinScale ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MinScale, QgsProperty() );
  }
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MaxScale ) )
  {
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MinimumScale, mDataDefinedProperties.property( QgsPalLayerSettings::Property::MaxScale ) );
    mDataDefinedProperties.setProperty( QgsPalLayerSettings::Property::MaxScale, QgsProperty() );
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
  placementElem.setAttribute( QStringLiteral( "placement" ), static_cast< int >( placement ) );
  placementElem.setAttribute( QStringLiteral( "polygonPlacementFlags" ), static_cast< int >( mPolygonPlacementFlags ) );
  placementElem.setAttribute( QStringLiteral( "placementFlags" ), static_cast< unsigned int >( mLineSettings.placementFlags() ) );
  placementElem.setAttribute( QStringLiteral( "centroidWhole" ), centroidWhole );
  placementElem.setAttribute( QStringLiteral( "centroidInside" ), centroidInside );
  placementElem.setAttribute( QStringLiteral( "predefinedPositionOrder" ), QgsLabelingUtils::encodePredefinedPositionOrder( mPointSettings.predefinedPositionOrder() ) );
  placementElem.setAttribute( QStringLiteral( "fitInPolygonOnly" ), fitInPolygonOnly );
  placementElem.setAttribute( QStringLiteral( "dist" ), dist );
  placementElem.setAttribute( QStringLiteral( "distUnits" ), QgsUnitTypes::encodeUnit( distUnits ) );
  placementElem.setAttribute( QStringLiteral( "distMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( distMapUnitScale ) );
  placementElem.setAttribute( QStringLiteral( "offsetType" ), static_cast< unsigned int >( offsetType ) );
  placementElem.setAttribute( QStringLiteral( "quadOffset" ), static_cast< unsigned int >( mPointSettings.quadrant() ) );
  placementElem.setAttribute( QStringLiteral( "xOffset" ), xOffset );
  placementElem.setAttribute( QStringLiteral( "yOffset" ), yOffset );
  placementElem.setAttribute( QStringLiteral( "offsetUnits" ), QgsUnitTypes::encodeUnit( offsetUnits ) );
  placementElem.setAttribute( QStringLiteral( "labelOffsetMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( labelOffsetMapUnitScale ) );
  placementElem.setAttribute( QStringLiteral( "rotationAngle" ), angleOffset );
  placementElem.setAttribute( QStringLiteral( "preserveRotation" ), preserveRotation );
  {
    // append Angle prefix to maintain compatibility with QGIS < 3.30
    const QString rotationUnitString = QStringLiteral( "Angle" ) + qgsEnumValueToKey( mRotationUnit );
    placementElem.setAttribute( QStringLiteral( "rotationUnit" ), rotationUnitString );
  }
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

  placementElem.setAttribute( QStringLiteral( "maximumDistance" ), mPointSettings.maximumDistance() );
  placementElem.setAttribute( QStringLiteral( "maximumDistanceUnit" ), QgsUnitTypes::encodeUnit( mPointSettings.maximumDistanceUnit() ) );
  placementElem.setAttribute( QStringLiteral( "maximumDistanceMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mPointSettings.maximumDistanceMapUnitScale() ) );

  placementElem.setAttribute( QStringLiteral( "geometryGenerator" ), geometryGenerator );
  placementElem.setAttribute( QStringLiteral( "geometryGeneratorEnabled" ), geometryGeneratorEnabled );
  placementElem.setAttribute( QStringLiteral( "geometryGeneratorType" ), qgsEnumValueToKey( geometryGeneratorType ) + QStringLiteral( "Geometry" ) );

  placementElem.setAttribute( QStringLiteral( "layerType" ), qgsEnumValueToKey( layerType ) + QStringLiteral( "Geometry" ) );

  placementElem.setAttribute( QStringLiteral( "overlapHandling" ), qgsEnumValueToKey( mPlacementSettings.overlapHandling() ) );
  placementElem.setAttribute( QStringLiteral( "prioritization" ), qgsEnumValueToKey( mPlacementSettings.prioritization() ) );
  placementElem.setAttribute( QStringLiteral( "allowDegraded" ), mPlacementSettings.allowDegradedPlacement() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  // rendering
  QDomElement renderingElem = doc.createElement( QStringLiteral( "rendering" ) );
  renderingElem.setAttribute( QStringLiteral( "drawLabels" ), drawLabels );
  renderingElem.setAttribute( QStringLiteral( "scaleVisibility" ), scaleVisibility );
  renderingElem.setAttribute( QStringLiteral( "scaleMin" ), maximumScale );
  renderingElem.setAttribute( QStringLiteral( "scaleMax" ), minimumScale );
  renderingElem.setAttribute( QStringLiteral( "fontLimitPixelSize" ), fontLimitPixelSize );
  renderingElem.setAttribute( QStringLiteral( "fontMinPixelSize" ), fontMinPixelSize );
  renderingElem.setAttribute( QStringLiteral( "fontMaxPixelSize" ), fontMaxPixelSize );
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

QPixmap QgsPalLayerSettings::labelSettingsPreviewPixmap( const QgsPalLayerSettings &settings, QSize size, const QString &previewText, int padding, const QgsScreenProperties &screen )
{
  const double devicePixelRatio = screen.isValid() ? screen.devicePixelRatio() : 1;

  // for now, just use format
  QgsTextFormat tempFormat = settings.format();
  QPixmap pixmap( size * devicePixelRatio );
  pixmap.fill( Qt::transparent );
  pixmap.setDevicePixelRatio( devicePixelRatio );
  QPainter painter;
  painter.begin( &pixmap );

  painter.setRenderHint( QPainter::Antialiasing );

  const QRectF rect( 0, 0, size.width(), size.height() );

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

  if ( screen.isValid() )
  {
    screen.updateRenderContextForScreen( context );
  }
  else
  {
    QWidget *activeWindow = QApplication::activeWindow();
    if ( QScreen *screen = activeWindow ? activeWindow->screen() : nullptr )
    {
      context.setScaleFactor( screen->physicalDotsPerInch() / 25.4 );
      context.setDevicePixelRatio( screen->devicePixelRatio() );
    }
    else
    {
      context.setScaleFactor( 96.0 / 25.4 );
      context.setDevicePixelRatio( 1.0 );
    }
  }

  context.setUseAdvancedEffects( true );
  context.setPainter( &painter );

  // slightly inset text to account for buffer/background
  const double fontSize = context.convertToPainterUnits( tempFormat.size(), tempFormat.sizeUnit(), tempFormat.sizeMapUnitScale() );
  double xtrans = 0;
  if ( tempFormat.buffer().enabled() )
    xtrans = tempFormat.buffer().sizeUnit() == Qgis::RenderUnit::Percentage
             ? fontSize * tempFormat.buffer().size() / 100
             : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() );
  if ( tempFormat.background().enabled() && tempFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat.background().size().width(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  double ytrans = 0.0;
  if ( tempFormat.buffer().enabled() )
    ytrans = std::max( ytrans, tempFormat.buffer().sizeUnit() == Qgis::RenderUnit::Percentage
                       ? fontSize * tempFormat.buffer().size() / 100
                       : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() ) );
  if ( tempFormat.background().enabled() )
    ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.background().size().height(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  const QStringList text = QStringList() << ( previewText.isEmpty() ? settings.legendString() : previewText );
  const double textHeight = QgsTextRenderer::textHeight( context, tempFormat, text, Qgis::TextLayoutMode::Rectangle );
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

  QgsTextRenderer::drawText( textRect, 0, Qgis::TextHorizontalAlignment::Center, text, context, tempFormat );

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

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF *fm, const QString &text, double &labelX, double &labelY, const QgsFeature *f, QgsRenderContext *context, double *rotatedLabelX, double *rotatedLabelY )
{
  // NOTE: This whole method is deprecated is only kept for 3.x API stability.
  // There is no requirement to update the logic here, just leave it as is till we can completely remove for 4.0
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
  Qgis::TextOrientation orientation = mFormat.orientation();

  bool addDirSymb = mLineSettings.addDirectionSymbol();
  QString leftDirSymb = mLineSettings.leftDirectionSymbol();
  QString rightDirSymb = mLineSettings.rightDirectionSymbol();
  QgsLabelLineSettings::DirectionSymbolPlacement placeDirSymb = mLineSettings.directionSymbolPlacement();

  if ( f == mCurFeat ) // called internally, use any stored data defined values
  {
    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::MultiLineWrapChar ) )
    {
      wrapchr = dataDefinedValues.value( QgsPalLayerSettings::Property::MultiLineWrapChar ).toString();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::AutoWrapLength ) )
    {
      evalAutoWrapLength = dataDefinedValues.value( QgsPalLayerSettings::Property::AutoWrapLength, evalAutoWrapLength ).toInt();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::MultiLineHeight ) )
    {
      multilineH = dataDefinedValues.value( QgsPalLayerSettings::Property::MultiLineHeight ).toDouble();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::TextOrientation ) )
    {
      orientation = QgsTextRendererUtils::decodeTextOrientation( dataDefinedValues.value( QgsPalLayerSettings::Property::TextOrientation ).toString() );
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbDraw ) )
    {
      addDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbDraw ).toBool();
    }

    if ( addDirSymb )
    {

      if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbLeft ) )
      {
        leftDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbLeft ).toString();
      }
      if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbRight ) )
      {
        rightDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbRight ).toString();
      }

      if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbPlacement ) )
      {
        placeDirSymb = static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbPlacement ).toInt() );
      }

    }

  }
  else // called externally with passed-in feature, evaluate data defined
  {
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MultiLineWrapChar ) )
    {
      rc->expressionContext().setOriginalValueVariable( wrapChar );
      wrapchr = mDataDefinedProperties.value( QgsPalLayerSettings::Property::MultiLineWrapChar, rc->expressionContext(), wrapchr ).toString();
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::AutoWrapLength ) )
    {
      rc->expressionContext().setOriginalValueVariable( evalAutoWrapLength );
      evalAutoWrapLength = mDataDefinedProperties.value( QgsPalLayerSettings::Property::AutoWrapLength, rc->expressionContext(), evalAutoWrapLength ).toInt();
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MultiLineHeight ) )
    {
      rc->expressionContext().setOriginalValueVariable( multilineH );
      multilineH = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::MultiLineHeight, rc->expressionContext(), multilineH );
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::TextOrientation ) )
    {
      QString encoded = QgsTextRendererUtils::encodeTextOrientation( orientation );
      rc->expressionContext().setOriginalValueVariable( encoded );
      orientation = QgsTextRendererUtils::decodeTextOrientation( mDataDefinedProperties.valueAsString( QgsPalLayerSettings::Property::TextOrientation, rc->expressionContext(), encoded ) );
    }

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::DirSymbDraw ) )
    {
      rc->expressionContext().setOriginalValueVariable( addDirSymb );
      addDirSymb = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::DirSymbDraw, rc->expressionContext(), addDirSymb );
    }

    if ( addDirSymb ) // don't do extra evaluations if not adding a direction symbol
    {
      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::DirSymbLeft ) )
      {
        rc->expressionContext().setOriginalValueVariable( leftDirSymb );
        leftDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::Property::DirSymbLeft, rc->expressionContext(), leftDirSymb ).toString();
      }

      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::DirSymbRight ) )
      {
        rc->expressionContext().setOriginalValueVariable( rightDirSymb );
        rightDirSymb = mDataDefinedProperties.value( QgsPalLayerSettings::Property::DirSymbRight, rc->expressionContext(), rightDirSymb ).toString();
      }

      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::DirSymbPlacement ) )
      {
        rc->expressionContext().setOriginalValueVariable( static_cast< int >( placeDirSymb ) );
        placeDirSymb = static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::Property::DirSymbPlacement, rc->expressionContext(), static_cast< int >( placeDirSymb ) ) );
      }
    }
  }

  if ( wrapchr.isEmpty() )
  {
    wrapchr = QStringLiteral( "\n" ); // default to new line delimiter
  }

  //consider the space needed for the direction symbol
  if ( addDirSymb && placement == Qgis::LabelPlacement::Line
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

  const QStringList multiLineSplit = QgsPalLabeling::splitToLines( textCopy, wrapchr, evalAutoWrapLength, useMaxLineLengthForAutoWrap );
  const int lines = multiLineSplit.size();

  const double lineHeightPainterUnits = rc->convertToPainterUnits( mFormat.lineHeight(), mFormat.lineHeightUnit() );

  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
    {
      h += fm->height() + static_cast< double >( ( lines - 1 ) * ( mFormat.lineHeightUnit() == Qgis::RenderUnit::Percentage ? ( labelHeight * multilineH ) : lineHeightPainterUnits ) );

      for ( const QString &line : std::as_const( multiLineSplit ) )
      {
        w = std::max( w, fm->horizontalAdvance( line ) );
      }
      break;
    }

    case Qgis::TextOrientation::Vertical:
    {
      double letterSpacing = mFormat.scaledFont( *context ).letterSpacing();
      double labelWidth = fm->maxWidth();
      w = labelWidth + ( lines - 1 ) * ( mFormat.lineHeightUnit() == Qgis::RenderUnit::Percentage ? ( labelWidth * multilineH ) : lineHeightPainterUnits );

      int maxLineLength = 0;
      for ( const QString &line : std::as_const( multiLineSplit ) )
      {
        maxLineLength = std::max( maxLineLength, static_cast<int>( line.length() ) );
      }
      h = fm->ascent() * maxLineLength + ( maxLineLength - 1 ) * letterSpacing;
      break;
    }

    case Qgis::TextOrientation::RotationBased:
    {
      double widthHorizontal = 0.0;
      for ( const QString &line : std::as_const( multiLineSplit ) )
      {
        widthHorizontal = std::max( w, fm->horizontalAdvance( line ) );
      }

      double widthVertical = 0.0;
      double letterSpacing = mFormat.scaledFont( *context ).letterSpacing();
      double labelWidth = fm->maxWidth();
      widthVertical = labelWidth + ( lines - 1 ) * ( mFormat.lineHeightUnit() == Qgis::RenderUnit::Percentage ? ( labelWidth * multilineH ) : lineHeightPainterUnits );

      double heightHorizontal = 0.0;
      heightHorizontal += fm->height() + static_cast< double >( ( lines - 1 ) * ( mFormat.lineHeightUnit() == Qgis::RenderUnit::Percentage ? ( labelHeight * multilineH ) : lineHeightPainterUnits ) );

      double heightVertical = 0.0;
      int maxLineLength = 0;
      for ( const QString &line : std::as_const( multiLineSplit ) )
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

  double uPP = xform->mapUnitsPerPixel();
  labelX = w * uPP;
  labelY = h * uPP;
  if ( rotatedLabelX && rotatedLabelY )
  {
    *rotatedLabelX = rw * uPP;
    *rotatedLabelY = rh * uPP;
  }
}

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF &fm, const QString &text, QgsRenderContext &context, const QgsTextFormat &format, QgsTextDocument *document, QgsTextDocumentMetrics *documentMetrics, QSizeF &size, QSizeF &rotatedSize, QRectF &outerBounds )
{
  if ( !mCurFeat )
  {
    return;
  }

  QString wrapchr = wrapChar;
  int evalAutoWrapLength = autoWrapLength;
  Qgis::TextOrientation orientation = format.orientation();

  bool addDirSymb = mLineSettings.addDirectionSymbol();
  QString leftDirSymb = mLineSettings.leftDirectionSymbol();
  QString rightDirSymb = mLineSettings.rightDirectionSymbol();
  QgsLabelLineSettings::DirectionSymbolPlacement placeDirSymb = mLineSettings.directionSymbolPlacement();
  double multilineH = mFormat.lineHeight();

  if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::MultiLineWrapChar ) )
  {
    wrapchr = dataDefinedValues.value( QgsPalLayerSettings::Property::MultiLineWrapChar ).toString();
  }

  if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::AutoWrapLength ) )
  {
    evalAutoWrapLength = dataDefinedValues.value( QgsPalLayerSettings::Property::AutoWrapLength, evalAutoWrapLength ).toInt();
  }

  if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::TextOrientation ) )
  {
    orientation = QgsTextRendererUtils::decodeTextOrientation( dataDefinedValues.value( QgsPalLayerSettings::Property::TextOrientation ).toString() );
  }

  if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::MultiLineHeight ) )
  {
    multilineH = dataDefinedValues.value( QgsPalLayerSettings::Property::MultiLineHeight ).toDouble();
  }

  if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbDraw ) )
  {
    addDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbDraw ).toBool();
  }

  if ( addDirSymb )
  {

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbLeft ) )
    {
      leftDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbLeft ).toString();
    }
    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbRight ) )
    {
      rightDirSymb = dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbRight ).toString();
    }

    if ( dataDefinedValues.contains( QgsPalLayerSettings::Property::DirSymbPlacement ) )
    {
      placeDirSymb = static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( dataDefinedValues.value( QgsPalLayerSettings::Property::DirSymbPlacement ).toInt() );
    }
  }

  if ( wrapchr.isEmpty() )
  {
    wrapchr = QStringLiteral( "\n" ); // default to new line delimiter
  }

  const double lineHeightPainterUnits = context.convertToPainterUnits( mFormat.lineHeight(), mFormat.lineHeightUnit() );

  //consider the space needed for the direction symbol
  QSizeF maximumExtraSpaceAllowance( 0, 0 );
  QSizeF minimumSize( 0, 0 );
  if ( addDirSymb && placement == Qgis::LabelPlacement::Line
       && ( !leftDirSymb.isEmpty() || !rightDirSymb.isEmpty() ) )
  {
    // we don't know which symbol we'll be rendering yet, so just assume the worst and that
    // we'll be rendering the larger one
    const QString dirSym = fm.horizontalAdvance( rightDirSymb ) > fm.horizontalAdvance( leftDirSymb )
                           ? rightDirSymb : leftDirSymb;

    switch ( placeDirSymb )
    {
      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight:
        maximumExtraSpaceAllowance = QSizeF( fm.horizontalAdvance( dirSym ), 0 );
        break;

      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove:
      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow:
        maximumExtraSpaceAllowance = QSizeF( 0, ( mFormat.lineHeightUnit() == Qgis::RenderUnit::Percentage ? ( ( fm.ascent() + fm.descent() ) * multilineH ) : lineHeightPainterUnits ) );
        minimumSize = QSizeF( fm.horizontalAdvance( dirSym ), 0 );
        break;
    }
  }

  double w = 0.0;
  double h = 0.0;
  double rw = 0.0;
  double rh = 0.0;

  if ( document )
  {
    document->splitLines( wrapchr, evalAutoWrapLength, useMaxLineLengthForAutoWrap );

    *documentMetrics = QgsTextDocumentMetrics::calculateMetrics( *document, format, context );
    const QSizeF size = documentMetrics->documentSize( Qgis::TextLayoutMode::Labeling, orientation != Qgis::TextOrientation::RotationBased ? orientation : Qgis::TextOrientation::Horizontal );
    w = std::max( minimumSize.width(), size.width() + maximumExtraSpaceAllowance.width() );
    h = std::max( minimumSize.height(), size.height() + maximumExtraSpaceAllowance.height() );

    if ( orientation == Qgis::TextOrientation::RotationBased )
    {
      const QSizeF rotatedSize = documentMetrics->documentSize( Qgis::TextLayoutMode::Labeling, Qgis::TextOrientation::Vertical );
      rh = std::max( minimumSize.width(), rotatedSize.width() + maximumExtraSpaceAllowance.width() );
      rw = std::max( minimumSize.height(), rotatedSize.height() + maximumExtraSpaceAllowance.height() );
    }
  }
  else
  {
    // this branch is ONLY hit if we are using curved labels without HTML formatting, as otherwise we're always using the document!
    // so here we have certain assumptions which apply to curved labels only:
    // - orientation is ignored
    // - labels are single lines only
    // - line direction symbol are (currently!) not supported (see https://github.com/qgis/QGIS/issues/14968 )

    h = fm.height();
    w = fm.horizontalAdvance( text );
  }

  const double uPP = xform->mapUnitsPerPixel();
  size = QSizeF( w * uPP, h * uPP );
  rotatedSize = QSizeF( rw * uPP, rh * uPP );

  if ( documentMetrics )
  {
    // TODO -- does this need to account for maximumExtraSpaceAllowance / minimumSize ? Right now the size
    // of line direction symbols will be ignored
    const QRectF outerBoundsPixels = documentMetrics->outerBounds( Qgis::TextLayoutMode::Labeling, orientation );

    outerBounds = QRectF( outerBoundsPixels.left() * uPP,
                          outerBoundsPixels.top() * uPP,
                          outerBoundsPixels.width() * uPP,
                          outerBoundsPixels.height() * uPP );
  }
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::IsObstacle ) )
    isObstacle = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::IsObstacle, context.expressionContext(), isObstacle ); // default to layer default

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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Show ) )
  {
    context.expressionContext().setOriginalValueVariable( true );
    if ( !mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::Show, context.expressionContext(), true ) )
    {
      return nullptr;
    }
  }

  // data defined scale visibility?
  bool useScaleVisibility = scaleVisibility;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ScaleVisibility ) )
    useScaleVisibility = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::ScaleVisibility, context.expressionContext(), scaleVisibility );

  if ( useScaleVisibility )
  {
    // data defined min scale?
    double maxScale = maximumScale;
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MaximumScale ) )
    {
      context.expressionContext().setOriginalValueVariable( maximumScale );
      maxScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::MaximumScale, context.expressionContext(), maxScale );
    }

    // scales closer than 1:1
    if ( maxScale < 0 )
    {
      maxScale = 1 / std::fabs( maxScale );
    }

    // maxScale is inclusive ( < --> no label )
    if ( !qgsDoubleNear( maxScale, 0.0 ) && QgsScaleUtils::lessThanMaximumScale( context.rendererScale(), maxScale ) )
    {
      return nullptr;
    }

    // data defined min scale?
    double minScale = minimumScale;
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MinimumScale ) )
    {
      context.expressionContext().setOriginalValueVariable( minimumScale );
      minScale = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::MinimumScale, context.expressionContext(), minScale );
    }

    // scales closer than 1:1
    if ( minScale < 0 )
    {
      minScale = 1 / std::fabs( minScale );
    }

    // minScale is exclusive ( >= --> no label )
    if ( !qgsDoubleNear( minScale, 0.0 ) && QgsScaleUtils::equalToOrGreaterThanMinimumScale( context.rendererScale(), minScale ) )
    {
      return nullptr;
    }
  }

  QgsTextFormat evaluatedFormat = mFormat;

  QFont labelFont = evaluatedFormat.font();
  // labelFont will be added to label feature for use during label painting

  // data defined font units?
  Qgis::RenderUnit fontunits = evaluatedFormat.sizeUnit();
  exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::FontSizeUnit, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        fontunits = res;
    }
  }

  //data defined label size?
  double fontSize = evaluatedFormat.size();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Size ) )
  {
    context.expressionContext().setOriginalValueVariable( fontSize );
    fontSize = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::Size, context.expressionContext(), fontSize );
  }
  if ( fontSize <= 0.0 )
  {
    return nullptr;
  }

  int fontPixelSize = QgsTextRenderer::sizeToPixel( fontSize, context, fontunits, evaluatedFormat.sizeMapUnitScale() );
  // don't try to show font sizes less than 1 pixel (Qt complains)
  if ( fontPixelSize < 1 )
  {
    return nullptr;
  }
  labelFont.setPixelSize( fontPixelSize );

  // NOTE: labelFont now always has pixelSize set, so pointSize or pointSizeF might return -1

  // defined 'minimum/maximum pixel font size'?
  if ( fontunits == Qgis::RenderUnit::MapUnits )
  {
    if ( mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::FontLimitPixel, context.expressionContext(), fontLimitPixelSize ) )
    {
      int fontMinPixel = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::Property::FontMinPixel, context.expressionContext(), fontMinPixelSize );
      int fontMaxPixel = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::Property::FontMaxPixel, context.expressionContext(), fontMaxPixelSize );

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

  evaluatedFormat.setFont( labelFont );
  // undo scaling by symbology reference scale, as this would have been applied in the previous call to QgsTextRenderer::sizeToPixel and we risk
  // double-applying it if we don't re-adjust, since all the text format metric calculations assume an unscaled format font size is present
  const double symbologyReferenceScaleFactor = context.symbologyReferenceScale() > 0 ? context.symbologyReferenceScale() / context.rendererScale() : 1;
  evaluatedFormat.setSize( labelFont.pixelSize() / symbologyReferenceScaleFactor );
  evaluatedFormat.setSizeUnit( Qgis::RenderUnit::Pixels );

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
    labelText = QgsVariantUtils::isNull( result ) ? QString() : result.toString();
  }
  else
  {
    const QVariant &v = feature.attribute( fieldIndex );
    labelText = QgsVariantUtils::isNull( v ) ? QString() : v.toString();
  }

  // apply text replacements
  if ( useSubstitutions )
  {
    labelText = substitutions.process( labelText );
  }

  // apply capitalization
  Qgis::Capitalization capitalization = evaluatedFormat.capitalization();
  // maintain API - capitalization may have been set in textFont
  if ( capitalization == Qgis::Capitalization::MixedCase && mFormat.font().capitalization() != QFont::MixedCase )
  {
    capitalization = static_cast< Qgis::Capitalization >( mFormat.font().capitalization() );
  }
  // data defined font capitalization?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontCase ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::FontCase, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::NumFormat ) )
  {
    evalFormatNumbers = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::NumFormat, context.expressionContext(), evalFormatNumbers );
  }
  if ( evalFormatNumbers )
  {
    // data defined decimal places?
    int decimalPlaces = mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::Property::NumDecimals, context.expressionContext(), decimals );
    if ( decimalPlaces <= 0 ) // needs to be positive
      decimalPlaces = decimals;

    // data defined plus sign?
    bool signPlus = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::NumPlusSign, context.expressionContext(), plusSign );

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
      labelText = numberFormat.arg( QLocale().toString( d, 'f', decimalPlaces ) );
    }
  }

  // NOTE: this should come AFTER any option that affects font metrics
  const QFontMetricsF labelFontMetrics( labelFont );
  QSizeF labelSize;
  QSizeF rotatedSize;

  QgsTextDocument doc;
  QgsTextDocumentMetrics documentMetrics;
  QRectF outerBounds;

  switch ( placement )
  {
    case Qgis::LabelPlacement::PerimeterCurved:
    case Qgis::LabelPlacement::Curved:
    {
      // avoid calculating document and metrics if we don't require them for curved labels
      if ( evaluatedFormat.allowHtmlFormatting() && !labelText.isEmpty() )
      {
        doc = QgsTextDocument::fromHtml( QStringList() << labelText );
        calculateLabelSize( labelFontMetrics, labelText, context, evaluatedFormat, &doc, &documentMetrics, labelSize, rotatedSize, outerBounds );
      }
      else
      {
        calculateLabelSize( labelFontMetrics, labelText, context, evaluatedFormat, nullptr, nullptr, labelSize, rotatedSize, outerBounds );
      }
      break;
    }

    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    case Qgis::LabelPlacement::OutsidePolygons:
    {
      // non-curved labels always require document and metrics
      doc = QgsTextDocument::fromTextAndFormat( {labelText }, evaluatedFormat );
      calculateLabelSize( labelFontMetrics, labelText, context, evaluatedFormat, &doc, &documentMetrics, labelSize, rotatedSize, outerBounds );
      break;
    }
  }

  // maximum angle between curved label characters (hardcoded defaults used in QGIS <2.0)
  //
  double maxcharanglein = 20.0; // range 20.0-60.0
  double maxcharangleout = -20.0; // range 20.0-95.0

  switch ( placement )
  {
    case Qgis::LabelPlacement::PerimeterCurved:
    case Qgis::LabelPlacement::Curved:
    {
      maxcharanglein = maxCurvedCharAngleIn;
      maxcharangleout = maxCurvedCharAngleOut;

      //data defined maximum angle between curved label characters?
      if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::CurvedCharAngleInOut ) )
      {
        exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::CurvedCharAngleInOut, context.expressionContext() );
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
      break;
    }

    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    case Qgis::LabelPlacement::OutsidePolygons:
      break;
  }

  // data defined centroid whole or clipped?
  bool wholeCentroid = centroidWhole;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::CentroidWhole ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::CentroidWhole, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
  if ( simplifyMethod.simplifyHints() != Qgis::VectorRenderingSimplificationFlags( Qgis::VectorRenderingSimplificationFlag::NoSimplification ) && simplifyMethod.forceLocalOptimization() )
  {
    unsigned int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    const Qgis::VectorSimplificationAlgorithm simplifyAlgorithm = simplifyMethod.simplifyAlgorithm();
    QgsMapToPixelSimplifier simplifier( simplifyHints, simplifyMethod.tolerance(), simplifyAlgorithm );
    geom = simplifier.simplify( geom );
  }

  if ( !context.featureClipGeometry().isEmpty() )
  {
    const Qgis::GeometryType expectedType = geom.type();
    geom = geom.intersection( context.featureClipGeometry() );
    geom.convertGeometryCollectionToSubclass( expectedType );
  }

  // whether we're going to create a centroid for polygon
  bool centroidPoly = ( ( placement == Qgis::LabelPlacement::AroundPoint
                          || placement == Qgis::LabelPlacement::OverPoint )
                        && geom.type() == Qgis::GeometryType::Polygon );

  // CLIP the geometry if it is bigger than the extent
  // don't clip if centroid is requested for whole feature
  bool doClip = false;
  if ( !centroidPoly || !wholeCentroid )
  {
    doClip = true;
  }


  Qgis::LabelPolygonPlacementFlags polygonPlacement = mPolygonPlacementFlags;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::PolygonLabelOutside ) )
  {
    const QVariant dataDefinedOutside = mDataDefinedProperties.value( QgsPalLayerSettings::Property::PolygonLabelOutside, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( dataDefinedOutside ) )
    {
      if ( dataDefinedOutside.userType() == QMetaType::Type::QString )
      {
        const QString value = dataDefinedOutside.toString().trimmed();
        if ( value.compare( QLatin1String( "force" ), Qt::CaseInsensitive ) == 0 )
        {
          // forced outside placement -- remove inside flag, add outside flag
          polygonPlacement &= ~static_cast< int >( Qgis::LabelPolygonPlacementFlag::AllowPlacementInsideOfPolygon );
          polygonPlacement |= Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else if ( value.compare( QLatin1String( "yes" ), Qt::CaseInsensitive ) == 0 )
        {
          // permit outside placement
          polygonPlacement |= Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else if ( value.compare( QLatin1String( "no" ), Qt::CaseInsensitive ) == 0 )
        {
          // block outside placement
          polygonPlacement &= ~static_cast< int >( Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon );
        }
      }
      else
      {
        if ( dataDefinedOutside.toBool() )
        {
          // permit outside placement
          polygonPlacement |= Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        }
        else
        {
          // block outside placement
          polygonPlacement &= ~static_cast< int >( Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon );
        }
      }
    }
  }

  QgsLabelLineSettings lineSettings = mLineSettings;
  lineSettings.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );

  QgsLabelPointSettings pointSettings = mPointSettings;
  pointSettings.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );

  if ( geom.type() == Qgis::GeometryType::Line || placement == Qgis::LabelPlacement::Line || placement == Qgis::LabelPlacement::PerimeterCurved )
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
  if ( geom.type() == Qgis::GeometryType::Polygon && ( fitInPolygonOnly || polygonPlacement & Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon ) )
  {
    permissibleZone = geom;
    if ( QgsPalLabeling::geometryRequiresPreparation( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() ) )
    {
      permissibleZone = QgsPalLabeling::prepareGeometry( permissibleZone, context, ct, doClip ? extentGeom : QgsGeometry(), lineSettings.mergeLines() );
    }
  }

  // if using perimeter based labeling for polygons, get the polygon's
  // linear boundary and use that for the label geometry
  if ( ( geom.type() == Qgis::GeometryType::Polygon )
       && ( placement == Qgis::LabelPlacement::Line || placement == Qgis::LabelPlacement::PerimeterCurved ) )
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
  geos_geom_clone = QgsGeos::asGeos( geom, 0, Qgis::GeosCreationFlag::SkipEmptyInteriorRings );

  if ( isObstacle || ( geom.type() == Qgis::GeometryType::Point && offsetType == Qgis::LabelOffsetType::FromSymbolBounds ) )
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
    if ( geom.type() == Qgis::GeometryType::Line && lineSettings.mergeLines() )
    {
      minimumSize = context.convertToMapUnits( featureThinningSettings.minimumFeatureSize(), Qgis::RenderUnit::Millimeters );
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
  double xPos = 0.0, yPos = 0.0;
  double angleInRadians = 0.0;
  double quadOffsetX = 0.0, quadOffsetY = 0.0;
  double offsetX = 0.0, offsetY = 0.0;
  QgsPointXY anchorPosition;

  if ( placement == Qgis::LabelPlacement::OverPoint )
  {
    anchorPosition = geom.centroid().asPoint();
  }
  //x/y shift in case of alignment
  double xdiff = 0.0;
  double ydiff = 0.0;

  //data defined quadrant offset?
  bool ddFixedQuad = false;
  Qgis::LabelQuadrantPosition quadOff = pointSettings.quadrant();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::OffsetQuad ) )
  {
    context.expressionContext().setOriginalValueVariable( static_cast< int >( quadOff ) );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::OffsetQuad, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      bool ok;
      int quadInt = exprVal.toInt( &ok );
      if ( ok && 0 <= quadInt && quadInt <= 8 )
      {
        quadOff = static_cast< Qgis::LabelQuadrantPosition >( quadInt );
        ddFixedQuad = true;
      }
    }
  }

  // adjust quadrant offset of labels
  switch ( quadOff )
  {
    case Qgis::LabelQuadrantPosition::AboveLeft:
      quadOffsetX = -1.0;
      quadOffsetY = 1.0;
      break;
    case Qgis::LabelQuadrantPosition::Above:
      quadOffsetX = 0.0;
      quadOffsetY = 1.0;
      break;
    case Qgis::LabelQuadrantPosition::AboveRight:
      quadOffsetX = 1.0;
      quadOffsetY = 1.0;
      break;
    case Qgis::LabelQuadrantPosition::Left:
      quadOffsetX = -1.0;
      quadOffsetY = 0.0;
      break;
    case Qgis::LabelQuadrantPosition::Right:
      quadOffsetX = 1.0;
      quadOffsetY = 0.0;
      break;
    case Qgis::LabelQuadrantPosition::BelowLeft:
      quadOffsetX = -1.0;
      quadOffsetY = -1.0;
      break;
    case Qgis::LabelQuadrantPosition::Below:
      quadOffsetX = 0.0;
      quadOffsetY = -1.0;
      break;
    case Qgis::LabelQuadrantPosition::BelowRight:
      quadOffsetX = 1.0;
      quadOffsetY = -1.0;
      break;
    case Qgis::LabelQuadrantPosition::Over:
      break;
  }

  //data defined label offset?
  double xOff = xOffset;
  double yOff = yOffset;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::OffsetXY ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( QPointF( xOffset, yOffset ) ) );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::OffsetXY, context.expressionContext() );
    bool ok = false;
    const QPointF ddOffPt = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      xOff = ddOffPt.x();
      yOff = ddOffPt.y();
    }
  }

  // data defined label offset units?
  Qgis::RenderUnit offUnit = offsetUnits;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::OffsetUnits ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::OffsetUnits, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString units = exprVal.toString().trimmed();
      if ( !units.isEmpty() )
      {
        bool ok = false;
        Qgis::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
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
    angleInRadians = ( 360 - angleOffset ) * M_PI / 180; // convert to radians counterclockwise
  }

  const QgsMapToPixel &m2p = context.mapToPixel();
  //data defined rotation?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::LabelRotation ) )
  {
    context.expressionContext().setOriginalValueVariable( angleOffset );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::LabelRotation, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      bool ok;
      const double rotation = exprVal.toDouble( &ok );
      if ( ok )
      {
        dataDefinedRotation = true;

        double rotationDegrees = rotation * QgsUnitTypes::fromUnitToUnitFactor( mRotationUnit,
                                 Qgis::AngleUnit::Degrees );

        // TODO: add setting to disable having data defined rotation follow
        //       map rotation ?
        rotationDegrees += m2p.mapRotation();
        angleInRadians = ( 360 - rotationDegrees ) * M_PI / 180.0;
      }
    }
  }

  bool hasDataDefinedPosition = false;
  {
    bool ddPosition = false;

    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::PositionX )
         && mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::PositionY ) )
    {
      const QVariant xPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::Property::PositionX, context.expressionContext() );
      const QVariant yPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::Property::PositionY, context.expressionContext() );
      if ( !QgsVariantUtils::isNull( xPosProperty )
           && !QgsVariantUtils::isNull( yPosProperty ) )
      {
        ddPosition = true;

        bool ddXPos = false, ddYPos = false;
        xPos = xPosProperty.toDouble( &ddXPos );
        yPos = yPosProperty.toDouble( &ddYPos );
        if ( ddXPos && ddYPos )
          hasDataDefinedPosition = true;
      }
    }
    else if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::PositionPoint ) )
    {
      const QVariant pointPosProperty = mDataDefinedProperties.value( QgsPalLayerSettings::Property::PositionPoint, context.expressionContext() );
      if ( !QgsVariantUtils::isNull( pointPosProperty ) )
      {
        ddPosition = true;

        QgsPoint point;
        if ( pointPosProperty.userType() == qMetaTypeId<QgsReferencedGeometry>() )
        {
          QgsReferencedGeometry referencedGeometryPoint = pointPosProperty.value<QgsReferencedGeometry>();
          point = QgsPoint( referencedGeometryPoint.asPoint() );

          if ( !referencedGeometryPoint.isNull()
               && ct.sourceCrs() != referencedGeometryPoint.crs() )
            QgsMessageLog::logMessage( QObject::tr( "Label position geometry is not in layer coordinates reference system. Layer CRS: '%1', Geometry CRS: '%2'" ).arg( ct.sourceCrs().userFriendlyIdentifier(), referencedGeometryPoint.crs().userFriendlyIdentifier() ), QObject::tr( "Labeling" ), Qgis::Warning );
        }
        else if ( pointPosProperty.userType() == qMetaTypeId< QgsGeometry>() )
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
          angleInRadians = 0.0;
        }

        //horizontal alignment
        if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Hali ) )
        {
          exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::Hali, context.expressionContext() );
          if ( !QgsVariantUtils::isNull( exprVal ) )
          {
            QString haliString = exprVal.toString();
            if ( haliString.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
            {
              xdiff -= labelSize.width() / 2.0;
            }
            else if ( haliString.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
            {
              xdiff -= labelSize.width();
            }
          }
        }

        //vertical alignment
        if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Vali ) )
        {
          exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::Vali, context.expressionContext() );
          if ( !QgsVariantUtils::isNull( exprVal ) )
          {
            QString valiString = exprVal.toString();
            if ( valiString.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( valiString.compare( QLatin1String( "Top" ), Qt::CaseInsensitive ) == 0 )
              {
                ydiff -= labelSize.height();
              }
              else
              {
                double descentRatio = labelFontMetrics.descent() / labelFontMetrics.height();
                if ( valiString.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
                {
                  ydiff -= labelSize.height() * descentRatio;
                }
                else //'Cap' or 'Half'
                {
                  double capHeightRatio = ( labelFontMetrics.boundingRect( 'H' ).height() + 1 + labelFontMetrics.descent() ) / labelFontMetrics.height();
                  ydiff -= labelSize.height() * capHeightRatio;
                  if ( valiString.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
                  {
                    ydiff += labelSize.height() * ( capHeightRatio - descentRatio ) / 2.0;
                  }
                }
              }
            }
          }
        }

        if ( dataDefinedRotation )
        {
          //adjust xdiff and ydiff because the hali/vali point needs to be the rotation center
          double xd = xdiff * std::cos( angleInRadians ) - ydiff * std::sin( angleInRadians );
          double yd = xdiff * std::sin( angleInRadians ) + ydiff * std::cos( angleInRadians );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::AlwaysShow ) )
  {
    alwaysShow = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::AlwaysShow, context.expressionContext(), false );
  }

  // set repeat distance
  // data defined repeat distance?
  double repeatDist = repeatDistance;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::RepeatDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( repeatDist );
    repeatDist = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::RepeatDistance, context.expressionContext(), repeatDist );
  }

  // data defined label-repeat distance units?
  Qgis::RenderUnit repeatUnits = repeatDistanceUnit;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::RepeatDistanceUnit ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::RepeatDistanceUnit, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString units = exprVal.toString().trimmed();
      if ( !units.isEmpty() )
      {
        bool ok = false;
        Qgis::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
        if ( ok )
        {
          repeatUnits = decodedUnits;
        }
      }
    }
  }

  if ( !qgsDoubleNear( repeatDist, 0.0 ) )
  {
    if ( repeatUnits != Qgis::RenderUnit::MapUnits )
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
  const double overrunSmoothDist = context.convertToMapUnits( 1, Qgis::RenderUnit::Millimeters );

  bool labelAll = labelPerPart && !hasDataDefinedPosition;
  if ( !hasDataDefinedPosition )
  {
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::LabelAllParts ) )
    {
      context.expressionContext().setOriginalValueVariable( labelPerPart );
      labelAll = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::LabelAllParts, context.expressionContext(), labelPerPart );
    }
  }

  // maximum distance
  double maximumDistanceEval = pointSettings.maximumDistance();
  if ( !qgsDoubleNear( maximumDistanceEval, 0.0 ) )
  {
    maximumDistanceEval = context.convertToMapUnits( maximumDistanceEval, pointSettings.maximumDistanceUnit(), pointSettings.maximumDistanceMapUnitScale() );
  }

  //  feature to the layer
  std::unique_ptr< QgsTextLabelFeature > labelFeature = std::make_unique< QgsTextLabelFeature>( feature.id(), std::move( geos_geom_clone ), labelSize );
  labelFeature->setAnchorPosition( anchorPosition );
  labelFeature->setFeature( feature );
  labelFeature->setSymbol( symbol );
  labelFeature->setDocument( doc, documentMetrics );
  if ( !qgsDoubleNear( rotatedSize.width(), 0.0 ) && !qgsDoubleNear( rotatedSize.height(), 0.0 ) )
    labelFeature->setRotatedSize( rotatedSize );
  mFeatsRegPal++;

  labelFeature->setHasFixedPosition( hasDataDefinedPosition );
  labelFeature->setFixedPosition( QgsPointXY( xPos, yPos ) );
  // use layer-level defined rotation, but not if position fixed
  labelFeature->setHasFixedAngle( dataDefinedRotation || ( !hasDataDefinedPosition && !qgsDoubleNear( angleInRadians, 0.0 ) ) );
  labelFeature->setFixedAngle( angleInRadians );
  labelFeature->setQuadOffset( QPointF( quadOffsetX, quadOffsetY ) );
  labelFeature->setPositionOffset( QgsPointXY( offsetX, offsetY ) );
  labelFeature->setOffsetType( offsetType );
  labelFeature->setAlwaysShow( alwaysShow );
  labelFeature->setRepeatDistance( repeatDist );
  labelFeature->setLabelText( labelText );
  labelFeature->setPermissibleZone( permissibleZone );
  labelFeature->setOverrunDistance( overrunDistanceEval );
  labelFeature->setOverrunSmoothDistance( overrunSmoothDist );
  labelFeature->setMaximumDistance( maximumDistanceEval );
  labelFeature->setLineAnchorPercent( lineSettings.lineAnchorPercent() );
  labelFeature->setLineAnchorType( lineSettings.anchorType() );
  labelFeature->setLineAnchorTextPoint( lineSettings.anchorTextPoint() );
  labelFeature->setLabelAllParts( labelAll );
  labelFeature->setOriginalFeatureCrs( context.coordinateTransform().sourceCrs() );
  labelFeature->setMinimumSize( minimumSize );
  if ( geom.type() == Qgis::GeometryType::Point && !obstacleGeometry.isNull() )
  {
    //register symbol size
    labelFeature->setSymbolSize( QSizeF( obstacleGeometry.boundingBox().width(),
                                         obstacleGeometry.boundingBox().height() ) );
  }

  if ( outerBounds.left() != 0 || outerBounds.top() != 0 || !qgsDoubleNear( outerBounds.width(), labelSize.width() ) || !qgsDoubleNear( outerBounds.height(), labelSize.height() ) )
  {
    labelFeature->setOuterBounds( outerBounds );
  }

  //set label's visual margin so that top visual margin is the leading, and bottom margin is the font's descent
  //this makes labels align to the font's baseline or highest character
  double topMargin = std::max( 0.25 * labelFontMetrics.ascent(), 0.0 );
  double bottomMargin = 1.0 + labelFontMetrics.descent();
  QgsMargins vm( 0.0, topMargin, 0.0, bottomMargin );
  vm *= xform->mapUnitsPerPixel();
  labelFeature->setVisualMargin( vm );

  // store the label's calculated font for later use during painting
  QgsDebugMsgLevel( QStringLiteral( "PAL font stored definedFont: %1, Style: %2" ).arg( labelFont.toString(), labelFont.styleName() ), 4 );
  labelFeature->setDefinedFont( labelFont );

  labelFeature->setMaximumCharacterAngleInside( std::clamp( maxcharanglein, 20.0, 60.0 ) * M_PI / 180 );
  labelFeature->setMaximumCharacterAngleOutside( std::clamp( maxcharangleout, -95.0, -20.0 ) * M_PI / 180 );
  switch ( placement )
  {
    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    case Qgis::LabelPlacement::OutsidePolygons:
      // these placements don't require text metrics
      break;

    case Qgis::LabelPlacement::Curved:
    case Qgis::LabelPlacement::PerimeterCurved:
      labelFeature->setTextMetrics( QgsTextLabelFeature::calculateTextMetrics( xform, context, evaluatedFormat, labelFont, labelFontMetrics, labelFont.letterSpacing(), labelFont.wordSpacing(), labelText, evaluatedFormat.allowHtmlFormatting() ? &doc : nullptr, evaluatedFormat.allowHtmlFormatting() ? &documentMetrics : nullptr ) );
      break;
  }

  // for labelFeature the LabelInfo is passed to feat when it is registered

  // TODO: allow layer-wide feature dist in PAL...?

  // data defined label-feature distance?
  double distance = dist;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::LabelDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( distance );
    distance = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::LabelDistance, context.expressionContext(), distance );
  }

  // data defined label-feature distance units?
  Qgis::RenderUnit distUnit = distUnits;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::DistanceUnits ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::DistanceUnits, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString units = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal DistanceUnits:%1" ).arg( units ), 4 );
      if ( !units.isEmpty() )
      {
        bool ok = false;
        Qgis::RenderUnit decodedUnits = QgsUnitTypes::decodeRenderUnit( units, &ok );
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
  switch ( placement )
  {
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Curved:
    case Qgis::LabelPlacement::PerimeterCurved:
      distance = ( distance < 0 ? -1 : 1 ) * std::max( std::fabs( distance ), 1.0 );
      break;

    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
      break;

    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
      if ( polygonPlacement & Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon )
      {
        distance = std::max( distance, 2.0 );
      }
      break;

    case Qgis::LabelPlacement::OutsidePolygons:
      distance = std::max( distance, 2.0 );
      break;
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ZIndex ) )
  {
    context.expressionContext().setOriginalValueVariable( z );
    z = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::ZIndex, context.expressionContext(), z );
  }
  labelFeature->setZIndex( z );

  // data defined priority?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Priority ) )
  {
    context.expressionContext().setOriginalValueVariable( priority );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::Priority, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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

  // data defined allow degraded placement
  {
    double allowDegradedPlacement = mPlacementSettings.allowDegradedPlacement();
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::AllowDegradedPlacement ) )
    {
      context.expressionContext().setOriginalValueVariable( allowDegradedPlacement );
      allowDegradedPlacement = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::AllowDegradedPlacement, context.expressionContext(), allowDegradedPlacement );
    }
    labelFeature->setAllowDegradedPlacement( allowDegradedPlacement );
  }

  // data defined overlap handling
  {
    Qgis::LabelOverlapHandling overlapHandling = mPlacementSettings.overlapHandling();
    if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::OverlapHandling ) )
    {
      const QString handlingString = mDataDefinedProperties.valueAsString( QgsPalLayerSettings::Property::OverlapHandling, context.expressionContext() );
      const QString cleanedString = handlingString.trimmed();
      if ( cleanedString.compare( QLatin1String( "prevent" ), Qt::CaseInsensitive ) == 0 )
        overlapHandling = Qgis::LabelOverlapHandling::PreventOverlap;
      else if ( cleanedString.compare( QLatin1String( "allowifneeded" ), Qt::CaseInsensitive ) == 0 )
        overlapHandling = Qgis::LabelOverlapHandling::AllowOverlapIfRequired;
      else if ( cleanedString.compare( QLatin1String( "alwaysallow" ), Qt::CaseInsensitive ) == 0 )
        overlapHandling = Qgis::LabelOverlapHandling::AllowOverlapAtNoCost;
    }
    labelFeature->setOverlapHandling( overlapHandling );
  }

  labelFeature->setPrioritization( mPlacementSettings.prioritization() );

  QgsLabelObstacleSettings os = mObstacleSettings;
  os.setIsObstacle( isObstacle );
  os.updateDataDefinedProperties( mDataDefinedProperties, context.expressionContext() );
  os.setObstacleGeometry( obstacleGeometry );
  labelFeature->setObstacleSettings( os );

  QVector< Qgis::LabelPredefinedPointPosition > positionOrder = pointSettings.predefinedPositionOrder();
  if ( positionOrder.isEmpty() )
    positionOrder = *DEFAULT_PLACEMENT_ORDER();

  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::PredefinedPositionOrder ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsLabelingUtils::encodePredefinedPositionOrder( pointSettings.predefinedPositionOrder() ) );
    QString dataDefinedOrder = mDataDefinedProperties.valueAsString( QgsPalLayerSettings::Property::PredefinedPositionOrder, context.expressionContext() );
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
  if ( simplifyMethod.simplifyHints() != Qgis::VectorRenderingSimplificationFlags( Qgis::VectorRenderingSimplificationFlag::NoSimplification ) && simplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = simplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    const Qgis::VectorSimplificationAlgorithm simplifyAlgorithm = simplifyMethod.simplifyAlgorithm();
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
  if ( !QgsVariantUtils::isNull( exprVal ) )
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
        QColor color = QgsColorUtils::colorFromString( colorstr );

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
    Qgis::RenderUnit fontunits,
    QgsRenderContext &context )
{
  // NOTE: labelFont already has pixelSize set, so pointSize or pointSizeF might return -1

  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // Two ways to generate new data defined font:
  // 1) Family + [bold] + [italic] (named style is ignored and font is built off of base family)
  // 2) Family + named style  (bold or italic is ignored)

  // data defined font family?
  QString ddFontFamily;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Family ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.family() );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::Family, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString family = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal Font family:%1" ).arg( family ), 4 );

      family = QgsApplication::fontManager()->processFontFamilyName( family );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontStyle ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::FontStyle, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString fontstyle = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal Font style:%1" ).arg( fontstyle ), 4 );
      ddFontStyle = fontstyle;
    }
  }

  // data defined bold font style?
  bool ddBold = false;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Bold ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.bold() );
    ddBold = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::Bold, context.expressionContext(), false );
  }

  // data defined italic font style?
  bool ddItalic = false;
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Italic ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.italic() );
    ddItalic = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::Italic, context.expressionContext(), false );
  }

  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  //       (currently defaults to what has been read in from layer settings)
  QFont newFont;
  QFont appFont = QApplication::font();
  bool newFontBuilt = false;
  if ( ddBold || ddItalic )
  {
    // new font needs built, since existing style needs removed
    newFont = QgsFontUtils::createFont( !ddFontFamily.isEmpty() ? ddFontFamily : labelFont.family() );
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
      newFont = QgsFontUtils::createFont( ddFontFamily );
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
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontWordSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( wordspace );
    wordspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::FontWordSpacing, context.expressionContext(), wordspace );
  }
  labelFont.setWordSpacing( context.convertToPainterUnits( wordspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined letter spacing?
  double letterspace = labelFont.letterSpacing();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontLetterSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( letterspace );
    letterspace = mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Property::FontLetterSpacing, context.expressionContext(), letterspace );
  }
  labelFont.setLetterSpacing( QFont::AbsoluteSpacing, context.convertToPainterUnits( letterspace, fontunits, mFormat.sizeMapUnitScale() ) );

  // data defined strikeout font style?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Strikeout ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.strikeOut() );
    bool strikeout = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::Strikeout, context.expressionContext(), false );
    labelFont.setStrikeOut( strikeout );
  }

  // data defined stretch
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::FontStretchFactor ) )
  {
    context.expressionContext().setOriginalValueVariable( mFormat.stretchFactor() );
    labelFont.setStretch( mDataDefinedProperties.valueAsInt( QgsPalLayerSettings::Property::FontStretchFactor, context.expressionContext(),  mFormat.stretchFactor() ) );
  }

  // data defined underline font style?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::Underline ) )
  {
    context.expressionContext().setOriginalValueVariable( labelFont.underline() );
    bool underline = mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Property::Underline, context.expressionContext(), false );
    labelFont.setUnderline( underline );
  }

  // pass the rest on to QgsPalLabeling::drawLabeling

  // data defined font color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Property::Color, exprVal, context.expressionContext(), QgsColorUtils::colorToString( mFormat.color() ) );

  // data defined font opacity?
  dataDefinedValEval( DDOpacity, QgsPalLayerSettings::Property::FontOpacity, exprVal, context.expressionContext(), mFormat.opacity() * 100 );

  // data defined font blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::Property::FontBlendMode, exprVal, context.expressionContext() );

}

void QgsPalLayerSettings::parseTextBuffer( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextBufferSettings buffer = mFormat.buffer();

  // data defined draw buffer?
  bool drawBuffer = mFormat.buffer().enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::BufferDraw, exprVal, context.expressionContext(), buffer.enabled() ) )
  {
    drawBuffer = exprVal.toBool();
  }
  else if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::BufferDraw ) && QgsVariantUtils::isNull( exprVal ) )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::Property::BufferDraw, QVariant( drawBuffer ) );
  }

  if ( !drawBuffer )
  {
    return;
  }

  // data defined buffer size?
  double bufrSize = buffer.size();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::Property::BufferSize, exprVal, context.expressionContext(), buffer.size() ) )
  {
    bufrSize = exprVal.toDouble();
  }

  // data defined buffer transparency?
  double bufferOpacity = buffer.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::Property::BufferOpacity, exprVal, context.expressionContext(), bufferOpacity ) )
  {
    bufferOpacity = exprVal.toDouble();
  }

  drawBuffer = ( drawBuffer && bufrSize > 0.0 && bufferOpacity > 0 );

  if ( !drawBuffer )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::Property::BufferDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::Property::BufferSize );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::BufferOpacity );
    return; // don't bother evaluating values that won't be used
  }

  // data defined buffer units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::BufferUnit, exprVal, context.expressionContext() );

  // data defined buffer color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Property::BufferColor, exprVal, context.expressionContext(), QgsColorUtils::colorToString( buffer.color() ) );

  // data defined buffer pen join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::Property::BufferJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( buffer.joinStyle() ) );

  // data defined buffer blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::Property::BufferBlendMode, exprVal, context.expressionContext() );
}

void QgsPalLayerSettings::parseTextMask( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextMaskSettings mask = mFormat.mask();

  // data defined enabled mask?
  bool maskEnabled = mask.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::MaskEnabled, exprVal, context.expressionContext(), mask.enabled() ) )
  {
    maskEnabled = exprVal.toBool();
  }

  if ( !maskEnabled )
  {
    return;
  }

  // data defined buffer size?
  double bufrSize = mask.size();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::Property::MaskBufferSize, exprVal, context.expressionContext(), mask.size() ) )
  {
    bufrSize = exprVal.toDouble();
  }

  // data defined opacity?
  double opacity = mask.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::Property::MaskOpacity, exprVal, context.expressionContext(), opacity ) )
  {
    opacity = exprVal.toDouble();
  }

  maskEnabled = ( maskEnabled && bufrSize > 0.0 && opacity > 0 );

  if ( !maskEnabled )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::Property::MaskEnabled, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::Property::MaskBufferSize );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::MaskOpacity );
    return; // don't bother evaluating values that won't be used
  }

  // data defined buffer units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::MaskBufferUnit, exprVal, context.expressionContext() );

  // data defined buffer pen join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::Property::MaskJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( mask.joinStyle() ) );
}

void QgsPalLayerSettings::parseTextFormatting( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  // data defined multiline wrap character?
  QString wrapchr = wrapChar;
  if ( dataDefinedValEval( DDString, QgsPalLayerSettings::Property::MultiLineWrapChar, exprVal, context.expressionContext(), wrapChar ) )
  {
    wrapchr = exprVal.toString();
  }

  int evalAutoWrapLength = autoWrapLength;
  if ( dataDefinedValEval( DDInt, QgsPalLayerSettings::Property::AutoWrapLength, exprVal, context.expressionContext(), evalAutoWrapLength ) )
  {
    evalAutoWrapLength = exprVal.toInt();
  }

  // data defined multiline height?
  dataDefinedValEval( DDDouble, QgsPalLayerSettings::Property::MultiLineHeight, exprVal, context.expressionContext() );

  // data defined multiline text align?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::MultiLineAlignment ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::MultiLineAlignment, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal MultiLineAlignment:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        // "Left"
        Qgis::LabelMultiLineAlignment aligntype = Qgis::LabelMultiLineAlignment::Left;

        if ( str.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
        {
          aligntype = Qgis::LabelMultiLineAlignment::Center;
        }
        else if ( str.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
        {
          aligntype = Qgis::LabelMultiLineAlignment::Right;
        }
        else if ( str.compare( QLatin1String( "Follow" ), Qt::CaseInsensitive ) == 0 )
        {
          aligntype = Qgis::LabelMultiLineAlignment::FollowPlacement;
        }
        else if ( str.compare( QLatin1String( "Justify" ), Qt::CaseInsensitive ) == 0 )
        {
          aligntype = Qgis::LabelMultiLineAlignment::Justify;
        }
        dataDefinedValues.insert( QgsPalLayerSettings::Property::MultiLineAlignment, QVariant( static_cast< int >( aligntype ) ) );
      }
    }
  }

  // text orientation
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::TextOrientation ) )
  {
    const QString encoded = QgsTextRendererUtils::encodeTextOrientation( mFormat.orientation() );
    context.expressionContext().setOriginalValueVariable( encoded );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::TextOrientation, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString str = exprVal.toString().trimmed();
      if ( !str.isEmpty() )
        dataDefinedValues.insert( QgsPalLayerSettings::Property::TextOrientation, str );
    }
  }

  // data defined direction symbol?
  bool drawDirSymb = mLineSettings.addDirectionSymbol();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::DirSymbDraw, exprVal, context.expressionContext(), drawDirSymb ) )
  {
    drawDirSymb = exprVal.toBool();
  }

  if ( drawDirSymb )
  {
    // data defined direction left symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::Property::DirSymbLeft, exprVal, context.expressionContext(), mLineSettings.leftDirectionSymbol() );

    // data defined direction right symbol?
    dataDefinedValEval( DDString, QgsPalLayerSettings::Property::DirSymbRight, exprVal, context.expressionContext(), mLineSettings.rightDirectionSymbol() );

    // data defined direction symbol placement?
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::DirSymbPlacement, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
        dataDefinedValues.insert( QgsPalLayerSettings::Property::DirSymbPlacement, QVariant( static_cast< int >( placetype ) ) );
      }
    }

    // data defined direction symbol reversed?
    dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::DirSymbReverse, exprVal, context.expressionContext(), mLineSettings.reverseDirectionSymbol() );
  }

  // formatting for numbers is inline with generation of base label text and not passed to label painting
}

void QgsPalLayerSettings::parseShapeBackground( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextBackgroundSettings background = mFormat.background();

  // data defined draw shape?
  bool drawShape = background.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::ShapeDraw, exprVal, context.expressionContext(), drawShape ) )
  {
    drawShape = exprVal.toBool();
  }

  if ( !drawShape )
  {
    return;
  }

  // data defined shape transparency?
  double shapeOpacity = background.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::Property::ShapeOpacity, exprVal, context.expressionContext(), shapeOpacity ) )
  {
    shapeOpacity = 100.0 * exprVal.toDouble();
  }

  drawShape = ( drawShape && shapeOpacity > 0 ); // size is not taken into account (could be)

  if ( !drawShape )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeOpacity );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape kind?
  QgsTextBackgroundSettings::ShapeType shapeKind = background.type();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeKind ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::ShapeKind, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString skind = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeKind:%1" ).arg( skind ), 4 );

      if ( !skind.isEmpty() )
      {
        shapeKind = QgsTextRendererUtils::decodeShapeType( skind );
        dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeKind, QVariant( static_cast< int >( shapeKind ) ) );
      }
    }
  }

  // data defined shape SVG path?
  QString svgPath = background.svgFile();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeSVGFile ) )
  {
    context.expressionContext().setOriginalValueVariable( svgPath );
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::ShapeSVGFile, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString svgfile = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSVGFile:%1" ).arg( svgfile ), 4 );

      // '' empty paths are allowed
      svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
      dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeSVGFile, QVariant( svgPath ) );
    }
  }

  // data defined shape size type?
  QgsTextBackgroundSettings::SizeType shpSizeType = background.sizeType();
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeSizeType ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::ShapeSizeType, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString stype = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeSizeType:%1" ).arg( stype ), 4 );

      if ( !stype.isEmpty() )
      {
        shpSizeType = QgsTextRendererUtils::decodeBackgroundSizeType( stype );
        dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeSizeType, QVariant( static_cast< int >( shpSizeType ) ) );
      }
    }
  }

  // data defined shape size X? (SVGs only use X for sizing)
  double ddShpSizeX = background.size().width();
  if ( dataDefinedValEval( DDDouble, QgsPalLayerSettings::Property::ShapeSizeX, exprVal, context.expressionContext(), ddShpSizeX ) )
  {
    ddShpSizeX = exprVal.toDouble();
  }

  // data defined shape size Y?
  double ddShpSizeY = background.size().height();
  if ( dataDefinedValEval( DDDouble, QgsPalLayerSettings::Property::ShapeSizeY, exprVal, context.expressionContext(), ddShpSizeY ) )
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
    dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeOpacity );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeKind );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeSVGFile );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeSizeX );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShapeSizeY );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shape size units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShapeSizeUnits, exprVal, context.expressionContext() );

  // data defined shape rotation type?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShapeRotationType ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::ShapeRotationType, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString rotstr = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShapeRotationType:%1" ).arg( rotstr ), 4 );

      if ( !rotstr.isEmpty() )
      {
        // "Sync"
        QgsTextBackgroundSettings::RotationType rottype = QgsTextRendererUtils::decodeBackgroundRotationType( rotstr );
        dataDefinedValues.insert( QgsPalLayerSettings::Property::ShapeRotationType, QVariant( static_cast< int >( rottype ) ) );
      }
    }
  }

  // data defined shape rotation?
  dataDefinedValEval( DDRotation180, QgsPalLayerSettings::Property::ShapeRotation, exprVal, context.expressionContext(), background.rotation() );

  // data defined shape offset?
  dataDefinedValEval( DDPointF, QgsPalLayerSettings::Property::ShapeOffset, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePoint( background.offset() ) );

  // data defined shape offset units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShapeOffsetUnits, exprVal, context.expressionContext() );

  // data defined shape radii?
  dataDefinedValEval( DDSizeF, QgsPalLayerSettings::Property::ShapeRadii, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodeSize( background.radii() ) );

  // data defined shape radii units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShapeRadiiUnits, exprVal, context.expressionContext() );

  // data defined shape blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::Property::ShapeBlendMode, exprVal, context.expressionContext() );

  // data defined shape fill color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Property::ShapeFillColor, exprVal, context.expressionContext(), QgsColorUtils::colorToString( background.fillColor() ) );

  // data defined shape stroke color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Property::ShapeStrokeColor, exprVal, context.expressionContext(), QgsColorUtils::colorToString( background.strokeColor() ) );

  // data defined shape stroke width?
  dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::Property::ShapeStrokeWidth, exprVal, context.expressionContext(), background.strokeWidth() );

  // data defined shape stroke width units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShapeStrokeWidthUnits, exprVal, context.expressionContext() );

  // data defined shape join style?
  dataDefinedValEval( DDJoinStyle, QgsPalLayerSettings::Property::ShapeJoinStyle, exprVal, context.expressionContext(), QgsSymbolLayerUtils::encodePenJoinStyle( background.joinStyle() ) );

}

void QgsPalLayerSettings::parseDropShadow( QgsRenderContext &context )
{
  QVariant exprVal; // value() is repeatedly nulled on data defined evaluation and replaced when successful

  QgsTextShadowSettings shadow = mFormat.shadow();

  // data defined draw shadow?
  bool drawShadow = shadow.enabled();
  if ( dataDefinedValEval( DDBool, QgsPalLayerSettings::Property::ShadowDraw, exprVal, context.expressionContext(), drawShadow ) )
  {
    drawShadow = exprVal.toBool();
  }

  if ( !drawShadow )
  {
    return;
  }

  // data defined shadow transparency?
  double shadowOpacity = shadow.opacity() * 100;
  if ( dataDefinedValEval( DDOpacity, QgsPalLayerSettings::Property::ShadowOpacity, exprVal, context.expressionContext(), shadowOpacity ) )
  {
    shadowOpacity = exprVal.toDouble();
  }

  // data defined shadow offset distance?
  double shadowOffDist = shadow.offsetDistance();
  if ( dataDefinedValEval( DDDoublePos, QgsPalLayerSettings::Property::ShadowOffsetDist, exprVal, context.expressionContext(), shadowOffDist ) )
  {
    shadowOffDist = exprVal.toDouble();
  }

  // data defined shadow offset distance?
  double shadowRad = shadow.blurRadius();
  if ( dataDefinedValEval( DDDoublePos,  QgsPalLayerSettings::Property::ShadowRadius, exprVal, context.expressionContext(), shadowRad ) )
  {
    shadowRad = exprVal.toDouble();
  }

  drawShadow = ( drawShadow && shadowOpacity > 0 && !( shadowOffDist == 0.0 && shadowRad == 0.0 ) );

  if ( !drawShadow )
  {
    dataDefinedValues.insert( QgsPalLayerSettings::Property::ShadowDraw, QVariant( false ) ); // trigger value
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShadowOpacity );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShadowOffsetDist );
    dataDefinedValues.remove( QgsPalLayerSettings::Property::ShadowRadius );
    return; // don't bother evaluating values that won't be used
  }

  // data defined shadow under type?
  if ( mDataDefinedProperties.isActive( QgsPalLayerSettings::Property::ShadowUnder ) )
  {
    exprVal = mDataDefinedProperties.value( QgsPalLayerSettings::Property::ShadowUnder, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString str = exprVal.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal ShadowUnder:%1" ).arg( str ), 4 );

      if ( !str.isEmpty() )
      {
        QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextRendererUtils::decodeShadowPlacementType( str );
        dataDefinedValues.insert( QgsPalLayerSettings::Property::ShadowUnder, QVariant( static_cast< int >( shdwtype ) ) );
      }
    }
  }

  // data defined shadow offset angle?
  dataDefinedValEval( DDRotation180, QgsPalLayerSettings::Property::ShadowOffsetAngle, exprVal, context.expressionContext(), shadow.offsetAngle() );

  // data defined shadow offset units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShadowOffsetUnits, exprVal, context.expressionContext() );

  // data defined shadow radius?
  dataDefinedValEval( DDDouble, QgsPalLayerSettings::Property::ShadowRadius, exprVal, context.expressionContext(), shadow.blurRadius() );

  // data defined shadow radius units?
  dataDefinedValEval( DDUnits, QgsPalLayerSettings::Property::ShadowRadiusUnits, exprVal, context.expressionContext() );

  // data defined shadow scale?  ( gui bounds to 0-2000, no upper bound here )
  dataDefinedValEval( DDIntPos, QgsPalLayerSettings::Property::ShadowScale, exprVal, context.expressionContext(), shadow.scale() );

  // data defined shadow color?
  dataDefinedValEval( DDColor, QgsPalLayerSettings::Property::ShadowColor, exprVal, context.expressionContext(), QgsColorUtils::colorToString( shadow.color() ) );

  // data defined shadow blend mode?
  dataDefinedValEval( DDBlendMode, QgsPalLayerSettings::Property::ShadowBlendMode, exprVal, context.expressionContext() );
}

// -------------


bool QgsPalLabeling::staticWillUseLayer( const QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer );
      return vl->labelsEnabled() || vl->diagramsEnabled() || ( vl->renderer() && vl->renderer()->flags().testFlag( Qgis::FeatureRendererFlag::AffectsLabeling ) );
    }

    case Qgis::LayerType::VectorTile:
    {
      const QgsVectorTileLayer *vl = qobject_cast< const QgsVectorTileLayer * >( layer );
      if ( !vl->labeling() )
        return false;

      if ( const QgsVectorTileBasicLabeling *labeling = dynamic_cast< const QgsVectorTileBasicLabeling *>( vl->labeling() ) )
        return !labeling->styles().empty();

      return false;
    }

    case Qgis::LayerType::Mesh:
    {
      const QgsMeshLayer *ml = qobject_cast< const QgsMeshLayer * >( layer );
      return ml->labeling() && ml->labelsEnabled();
    }

    case Qgis::LayerType::Raster:
    {
      const QgsRasterLayer *rl = qobject_cast< const QgsRasterLayer * >( layer );
      return rl->labeling() && rl->labelsEnabled();
    }

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
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

  if ( geometry.type() == Qgis::GeometryType::Line && geometry.isMultipart() && mergeLines )
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
  if ( geometry.type() == Qgis::GeometryType::Polygon && !geometry.isGeosValid() )
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

  if ( geom.type() == Qgis::GeometryType::Line && geom.isMultipart() && mergeLines )
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

  // Rotate the geometry if needed
  const QgsMapToPixel &m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPointXY center = context.mapExtent().center();
    if ( geom.rotate( m2p.mapRotation(), center ) != Qgis::GeometryOperationResult::Success )
    {
      QgsDebugError( QStringLiteral( "Error rotating geometry" ).arg( geom.asWkt() ) );
      return QgsGeometry();
    }
  }

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
  if ( geom.type() == Qgis::GeometryType::Polygon )
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
        QgsDebugError( QStringLiteral( "Could not repair geometry: %1" ).arg( bufferGeom.lastError() ) );
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

  Qgis::GeometryType featureType = geom.type();
  if ( featureType == Qgis::GeometryType::Point ) //minimum size does not apply to point features
  {
    return true;
  }

  double mapUnitsPerMM = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();
  if ( featureType == Qgis::GeometryType::Line )
  {
    double length = geom.length();
    if ( length >= 0.0 )
    {
      return ( length >= ( minSize * mapUnitsPerMM ) );
    }
  }
  else if ( featureType == Qgis::GeometryType::Polygon )
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::Color ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Property::Color );
    format.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  //font transparency
  if ( ddValues.contains( QgsPalLayerSettings::Property::FontOpacity ) )
  {
    format.setOpacity( ddValues.value( QgsPalLayerSettings::Property::FontOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  //font blend mode
  if ( ddValues.contains( QgsPalLayerSettings::Property::FontBlendMode ) )
  {
    format.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::Property::FontBlendMode ).toInt() ) );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::MultiLineWrapChar ) )
  {
    tmpLyr.wrapChar = ddValues.value( QgsPalLayerSettings::Property::MultiLineWrapChar ).toString();
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::AutoWrapLength ) )
  {
    tmpLyr.autoWrapLength = ddValues.value( QgsPalLayerSettings::Property::AutoWrapLength ).toInt();
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::MultiLineHeight ) )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setLineHeight( ddValues.value( QgsPalLayerSettings::Property::MultiLineHeight ).toDouble() );
    tmpLyr.setFormat( format );
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::MultiLineAlignment ) )
  {
    tmpLyr.multilineAlign = static_cast< Qgis::LabelMultiLineAlignment >( ddValues.value( QgsPalLayerSettings::Property::MultiLineAlignment ).toInt() );
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::TextOrientation ) )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setOrientation( QgsTextRendererUtils::decodeTextOrientation( ddValues.value( QgsPalLayerSettings::Property::TextOrientation ).toString() ) );
    tmpLyr.setFormat( format );
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::DirSymbDraw ) )
  {
    tmpLyr.lineSettings().setAddDirectionSymbol( ddValues.value( QgsPalLayerSettings::Property::DirSymbDraw ).toBool() );
  }

  if ( tmpLyr.lineSettings().addDirectionSymbol() )
  {

    if ( ddValues.contains( QgsPalLayerSettings::Property::DirSymbLeft ) )
    {
      tmpLyr.lineSettings().setLeftDirectionSymbol( ddValues.value( QgsPalLayerSettings::Property::DirSymbLeft ).toString() );
    }
    if ( ddValues.contains( QgsPalLayerSettings::Property::DirSymbRight ) )
    {
      tmpLyr.lineSettings().setRightDirectionSymbol( ddValues.value( QgsPalLayerSettings::Property::DirSymbRight ).toString() );
    }

    if ( ddValues.contains( QgsPalLayerSettings::Property::DirSymbPlacement ) )
    {
      tmpLyr.lineSettings().setDirectionSymbolPlacement( static_cast< QgsLabelLineSettings::DirectionSymbolPlacement >( ddValues.value( QgsPalLayerSettings::Property::DirSymbPlacement ).toInt() ) );
    }

    if ( ddValues.contains( QgsPalLayerSettings::Property::DirSymbReverse ) )
    {
      tmpLyr.lineSettings().setReverseDirectionSymbol( ddValues.value( QgsPalLayerSettings::Property::DirSymbReverse ).toBool() );
    }

  }
}

void QgsPalLabeling::dataDefinedTextBuffer( QgsPalLayerSettings &tmpLyr,
    const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues )
{
  QgsTextBufferSettings buffer = tmpLyr.format().buffer();
  bool changed = false;

  //buffer draw
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferDraw ) )
  {
    buffer.setEnabled( ddValues.value( QgsPalLayerSettings::Property::BufferDraw ).toBool() );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferSize ) )
  {
    buffer.setSize( ddValues.value( QgsPalLayerSettings::Property::BufferSize ).toDouble() );
    changed = true;
  }

  //buffer opacity
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferOpacity ) )
  {
    buffer.setOpacity( ddValues.value( QgsPalLayerSettings::Property::BufferOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  //buffer size units
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferUnit ) )
  {
    Qgis::RenderUnit bufunit = static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::BufferUnit ).toInt() );
    buffer.setSizeUnit( bufunit );
    changed = true;
  }

  //buffer color
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Property::BufferColor );
    buffer.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  //buffer pen join style
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferJoinStyle ) )
  {
    buffer.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::Property::BufferJoinStyle ).toInt() ) );
    changed = true;
  }

  //buffer blend mode
  if ( ddValues.contains( QgsPalLayerSettings::Property::BufferBlendMode ) )
  {
    buffer.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::Property::BufferBlendMode ).toInt() ) );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::MaskEnabled ) )
  {
    mask.setEnabled( ddValues.value( QgsPalLayerSettings::Property::MaskEnabled ).toBool() );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::MaskBufferSize ) )
  {
    mask.setSize( ddValues.value( QgsPalLayerSettings::Property::MaskBufferSize ).toDouble() );
    changed = true;
  }

  // opacity
  if ( ddValues.contains( QgsPalLayerSettings::Property::MaskOpacity ) )
  {
    mask.setOpacity( ddValues.value( QgsPalLayerSettings::Property::MaskOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  // buffer size units
  if ( ddValues.contains( QgsPalLayerSettings::Property::MaskBufferUnit ) )
  {
    Qgis::RenderUnit bufunit = static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::MaskBufferUnit ).toInt() );
    mask.setSizeUnit( bufunit );
    changed = true;
  }

  // pen join style
  if ( ddValues.contains( QgsPalLayerSettings::Property::MaskJoinStyle ) )
  {
    mask.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::Property::MaskJoinStyle ).toInt() ) );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeDraw ) )
  {
    background.setEnabled( ddValues.value( QgsPalLayerSettings::Property::ShapeDraw ).toBool() );
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

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeKind ) )
  {
    background.setType( static_cast< QgsTextBackgroundSettings::ShapeType >( ddValues.value( QgsPalLayerSettings::Property::ShapeKind ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeSVGFile ) )
  {
    background.setSvgFile( ddValues.value( QgsPalLayerSettings::Property::ShapeSVGFile ).toString() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeSizeType ) )
  {
    background.setSizeType( static_cast< QgsTextBackgroundSettings::SizeType >( ddValues.value( QgsPalLayerSettings::Property::ShapeSizeType ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeSizeX ) )
  {
    QSizeF size = background.size();
    size.setWidth( ddValues.value( QgsPalLayerSettings::Property::ShapeSizeX ).toDouble() );
    background.setSize( size );
    changed = true;
  }
  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeSizeY ) )
  {
    QSizeF size = background.size();
    size.setHeight( ddValues.value( QgsPalLayerSettings::Property::ShapeSizeY ).toDouble() );
    background.setSize( size );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeSizeUnits ) )
  {
    background.setSizeUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShapeSizeUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeRotationType ) )
  {
    background.setRotationType( static_cast< QgsTextBackgroundSettings::RotationType >( ddValues.value( QgsPalLayerSettings::Property::ShapeRotationType ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeRotation ) )
  {
    background.setRotation( ddValues.value( QgsPalLayerSettings::Property::ShapeRotation ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeOffset ) )
  {
    background.setOffset( ddValues.value( QgsPalLayerSettings::Property::ShapeOffset ).toPointF() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeOffsetUnits ) )
  {
    background.setOffsetUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShapeOffsetUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeRadii ) )
  {
    background.setRadii( ddValues.value( QgsPalLayerSettings::Property::ShapeRadii ).toSizeF() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeRadiiUnits ) )
  {
    background.setRadiiUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShapeRadiiUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeBlendMode ) )
  {
    background.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::Property::ShapeBlendMode ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeFillColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Property::ShapeFillColor );
    background.setFillColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeStrokeColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Property::ShapeStrokeColor );
    background.setStrokeColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeOpacity ) )
  {
    background.setOpacity( ddValues.value( QgsPalLayerSettings::Property::ShapeOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeStrokeWidth ) )
  {
    background.setStrokeWidth( ddValues.value( QgsPalLayerSettings::Property::ShapeStrokeWidth ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeStrokeWidthUnits ) )
  {
    background.setStrokeWidthUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShapeStrokeWidthUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShapeJoinStyle ) )
  {
    background.setJoinStyle( static_cast< Qt::PenJoinStyle >( ddValues.value( QgsPalLayerSettings::Property::ShapeJoinStyle ).toInt() ) );
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
  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowDraw ) )
  {
    shadow.setEnabled( ddValues.value( QgsPalLayerSettings::Property::ShadowDraw ).toBool() );
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

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowUnder ) )
  {
    shadow.setShadowPlacement( QgsTextShadowSettings::ShadowPlacement( ddValues.value( QgsPalLayerSettings::Property::ShadowUnder ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowOffsetAngle ) )
  {
    shadow.setOffsetAngle( ddValues.value( QgsPalLayerSettings::Property::ShadowOffsetAngle ).toInt() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowOffsetDist ) )
  {
    shadow.setOffsetDistance( ddValues.value( QgsPalLayerSettings::Property::ShadowOffsetDist ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowOffsetUnits ) )
  {
    shadow.setOffsetUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShadowOffsetUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowRadius ) )
  {
    shadow.setBlurRadius( ddValues.value( QgsPalLayerSettings::Property::ShadowRadius ).toDouble() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowRadiusUnits ) )
  {
    shadow.setBlurRadiusUnit( static_cast< Qgis::RenderUnit >( ddValues.value( QgsPalLayerSettings::Property::ShadowRadiusUnits ).toInt() ) );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowColor ) )
  {
    QVariant ddColor = ddValues.value( QgsPalLayerSettings::Property::ShadowColor );
    shadow.setColor( ddColor.value<QColor>() );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowOpacity ) )
  {
    shadow.setOpacity( ddValues.value( QgsPalLayerSettings::Property::ShadowOpacity ).toDouble() / 100.0 );
    changed = true;
  }

  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowScale ) )
  {
    shadow.setScale( ddValues.value( QgsPalLayerSettings::Property::ShadowScale ).toInt() );
    changed = true;
  }


  if ( ddValues.contains( QgsPalLayerSettings::Property::ShadowBlendMode ) )
  {
    shadow.setBlendMode( static_cast< QPainter::CompositionMode >( ddValues.value( QgsPalLayerSettings::Property::ShadowBlendMode ).toInt() ) );
    changed = true;
  }

  if ( changed )
  {
    QgsTextFormat format = tmpLyr.format();
    format.setShadow( shadow );
    tmpLyr.setFormat( format );
  }
}
