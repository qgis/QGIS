/***************************************************************************
  qgscompositionconverter.cpp - QgsCompositionConverter

 ---------------------
 begin                : 13.12.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgscompositionconverter.h"
#include "qgsreadwritecontext.h"
#include "qgslayout.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemgroup.h"
#include "qgsfontutils.h"
#include "qgspainting.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgssymbollayer.h"
#include "qgsproject.h"

#include "qgslayoutitemregistry.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"

QgsPropertiesDefinition QgsCompositionConverter::sPropertyDefinitions;

void QgsCompositionConverter::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsCompositionConverter::TestProperty, QgsPropertyDefinition( "dataDefinedProperty", QgsPropertyDefinition::DataTypeString, "invalid property", QString() ) },
    {
      QgsCompositionConverter::PresetPaperSize, QgsPropertyDefinition( "dataDefinedPaperSize", QgsPropertyDefinition::DataTypeString, QObject::tr( "Paper size" ), QObject::tr( "string " ) + QStringLiteral( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
          "<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
          "<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
          "<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                                                                                                                                                                                            ) )
    },
    { QgsCompositionConverter::PaperWidth, QgsPropertyDefinition( "dataDefinedPaperWidth", QObject::tr( "Page width" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::PaperHeight, QgsPropertyDefinition( "dataDefinedPaperHeight", QObject::tr( "Page height" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::NumPages, QgsPropertyDefinition( "dataDefinedNumPages", QObject::tr( "Number of pages" ), QgsPropertyDefinition::IntegerPositive ) },
    { QgsCompositionConverter::PaperOrientation, QgsPropertyDefinition( "dataDefinedPaperOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + QStringLiteral( "[<b>portrait</b>|<b>landscape</b>]" ) ) },
    { QgsCompositionConverter::PageNumber, QgsPropertyDefinition( "dataDefinedPageNumber", QObject::tr( "Page number" ), QgsPropertyDefinition::IntegerPositive ) },
    { QgsCompositionConverter::PositionX, QgsPropertyDefinition( "dataDefinedPositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::PositionY, QgsPropertyDefinition( "dataDefinedPositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::ItemWidth, QgsPropertyDefinition( "dataDefinedWidth", QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::ItemHeight, QgsPropertyDefinition( "dataDefinedHeight", QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::ItemRotation, QgsPropertyDefinition( "dataDefinedRotation", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation ) },
    { QgsCompositionConverter::Transparency, QgsPropertyDefinition( "dataDefinedTransparency", QObject::tr( "Transparency" ), QgsPropertyDefinition::Opacity ) },
    { QgsCompositionConverter::Opacity, QgsPropertyDefinition( "dataDefinedOpacity", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity ) },
    { QgsCompositionConverter::BlendMode, QgsPropertyDefinition( "dataDefinedBlendMode", QObject::tr( "Blend mode" ), QgsPropertyDefinition::BlendMode ) },
    { QgsCompositionConverter::ExcludeFromExports, QgsPropertyDefinition( "dataDefinedExcludeExports", QObject::tr( "Exclude item from exports" ), QgsPropertyDefinition::Boolean ) },
    { QgsCompositionConverter::FrameColor, QgsPropertyDefinition( "dataDefinedFrameColor", QObject::tr( "Frame color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::BackgroundColor, QgsPropertyDefinition( "dataDefinedBackgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::MapRotation, QgsPropertyDefinition( "dataDefinedMapRotation", QObject::tr( "Map rotation" ), QgsPropertyDefinition::Rotation ) },
    { QgsCompositionConverter::MapScale, QgsPropertyDefinition( "dataDefinedMapScale", QObject::tr( "Map scale" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::MapXMin, QgsPropertyDefinition( "dataDefinedMapXMin", QObject::tr( "Extent minimum X" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::MapYMin, QgsPropertyDefinition( "dataDefinedMapYMin", QObject::tr( "Extent minimum Y" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::MapXMax, QgsPropertyDefinition( "dataDefinedMapXMax", QObject::tr( "Extent maximum X" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::MapYMax, QgsPropertyDefinition( "dataDefinedMapYMax", QObject::tr( "Extent maximum Y" ), QgsPropertyDefinition::Double ) },
    { QgsCompositionConverter::MapAtlasMargin, QgsPropertyDefinition( "dataDefinedMapAtlasMargin", QObject::tr( "Atlas margin" ), QgsPropertyDefinition::DoublePositive ) },
    { QgsCompositionConverter::MapLayers, QgsPropertyDefinition( "dataDefinedMapLayers", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "list of map layer names separated by | characters" ) ) },
    { QgsCompositionConverter::MapStylePreset, QgsPropertyDefinition( "dataDefinedMapStylePreset", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "list of map layer names separated by | characters" ) ) },
    { QgsCompositionConverter::PictureSource, QgsPropertyDefinition( "dataDefinedSource", QObject::tr( "Picture source (URL)" ), QgsPropertyDefinition::String ) },
    { QgsCompositionConverter::SourceUrl, QgsPropertyDefinition( "dataDefinedSourceUrl", QObject::tr( "Source URL" ), QgsPropertyDefinition::String ) },
    { QgsCompositionConverter::PictureSvgBackgroundColor, QgsPropertyDefinition( "dataDefinedSvgBackgroundColor", QObject::tr( "SVG background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::PictureSvgStrokeColor, QgsPropertyDefinition( "dataDefinedSvgStrokeColor", QObject::tr( "SVG stroke color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::PictureSvgStrokeWidth, QgsPropertyDefinition( "dataDefinedSvgStrokeWidth", QObject::tr( "SVG stroke width" ), QgsPropertyDefinition::StrokeWidth ) },
    { QgsCompositionConverter::LegendTitle, QgsPropertyDefinition( "dataDefinedLegendTitle", QObject::tr( "Legend title" ), QgsPropertyDefinition::String ) },
    { QgsCompositionConverter::LegendColumnCount, QgsPropertyDefinition( "dataDefinedLegendColumns", QObject::tr( "Number of columns" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) },
    { QgsCompositionConverter::ScalebarFillColor, QgsPropertyDefinition( "dataDefinedScalebarFill", QObject::tr( "Fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::ScalebarFillColor2, QgsPropertyDefinition( "dataDefinedScalebarFill2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::ScalebarLineColor, QgsPropertyDefinition( "dataDefinedScalebarLineColor", QObject::tr( "Line color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { QgsCompositionConverter::ScalebarLineWidth, QgsPropertyDefinition( "dataDefinedScalebarLineWidth", QObject::tr( "Line width" ), QgsPropertyDefinition::StrokeWidth ) },
  };
}

QgsPropertiesDefinition QgsCompositionConverter::propertyDefinitions()
{
  QgsCompositionConverter::initPropertyDefinitions();
  return sPropertyDefinitions;
}


QgsLayout *QgsCompositionConverter::createLayoutFromCompositionXml( const QDomElement &parentElement, const QgsReadWriteContext &context )
{
  initPropertyDefinitions();
  QgsLayout *layout = new QgsLayout( QgsProject::instance( ) );
  // Create pages
  int pages = parentElement.attribute( QStringLiteral( "numPages" ) ).toInt( );
  float paperHeight = parentElement.attribute( QStringLiteral( "paperHeight" ) ).toFloat( );
  float paperWidth = parentElement.attribute( QStringLiteral( "paperWidth" ) ).toFloat( );

  if ( parentElement.elementsByTagName( QStringLiteral( "symbol" ) ).size() )
  {
    QDomElement symbolElement = parentElement.elementsByTagName( QStringLiteral( "symbol" ) ).at( 0 ).toElement();
    QgsFillSymbol *pageSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context );
    if ( pageSymbol )
      layout->pageCollection()->setPageStyleSymbol( pageSymbol );
  }

  QString name = parentElement.attribute( QStringLiteral( "name" ) );
  layout->setName( name );
  // TODO: check that it is always landscape
  QgsLayoutSize pageSize( paperWidth, paperHeight );
  for ( int j = 0; j < pages; j++ )
  {
    QgsLayoutItemPage *page = QgsLayoutItemPage::create( layout );
    page->setPageSize( pageSize );
    layout->pageCollection()->addPage( page );
  }
  addItemsFromCompositionXml( layout, parentElement, context );
  return layout;
}

void QgsCompositionConverter::adjustPos( QgsLayout *layout, QgsLayoutItem *layoutItem, QDomNode &itemNode, QPointF *position, bool &pasteInPlace, int zOrderOffset, QPointF &pasteShiftPos, int &pageNumber )
{
  if ( position )
  {
    if ( pasteInPlace )
    {
      QgsLayoutPoint posOnPage = QgsLayoutPoint::decodePoint( itemNode.toElement().attribute( QStringLiteral( "positionOnPage" ) ) );
      layoutItem->attemptMove( posOnPage, true, false, pageNumber );
    }
    else
    {
      layoutItem->attemptMoveBy( pasteShiftPos.x(), pasteShiftPos.y() );
    }
  }

  layout->addLayoutItem( layoutItem );
  layoutItem->setZValue( layoutItem->zValue() + zOrderOffset );
}

void QgsCompositionConverter::restoreGeneralComposeItemProperties( QgsLayoutItem *layoutItem, const QDomElement &itemElem )
{
  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    //rotation
    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
    {
      //check for old (pre 2.1) rotation attribute
      layoutItem->setItemRotation( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), false );
    }

    QgsCompositionConverter::readXml( layoutItem, composerItemElem );

    // Frame color

    // Background color
  }
}

QList<QgsLayoutItem *> QgsCompositionConverter::addItemsFromCompositionXml( QgsLayout *layout, const QDomElement &parentElement, const QgsReadWriteContext &context, QPointF *position, bool pasteInPlace )
{

  initPropertyDefinitions();

  QList< QgsLayoutItem * > newItems;
  // Not used: QList< QgsLayoutMultiFrame * > newMultiFrames;

  //if we are adding items to a layout which already contains items, we need to make sure
  //these items are placed at the top of the layout and that zValues are not duplicated
  //so, calculate an offset which needs to be added to the zValue of created items
  int zOrderOffset = layout->mItemsModel->zOrderListSize();

  QPointF pasteShiftPos;
  int pageNumber = -1;
  if ( position )
  {
    //If we are placing items relative to a certain point, then calculate how much we need
    //to shift the items by so that they are placed at this point
    //First, calculate the minimum position from the xml
    // TODO: check this!
    QPointF minItemPos = layout->minPointFromXml( parentElement );
    //next, calculate how much each item needs to be shifted from its original position
    //so that it's placed at the correct relative position
    pasteShiftPos = *position - minItemPos;
    if ( pasteInPlace )
    {
      pageNumber = layout->mPageCollection->pageNumberForPoint( *position );
    }
  }

  /*
  LayoutPage, //!< Page items
  LayoutMap, //!< Map item
  LayoutPicture, //!< Picture item
  LayoutLabel, //!< Label item
  LayoutLegend, //!< Legend item
  LayoutShape, //!< Shape item
  LayoutPolygon, //!< Polygon shape item
  LayoutPolyline, //!< Polyline shape item
  LayoutScaleBar, //!< Scale bar item
  LayoutFrame, //!< Frame item, part of a QgsLayoutMultiFrame object

  // known multi-frame types
  LayoutHtml, //!< Html multiframe item
  LayoutAttributeTable, //!< Attribute table
  LayoutTextTable, //!< Preset text table
  */



  // Label
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerLabel" ) ).size(); i++ )
  {
    QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerLabel" ) ).at( i ) );
    QgsLayoutItemLabel *layoutItem = new QgsLayoutItemLabel( layout );
    readLabelXml( layoutItem, itemNode.toElement(), context );
    adjustPos( layout, layoutItem, itemNode, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Shape
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerShape" ) ).size(); i++ )
  {
    QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerShape" ) ).at( i ) );
    QgsLayoutItemShape *layoutItem = new QgsLayoutItemShape( layout );
    readShapeXml( layoutItem, itemNode.toElement() );
    adjustPos( layout, layoutItem, itemNode, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  return newItems;
}

bool QgsCompositionConverter::readLabelXml( QgsLayoutItemLabel *label, const QDomElement &itemElem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );
  if ( itemElem.isNull() )
  {
    return false;
  }

  restoreGeneralComposeItemProperties( label, itemElem );

  //restore label specific properties

  //text
  label->setText( itemElem.attribute( QStringLiteral( "labelText" ) ) );

  //html state
  label->setMode( itemElem.attribute( QStringLiteral( "htmlState" ) ).toInt() == Qt::Checked ? QgsLayoutItemLabel::Mode::ModeHtml : QgsLayoutItemLabel::Mode::ModeFont );

  //margin
  bool marginXOk = false;
  bool marginYOk = false;
  double marginX = itemElem.attribute( QStringLiteral( "marginX" ) ).toDouble( &marginXOk );
  double marginY = itemElem.attribute( QStringLiteral( "marginY" ) ).toDouble( &marginYOk );
  if ( !marginXOk || !marginYOk )
  {
    //upgrade old projects where margins where stored in a single attribute
    double margin = itemElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "1.0" ) ).toDouble();
    marginX = margin;
    marginY = margin;
  }
  label->setMarginX( marginX );
  label->setMarginY( marginY );

  //Horizontal alignment
  label->setHAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "halign" ) ).toInt() ) );

  //Vertical alignment
  label->setVAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "valign" ) ).toInt() ) );


  QFont font;
  //font
  QgsFontUtils::setFromXmlChildNode( font, itemElem, QStringLiteral( "LabelFont" ) );
  label->setFont( font );

  //font color
  QDomNodeList fontColorList = itemElem.elementsByTagName( QStringLiteral( "FontColor" ) );
  if ( !fontColorList.isEmpty() )
  {
    QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
    int red = fontColorElem.attribute( QStringLiteral( "red" ), QStringLiteral( "0" ) ).toInt();
    int green = fontColorElem.attribute( QStringLiteral( "green" ), QStringLiteral( "0" ) ).toInt();
    int blue = fontColorElem.attribute( QStringLiteral( "blue" ), QStringLiteral( "0" ) ).toInt();
    label->setFontColor( QColor( red, green, blue ) );
  }
  else
  {
    label->setFontColor( QColor( 0, 0, 0 ) );
  }

  return true;
}

