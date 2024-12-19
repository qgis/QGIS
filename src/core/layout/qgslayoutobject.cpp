/***************************************************************************
                            qgslayoutobject.cpp
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutobject.h"
#include "qgsexpressioncontextutils.h"
#include "qgslayout.h"
#include "qgslayoutreportcontext.h"

#include <QPainter>

QgsPropertiesDefinition QgsLayoutObject::sPropertyDefinitions;

void QgsLayoutObject::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::TestProperty ), QgsPropertyDefinition( "dataDefinedProperty", QgsPropertyDefinition::DataTypeString, "invalid property", QString() ) },
    {
      static_cast< int >( QgsLayoutObject::DataDefinedProperty::PresetPaperSize ), QgsPropertyDefinition( "dataDefinedPaperSize", QgsPropertyDefinition::DataTypeString, QObject::tr( "Paper size" ), QObject::tr( "string " ) + QLatin1String( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
          "|<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
          "|<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
          "|<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                                                                                                                                                                                                                              ) )
    },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PaperWidth ), QgsPropertyDefinition( "dataDefinedPaperWidth", QObject::tr( "Page width" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PaperHeight ), QgsPropertyDefinition( "dataDefinedPaperHeight", QObject::tr( "Page height" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::NumPages ), QgsPropertyDefinition( "dataDefinedNumPages", QObject::tr( "Number of pages" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PaperOrientation ), QgsPropertyDefinition( "dataDefinedPaperOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QLatin1String( "[<b>portrait</b>|<b>landscape</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PageNumber ), QgsPropertyDefinition( "dataDefinedPageNumber", QObject::tr( "Page number" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PositionX ), QgsPropertyDefinition( "dataDefinedPositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PositionY ), QgsPropertyDefinition( "dataDefinedPositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ItemWidth ), QgsPropertyDefinition( "dataDefinedWidth", QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ItemHeight ), QgsPropertyDefinition( "dataDefinedHeight", QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ItemRotation ), QgsPropertyDefinition( "dataDefinedRotation", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::Transparency ), QgsPropertyDefinition( "dataDefinedTransparency", QObject::tr( "Transparency" ), QgsPropertyDefinition::Opacity ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::Opacity ), QgsPropertyDefinition( "dataDefinedOpacity", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::BlendMode ), QgsPropertyDefinition( "dataDefinedBlendMode", QObject::tr( "Blend mode" ), QgsPropertyDefinition::BlendMode ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ExcludeFromExports ), QgsPropertyDefinition( "dataDefinedExcludeExports", QObject::tr( "Exclude item from exports" ), QgsPropertyDefinition::Boolean ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::FrameColor ), QgsPropertyDefinition( "dataDefinedFrameColor", QObject::tr( "Frame color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::BackgroundColor ), QgsPropertyDefinition( "dataDefinedBackgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MarginLeft ), QgsPropertyDefinition( "dataDefinedMarginLeft", QObject::tr( "Left margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MarginTop ), QgsPropertyDefinition( "dataDefinedMarginTop", QObject::tr( "Top margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MarginRight ), QgsPropertyDefinition( "dataDefinedMarginRight", QObject::tr( "Right margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MarginBottom ), QgsPropertyDefinition( "dataDefinedMarginBottom", QObject::tr( "Bottom margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapRotation ), QgsPropertyDefinition( "dataDefinedMapRotation", QObject::tr( "Map rotation" ), QgsPropertyDefinition::Rotation ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapScale ), QgsPropertyDefinition( "dataDefinedMapScale", QObject::tr( "Map scale" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapXMin ), QgsPropertyDefinition( "dataDefinedMapXMin", QObject::tr( "Extent minimum X" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapYMin ), QgsPropertyDefinition( "dataDefinedMapYMin", QObject::tr( "Extent minimum Y" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapXMax ), QgsPropertyDefinition( "dataDefinedMapXMax", QObject::tr( "Extent maximum X" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapYMax ), QgsPropertyDefinition( "dataDefinedMapYMax", QObject::tr( "Extent maximum Y" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapLabelMargin ), QgsPropertyDefinition( "dataDefinedMapLabelMargin", QObject::tr( "Label margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapAtlasMargin ), QgsPropertyDefinition( "dataDefinedMapAtlasMargin", QObject::tr( "Atlas margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapLayers ), QgsPropertyDefinition( "dataDefinedMapLayers", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map Layers" ), tr( "list of map layer names separated by | characters" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapStylePreset ), QgsPropertyDefinition( "dataDefinedMapStylePreset", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map theme" ), tr( "name of an existing map theme (case-sensitive)" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridEnabled ), QgsPropertyDefinition( "dataDefinedMapGridEnabled", QObject::tr( "Grid enabled" ), QgsPropertyDefinition::Boolean ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridIntervalX ), QgsPropertyDefinition( "dataDefinedMapGridIntervalX", QObject::tr( "Grid interval X" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridIntervalY ), QgsPropertyDefinition( "dataDefinedMapGridIntervalY", QObject::tr( "Grid interval Y" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridOffsetX ), QgsPropertyDefinition( "dataDefinedMapGridOffsetX", QObject::tr( "Grid offset X" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridOffsetY ), QgsPropertyDefinition( "dataDefinedMapGridOffsetY", QObject::tr( "Grid offset Y" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameSize ), QgsPropertyDefinition( "dataDefinedMapGridFrameSize", QObject::tr( "Grid frame size" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameLineThickness ), QgsPropertyDefinition( "dataDefinedMapGridFrameLineThickness", QObject::tr( "Grid frame line thickness" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridCrossSize ), QgsPropertyDefinition( "dataDefinedMapGridCrossSize", QObject::tr( "Grid cross size" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameMargin ), QgsPropertyDefinition( "dataDefinedMapGridFrameMargin", QObject::tr( "Grid frame margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridLabelDistance ), QgsPropertyDefinition( "dataDefinedMapGridLabelDistance", QObject::tr( "Grid label distance" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayLeft ), QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayLeft", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display left" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayRight ), QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayRight", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display right" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayTop ), QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayTop", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display top" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayBottom ), QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayBottom", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display bottom" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsLeft ), QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsLeft", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display left" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsRight ), QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsRight", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display right" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsTop ), QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsTop", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display top" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsBottom ), QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsBottom", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display bottom" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapCrs ), QgsPropertyDefinition( "dataDefinedCrs", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map CRS" ), QObject::tr( "string representing a CRS, either an authority/id pair (e.g. 'EPSG:4326'), a proj string prefixes by \"PROJ:\" (e.g. 'PROJ: +proj=...') or a WKT string prefixed by \"WKT:\" (e.g. 'WKT:GEOGCRS[\"WGS 84\"...]')" ) ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::StartDateTime ), QgsPropertyDefinition( "dataDefinedStartDateTime", QObject::tr( "Temporal range start date / time" ), QgsPropertyDefinition::DateTime ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::EndDateTime ), QgsPropertyDefinition( "dataDefinedEndDateTime", QObject::tr( "Temporal range end date / time" ), QgsPropertyDefinition::DateTime ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapZRangeLower ), QgsPropertyDefinition( "dataDefinedZRangeLower", QObject::tr( "Z range lower limit" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::MapZRangeUpper ), QgsPropertyDefinition( "dataDefinedZRangeUpper", QObject::tr( "Z range upper limit" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PictureSource ), QgsPropertyDefinition( "dataDefinedSource", QObject::tr( "Picture source (URL)" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::SourceUrl ), QgsPropertyDefinition( "dataDefinedSourceUrl", QObject::tr( "Source URL" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PictureSvgBackgroundColor ), QgsPropertyDefinition( "dataDefinedSvgBackgroundColor", QObject::tr( "SVG background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PictureSvgStrokeColor ), QgsPropertyDefinition( "dataDefinedSvgStrokeColor", QObject::tr( "SVG stroke color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::PictureSvgStrokeWidth ), QgsPropertyDefinition( "dataDefinedSvgStrokeWidth", QObject::tr( "SVG stroke width" ), QgsPropertyDefinition::StrokeWidth ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::LegendTitle ), QgsPropertyDefinition( "dataDefinedLegendTitle", QObject::tr( "Legend title" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::LegendColumnCount ), QgsPropertyDefinition( "dataDefinedLegendColumns", QObject::tr( "Number of columns" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor ), QgsPropertyDefinition( "dataDefinedScalebarFill", QObject::tr( "Fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2 ), QgsPropertyDefinition( "dataDefinedScalebarFill2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor ), QgsPropertyDefinition( "dataDefinedScalebarLineColor", QObject::tr( "Line color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth ), QgsPropertyDefinition( "dataDefinedScalebarLineWidth", QObject::tr( "Line width" ), QgsPropertyDefinition::StrokeWidth ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::AttributeTableSourceLayer ), QgsPropertyDefinition( "dataDefinedAttributeTableSourceLayer", QObject::tr( "Table source layer" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments ), QgsPropertyDefinition( "dataDefinedScaleBarLeftSegments", QObject::tr( "Segments to the left of 0" ), QgsPropertyDefinition::IntegerPositive )},
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments ), QgsPropertyDefinition( "dataDefinedScaleBarRightSegments", QObject::tr( "Segments to the right of 0" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth ), QgsPropertyDefinition( "dataDefinedScalebarSegmentWidth", QObject::tr( "Length of a segment in map units" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth ), QgsPropertyDefinition( "dataDefinedScalebarMinWidth", QObject::tr( "Minimum length of a segment in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth ), QgsPropertyDefinition( "dataDefinedScalebarMaxWidth", QObject::tr( "Maximum length of a segment in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarHeight ), QgsPropertyDefinition( "dataDefinedScalebarHeight", QObject::tr( "Scalebar height in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight ), QgsPropertyDefinition( "dataDefinedScalebarSubdivisionHeight", QObject::tr( "Subdivision height in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions ), QgsPropertyDefinition( "dataDefinedScalebarRightSegmentSubdivisions", QObject::tr( "Number of subdivisions in segments to the right of 0" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance ), QgsPropertyDefinition( "dataDefinedElevationProfileTolerance", QObject::tr( "Tolerance" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileDistanceMajorInterval", QObject::tr( "Major grid line interval for elevation axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileDistanceMinorInterval", QObject::tr( "Minor grid line interval for elevation axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileDistanceLabelInterval", QObject::tr( "Label interval for elevation axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileElevationMajorInterval", QObject::tr( "Major grid line interval for distance axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileElevationMinorInterval", QObject::tr( "Minor grid line interval for distance axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval ), QgsPropertyDefinition( "dataDefinedElevationProfileElevationLabelInterval", QObject::tr( "Label interval for distance axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance ), QgsPropertyDefinition( "dataDefinedElevationProfileMinimumDistance", QObject::tr( "Minimum distance" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance ), QgsPropertyDefinition( "dataDefinedElevationProfileMaximumDistance", QObject::tr( "Maximum distance" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation ), QgsPropertyDefinition( "dataDefinedElevationProfileMinimumElevation", QObject::tr( "Minimum elevation" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation ), QgsPropertyDefinition( "dataDefinedElevationProfileMaximumElevation", QObject::tr( "Maximum elevation" ), QgsPropertyDefinition::Double ) },
  };
}

const QgsPropertiesDefinition &QgsLayoutObject::propertyDefinitions()
{
  QgsLayoutObject::initPropertyDefinitions();
  return sPropertyDefinitions;
}

bool QgsLayoutObject::propertyAssociatesWithParentMultiframe( QgsLayoutObject::DataDefinedProperty property )
{
  switch ( property )
  {
    case QgsLayoutObject::DataDefinedProperty::SourceUrl:
    case QgsLayoutObject::DataDefinedProperty::AttributeTableSourceLayer:
      return true;

    case QgsLayoutObject::DataDefinedProperty::NoProperty:
    case QgsLayoutObject::DataDefinedProperty::AllProperties:
    case QgsLayoutObject::DataDefinedProperty::TestProperty:
    case QgsLayoutObject::DataDefinedProperty::PresetPaperSize:
    case QgsLayoutObject::DataDefinedProperty::PaperWidth:
    case QgsLayoutObject::DataDefinedProperty::PaperHeight:
    case QgsLayoutObject::DataDefinedProperty::NumPages:
    case QgsLayoutObject::DataDefinedProperty::PaperOrientation:
    case QgsLayoutObject::DataDefinedProperty::PageNumber:
    case QgsLayoutObject::DataDefinedProperty::PositionX:
    case QgsLayoutObject::DataDefinedProperty::PositionY:
    case QgsLayoutObject::DataDefinedProperty::ItemWidth:
    case QgsLayoutObject::DataDefinedProperty::ItemHeight:
    case QgsLayoutObject::DataDefinedProperty::ItemRotation:
    case QgsLayoutObject::DataDefinedProperty::Transparency:
    case QgsLayoutObject::DataDefinedProperty::Opacity:
    case QgsLayoutObject::DataDefinedProperty::BlendMode:
    case QgsLayoutObject::DataDefinedProperty::ExcludeFromExports:
    case QgsLayoutObject::DataDefinedProperty::FrameColor:
    case QgsLayoutObject::DataDefinedProperty::BackgroundColor:
    case QgsLayoutObject::DataDefinedProperty::MapRotation:
    case QgsLayoutObject::DataDefinedProperty::MapScale:
    case QgsLayoutObject::DataDefinedProperty::MapXMin:
    case QgsLayoutObject::DataDefinedProperty::MapYMin:
    case QgsLayoutObject::DataDefinedProperty::MapXMax:
    case QgsLayoutObject::DataDefinedProperty::MapYMax:
    case QgsLayoutObject::DataDefinedProperty::MapAtlasMargin:
    case QgsLayoutObject::DataDefinedProperty::MapLayers:
    case QgsLayoutObject::DataDefinedProperty::MapStylePreset:
    case QgsLayoutObject::DataDefinedProperty::MapLabelMargin:
    case QgsLayoutObject::DataDefinedProperty::MapGridEnabled:
    case QgsLayoutObject::DataDefinedProperty::MapGridIntervalX:
    case QgsLayoutObject::DataDefinedProperty::MapGridIntervalY:
    case QgsLayoutObject::DataDefinedProperty::MapGridOffsetX:
    case QgsLayoutObject::DataDefinedProperty::MapGridOffsetY:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameSize:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameMargin:
    case QgsLayoutObject::DataDefinedProperty::MapGridLabelDistance:
    case QgsLayoutObject::DataDefinedProperty::MapGridCrossSize:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameLineThickness:
    case QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayLeft:
    case QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayRight:
    case QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayTop:
    case QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayBottom:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsLeft:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsRight:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsTop:
    case QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsBottom:
    case QgsLayoutObject::DataDefinedProperty::PictureSource:
    case QgsLayoutObject::DataDefinedProperty::PictureSvgBackgroundColor:
    case QgsLayoutObject::DataDefinedProperty::PictureSvgStrokeColor:
    case QgsLayoutObject::DataDefinedProperty::PictureSvgStrokeWidth:
    case QgsLayoutObject::DataDefinedProperty::LegendTitle:
    case QgsLayoutObject::DataDefinedProperty::LegendColumnCount:
    case QgsLayoutObject::DataDefinedProperty::ScalebarFillColor:
    case QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2:
    case QgsLayoutObject::DataDefinedProperty::ScalebarLineColor:
    case QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth:
    case QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments:
    case QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments:
    case QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth:
    case QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth:
    case QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth:
    case QgsLayoutObject::DataDefinedProperty::ScalebarHeight:
    case QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions:
    case QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight:
    case QgsLayoutObject::DataDefinedProperty::MapCrs:
    case QgsLayoutObject::DataDefinedProperty::StartDateTime:
    case QgsLayoutObject::DataDefinedProperty::EndDateTime:
    case QgsLayoutObject::DataDefinedProperty::MapZRangeLower:
    case QgsLayoutObject::DataDefinedProperty::MapZRangeUpper:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation:
    case QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation:
    case QgsLayoutObject::DataDefinedProperty::MarginLeft:
    case QgsLayoutObject::DataDefinedProperty::MarginRight:
    case QgsLayoutObject::DataDefinedProperty::MarginTop:
    case QgsLayoutObject::DataDefinedProperty::MarginBottom:
      return false;
  }
  return false;
}

QgsLayoutObject::QgsLayoutObject( QgsLayout *layout )
  : QObject( nullptr )
  , mLayout( layout )
{
  initPropertyDefinitions();

  if ( mLayout )
  {
    connect( mLayout, &QgsLayout::refreshed, this, &QgsLayoutObject::refresh );
    connect( &mLayout->reportContext(), &QgsLayoutReportContext::changed, this, &QgsLayoutObject::refresh );
  }
}

const QgsLayout *QgsLayoutObject::layout() const
{
  return mLayout.data();
}

QgsLayout *QgsLayoutObject::layout()
{
  return mLayout.data();
}

void QgsLayoutObject::setCustomProperty( const QString &key, const QVariant &value )
{
  mCustomProperties.setValue( key, value );
}

QVariant QgsLayoutObject::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsLayoutObject::removeCustomProperty( const QString &key )
{
  mCustomProperties.remove( key );
}

QStringList QgsLayoutObject::customProperties() const
{
  return mCustomProperties.keys();
}

QgsExpressionContext QgsLayoutObject::createExpressionContext() const
{
  if ( mLayout )
  {
    return mLayout->createExpressionContext();
  }
  else
  {
    return QgsExpressionContext() << QgsExpressionContextUtils::globalScope();
  }
}

bool QgsLayoutObject::writeObjectPropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  if ( parentElement.isNull() )
  {
    return false;
  }

  //create object element
  QDomElement objectElement = document.createElement( QStringLiteral( "LayoutObject" ) );

  QDomElement ddPropsElement = document.createElement( QStringLiteral( "dataDefinedProperties" ) );
  mDataDefinedProperties.writeXml( ddPropsElement, sPropertyDefinitions );
  objectElement.appendChild( ddPropsElement );

  //custom properties
  mCustomProperties.writeXml( objectElement, document );

  parentElement.appendChild( objectElement );
  return true;
}

bool QgsLayoutObject::readObjectPropertiesFromElement( const QDomElement &parentElement, const QDomDocument &document, const QgsReadWriteContext & )
{
  Q_UNUSED( document )
  if ( parentElement.isNull() )
  {
    return false;
  }

  const QDomNodeList objectNodeList = parentElement.elementsByTagName( QStringLiteral( "LayoutObject" ) );
  if ( objectNodeList.size() < 1 )
  {
    return false;
  }
  const QDomElement objectElement = objectNodeList.at( 0 ).toElement();

  const QDomNode propsNode = objectElement.namedItem( QStringLiteral( "dataDefinedProperties" ) );
  if ( !propsNode.isNull() )
  {
    mDataDefinedProperties.readXml( propsNode.toElement(), sPropertyDefinitions );
  }

  //custom properties
  mCustomProperties.readXml( objectElement );

  return true;
}
