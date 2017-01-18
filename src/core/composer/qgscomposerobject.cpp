/***************************************************************************
                            qgscomposerobject.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson,Radim Blazek
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

#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgscomposerobject.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

const QgsPropertiesDefinition QgsComposerObject::PROPERTY_DEFINITIONS
{
  { QgsComposerObject::TestProperty, QgsPropertyDefinition( "dataDefinedProperty" , QgsPropertyDefinition::DataTypeString, "invalid property", QString() ) },
  { QgsComposerObject::PresetPaperSize, QgsPropertyDefinition( "dataDefinedPaperSize" , QgsPropertyDefinition::DataTypeString, QObject::tr( "Paper size" ), QObject::tr( "string " ) + QLatin1String( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
      "<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
      "<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
      "<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                                                                                                                                                                                    ) ) },
  { QgsComposerObject::PaperWidth, QgsPropertyDefinition( "dataDefinedPaperWidth", QObject::tr( "Page width" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::PaperHeight, QgsPropertyDefinition( "dataDefinedPaperHeight" , QObject::tr( "Page height" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::NumPages, QgsPropertyDefinition( "dataDefinedNumPages" , QObject::tr( "Number of pages" ), QgsPropertyDefinition::IntegerPositive ) },
  { QgsComposerObject::PaperOrientation, QgsPropertyDefinition( "dataDefinedPaperOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QLatin1String( "[<b>portrait</b>|<b>landscape</b>]" ) ) },
  { QgsComposerObject::PageNumber, QgsPropertyDefinition( "dataDefinedPageNumber" , QObject::tr( "Page number" ), QgsPropertyDefinition::IntegerPositive ) },
  { QgsComposerObject::PositionX, QgsPropertyDefinition( "dataDefinedPositionX" , QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::PositionY, QgsPropertyDefinition( "dataDefinedPositionY" , QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::ItemWidth, QgsPropertyDefinition( "dataDefinedWidth" , QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::ItemHeight, QgsPropertyDefinition( "dataDefinedHeight" , QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::ItemRotation, QgsPropertyDefinition( "dataDefinedRotation" , QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::Transparency, QgsPropertyDefinition( "dataDefinedTransparency" , QObject::tr( "Transparency" ), QgsPropertyDefinition::Transparency ) },
  { QgsComposerObject::BlendMode, QgsPropertyDefinition( "dataDefinedBlendMode" , QObject::tr( "Blend mode" ), QgsPropertyDefinition::BlendMode ) },
  { QgsComposerObject::ExcludeFromExports, QgsPropertyDefinition( "dataDefinedExcludeExports", QObject::tr( "Exclude item from exports" ), QgsPropertyDefinition::Boolean ) },
  { QgsComposerObject::FrameColor, QgsPropertyDefinition( "dataDefinedFrameColor", QObject::tr( "Frame color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::BackgroundColor, QgsPropertyDefinition( "dataDefinedBackgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::MapRotation, QgsPropertyDefinition( "dataDefinedMapRotation" , QObject::tr( "Map rotation" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::MapScale, QgsPropertyDefinition( "dataDefinedMapScale" , QObject::tr( "Map scale" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::MapXMin, QgsPropertyDefinition( "dataDefinedMapXMin" , QObject::tr( "Extent minimum X" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::MapYMin, QgsPropertyDefinition( "dataDefinedMapYMin" , QObject::tr( "Extent minimum Y" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::MapXMax, QgsPropertyDefinition( "dataDefinedMapXMax" , QObject::tr( "Extent maximum X" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::MapYMax, QgsPropertyDefinition( "dataDefinedMapYMax" , QObject::tr( "Extent maximum Y" ), QgsPropertyDefinition::Double ) },
  { QgsComposerObject::MapAtlasMargin, QgsPropertyDefinition( "dataDefinedMapAtlasMargin" , QObject::tr( "Atlas margin" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::MapLayers, QgsPropertyDefinition( "dataDefinedMapLayers", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), tr( "list of map layer names separated by | characters" ) ) },
  { QgsComposerObject::MapStylePreset, QgsPropertyDefinition( "dataDefinedMapStylePreset", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), tr( "list of map layer names separated by | characters" ) ) },
  { QgsComposerObject::PictureSource, QgsPropertyDefinition( "dataDefinedSource" , QObject::tr( "Picture source (URL)" ), QgsPropertyDefinition::String ) },
  { QgsComposerObject::SourceUrl, QgsPropertyDefinition( "dataDefinedSourceUrl" , QObject::tr( "Source URL" ), QgsPropertyDefinition::String ) },
  { QgsComposerObject::PictureSvgBackgroundColor, QgsPropertyDefinition( "dataDefinedSvgBackgroundColor" , QObject::tr( "SVG background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::PictureSvgOutlineColor, QgsPropertyDefinition( "dataDefinedSvgOutlineColor" , QObject::tr( "SVG outline color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::PictureSvgOutlineWidth, QgsPropertyDefinition( "dataDefinedSvgOutlineWidth" , QObject::tr( "SVG outline width" ), QgsPropertyDefinition::DoublePositive ) },
  { QgsComposerObject::LegendTitle, QgsPropertyDefinition( "dataDefinedLegendTitle" , QObject::tr( "Legend title" ), QgsPropertyDefinition::String ) },
  { QgsComposerObject::LegendColumnCount, QgsPropertyDefinition( "dataDefinedLegendColumns" , QObject::tr( "Number of columns" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) },
  { QgsComposerObject::ScalebarFillColor, QgsPropertyDefinition( "dataDefinedScalebarFill" , QObject::tr( "Fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::ScalebarFillColor2, QgsPropertyDefinition( "dataDefinedScalebarFill2" , QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::ScalebarLineColor, QgsPropertyDefinition( "dataDefinedScalebarLineColor" , QObject::tr( "Line color" ), QgsPropertyDefinition::ColorWithAlpha ) },
  { QgsComposerObject::ScalebarLineWidth, QgsPropertyDefinition( "dataDefinedScalebarLineWidth" , QObject::tr( "Line width" ), QgsPropertyDefinition::DoublePositive ) },
};


QgsComposerObject::QgsComposerObject( QgsComposition* composition )
    : QObject( nullptr )
    , mComposition( composition )
{

  // data defined strings

  if ( mComposition )
  {
    //connect to atlas toggling on/off and coverage layer and feature changes
    //to update data defined values
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::toggled, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, [this] { refreshDataDefinedProperty(); } );
    //also, refreshing composition triggers a recalculation of data defined properties
    connect( mComposition, &QgsComposition::refreshItemsTriggered, this, [this] { refreshDataDefinedProperty(); } );

    //toggling atlas or changing coverage layer requires data defined expressions to be reprepared
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::toggled, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, [this] { refreshDataDefinedProperty(); } );
  }

}

bool QgsComposerObject::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement ddPropsElement = doc.createElement( QStringLiteral( "dataDefinedProperties" ) );
  mProperties.writeXml( ddPropsElement, doc, PROPERTY_DEFINITIONS );
  elem.appendChild( ddPropsElement );

  //custom properties
  mCustomProperties.writeXml( elem, doc );

  return true;
}

bool QgsComposerObject::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  //old (pre 3.0) data defined properties
  QgsComposerUtils::readOldDataDefinedPropertyMap( itemElem, mProperties );

  QDomNode propsNode = itemElem.namedItem( QStringLiteral( "dataDefinedProperties" ) );
  if ( !propsNode.isNull() )
  {
    mProperties.readXml( propsNode.toElement(), doc, PROPERTY_DEFINITIONS );
  }

  //custom properties
  mCustomProperties.readXml( itemElem );

  return true;
}

void QgsComposerObject::repaint()
{
  //nothing to do in base class for now
}

void QgsComposerObject::refreshDataDefinedProperty( const DataDefinedProperty property, const QgsExpressionContext *context )
{
  Q_UNUSED( property );
  Q_UNUSED( context );

  //nothing to do in base class for now
}

void QgsComposerObject::prepareProperties() const
{
  QgsExpressionContext context = createExpressionContext();
  mProperties.prepare( context );
}

void QgsComposerObject::setCustomProperty( const QString& key, const QVariant& value )
{
  mCustomProperties.setValue( key, value );
}

QVariant QgsComposerObject::customProperty( const QString& key, const QVariant& defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsComposerObject::removeCustomProperty( const QString& key )
{
  mCustomProperties.remove( key );
}

QStringList QgsComposerObject::customProperties() const
{
  return mCustomProperties.keys();
}

QgsExpressionContext QgsComposerObject::createExpressionContext() const
{
  if ( mComposition )
  {
    return mComposition->createExpressionContext();
  }
  else
  {
    return QgsExpressionContext() << QgsExpressionContextUtils::globalScope();
  }
}