bool QgsCompositionConverter::readShapeXml( QgsLayoutItemShape *layoutItem, const QDomElement &itemElem )
{
  layoutItem->setShapeType( static_cast<QgsLayoutItemShape::Shape>( itemElem.attribute( QStringLiteral( "shapeType" ), QStringLiteral( "0" ) ).toInt() ) );
  layoutItem->setCornerRadius( QgsLayoutMeasurement( itemElem.attribute( QStringLiteral( "cornerRadius" ), QStringLiteral( "0" ) ).toDouble() ) );

  restoreGeneralComposeItemProperties( layoutItem, itemElem );


  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );

  if ( itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).size() )
  {
    QDomElement symbolElement = itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).at( 0 ).toElement();
    QgsFillSymbol *shapeStyleSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context );
    if ( shapeStyleSymbol )
      layoutItem->setSymbol( shapeStyleSymbol );
  } /*
  QDomElement shapeStyleSymbolElem = itemElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    layoutItem->setSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( shapeStyleSymbolElem, context ) );
  } */
  else
  {
    //upgrade project file from 2.0 to use symbol styling
    QgsStringMap properties;
    properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( layoutItem->brush().color() ) );
    if ( layoutItem->hasBackground() )
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
    }
    if ( layoutItem->hasFrame() )
    {
      properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
    }
    properties.insert( QStringLiteral( "color_border" ), QgsSymbolLayerUtils::encodeColor( layoutItem->pen().color() ) );
    properties.insert( QStringLiteral( "width_border" ), QString::number( layoutItem->pen().widthF() ) );

    //for pre 2.0 projects, shape color and outline were specified in a different element...
    QDomNodeList outlineColorList = itemElem.elementsByTagName( QStringLiteral( "OutlineColor" ) );
    if ( !outlineColorList.isEmpty() )
    {
      QDomElement frameColorElem = outlineColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk, widthOk;
      int penRed, penGreen, penBlue, penAlpha;
      double penWidth;

      penWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
      penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk && widthOk )
      {
        properties.insert( QStringLiteral( "color_border" ), QgsSymbolLayerUtils::encodeColor( QColor( penRed, penGreen, penBlue, penAlpha ) ) );
        properties.insert( QStringLiteral( "width_border" ), QString::number( penWidth ) );
      }
    }
    QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "FillColor" ) );
    if ( !fillColorList.isEmpty() )
    {
      QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) ) );
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
      }
    }
    if ( itemElem.hasAttribute( QStringLiteral( "transparentFill" ) ) )
    {
      //old style (pre 2.0) of specifying that shapes had no fill
      bool hasOldTransparentFill = itemElem.attribute( QStringLiteral( "transparentFill" ), QStringLiteral( "0" ) ).toInt();
      if ( hasOldTransparentFill )
      {
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
      }
    }

    layoutItem->setSymbol( QgsFillSymbol::createSimple( properties ) );
  }
  // Diable frame for shapes
  layoutItem->setFrameEnabled( false );
  layoutItem->setBackgroundEnabled( false );

  return true;
}


