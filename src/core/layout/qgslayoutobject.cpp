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

#include <QPainter>

#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoutobject.h"
#include "qgsfeedback.h"
#include "qgsexpressioncontextutils.h"


QgsPropertiesDefinition QgsLayoutObject::sPropertyDefinitions;

void QgsLayoutObject::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsLayoutObject::TestProperty, QgsPropertyDefinition( "dataDefinedProperty", QgsPropertyDefinition::DataTypeString, "invalid property", QString() ) },
    {
      QgsLayoutObject::PresetPaperSize, QgsPropertyDefinition( "dataDefinedPaperSize", QgsPropertyDefinition::DataTypeString, QObject::tr( "Paper size" ), QObject::tr( "string " ) + QLatin1String( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
          "|<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
          "|<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
          "|<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                                                                                                                                                                                   ) )
    },
    { QgsLayoutObject::PaperWidth, QgsPropertyDefinition( "dataDefinedPaperWidth", QObject::tr( "Page width" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::PaperHeight, QgsPropertyDefinition( "dataDefinedPaperHeight", QObject::tr( "Page height" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::NumPages, QgsPropertyDefinition( "dataDefinedNumPages", QObject::tr( "Number of pages" ), QgsPropertyDefinition::IntegerPositive ) },
    { QgsLayoutObject::PaperOrientation, QgsPropertyDefinition( "dataDefinedPaperOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QLatin1String( "[<b>portrait</b>|<b>landscape</b>]" ) ) },
    { QgsLayoutObject::PageNumber, QgsPropertyDefinition( "dataDefinedPageNumber", QObject::tr( "Page number" ), QgsPropertyDefinition::IntegerPositive ) },
    { QgsLayoutObject::PositionX, QgsPropertyDefinition( "dataDefinedPositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::PositionY, QgsPropertyDefinition( "dataDefinedPositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::ItemWidth, QgsPropertyDefinition( "dataDefinedWidth", QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ItemHeight, QgsPropertyDefinition( "dataDefinedHeight", QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ItemRotation, QgsPropertyDefinition( "dataDefinedRotation", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation ) },
    { QgsLayoutObject::Transparency, QgsPropertyDefinition( "dataDefinedTransparency", QObject::tr( "Transparency" ), QgsPropertyDefinition::Opacity ) },
    { QgsLayoutObject::Opacity, QgsPropertyDefinition( "dataDefinedOpacity", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity ) },
    { QgsLayoutObject::BlendMode, QgsPropertyDefinition( "dataDefinedBlendMode", QObject::tr( "Blend mode" ), QgsPropertyDefinition::BlendMode ) },
    { QgsLayoutObject::ExcludeFromExports, QgsPropertyDefinition( "dataDefinedExcludeExports", QObject::tr( "Exclude item from exports" ), QgsPropertyDefinition::Boolean ) },
    { QgsLayoutObject::FrameColor, QgsPropertyDefinition( "dataDefinedFrameColor", QObject::tr( "Frame color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::BackgroundColor, QgsPropertyDefinition( "dataDefinedBackgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::MapRotation, QgsPropertyDefinition( "dataDefinedMapRotation", QObject::tr( "Map rotation" ), QgsPropertyDefinition::Rotation ) },
    { QgsLayoutObject::MapScale, QgsPropertyDefinition( "dataDefinedMapScale", QObject::tr( "Map scale" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapXMin, QgsPropertyDefinition( "dataDefinedMapXMin", QObject::tr( "Extent minimum X" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapYMin, QgsPropertyDefinition( "dataDefinedMapYMin", QObject::tr( "Extent minimum Y" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapXMax, QgsPropertyDefinition( "dataDefinedMapXMax", QObject::tr( "Extent maximum X" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapYMax, QgsPropertyDefinition( "dataDefinedMapYMax", QObject::tr( "Extent maximum Y" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapLabelMargin, QgsPropertyDefinition( "dataDefinedMapLabelMargin", QObject::tr( "Label margin" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapAtlasMargin, QgsPropertyDefinition( "dataDefinedMapAtlasMargin", QObject::tr( "Atlas margin" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapLayers, QgsPropertyDefinition( "dataDefinedMapLayers", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map Layers" ), tr( "list of map layer names separated by | characters" ) ) },
    { QgsLayoutObject::MapStylePreset, QgsPropertyDefinition( "dataDefinedMapStylePreset", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map theme" ), tr( "name of an existing map theme (case-sensitive)" ) ) },
    { QgsLayoutObject::MapGridEnabled, QgsPropertyDefinition( "dataDefinedMapGridEnabled", QObject::tr( "Grid enabled" ), QgsPropertyDefinition::Boolean ) },
    { QgsLayoutObject::MapGridIntervalX, QgsPropertyDefinition( "dataDefinedMapGridIntervalX", QObject::tr( "Grid interval X" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridIntervalY, QgsPropertyDefinition( "dataDefinedMapGridIntervalY", QObject::tr( "Grid interval Y" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridOffsetX, QgsPropertyDefinition( "dataDefinedMapGridOffsetX", QObject::tr( "Grid offset X" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapGridOffsetY, QgsPropertyDefinition( "dataDefinedMapGridOffsetY", QObject::tr( "Grid offset Y" ), QgsPropertyDefinition::Double ) },
    { QgsLayoutObject::MapGridFrameSize, QgsPropertyDefinition( "dataDefinedMapGridFrameSize", QObject::tr( "Grid frame size" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridFrameLineThickness, QgsPropertyDefinition( "dataDefinedMapGridFrameLineThickness", QObject::tr( "Grid frame line thickness" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridCrossSize, QgsPropertyDefinition( "dataDefinedMapGridCrossSize", QObject::tr( "Grid cross size" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridFrameMargin, QgsPropertyDefinition( "dataDefinedMapGridFrameMargin", QObject::tr( "Grid frame margin" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridLabelDistance, QgsPropertyDefinition( "dataDefinedMapGridLabelDistance", QObject::tr( "Grid label distance" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::MapGridAnnotationDisplayLeft, QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayLeft", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display left" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridAnnotationDisplayRight, QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayRight", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display right" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridAnnotationDisplayTop, QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayTop", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display top" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridAnnotationDisplayBottom, QgsPropertyDefinition( "dataDefinedMapGridAnnotationDisplayBottom", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid annotation display bottom" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridFrameDivisionsLeft, QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsLeft", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display left" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridFrameDivisionsRight, QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsRight", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display right" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridFrameDivisionsTop, QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsTop", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display top" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::MapGridFrameDivisionsBottom, QgsPropertyDefinition( "dataDefinedMapGridFrameDivisionsBottom", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map grid frame divisions display bottom" ), QObject::tr( "string " ) + QLatin1String( "[<b>all</b>|<b>x_only</b>|<b>y_only</b>|<b>disabled</b>]" ) ) },
    { QgsLayoutObject::PictureSource, QgsPropertyDefinition( "dataDefinedSource", QObject::tr( "Picture source (URL)" ), QgsPropertyDefinition::String ) },
    { QgsLayoutObject::SourceUrl, QgsPropertyDefinition( "dataDefinedSourceUrl", QObject::tr( "Source URL" ), QgsPropertyDefinition::String ) },
    { QgsLayoutObject::PictureSvgBackgroundColor, QgsPropertyDefinition( "dataDefinedSvgBackgroundColor", QObject::tr( "SVG background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::PictureSvgStrokeColor, QgsPropertyDefinition( "dataDefinedSvgStrokeColor", QObject::tr( "SVG stroke color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::PictureSvgStrokeWidth, QgsPropertyDefinition( "dataDefinedSvgStrokeWidth", QObject::tr( "SVG stroke width" ), QgsPropertyDefinition::StrokeWidth ) },
    { QgsLayoutObject::LegendTitle, QgsPropertyDefinition( "dataDefinedLegendTitle", QObject::tr( "Legend title" ), QgsPropertyDefinition::String ) },
    { QgsLayoutObject::LegendColumnCount, QgsPropertyDefinition( "dataDefinedLegendColumns", QObject::tr( "Number of columns" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) },
    { QgsLayoutObject::ScalebarFillColor, QgsPropertyDefinition( "dataDefinedScalebarFill", QObject::tr( "Fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::ScalebarFillColor2, QgsPropertyDefinition( "dataDefinedScalebarFill2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::ScalebarLineColor, QgsPropertyDefinition( "dataDefinedScalebarLineColor", QObject::tr( "Line color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsLayoutObject::ScalebarLineWidth, QgsPropertyDefinition( "dataDefinedScalebarLineWidth", QObject::tr( "Line width" ), QgsPropertyDefinition::StrokeWidth ) },
    { QgsLayoutObject::AttributeTableSourceLayer, QgsPropertyDefinition( "dataDefinedAttributeTableSourceLayer", QObject::tr( "Table source layer" ), QgsPropertyDefinition::String ) },
    { QgsLayoutObject::MapCrs, QgsPropertyDefinition( "dataDefinedCrs", QgsPropertyDefinition::DataTypeString, QObject::tr( "Map CRS" ), QObject::tr( "string representing a CRS, either an authority/id pair (e.g. 'EPSG:4326'), a proj string prefixes by \"PROJ:\" (e.g. 'PROJ: +proj=...') or a WKT string prefixed by \"WKT:\" (e.g. 'WKT:GEOGCRS[\"WGS 84\"...]')" ) ) },
    { QgsLayoutObject::StartDateTime, QgsPropertyDefinition( "dataDefinedStartDateTime", QObject::tr( "Temporal range start date / time" ), QgsPropertyDefinition::DateTime ) },
    { QgsLayoutObject::EndDateTime, QgsPropertyDefinition( "dataDefinedEndDateTime", QObject::tr( "Temporal range end date / time" ), QgsPropertyDefinition::DateTime ) },
    { QgsLayoutObject::ScalebarLeftSegments, QgsPropertyDefinition( "dataDefinedScaleBarLeftSegments", QObject::tr( "Segments to the left of 0" ), QgsPropertyDefinition::IntegerPositive )},
    { QgsLayoutObject::ScalebarRightSegments, QgsPropertyDefinition( "dataDefinedScaleBarRightSegments", QObject::tr( "Segments to the right of 0" ), QgsPropertyDefinition::IntegerPositive ) },
    { QgsLayoutObject::ScalebarSegmentWidth, QgsPropertyDefinition( "dataDefinedScalebarSegmentWidth", QObject::tr( "Length of a segment in map units" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ScalebarMinimumWidth, QgsPropertyDefinition( "dataDefinedScalebarMinWidth", QObject::tr( "Minimum length of a segment in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ScalebarMaximumWidth, QgsPropertyDefinition( "dataDefinedScalebarMaxWidth", QObject::tr( "Maximum length of a segment in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ScalebarHeight, QgsPropertyDefinition( "dataDefinedScalebarHeight", QObject::tr( "Scalebar height in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ScalebarSubdivisionHeight, QgsPropertyDefinition( "dataDefinedScalebarSubdivisionHeight", QObject::tr( "Subdivision height in mm" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsLayoutObject::ScalebarRightSegmentSubdivisions, QgsPropertyDefinition( "dataDefinedScalebarRightSegmentSubdivisions", QObject::tr( "Number of subdivisions in segments to the right of 0" ), QgsPropertyDefinition::IntegerPositive ) },
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
    case QgsLayoutObject::SourceUrl:
    case QgsLayoutObject::AttributeTableSourceLayer:
      return true;

    case QgsLayoutObject::NoProperty:
    case QgsLayoutObject::AllProperties:
    case QgsLayoutObject::TestProperty:
    case QgsLayoutObject::PresetPaperSize:
    case QgsLayoutObject::PaperWidth:
    case QgsLayoutObject::PaperHeight:
    case QgsLayoutObject::NumPages:
    case QgsLayoutObject::PaperOrientation:
    case QgsLayoutObject::PageNumber:
    case QgsLayoutObject::PositionX:
    case QgsLayoutObject::PositionY:
    case QgsLayoutObject::ItemWidth:
    case QgsLayoutObject::ItemHeight:
    case QgsLayoutObject::ItemRotation:
    case QgsLayoutObject::Transparency:
    case QgsLayoutObject::Opacity:
    case QgsLayoutObject::BlendMode:
    case QgsLayoutObject::ExcludeFromExports:
    case QgsLayoutObject::FrameColor:
    case QgsLayoutObject::BackgroundColor:
    case QgsLayoutObject::MapRotation:
    case QgsLayoutObject::MapScale:
    case QgsLayoutObject::MapXMin:
    case QgsLayoutObject::MapYMin:
    case QgsLayoutObject::MapXMax:
    case QgsLayoutObject::MapYMax:
    case QgsLayoutObject::MapAtlasMargin:
    case QgsLayoutObject::MapLayers:
    case QgsLayoutObject::MapStylePreset:
    case QgsLayoutObject::MapLabelMargin:
    case QgsLayoutObject::MapGridEnabled:
    case QgsLayoutObject::MapGridIntervalX:
    case QgsLayoutObject::MapGridIntervalY:
    case QgsLayoutObject::MapGridOffsetX:
    case QgsLayoutObject::MapGridOffsetY:
    case QgsLayoutObject::MapGridFrameSize:
    case QgsLayoutObject::MapGridFrameMargin:
    case QgsLayoutObject::MapGridLabelDistance:
    case QgsLayoutObject::MapGridCrossSize:
    case QgsLayoutObject::MapGridFrameLineThickness:
    case QgsLayoutObject::MapGridAnnotationDisplayLeft:
    case QgsLayoutObject::MapGridAnnotationDisplayRight:
    case QgsLayoutObject::MapGridAnnotationDisplayTop:
    case QgsLayoutObject::MapGridAnnotationDisplayBottom:
    case QgsLayoutObject::MapGridFrameDivisionsLeft:
    case QgsLayoutObject::MapGridFrameDivisionsRight:
    case QgsLayoutObject::MapGridFrameDivisionsTop:
    case QgsLayoutObject::MapGridFrameDivisionsBottom:
    case QgsLayoutObject::PictureSource:
    case QgsLayoutObject::PictureSvgBackgroundColor:
    case QgsLayoutObject::PictureSvgStrokeColor:
    case QgsLayoutObject::PictureSvgStrokeWidth:
    case QgsLayoutObject::LegendTitle:
    case QgsLayoutObject::LegendColumnCount:
    case QgsLayoutObject::ScalebarFillColor:
    case QgsLayoutObject::ScalebarFillColor2:
    case QgsLayoutObject::ScalebarLineColor:
    case QgsLayoutObject::ScalebarLineWidth:
    case QgsLayoutObject::ScalebarLeftSegments:
    case QgsLayoutObject::ScalebarRightSegments:
    case QgsLayoutObject::ScalebarSegmentWidth:
    case QgsLayoutObject::ScalebarMinimumWidth:
    case QgsLayoutObject::ScalebarMaximumWidth:
    case QgsLayoutObject::ScalebarHeight:
    case QgsLayoutObject::ScalebarRightSegmentSubdivisions:
    case QgsLayoutObject::ScalebarSubdivisionHeight:
    case QgsLayoutObject::MapCrs:
    case QgsLayoutObject::StartDateTime:
    case QgsLayoutObject::EndDateTime:
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