bool QgsCompositionConverter::readXml( QgsLayoutItem *layoutItem, const QDomElement &itemElem )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  readOldComposerObjectXml( layoutItem, itemElem );

  //uuid
  layoutItem->mUuid = itemElem.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );

  // temporary for groups imported from templates
  layoutItem->mTemplateUuid = itemElem.attribute( QStringLiteral( "templateUuid" ) );

  //id
  QString id = itemElem.attribute( QStringLiteral( "id" ), QStringLiteral( "" ) );
  layoutItem->setId( id );

  //frame
  QString frame = itemElem.attribute( QStringLiteral( "frame" ) );
  layoutItem->setFrameEnabled( frame.compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 ) ;

  //frame
  QString background = itemElem.attribute( QStringLiteral( "background" ) );
  layoutItem->setBackgroundEnabled( background.compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 );

  //position lock for mouse moves/resizes
  QString positionLock = itemElem.attribute( QStringLiteral( "positionLock" ) );
  layoutItem->setLocked( positionLock.compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 );

  //visibility
  layoutItem->setVisibility( itemElem.attribute( QStringLiteral( "visibility" ), QStringLiteral( "1" ) ) != QStringLiteral( "0" ) );

  layoutItem->mParentGroupUuid = itemElem.attribute( QStringLiteral( "groupUuid" ) );
  if ( !layoutItem->mParentGroupUuid.isEmpty() )
  {
    if ( QgsLayoutItemGroup *group = layoutItem->parentGroup() )
    {
      group->addItem( layoutItem );
    }
  }
  layoutItem->mTemplateUuid = itemElem.attribute( "templateUuid" );

  int page;
  double x, y, pagex, pagey, width, height;
  bool xOk, yOk, pageOk, pagexOk, pageyOk, widthOk, heightOk, positionModeOK;

  x = itemElem.attribute( QStringLiteral( "x" ) ).toDouble( &xOk );
  y = itemElem.attribute( QStringLiteral( "y" ) ).toDouble( &yOk );
  page = itemElem.attribute( QStringLiteral( "page" ) ).toInt( &pageOk );
  pagex = itemElem.attribute( QStringLiteral( "pagex" ) ).toDouble( &pagexOk );
  pagey = itemElem.attribute( QStringLiteral( "pagey" ) ).toDouble( &pageyOk );
  width = itemElem.attribute( QStringLiteral( "width" ) ).toDouble( &widthOk );
  height = itemElem.attribute( QStringLiteral( "height" ) ).toDouble( &heightOk );


  layoutItem->mReferencePoint = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( QStringLiteral( "positionMode" ) ).toInt( &positionModeOK ) );
  if ( !positionModeOK )
  {
    layoutItem->setReferencePoint( QgsLayoutItem::ReferencePoint::UpperLeft );
  }

  if ( pageOk && pagexOk && pageyOk )
  {
    xOk = true;
    yOk = true;
    x = pagex;
    // position in the page (1-based)
    if ( page <= layoutItem->layout()->pageCollection()->pageCount() )
    {
      QgsLayoutItemPage *pageObject = layoutItem->layout()->pageCollection()->pages().at( page - 1 );
      y = ( page - 1 )
          * ( pageObject->sizeWithUnits().height()
              + layoutItem->layout()->pageCollection()->spaceBetweenPages() )
          + pagey;
    }
    else
    {
      y = pagey;
    }
  }

  if ( !xOk || !yOk || !widthOk || !heightOk )
  {
    return false;
  }

  // TODO: missing?
  // mLastValidViewScaleFactor = itemElem.attribute( QStringLiteral( "lastValidViewScaleFactor" ), QStringLiteral( "-1" ) ).toDouble();

  layoutItem->setZValue( itemElem.attribute( QStringLiteral( "zValue" ) ).toDouble() );

  // TODO: context in not used
  // QgsExpressionContext context = layoutItem->layout()->createExpressionContext();

  //pen
  QDomNodeList frameColorList = itemElem.elementsByTagName( QStringLiteral( "FrameColor" ) );
  if ( !frameColorList.isEmpty() )
  {
    QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk, widthOk;
    int penRed, penGreen, penBlue, penAlpha;
    double penWidth;

    penWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
    penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
    layoutItem->setFrameJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( QStringLiteral( "frameJoinStyle" ), QStringLiteral( "miter" ) ) ) );

    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      layoutItem->setFrameStrokeColor( QColor( penRed, penGreen, penBlue, penAlpha ) );
      layoutItem->setFrameStrokeWidth( QgsLayoutMeasurement( penWidth ) );
      QPen framePen( layoutItem->frameStrokeColor() );
      framePen.setWidthF( layoutItem->frameStrokeWidth( ).length() );
      framePen.setJoinStyle( layoutItem->frameJoinStyle( ) );
      layoutItem->setPen( framePen );
      //apply any data defined settings
      // TODO: check signature (no context!)
      layoutItem->refreshFrame( false );
    }
  }

  //brush
  QDomNodeList bgColorList = itemElem.elementsByTagName( QStringLiteral( "BackgroundColor" ) );
  if ( !bgColorList.isEmpty() )
  {
    QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
      layoutItem->setBrush( QBrush( layoutItem->mBackgroundColor, Qt::SolidPattern ) );
    }
    //apply any data defined settings
    layoutItem->refreshBackgroundColor( false );
  }

  //blend mode
  layoutItem->setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( itemElem.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );

  //opacity
  if ( itemElem.hasAttribute( QStringLiteral( "opacity" ) ) )
  {
    layoutItem->setItemOpacity( itemElem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1" ) ).toDouble() );
  }
  else
  {
    layoutItem->setItemOpacity( 1.0 - itemElem.attribute( QStringLiteral( "transparency" ), QStringLiteral( "0" ) ).toInt() / 100.0 );
  }

  layoutItem->mExcludeFromExports = itemElem.attribute( QStringLiteral( "excludeFromExports" ), QStringLiteral( "0" ) ).toInt();
  layoutItem->mEvaluatedExcludeFromExports = layoutItem->mExcludeFromExports;

  // positioning
  layoutItem->attemptSetSceneRect( QRectF( x, y, width, height ) );
  //rotation
  layoutItem->setItemRotation( itemElem.attribute( QStringLiteral( "itemRotation" ), QStringLiteral( "0" ) ).toDouble(), false );

  layoutItem->mBlockUndoCommands = false;

  // TODO: update and emit changed (in the calling scope?)
  return true;
}



bool QgsCompositionConverter::readOldComposerObjectXml( QgsLayoutItem *layoutItem,
    const QDomElement &itemElem )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //old (pre 3.0) data defined properties
  QgsCompositionConverter::readOldDataDefinedPropertyMap( itemElem, layoutItem->mDataDefinedProperties );

  QDomNode propsNode = itemElem.namedItem( QStringLiteral( "dataDefinedProperties" ) );
  if ( !propsNode.isNull() )
  {
    layoutItem->mDataDefinedProperties.readXml( propsNode.toElement(), sPropertyDefinitions );
  }
  if ( layoutItem->mDataDefinedProperties.isActive( QgsCompositionConverter::Transparency ) )
  {
    // upgrade transparency -> opacity
    QString exp = layoutItem->mDataDefinedProperties.property( QgsCompositionConverter::Transparency ).asExpression();
    exp = QStringLiteral( "100.0 - (%1)" ).arg( exp );
    layoutItem->mDataDefinedProperties.setProperty( QgsCompositionConverter::Opacity, QgsProperty::fromExpression( exp ) );
    layoutItem->mDataDefinedProperties.setProperty( QgsCompositionConverter::Transparency, QgsProperty() );
  }

  //custom properties
  layoutItem->mCustomProperties.readXml( itemElem );

  return true;
}


void QgsCompositionConverter::readOldDataDefinedPropertyMap( const QDomElement &itemElem, QgsPropertyCollection &dataDefinedProperties )
{
  QgsPropertiesDefinition::const_iterator i = QgsCompositionConverter::propertyDefinitions().constBegin();
  for ( ; i != QgsCompositionConverter::propertyDefinitions().constEnd(); ++i )
  {
    QString elemName = i.value().name();
    QDomNodeList ddNodeList = itemElem.elementsByTagName( elemName );
    if ( !ddNodeList.isEmpty() )
    {
      QDomElement ddElem = ddNodeList.at( 0 ).toElement();
      QgsProperty prop = readOldDataDefinedProperty( static_cast< QgsCompositionConverter::DataDefinedProperty >( i.key() ), ddElem );
      if ( prop )
        dataDefinedProperties.setProperty( i.key(), prop );
    }
  }
}

QgsProperty QgsCompositionConverter::readOldDataDefinedProperty( const QgsCompositionConverter::DataDefinedProperty property, const QDomElement &ddElem )
{
  if ( property == QgsCompositionConverter::AllProperties || property == QgsCompositionConverter::NoProperty )
  {
    //invalid property
    return QgsProperty();
  }

  QString active = ddElem.attribute( QStringLiteral( "active" ) );
  bool isActive = false;
  if ( active.compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isActive = true;
  }
  QString field = ddElem.attribute( QStringLiteral( "field" ) );
  QString expr = ddElem.attribute( QStringLiteral( "expr" ) );

  QString useExpr = ddElem.attribute( QStringLiteral( "useExpr" ) );
  bool isExpression = false;
  if ( useExpr.compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isExpression = true;
  }

  if ( isExpression )
    return QgsProperty::fromExpression( expr, isActive );
  else
    return QgsProperty::fromField( field, isActive );
}
