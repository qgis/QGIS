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

#include "qgscompositionconverter.h"

#include "qgsfillsymbol.h"
#include "qgsfontutils.h"
#include "qgslayertree.h"
#include "qgslayoutatlas.h"
#include "qgslayoutframe.h"
#include "qgslayoutguidecollection.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapgrid.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutmodel.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutnortharrowhandler.h"
#include "qgslayoutobject.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutrendercontext.h"
#include "qgslayouttable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutundostack.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmaplayerstyle.h"
#include "qgspainting.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgsproperty.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"
#include "qgsvectorlayer.h"

#include <QObject>
#include <QUuid>

QgsPropertiesDefinition QgsCompositionConverter::sPropertyDefinitions;

void QgsCompositionConverter::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::TestProperty ), QgsPropertyDefinition( "dataDefinedProperty", QgsPropertyDefinition::DataTypeString, "invalid property", QString() ) },
    {
      static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PresetPaperSize ), QgsPropertyDefinition( "dataDefinedPaperSize", QgsPropertyDefinition::DataTypeString, QObject::tr( "Paper size" ), QObject::tr( "string " ) + QStringLiteral( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
          "|<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
          "|<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
          "|<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                                                                                                                                                                                                                                       ) )
    },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PaperWidth ), QgsPropertyDefinition( "dataDefinedPaperWidth", QObject::tr( "Page width" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PaperHeight ), QgsPropertyDefinition( "dataDefinedPaperHeight", QObject::tr( "Page height" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::NumPages ), QgsPropertyDefinition( "dataDefinedNumPages", QObject::tr( "Number of pages" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PaperOrientation ), QgsPropertyDefinition( "dataDefinedPaperOrientation", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "string " ) + u"[<b>portrait</b>|<b>landscape</b>]"_s ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PageNumber ), QgsPropertyDefinition( "dataDefinedPageNumber", QObject::tr( "Page number" ), QgsPropertyDefinition::IntegerPositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PositionX ), QgsPropertyDefinition( "dataDefinedPositionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PositionY ), QgsPropertyDefinition( "dataDefinedPositionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ItemWidth ), QgsPropertyDefinition( "dataDefinedWidth", QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ItemHeight ), QgsPropertyDefinition( "dataDefinedHeight", QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ItemRotation ), QgsPropertyDefinition( "dataDefinedRotation", QObject::tr( "Rotation angle" ), QgsPropertyDefinition::Rotation ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::Transparency ), QgsPropertyDefinition( "dataDefinedTransparency", QObject::tr( "Transparency" ), QgsPropertyDefinition::Opacity ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::Opacity ), QgsPropertyDefinition( "dataDefinedOpacity", QObject::tr( "Opacity" ), QgsPropertyDefinition::Opacity ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::BlendMode ), QgsPropertyDefinition( "dataDefinedBlendMode", QObject::tr( "Blend mode" ), QgsPropertyDefinition::BlendMode ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ExcludeFromExports ), QgsPropertyDefinition( "dataDefinedExcludeExports", QObject::tr( "Exclude item from exports" ), QgsPropertyDefinition::Boolean ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::FrameColor ), QgsPropertyDefinition( "dataDefinedFrameColor", QObject::tr( "Frame color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::BackgroundColor ), QgsPropertyDefinition( "dataDefinedBackgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapRotation ), QgsPropertyDefinition( "dataDefinedMapRotation", QObject::tr( "Map rotation" ), QgsPropertyDefinition::Rotation ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapScale ), QgsPropertyDefinition( "dataDefinedMapScale", QObject::tr( "Map scale" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapXMin ), QgsPropertyDefinition( "dataDefinedMapXMin", QObject::tr( "Extent minimum X" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapYMin ), QgsPropertyDefinition( "dataDefinedMapYMin", QObject::tr( "Extent minimum Y" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapXMax ), QgsPropertyDefinition( "dataDefinedMapXMax", QObject::tr( "Extent maximum X" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapYMax ), QgsPropertyDefinition( "dataDefinedMapYMax", QObject::tr( "Extent maximum Y" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapAtlasMargin ), QgsPropertyDefinition( "dataDefinedMapAtlasMargin", QObject::tr( "Atlas margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapLayers ), QgsPropertyDefinition( "dataDefinedMapLayers", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "list of map layer names separated by | characters" ) ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::MapStylePreset ), QgsPropertyDefinition( "dataDefinedMapStylePreset", QgsPropertyDefinition::DataTypeString, QObject::tr( "Symbol size" ), QObject::tr( "list of map layer names separated by | characters" ) ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PictureSource ), QgsPropertyDefinition( "dataDefinedSource", QObject::tr( "Picture source (URL)" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::SourceUrl ), QgsPropertyDefinition( "dataDefinedSourceUrl", QObject::tr( "Source URL" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PictureSvgBackgroundColor ), QgsPropertyDefinition( "dataDefinedSvgBackgroundColor", QObject::tr( "SVG background color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PictureSvgStrokeColor ), QgsPropertyDefinition( "dataDefinedSvgStrokeColor", QObject::tr( "SVG stroke color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::PictureSvgStrokeWidth ), QgsPropertyDefinition( "dataDefinedSvgStrokeWidth", QObject::tr( "SVG stroke width" ), QgsPropertyDefinition::StrokeWidth ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::LegendTitle ), QgsPropertyDefinition( "dataDefinedLegendTitle", QObject::tr( "Legend title" ), QgsPropertyDefinition::String ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::LegendColumnCount ), QgsPropertyDefinition( "dataDefinedLegendColumns", QObject::tr( "Number of columns" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ScalebarFillColor ), QgsPropertyDefinition( "dataDefinedScalebarFill", QObject::tr( "Fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ScalebarFillColor2 ), QgsPropertyDefinition( "dataDefinedScalebarFill2", QObject::tr( "Secondary fill color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ScalebarLineColor ), QgsPropertyDefinition( "dataDefinedScalebarLineColor", QObject::tr( "Line color" ), QgsPropertyDefinition::ColorWithAlpha ) },
    { static_cast< int >( QgsCompositionConverter::DataDefinedProperty::ScalebarLineWidth ), QgsPropertyDefinition( "dataDefinedScalebarLineWidth", QObject::tr( "Line width" ), QgsPropertyDefinition::StrokeWidth ) },
  };
}

QgsPropertiesDefinition QgsCompositionConverter::propertyDefinitions()
{
  QgsCompositionConverter::initPropertyDefinitions();
  return sPropertyDefinitions;
}


std::unique_ptr< QgsPrintLayout > QgsCompositionConverter::createLayoutFromCompositionXml( const QDomElement &composerElement, QgsProject *project )
{
  initPropertyDefinitions();

  const QDomElement parentElement = composerElement.parentNode().toElement();

  auto layout = std::make_unique< QgsPrintLayout >( project );
  layout->undoStack()->blockCommands( true );

  layout->mCustomProperties.readXml( composerElement );

  // Guides
  layout->guides().setVisible( composerElement.attribute( u"guidesVisible"_s, u"1"_s ).toInt() != 0 );

  const int printResolution = composerElement.attribute( u"printResolution"_s, u"300"_s ).toInt();
  layout->renderContext().setDpi( printResolution );

  // Create pages
  const int pages = composerElement.attribute( u"numPages"_s ).toInt( );
  const float paperHeight = composerElement.attribute( u"paperHeight"_s ).toDouble( );
  const float paperWidth = composerElement.attribute( u"paperWidth"_s ).toDouble( );

  QString name = composerElement.attribute( u"name"_s );
  // Try title
  if ( name.isEmpty() )
    name = composerElement.attribute( u"title"_s );
  // Try title on parent element
  if ( name.isEmpty() )
    name = parentElement.attribute( u"title"_s );
  layout->setName( name );
  const QgsLayoutSize pageSize( paperWidth, paperHeight );
  for ( int j = 0; j < pages; j++ )
  {
    QgsLayoutItemPage *page = QgsLayoutItemPage::create( layout.get() );
    page->setPageSize( pageSize );
    layout->pageCollection()->addPage( page );
    //custom snap lines
    const QDomNodeList snapLineNodes = composerElement.elementsByTagName( u"SnapLine"_s );
    for ( int i = 0; i < snapLineNodes.size(); ++i )
    {
      const QDomElement snapLineElem = snapLineNodes.at( i ).toElement();
      const double x1 = snapLineElem.attribute( u"x1"_s ).toDouble();
      const double y1 = snapLineElem.attribute( u"y1"_s ).toDouble();
      const double x2 = snapLineElem.attribute( u"x2"_s ).toDouble();
      // Not necessary: double y2 = snapLineElem.attribute( u"y2"_s ).toDouble();
      const Qt::Orientation orientation( x1 == x2 ? Qt::Orientation::Vertical : Qt::Orientation::Horizontal );
      const QgsLayoutMeasurement position( x1 == x2 ? x1 : y1 );
      auto guide = std::make_unique< QgsLayoutGuide >( orientation, position, page );
      layout->guides().addGuide( guide.release() );
    }
  }


  if ( composerElement.elementsByTagName( u"symbol"_s ).size() )
  {
    const QDomElement symbolElement = composerElement.elementsByTagName( u"symbol"_s ).at( 0 ).toElement();
    QgsReadWriteContext context;
    if ( project )
      context.setPathResolver( project->pathResolver() );
    const std::unique_ptr< QgsFillSymbol > symbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context ) );
    if ( symbol )
      layout->pageCollection()->setPageStyleSymbol( symbol.get() );
  }

  addItemsFromCompositionXml( layout.get(), composerElement );

  // Read atlas from the parent element (Composer)
  if ( parentElement.elementsByTagName( u"Atlas"_s ).size() )
  {
    const QDomElement atlasElement = parentElement.elementsByTagName( u"Atlas"_s ).at( 0 ).toElement();
    readAtlasXml( layout->atlas(), atlasElement, layout->project() );
  }

  layout->undoStack()->blockCommands( false );
  return layout;
}


void QgsCompositionConverter::adjustPos( QgsPrintLayout *layout, QgsLayoutItem *layoutItem, QPointF *position, bool &pasteInPlace, int zOrderOffset, QPointF &pasteShiftPos, int &pageNumber )
{
  if ( position )
  {
    if ( pasteInPlace )
    {
      layoutItem->attemptMove( QgsLayoutPoint( *position ), true, false, pageNumber );
    }
    else
    {
      layoutItem->attemptMoveBy( pasteShiftPos.x(), pasteShiftPos.y() );
    }
  }

  if ( !layoutItem->scene() )
    layout->addLayoutItem( layoutItem );
  layoutItem->setZValue( layoutItem->zValue() + zOrderOffset );
}

void QgsCompositionConverter::restoreGeneralComposeItemProperties( QgsLayoutItem *layoutItem, const QDomElement &itemElem )
{
  //restore general composer item properties
  const QDomNodeList composerItemList = itemElem.elementsByTagName( u"ComposerItem"_s );
  if ( !composerItemList.isEmpty() )
  {
    const QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    //rotation
    if ( !qgsDoubleNear( composerItemElem.attribute( u"rotation"_s, u"0"_s ).toDouble(), 0.0 ) )
    {
      //check for old (pre 2.1) rotation attribute
      layoutItem->setItemRotation( composerItemElem.attribute( u"rotation"_s, u"0"_s ).toDouble(), false );
    }
    QgsCompositionConverter::readXml( layoutItem, composerItemElem );
  }
}

QRectF QgsCompositionConverter::itemPosition( QgsLayoutItem *layoutItem, const QDomElement &itemElem )
{
  int page;
  double x, y, pagex, pagey, width, height;
  bool xOk, yOk, pageOk, pagexOk, pageyOk, widthOk, heightOk, positionModeOk;

  x = itemElem.attribute( u"x"_s ).toDouble( &xOk );
  y = itemElem.attribute( u"y"_s ).toDouble( &yOk );
  page = itemElem.attribute( u"page"_s ).toInt( &pageOk );
  pagex = itemElem.attribute( u"pagex"_s ).toDouble( &pagexOk );
  pagey = itemElem.attribute( u"pagey"_s ).toDouble( &pageyOk );
  width = itemElem.attribute( u"width"_s ).toDouble( &widthOk );
  height = itemElem.attribute( u"height"_s ).toDouble( &heightOk );


  layoutItem->mReferencePoint = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( u"positionMode"_s ).toInt( &positionModeOk ) );
  if ( !positionModeOk )
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
  return QRectF( x, y, width, height );
}

QPointF QgsCompositionConverter::minPointFromXml( const QDomElement &elem )
{
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  const QDomNodeList composerItemList = elem.elementsByTagName( u"ComposerItem"_s );
  for ( int i = 0; i < composerItemList.size(); ++i )
  {
    const QDomElement currentComposerItemElem = composerItemList.at( i ).toElement();
    double x, y;
    bool xOk, yOk;
    x = currentComposerItemElem.attribute( u"x"_s ).toDouble( &xOk );
    y = currentComposerItemElem.attribute( u"y"_s ).toDouble( &yOk );
    if ( !xOk || !yOk )
    {
      continue;
    }
    minX = std::min( minX, x );
    minY = std::min( minY, y );
  }
  if ( minX < std::numeric_limits<double>::max() )
  {
    return QPointF( minX, minY );
  }
  else
  {
    return QPointF( 0, 0 );
  }
}

QList<QgsLayoutObject *> QgsCompositionConverter::addItemsFromCompositionXml( QgsPrintLayout *layout, const QDomElement &parentElement, QPointF *position, bool pasteInPlace )
{

  initPropertyDefinitions();

  QList< QgsLayoutObject * > newItems;

  //if we are adding items to a layout which already contains items, we need to make sure
  //these items are placed at the top of the layout and that zValues are not duplicated
  //so, calculate an offset which needs to be added to the zValue of created items
  const int zOrderOffset = layout->mItemsModel->zOrderListSize();

  QPointF pasteShiftPos;
  int pageNumber = -1;
  if ( position )
  {
    //If we are placing items relative to a certain point, then calculate how much we need
    //to shift the items by so that they are placed at this point
    //First, calculate the minimum position from the xml
    const QPointF minItemPos = minPointFromXml( parentElement );
    //next, calculate how much each item needs to be shifted from its original position
    //so that it's placed at the correct relative position
    pasteShiftPos = *position - minItemPos;
    if ( pasteInPlace )
    {
      pageNumber = layout->mPageCollection->pageNumberForPoint( *position );
    }
  }

  QgsStringMap mapIdUiidMap;

  // Map (this needs to come first to build the uuid <-> ID map for map composer items
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerMap"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerMap"_s ).at( i ) );
    QgsLayoutItemMap *layoutItem = new QgsLayoutItemMap( layout );
    readMapXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Label
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerLabel"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerLabel"_s ).at( i ) );
    QgsLayoutItemLabel *layoutItem = new QgsLayoutItemLabel( layout );
    readLabelXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Shape
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerShape"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerShape"_s ).at( i ) );
    QgsLayoutItemShape *layoutItem = new QgsLayoutItemShape( layout );
    readShapeXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Picture
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerPicture"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerPicture"_s ).at( i ) );
    QgsLayoutItemPicture *layoutItem = new QgsLayoutItemPicture( layout );
    readPictureXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Polygon
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerPolygon"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerPolygon"_s ).at( i ) );
    QgsLayoutItemPolygon *layoutItem = new QgsLayoutItemPolygon( layout );
    readPolyXml<QgsLayoutItemPolygon, QgsFillSymbol>( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Polyline
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerPolyline"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerPolyline"_s ).at( i ) );
    QgsLayoutItemPolyline *layoutItem = new QgsLayoutItemPolyline( layout );
    readPolyXml<QgsLayoutItemPolyline, QgsLineSymbol>( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Arrow
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerArrow"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerArrow"_s ).at( i ) );
    QgsLayoutItemPolyline *layoutItem = new QgsLayoutItemPolyline( layout );
    readArrowXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Scalebar
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerScaleBar"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerScaleBar"_s ).at( i ) );
    QgsLayoutItemScaleBar *layoutItem = new QgsLayoutItemScaleBar( layout );
    readScaleBarXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Legend
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerLegend"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerLegend"_s ).at( i ) );
    QgsLayoutItemLegend *layoutItem = new QgsLayoutItemLegend( layout );
    readLegendXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Html
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerHtml"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerHtml"_s ).at( i ) );
    QgsLayoutItemHtml *layoutItem = new QgsLayoutItemHtml( layout );
    readHtmlXml( layoutItem, itemNode.toElement(), layout->project() );
    // Adjust position for frames
    const QList<QgsLayoutFrame *> framesList( layoutItem->frames() );
    for ( const auto &frame : framesList )
    {
      adjustPos( layout, frame, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    }
    newItems << layoutItem ;
  }

  // Attribute Table
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerAttributeTableV2"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerAttributeTableV2"_s ).at( i ) );
    QgsLayoutItemAttributeTable *layoutItem = new QgsLayoutItemAttributeTable( layout );
    readTableXml( layoutItem, itemNode.toElement(), layout->project() );
    // Adjust position for frames
    const QList<QgsLayoutFrame *> framesList( layoutItem->frames() );
    for ( const auto &frame : framesList )
    {
      adjustPos( layout, frame, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    }
    newItems << layoutItem ;
  }

  // Group
  for ( int i = 0; i < parentElement.elementsByTagName( u"ComposerItemGroup"_s ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( u"ComposerItemGroup"_s ).at( i ) );
    QgsLayoutItemGroup *layoutItem = new QgsLayoutItemGroup( layout );
    readGroupXml( layoutItem, itemNode.toElement(), layout->project(), newItems );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  return newItems;
}

bool QgsCompositionConverter::isCompositionTemplate( const QDomDocument &document )
{
  return document.elementsByTagName( u"Composition"_s ).count() > 0;
}

QDomDocument QgsCompositionConverter::convertCompositionTemplate( const QDomDocument &document, QgsProject *project )
{
  QDomDocument doc;
  QgsReadWriteContext context;
  if ( project )
    context.setPathResolver( project->pathResolver() );
  if ( document.elementsByTagName( u"Composition"_s ).count( ) > 0 )
  {
    const QDomElement composerElem = document.elementsByTagName( u"Composition"_s ).at( 0 ).toElement( );

    std::unique_ptr<QgsLayout> layout = createLayoutFromCompositionXml( composerElem,
                                        project );
    const QDomElement elem = layout->writeXml( doc, context );
    doc.appendChild( elem );
  }
  return doc;
}

bool QgsCompositionConverter::readLabelXml( QgsLayoutItemLabel *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  Q_UNUSED( project )
  if ( itemElem.isNull() )
  {
    return false;
  }

  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  //text
  layoutItem->setText( itemElem.attribute( u"labelText"_s ) );

  //html state
  layoutItem->setMode( itemElem.attribute( u"htmlState"_s ).toInt() == Qt::Checked ? QgsLayoutItemLabel::Mode::ModeHtml : QgsLayoutItemLabel::Mode::ModeFont );

  //margin
  bool marginXOk = false;
  bool marginYOk = false;
  double marginX = itemElem.attribute( u"marginX"_s ).toDouble( &marginXOk );
  double marginY = itemElem.attribute( u"marginY"_s ).toDouble( &marginYOk );
  if ( !marginXOk || !marginYOk )
  {
    //upgrade old projects where margins where stored in a single attribute
    const double margin = itemElem.attribute( u"margin"_s, u"1.0"_s ).toDouble();
    marginX = margin;
    marginY = margin;
  }
  layoutItem->setMarginX( marginX );
  layoutItem->setMarginY( marginY );

  //Horizontal alignment
  layoutItem->setHAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( u"halign"_s ).toInt() ) );

  //Vertical alignment
  layoutItem->setVAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( u"valign"_s ).toInt() ) );


  QFont font;
  //font
  QgsFontUtils::setFromXmlChildNode( font, itemElem, u"LabelFont"_s );
  QgsTextFormat format;
  format.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    format.setSize( font.pointSizeF() );
    format.setSizeUnit( Qgis::RenderUnit::Points );
  }
  else if ( font.pixelSize() > 0 )
  {
    format.setSize( font.pixelSize() );
    format.setSizeUnit( Qgis::RenderUnit::Pixels );
  }

  //font color
  const QDomNodeList fontColorList = itemElem.elementsByTagName( u"FontColor"_s );
  if ( !fontColorList.isEmpty() )
  {
    const QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
    const int red = fontColorElem.attribute( u"red"_s, u"0"_s ).toInt();
    const int green = fontColorElem.attribute( u"green"_s, u"0"_s ).toInt();
    const int blue = fontColorElem.attribute( u"blue"_s, u"0"_s ).toInt();
    format.setColor( QColor( red, green, blue ) );
  }
  else
  {
    format.setColor( QColor( 0, 0, 0 ) );
  }
  layoutItem->setTextFormat( format );

  return true;
}

bool QgsCompositionConverter::readShapeXml( QgsLayoutItemShape *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  Q_UNUSED( project )
  layoutItem->setShapeType( static_cast<QgsLayoutItemShape::Shape>( itemElem.attribute( u"shapeType"_s, u"0"_s ).toInt() ) );
  layoutItem->setCornerRadius( QgsLayoutMeasurement( itemElem.attribute( u"cornerRadius"_s, u"0"_s ).toDouble() ) );

  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  QgsReadWriteContext context;
  if ( project )
    context.setPathResolver( project->pathResolver() );

  if ( itemElem.elementsByTagName( u"symbol"_s ).size() )
  {
    const QDomElement symbolElement = itemElem.elementsByTagName( u"symbol"_s ).at( 0 ).toElement();
    const std::unique_ptr< QgsFillSymbol > shapeStyleSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context ) );
    if ( shapeStyleSymbol )
      layoutItem->setSymbol( shapeStyleSymbol.get() );
  }
  else
  {
    //upgrade project file from 2.0 to use symbol styling
    QVariantMap properties;
    properties.insert( u"color"_s, QgsSymbolLayerUtils::encodeColor( layoutItem->brush().color() ) );
    if ( layoutItem->hasBackground() )
    {
      properties.insert( u"style"_s, u"solid"_s );
    }
    else
    {
      properties.insert( u"style"_s, u"no"_s );
    }
    if ( layoutItem->frameEnabled() )
    {
      properties.insert( u"style_border"_s, u"solid"_s );
    }
    else
    {
      properties.insert( u"style_border"_s, u"no"_s );
    }
    properties.insert( u"color_border"_s, QgsSymbolLayerUtils::encodeColor( layoutItem->pen().color() ) );
    properties.insert( u"width_border"_s, QString::number( layoutItem->pen().widthF() ) );

    //for pre 2.0 projects, shape color and outline were specified in a different element...
    const QDomNodeList outlineColorList = itemElem.elementsByTagName( u"OutlineColor"_s );
    if ( !outlineColorList.isEmpty() )
    {
      const QDomElement frameColorElem = outlineColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk, widthOk;
      int penRed, penGreen, penBlue, penAlpha;
      double penWidth;

      penWidth = itemElem.attribute( u"outlineWidth"_s ).toDouble( &widthOk );
      penRed = frameColorElem.attribute( u"red"_s ).toInt( &redOk );
      penGreen = frameColorElem.attribute( u"green"_s ).toInt( &greenOk );
      penBlue = frameColorElem.attribute( u"blue"_s ).toInt( &blueOk );
      penAlpha = frameColorElem.attribute( u"alpha"_s ).toInt( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk && widthOk )
      {
        properties.insert( u"color_border"_s, QgsSymbolLayerUtils::encodeColor( QColor( penRed, penGreen, penBlue, penAlpha ) ) );
        properties.insert( u"width_border"_s, QString::number( penWidth ) );
      }
    }
    const QDomNodeList fillColorList = itemElem.elementsByTagName( u"FillColor"_s );
    if ( !fillColorList.isEmpty() )
    {
      const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( u"red"_s ).toInt( &redOk );
      fillGreen = fillColorElem.attribute( u"green"_s ).toInt( &greenOk );
      fillBlue = fillColorElem.attribute( u"blue"_s ).toInt( &blueOk );
      fillAlpha = fillColorElem.attribute( u"alpha"_s ).toInt( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        properties.insert( u"color"_s, QgsSymbolLayerUtils::encodeColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) ) );
        properties.insert( u"style"_s, u"solid"_s );
      }
    }
    if ( itemElem.hasAttribute( u"transparentFill"_s ) )
    {
      //old style (pre 2.0) of specifying that shapes had no fill
      const bool hasOldTransparentFill = itemElem.attribute( u"transparentFill"_s, u"0"_s ).toInt();
      if ( hasOldTransparentFill )
      {
        properties.insert( u"style"_s, u"no"_s );
      }
    }

    const std::unique_ptr< QgsFillSymbol > shapeStyleSymbol( QgsFillSymbol::createSimple( properties ) );
    layoutItem->setSymbol( shapeStyleSymbol.get() );
  }
  // Disable frame for shapes
  layoutItem->setFrameEnabled( false );
  layoutItem->setBackgroundEnabled( false );

  return true;
}

bool QgsCompositionConverter::readPictureXml( QgsLayoutItemPicture *layoutItem, const QDomElement &itemElem, const QgsProject *project, const QgsStringMap &mapId2Uuid )
{
  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  layoutItem->mResizeMode = QgsLayoutItemPicture::ResizeMode( itemElem.attribute( u"resizeMode"_s, u"0"_s ).toInt() );
  //when loading from xml, default to anchor point of middle to match pre 2.4 behavior
  bool positionModeOk = false;
  layoutItem->mReferencePoint = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( u"positionMode"_s ).toInt( &positionModeOk ) );
  if ( !positionModeOk )
  {
    layoutItem->mReferencePoint = QgsLayoutItem::ReferencePoint::UpperLeft;
  }
  bool anchorPointOk = false;

  layoutItem->setPictureAnchor( static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( u"anchorPoint"_s, QString::number( QgsLayoutItem::ReferencePoint::Middle ) ).toInt( &anchorPointOk ) ) );
  if ( !anchorPointOk )
  {
    layoutItem->mPictureAnchor = QgsLayoutItem::ReferencePoint::UpperLeft;
  }
  layoutItem->mSvgFillColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"svgFillColor"_s, QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) );
  layoutItem->mSvgStrokeColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"svgBorderColor"_s, QgsSymbolLayerUtils::encodeColor( QColor( 0, 0, 0 ) ) ) );
  layoutItem->mSvgStrokeWidth = itemElem.attribute( u"svgBorderWidth"_s, u"0.2"_s ).toDouble();

  QString imagePath = itemElem.attribute( u"file"_s );
  if ( project )
  {
    // convert from relative path to absolute. For SVG we also need to consider system SVG paths
    const QgsPathResolver pathResolver = project->pathResolver();
    if ( imagePath.endsWith( ".svg"_L1, Qt::CaseInsensitive ) )
      imagePath = QgsSymbolLayerUtils::svgSymbolNameToPath( imagePath, pathResolver );
    else
      imagePath = pathResolver.readPath( imagePath );
  }
  layoutItem->setPicturePath( imagePath );
  layoutItem->mPictureHeight = itemElem.attribute( u"pictureHeight"_s, u"10"_s ).toDouble();
  layoutItem->mPictureWidth = itemElem.attribute( u"pictureWidth"_s, u"10"_s ).toDouble();

  //picture rotation
  if ( !qgsDoubleNear( itemElem.attribute( u"pictureRotation"_s, u"0"_s ).toDouble(), 0.0 ) )
  {
    layoutItem->mPictureRotation = itemElem.attribute( u"pictureRotation"_s, u"0"_s ).toDouble();
  }

  //rotation map
  layoutItem->mNorthArrowHandler->setNorthMode( static_cast< QgsLayoutNorthArrowHandler::NorthMode >( itemElem.attribute( u"northMode"_s, u"0"_s ).toInt() ) );
  layoutItem->mNorthArrowHandler->setNorthOffset( itemElem.attribute( u"northOffset"_s, u"0"_s ).toDouble() );

  const QString rotationMapId = itemElem.attribute( u"mapId"_s, u"-1"_s );
  if ( rotationMapId != "-1"_L1 )
  {
    // Find uuid for map with given id
    QgsLayoutItemMap *mapInstance = qobject_cast<QgsLayoutItemMap *>( layoutItem->layout()->itemByUuid( mapId2Uuid[ rotationMapId ] ) );
    if ( mapInstance )
    {
      layoutItem->setLinkedMap( mapInstance );
    }
  }
  return true;
}

bool QgsCompositionConverter::readArrowXml( QgsLayoutItemPolyline *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  readPolyXml<QgsLayoutItemPolyline, QgsLineSymbol>( layoutItem, itemElem, project );
  QPolygonF polygon;
  const QDomNodeList startPointList = itemElem.elementsByTagName( u"StartPoint"_s );
  if ( ! startPointList.isEmpty() )
  {
    const QDomElement node = startPointList.at( 0 ).toElement();
    polygon.append( QPointF( node.attribute( u"x"_s ).toDouble( ), node.attribute( u"y"_s ).toDouble() ) );
  }
  const QDomNodeList stopPointList = itemElem.elementsByTagName( u"StopPoint"_s );
  if ( ! stopPointList.isEmpty() )
  {
    const QDomElement node = stopPointList.at( 0 ).toElement();
    polygon.append( QPointF( node.attribute( u"x"_s ).toDouble( ), node.attribute( u"y"_s ).toDouble() ) );
  }

  const QgsCompositionConverter::MarkerMode markerMode = static_cast< QgsCompositionConverter::MarkerMode>( itemElem.attribute( u"markerMode"_s, u"0"_s ).toInt( ) );

  if ( markerMode == QgsCompositionConverter::MarkerMode::DefaultMarker )
  {
    layoutItem->setEndMarker( QgsLayoutItemPolyline::MarkerMode::ArrowHead );
    layoutItem->setStartMarker( QgsLayoutItemPolyline::MarkerMode::NoMarker );
    layoutItem->setArrowHeadFillColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"arrowHeadFillColor"_s, QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) ) );
    layoutItem->setArrowHeadStrokeColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"arrowHeadOutlineColor"_s, QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) ) );
    layoutItem->setArrowHeadStrokeWidth( itemElem.attribute( u"outlineWidth"_s, u"1.0"_s ).toDouble( ) );
    layoutItem->setArrowHeadWidth( itemElem.attribute( u"arrowHeadWidth"_s, u"1.0"_s ).toDouble( ) );
  }
  else if ( markerMode == QgsCompositionConverter::MarkerMode::SVGMarker )
  {
    QString endMarkerFile = itemElem.attribute( u"endMarkerFile"_s );
    QString startMarkerFile = itemElem.attribute( u"endMarkerFile"_s );

    // Fix the paths
    if ( project )
    {
      // convert from relative path to absolute. For SVG we also need to consider system SVG paths
      const QgsPathResolver pathResolver = project->pathResolver();
      if ( !endMarkerFile.isEmpty() )
      {
        if ( endMarkerFile.endsWith( ".svg"_L1, Qt::CaseInsensitive ) )
          endMarkerFile = QgsSymbolLayerUtils::svgSymbolNameToPath( endMarkerFile, pathResolver );
        else
          endMarkerFile = pathResolver.readPath( endMarkerFile );
      }
      if ( !startMarkerFile.isEmpty() )
      {
        if ( startMarkerFile.endsWith( ".svg"_L1, Qt::CaseInsensitive ) )
          startMarkerFile = QgsSymbolLayerUtils::svgSymbolNameToPath( startMarkerFile, pathResolver );
        else
          startMarkerFile = pathResolver.readPath( startMarkerFile );
      }
    }
    if ( !endMarkerFile.isEmpty() )
    {
      layoutItem->setEndMarker( QgsLayoutItemPolyline::MarkerMode::SvgMarker );
      layoutItem->setEndSvgMarkerPath( endMarkerFile );
    }
    if ( !startMarkerFile.isEmpty() )
    {
      layoutItem->setStartMarker( QgsLayoutItemPolyline::MarkerMode::SvgMarker );
      layoutItem->setStartSvgMarkerPath( startMarkerFile );
    }
  }
  else // NoMarker
  {
    layoutItem->setEndMarker( QgsLayoutItemPolyline::MarkerMode::NoMarker );
    layoutItem->setStartMarker( QgsLayoutItemPolyline::MarkerMode::NoMarker );
  }
  // Calculate the margin
  const double margin = polygon.boundingRect().left() - layoutItem->pos().x();
  polygon.translate( - polygon.boundingRect().left() + margin, - polygon.boundingRect().top() + margin );
  layoutItem->setNodes( polygon );

  return true;
}

bool QgsCompositionConverter::readMapXml( QgsLayoutItemMap *layoutItem, const QDomElement &itemElem, const QgsProject *project, QgsStringMap &mapId2Uuid )
{
  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  mapId2Uuid[ itemElem.attribute( u"id"_s ) ] = layoutItem->uuid();

  // TODO: Unused but all the layouts readXML require it (I'd suggest to remove it from the API)
  const QDomDocument doc;

  QgsReadWriteContext context;

  if ( project )
    context.setPathResolver( project->pathResolver() );

  //extent
  const QDomNodeList extentNodeList = itemElem.elementsByTagName( u"Extent"_s );
  if ( !extentNodeList.isEmpty() )
  {
    const QDomElement extentElem = extentNodeList.at( 0 ).toElement();
    double xmin, xmax, ymin, ymax;
    xmin = extentElem.attribute( u"xmin"_s ).toDouble();
    xmax = extentElem.attribute( u"xmax"_s ).toDouble();
    ymin = extentElem.attribute( u"ymin"_s ).toDouble();
    ymax = extentElem.attribute( u"ymax"_s ).toDouble();
    layoutItem->setExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );
  }

  const QDomNodeList crsNodeList = itemElem.elementsByTagName( u"crs"_s );
  if ( !crsNodeList.isEmpty() )
  {
    const QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    layoutItem->crs().readXml( crsElem );
  }
  else
  {
    layoutItem->setCrs( QgsCoordinateReferenceSystem() );
  }

  //map rotation
  if ( !qgsDoubleNear( itemElem.attribute( u"mapRotation"_s, u"0"_s ).toDouble(), 0.0 ) )
  {
    layoutItem->setMapRotation( itemElem.attribute( u"mapRotation"_s, u"0"_s ).toDouble() );
  }

  // follow map theme
  layoutItem->setFollowVisibilityPreset( itemElem.attribute( u"followPreset"_s ).compare( "true"_L1 ) == 0 );
  layoutItem->setFollowVisibilityPresetName( itemElem.attribute( u"followPresetName"_s ) );

  //mKeepLayerSet flag
  const QString keepLayerSetFlag = itemElem.attribute( u"keepLayerSet"_s );
  if ( keepLayerSetFlag.compare( "true"_L1, Qt::CaseInsensitive ) == 0 )
  {
    layoutItem->setKeepLayerSet( true );
  }
  else
  {
    layoutItem->setKeepLayerSet( false );
  }

  const QString drawCanvasItemsFlag = itemElem.attribute( u"drawCanvasItems"_s, u"true"_s );
  if ( drawCanvasItemsFlag.compare( "true"_L1, Qt::CaseInsensitive ) == 0 )
  {
    layoutItem->setDrawAnnotations( true );
  }
  else
  {
    layoutItem->setDrawAnnotations( false );
  }

  layoutItem->mLayerStyleOverrides.clear();

  //mLayers
  layoutItem->mLayers.clear();

  const QDomNodeList layerSetNodeList = itemElem.elementsByTagName( u"LayerSet"_s );
  if ( !layerSetNodeList.isEmpty() )
  {
    const QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    const QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( u"Layer"_s );
    layoutItem->mLayers.reserve( layerIdNodeList.size() );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      const QDomElement layerElem = layerIdNodeList.at( i ).toElement();
      const QString layerId = layerElem.text();
      const QString layerName = layerElem.attribute( u"name"_s );
      const QString layerSource = layerElem.attribute( u"source"_s );
      const QString layerProvider = layerElem.attribute( u"provider"_s );

      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( project );
      layoutItem->mLayers << ref;
    }
  }

  // override styles
  const QDomNodeList layerStylesNodeList = itemElem.elementsByTagName( u"LayerStyles"_s );
  layoutItem->mKeepLayerStyles = !layerStylesNodeList.isEmpty();
  if ( layoutItem->mKeepLayerStyles )
  {
    const QDomElement layerStylesElem = layerStylesNodeList.at( 0 ).toElement();
    const QDomNodeList layerStyleNodeList = layerStylesElem.elementsByTagName( u"LayerStyle"_s );
    for ( int i = 0; i < layerStyleNodeList.size(); ++i )
    {
      const QDomElement &layerStyleElement = layerStyleNodeList.at( i ).toElement();
      const QString layerId = layerStyleElement.attribute( u"layerid"_s );
      const QString layerName = layerStyleElement.attribute( u"name"_s );
      const QString layerSource = layerStyleElement.attribute( u"source"_s );
      const QString layerProvider = layerStyleElement.attribute( u"provider"_s );
      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( project );

      QgsMapLayerStyle style;
      style.readXml( layerStyleElement );
      layoutItem->mLayerStyleOverrides.insert( ref.layerId, style.xmlData() );
    }
  }

  layoutItem->mDrawing = false;
  layoutItem->mNumCachedLayers = 0;
  layoutItem->mCacheInvalidated = true;

  //overviews
  //read overview stack
  const QDomNodeList mapOverviewNodeList = itemElem.elementsByTagName( u"ComposerMapOverview"_s );
  for ( int i = 0; i < mapOverviewNodeList.size(); ++i )
  {
    const QDomElement mapOverviewElem = mapOverviewNodeList.at( i ).toElement();
    auto mapOverview = std::make_unique<QgsLayoutItemMapOverview>( mapOverviewElem.attribute( u"name"_s ), layoutItem );
    mapOverview->readXml( mapOverviewElem, doc, context );
    const QString frameMapId = mapOverviewElem.attribute( u"frameMap"_s, u"-1"_s );
    if ( frameMapId != "-1"_L1 && mapId2Uuid.contains( frameMapId ) )
    {
      QgsLayoutItemMap *mapInstance = qobject_cast<QgsLayoutItemMap *>( layoutItem->layout()->itemByUuid( mapId2Uuid[ frameMapId ] ) );
      if ( mapInstance )
      {
        mapOverview->setLinkedMap( mapInstance );
      }
      layoutItem->mOverviewStack->addOverview( mapOverview.release() );
    }
  }

  //grids
  layoutItem->mGridStack->readXml( itemElem, doc, context );

  //load grid / grid annotation in old xml format
  //only do this if the grid stack didn't load any grids, otherwise this will
  //be the dummy element created by QGIS >= 2.5 (refs #10905)
  const QDomNodeList gridNodeList = itemElem.elementsByTagName( u"Grid"_s );
  if ( layoutItem->mGridStack->size() == 0 && !gridNodeList.isEmpty() )
  {
    const QDomElement gridElem = gridNodeList.at( 0 ).toElement();
    QgsLayoutItemMapGrid *mapGrid = new QgsLayoutItemMapGrid( QObject::tr( "Grid %1" ).arg( 1 ), layoutItem );
    mapGrid->setEnabled( gridElem.attribute( u"show"_s, u"0"_s ) != "0"_L1 );
    mapGrid->setStyle( static_cast< Qgis::MapGridStyle >( gridElem.attribute( u"gridStyle"_s, u"0"_s ).toInt() ) );
    mapGrid->setIntervalX( gridElem.attribute( u"intervalX"_s, u"0"_s ).toDouble() );
    mapGrid->setIntervalY( gridElem.attribute( u"intervalY"_s, u"0"_s ).toDouble() );
    mapGrid->setOffsetX( gridElem.attribute( u"offsetX"_s, u"0"_s ).toDouble() );
    mapGrid->setOffsetY( gridElem.attribute( u"offsetY"_s, u"0"_s ).toDouble() );
    mapGrid->setCrossLength( gridElem.attribute( u"crossLength"_s, u"3"_s ).toDouble() );
    mapGrid->setFrameStyle( static_cast< Qgis::MapGridFrameStyle >( gridElem.attribute( u"gridFrameStyle"_s, u"0"_s ).toInt() ) );
    mapGrid->setFrameWidth( gridElem.attribute( u"gridFrameWidth"_s, u"2.0"_s ).toDouble() );
    mapGrid->setFramePenSize( gridElem.attribute( u"gridFramePenThickness"_s, u"0.5"_s ).toDouble() );
    mapGrid->setFramePenColor( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( u"framePenColor"_s, u"0,0,0"_s ) ) );
    mapGrid->setFrameFillColor1( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( u"frameFillColor1"_s, u"255,255,255,255"_s ) ) );
    mapGrid->setFrameFillColor2( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( u"frameFillColor2"_s, u"0,0,0,255"_s ) ) );
    mapGrid->setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( itemElem.attribute( u"gridBlendMode"_s, u"0"_s ).toUInt() ) ) );
    const QDomElement gridSymbolElem = gridElem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsLineSymbol > lineSymbol;
    if ( gridSymbolElem.isNull() )
    {
      //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
      lineSymbol = QgsLineSymbol::createSimple( QVariantMap() );
      lineSymbol->setWidth( gridElem.attribute( u"penWidth"_s, u"0"_s ).toDouble() );
      lineSymbol->setColor( QColor( gridElem.attribute( u"penColorRed"_s, u"0"_s ).toInt(),
                                    gridElem.attribute( u"penColorGreen"_s, u"0"_s ).toInt(),
                                    gridElem.attribute( u"penColorBlue"_s, u"0"_s ).toInt() ) );
    }
    else
    {
      lineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( gridSymbolElem, context );
    }
    mapGrid->setLineSymbol( lineSymbol.release() );

    //annotation
    const QDomNodeList annotationNodeList = gridElem.elementsByTagName( u"Annotation"_s );
    if ( !annotationNodeList.isEmpty() )
    {
      const QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mapGrid->setAnnotationEnabled( annotationElem.attribute( u"show"_s, u"0"_s ) != "0"_L1 );
      mapGrid->setAnnotationFormat( static_cast< Qgis::MapGridAnnotationFormat >( annotationElem.attribute( u"format"_s, u"0"_s ).toInt() ) );
      mapGrid->setAnnotationPosition( static_cast< Qgis::MapGridAnnotationPosition >( annotationElem.attribute( u"leftPosition"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Left );
      mapGrid->setAnnotationPosition( static_cast< Qgis::MapGridAnnotationPosition >( annotationElem.attribute( u"rightPosition"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Right );
      mapGrid->setAnnotationPosition( static_cast< Qgis::MapGridAnnotationPosition >( annotationElem.attribute( u"topPosition"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Top );
      mapGrid->setAnnotationPosition( static_cast< Qgis::MapGridAnnotationPosition >( annotationElem.attribute( u"bottomPosition"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Bottom );
      mapGrid->setAnnotationDirection( static_cast< Qgis::MapGridAnnotationDirection >( annotationElem.attribute( u"leftDirection"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Left );
      mapGrid->setAnnotationDirection( static_cast< Qgis::MapGridAnnotationDirection >( annotationElem.attribute( u"rightDirection"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Right );
      mapGrid->setAnnotationDirection( static_cast< Qgis::MapGridAnnotationDirection >( annotationElem.attribute( u"topDirection"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Top );
      mapGrid->setAnnotationDirection( static_cast< Qgis::MapGridAnnotationDirection >( annotationElem.attribute( u"bottomDirection"_s, u"0"_s ).toInt() ), Qgis::MapGridBorderSide::Bottom );
      mapGrid->setAnnotationFrameDistance( annotationElem.attribute( u"frameDistance"_s, u"0"_s ).toDouble() );
      QFont annotationFont;
      annotationFont.fromString( annotationElem.attribute( u"font"_s, QString() ) );

      QgsTextFormat annotationFormat = mapGrid->annotationTextFormat();
      annotationFormat.setFont( annotationFont );
      if ( annotationFont.pointSizeF() > 0 )
      {
        annotationFormat.setSize( annotationFont.pointSizeF() );
        annotationFormat.setSizeUnit( Qgis::RenderUnit::Points );
      }
      else if ( annotationFont.pixelSize() > 0 )
      {
        annotationFormat.setSize( annotationFont.pixelSize() );
        annotationFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
      }
      annotationFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"fontColor"_s, u"0,0,0,255"_s ) ) );
      mapGrid->setAnnotationTextFormat( annotationFormat );

      mapGrid->setAnnotationPrecision( annotationElem.attribute( u"precision"_s, u"3"_s ).toInt() );
    }
    layoutItem->mGridStack->addGrid( mapGrid );
  }

  //atlas
  const QDomNodeList atlasNodeList = itemElem.elementsByTagName( u"AtlasMap"_s );
  if ( !atlasNodeList.isEmpty() )
  {
    const QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    layoutItem->mAtlasDriven = ( atlasElem.attribute( u"atlasDriven"_s, u"0"_s ) != "0"_L1 );
    if ( atlasElem.hasAttribute( u"fixedScale"_s ) ) // deprecated XML
    {
      layoutItem->setAtlasScalingMode( atlasElem.attribute( u"fixedScale"_s, u"0"_s ) != "0"_L1  ? QgsLayoutItemMap::AtlasScalingMode::Fixed : QgsLayoutItemMap::AtlasScalingMode::Auto );
    }
    else if ( atlasElem.hasAttribute( u"scalingMode"_s ) )
    {
      layoutItem->setAtlasScalingMode( static_cast<QgsLayoutItemMap::AtlasScalingMode>( atlasElem.attribute( u"scalingMode"_s ).toInt() ) );
    }
    layoutItem->setAtlasMargin( atlasElem.attribute( u"margin"_s, u"0.1"_s ).toDouble() );
  }

  layoutItem->updateBoundingRect();

  return true;
}

bool QgsCompositionConverter::readScaleBarXml( QgsLayoutItemScaleBar *layoutItem, const QDomElement &itemElem, const QgsProject *project, const QgsStringMap &mapId2Uuid )
{
  Q_UNUSED( project )
  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  layoutItem->setHeight( itemElem.attribute( u"height"_s, u"5.0"_s ).toDouble() );
  layoutItem->setHeight( itemElem.attribute( u"height"_s, u"5.0"_s ).toDouble() );
  layoutItem->setLabelBarSpace( itemElem.attribute( u"labelBarSpace"_s, u"3.0"_s ).toDouble() );
  layoutItem->setBoxContentSpace( itemElem.attribute( u"boxContentSpace"_s, u"1.0"_s ).toDouble() );
  layoutItem->setNumberOfSegments( itemElem.attribute( u"numSegments"_s, u"2"_s ).toInt() );
  layoutItem->setNumberOfSegmentsLeft( itemElem.attribute( u"numSegmentsLeft"_s, u"0"_s ).toInt() );
  layoutItem->setUnitsPerSegment( itemElem.attribute( u"numUnitsPerSegment"_s, u"1.0"_s ).toDouble() );
  layoutItem->setSegmentSizeMode( static_cast<Qgis::ScaleBarSegmentSizeMode>( itemElem.attribute( u"segmentSizeMode"_s, u"0"_s ).toInt() ) );
  layoutItem->setMinimumBarWidth( itemElem.attribute( u"minBarWidth"_s, u"50"_s ).toDouble() );
  layoutItem->setMaximumBarWidth( itemElem.attribute( u"maxBarWidth"_s, u"150"_s ).toDouble() );
  layoutItem->mSegmentMillimeters = itemElem.attribute( u"segmentMillimeters"_s, u"0.0"_s ).toDouble();
  layoutItem->setMapUnitsPerScaleBarUnit( itemElem.attribute( u"numMapUnitsPerScaleBarUnit"_s, u"1.0"_s ).toDouble() );
  layoutItem->setUnitLabel( itemElem.attribute( u"unitLabel"_s ) );

  QFont f;
  if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, u"scaleBarFont"_s ) )
  {
    f.fromString( itemElem.attribute( u"font"_s, QString() ) );
  }
  Q_NOWARN_DEPRECATED_PUSH
  layoutItem->setFont( f );
  Q_NOWARN_DEPRECATED_POP

  //colors
  //fill color
  const QDomNodeList fillColorList = itemElem.elementsByTagName( u"fillColor"_s );
  if ( !fillColorList.isEmpty() )
  {
    const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColorElem.attribute( u"red"_s ).toDouble( &redOk );
    fillGreen = fillColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    fillBlue = fillColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    fillAlpha = fillColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->fillSymbol()->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    layoutItem->fillSymbol()->setColor( QColor( itemElem.attribute( u"brushColor"_s, u"#000000"_s ) ) );
  }

  //fill color 2
  const QDomNodeList fillColor2List = itemElem.elementsByTagName( u"fillColor2"_s );
  if ( !fillColor2List.isEmpty() )
  {
    const QDomElement fillColor2Elem = fillColor2List.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColor2Elem.attribute( u"red"_s ).toDouble( &redOk );
    fillGreen = fillColor2Elem.attribute( u"green"_s ).toDouble( &greenOk );
    fillBlue = fillColor2Elem.attribute( u"blue"_s ).toDouble( &blueOk );
    fillAlpha = fillColor2Elem.attribute( u"alpha"_s ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->alternateFillSymbol()->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    layoutItem->alternateFillSymbol()->setColor( QColor( itemElem.attribute( u"brush2Color"_s, u"#ffffff"_s ) ) );
  }

  auto lineSymbol = std::make_unique< QgsLineSymbol >();
  auto lineSymbolLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( itemElem.attribute( u"outlineWidth"_s, u"0.3"_s ).toDouble() );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( u"lineJoinStyle"_s, u"miter"_s ) ) );
  lineSymbolLayer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( itemElem.attribute( u"lineCapStyle"_s, u"square"_s ) ) );
  //stroke color
  const QDomNodeList strokeColorList = itemElem.elementsByTagName( u"strokeColor"_s );
  if ( !strokeColorList.isEmpty() )
  {
    const QDomElement strokeColorElem = strokeColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int strokeRed, strokeGreen, strokeBlue, strokeAlpha;

    strokeRed = strokeColorElem.attribute( u"red"_s ).toDouble( &redOk );
    strokeGreen = strokeColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    strokeBlue = strokeColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    strokeAlpha = strokeColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      lineSymbolLayer->setColor( QColor( strokeRed, strokeGreen, strokeBlue, strokeAlpha ) );
    }
  }
  else
  {
    lineSymbolLayer->setColor( QColor( itemElem.attribute( u"penColor"_s, u"#000000"_s ) ) );
  }
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
  layoutItem->setDivisionLineSymbol( lineSymbol->clone() );
  layoutItem->setSubdivisionLineSymbol( lineSymbol->clone() );
  layoutItem->setLineSymbol( lineSymbol.release() );

  //font color
  const QDomNodeList textColorList = itemElem.elementsByTagName( u"textColor"_s );
  if ( !textColorList.isEmpty() )
  {
    const QDomElement textColorElem = textColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int textRed, textGreen, textBlue, textAlpha;

    textRed = textColorElem.attribute( u"red"_s ).toDouble( &redOk );
    textGreen = textColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    textBlue = textColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    textAlpha = textColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      Q_NOWARN_DEPRECATED_PUSH
      layoutItem->setFontColor( QColor( textRed, textGreen, textBlue, textAlpha ) );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  else
  {
    QColor c;
    c.setNamedColor( itemElem.attribute( u"fontColor"_s, u"#000000"_s ) );
    Q_NOWARN_DEPRECATED_PUSH
    layoutItem->setFontColor( c );
    Q_NOWARN_DEPRECATED_POP
  }

  //style
  const QString styleString = itemElem.attribute( u"style"_s, QString() );
  layoutItem->setStyle( QObject::tr( styleString.toLocal8Bit().data() ) );

  if ( itemElem.attribute( u"unitType"_s ).isEmpty() )
  {
    Qgis::DistanceUnit u = Qgis::DistanceUnit::Unknown;
    switch ( itemElem.attribute( u"units"_s ).toInt() )
    {
      case 0:
        u = Qgis::DistanceUnit::Unknown;
        break;
      case 1:
        u = Qgis::DistanceUnit::Meters;
        break;
      case 2:
        u = Qgis::DistanceUnit::Feet;
        break;
      case 3:
        u = Qgis::DistanceUnit::NauticalMiles;
        break;
    }
    layoutItem->setUnits( u );
  }
  else
  {
    layoutItem->setUnits( QgsUnitTypes::decodeDistanceUnit( itemElem.attribute( u"unitType"_s ) ) );
  }
  layoutItem->setAlignment( static_cast< Qgis::ScaleBarAlignment >( itemElem.attribute( u"alignment"_s, u"0"_s ).toInt() ) );

  //composer map: use uuid
  const QString mapId = itemElem.attribute( u"mapId"_s, u"-1"_s );
  if ( mapId != "-1"_L1 && mapId2Uuid.contains( mapId ) )
  {
    QgsLayoutItemMap *mapInstance = qobject_cast<QgsLayoutItemMap *>( layoutItem->layout()->itemByUuid( mapId2Uuid[ mapId ] ) );
    if ( mapInstance )
    {
      layoutItem->setLinkedMap( mapInstance );
    }
  }

  return true;
}

bool QgsCompositionConverter::readLegendXml( QgsLayoutItemLegend *layoutItem, const QDomElement &itemElem, const QgsProject *project, const QgsStringMap &mapId2Uuid )
{
  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  QgsPathResolver pathResolver;
  if ( project )
    pathResolver = project->pathResolver();
  QgsReadWriteContext context;
  context.setPathResolver( pathResolver );
  context.setProjectTranslator( const_cast<QgsProject *>( project ) );

  //composer map: use uuid
  const QString mapId = itemElem.attribute( u"map"_s, u"-1"_s );
  if ( mapId != "-1"_L1 && mapId2Uuid.contains( mapId ) )
  {
    QgsLayoutItemMap *mapInstance = qobject_cast<QgsLayoutItemMap *>( layoutItem->layout()->itemByUuid( mapId2Uuid[ mapId ] ) );
    if ( mapInstance )
    {
      layoutItem->setLinkedMap( mapInstance );
    }
  }

  //read general properties
  layoutItem->setTitle( itemElem.attribute( u"title"_s ) );
  if ( !itemElem.attribute( u"titleAlignment"_s ).isEmpty() )
  {
    layoutItem->setTitleAlignment( static_cast< Qt::AlignmentFlag >( itemElem.attribute( u"titleAlignment"_s ).toInt() ) );
  }
  int colCount = itemElem.attribute( u"columnCount"_s, u"1"_s ).toInt();
  if ( colCount < 1 ) colCount = 1;
  layoutItem->setColumnCount( colCount );
  layoutItem->setSplitLayer( itemElem.attribute( u"splitLayer"_s, u"0"_s ).toInt() == 1 );
  layoutItem->setEqualColumnWidth( itemElem.attribute( u"equalColumnWidth"_s, u"0"_s ).toInt() == 1 );

  const QDomNodeList stylesNodeList = itemElem.elementsByTagName( u"styles"_s );
  if ( !stylesNodeList.isEmpty() )
  {
    const QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      const QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsLegendStyle style;
      style.readXml( styleElem, QDomDocument() );
      const QString name = styleElem.attribute( u"name"_s );
      Qgis::LegendComponent s;
      if ( name == "title"_L1 ) s = Qgis::LegendComponent::Title;
      else if ( name == "group"_L1 ) s = Qgis::LegendComponent::Group;
      else if ( name == "subgroup"_L1 ) s = Qgis::LegendComponent::Subgroup;
      else if ( name == "symbol"_L1 ) s = Qgis::LegendComponent::Symbol;
      else if ( name == "symbolLabel"_L1 ) s = Qgis::LegendComponent::SymbolLabel;
      else continue;
      layoutItem->setStyle( s, style );
    }
  }

  //font color
  QColor fontClr;
  fontClr.setNamedColor( itemElem.attribute( u"fontColor"_s, u"#000000"_s ) );
  layoutItem->rstyle( Qgis::LegendComponent::Title ).textFormat().setColor( fontClr );
  layoutItem->rstyle( Qgis::LegendComponent::Group ).textFormat().setColor( fontClr );
  layoutItem->rstyle( Qgis::LegendComponent::Subgroup ).textFormat().setColor( fontClr );
  layoutItem->rstyle( Qgis::LegendComponent::SymbolLabel ).textFormat().setColor( fontClr );

  //spaces
  layoutItem->setBoxSpace( itemElem.attribute( u"boxSpace"_s, u"2.0"_s ).toDouble() );
  layoutItem->setColumnSpace( itemElem.attribute( u"columnSpace"_s, u"2.0"_s ).toDouble() );

  layoutItem->setSymbolWidth( itemElem.attribute( u"symbolWidth"_s, u"7.0"_s ).toDouble() );
  layoutItem->setSymbolHeight( itemElem.attribute( u"symbolHeight"_s, u"14.0"_s ).toDouble() );
  layoutItem->setWmsLegendWidth( itemElem.attribute( u"wmsLegendWidth"_s, u"50"_s ).toDouble() );
  layoutItem->setWmsLegendHeight( itemElem.attribute( u"wmsLegendHeight"_s, u"25"_s ).toDouble() );
  Q_NOWARN_DEPRECATED_PUSH
  layoutItem->setLineSpacing( itemElem.attribute( u"lineSpacing"_s, u"1.0"_s ).toDouble() );
  Q_NOWARN_DEPRECATED_POP

  layoutItem->setDrawRasterStroke( itemElem.attribute( u"rasterBorder"_s, u"1"_s ) != "0"_L1 );
  layoutItem->setRasterStrokeColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"rasterBorderColor"_s, u"0,0,0"_s ) ) );
  layoutItem->setRasterStrokeWidth( itemElem.attribute( u"rasterBorderWidth"_s, u"0"_s ).toDouble() );

  layoutItem->setWrapString( itemElem.attribute( u"wrapChar"_s ) );

  layoutItem->mSizeToContents = itemElem.attribute( u"resizeToContents"_s, u"1"_s ) != "0"_L1;
  layoutItem->mLegendFilterByMap = itemElem.attribute( u"legendFilterByMap"_s, u"0"_s ).toInt();
  layoutItem->mFilterOutAtlas = itemElem.attribute( u"legendFilterByAtlas"_s, u"0"_s ).toInt();

  // QGIS >= 2.6
  QDomElement layerTreeElem = itemElem.firstChildElement( u"layer-tree"_s );
  if ( layerTreeElem.isNull() )
    layerTreeElem = itemElem.firstChildElement( u"layer-tree-group"_s );

  if ( !layerTreeElem.isNull() )
  {
    std::unique_ptr< QgsLayerTree > tree( QgsLayerTree::readXml( layerTreeElem, context ) );
    if ( project )
      tree->resolveReferences( project, true );
    layoutItem->setCustomLayerTree( tree.release() );
  }
  else
  {
    layoutItem->setCustomLayerTree( nullptr );
  }

  return true;
}

bool QgsCompositionConverter::readAtlasXml( QgsLayoutAtlas *atlasItem, const QDomElement &itemElem, const QgsProject *project )
{
  atlasItem->setEnabled( itemElem.attribute( u"enabled"_s, u"false"_s ) == "true"_L1 );

  // look for stored layer name
  const QString layerId = itemElem.attribute( u"coverageLayer"_s );
  const QString layerName = itemElem.attribute( u"coverageLayerName"_s );
  const QString layerSource = itemElem.attribute( u"coverageLayerSource"_s );
  const QString layerProvider = itemElem.attribute( u"coverageLayerProvider"_s );

  QgsVectorLayerRef layerRef( layerId, layerName, layerSource, layerProvider );
  atlasItem->setCoverageLayer( layerRef.resolveWeakly( project ) );

  atlasItem->setPageNameExpression( itemElem.attribute( u"pageNameExpression"_s, QString() ) );
  QString errorString;
  atlasItem->setFilenameExpression( itemElem.attribute( u"filenamePattern"_s, QString() ), errorString );
  // note: no error reporting for errorString
  atlasItem->setSortFeatures( itemElem.attribute( u"sortFeatures"_s, u"false"_s ) == "true"_L1 );
  if ( atlasItem->sortFeatures() )
  {
    atlasItem->setSortExpression( itemElem.attribute( u"sortKey"_s, QString() ) );
    atlasItem->setSortAscending( itemElem.attribute( u"sortAscending"_s, u"true"_s ) == "true"_L1 );
  }
  atlasItem->setFilterFeatures( itemElem.attribute( u"filterFeatures"_s, u"false"_s ) == "true"_L1 );
  if ( atlasItem->filterFeatures( ) )
  {
    QString expErrorString;
    atlasItem->setFilterExpression( itemElem.attribute( u"featureFilter"_s, QString() ), expErrorString );
    // note: no error reporting for errorString
  }

  atlasItem->setHideCoverage( itemElem.attribute( u"hideCoverage"_s, u"false"_s ) == "true"_L1 );

  return true;

}

bool QgsCompositionConverter::readHtmlXml( QgsLayoutItemHtml *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  Q_UNUSED( project )
  readOldComposerObjectXml( layoutItem, itemElem );

  //first create the frames
  layoutItem->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( itemElem.attribute( u"resizeMode"_s, u"0"_s ).toInt() ) );
  const QDomNodeList frameList = itemElem.elementsByTagName( u"ComposerFrame"_s );
  for ( int i = 0; i < frameList.size(); ++i )
  {
    const QDomElement frameElem = frameList.at( i ).toElement();
    QgsLayoutFrame *newFrame = new QgsLayoutFrame( layoutItem->layout(), layoutItem );
    restoreGeneralComposeItemProperties( newFrame, frameElem );
    // Read frame XML
    const double x = itemElem.attribute( u"sectionX"_s ).toDouble();
    const double y = itemElem.attribute( u"sectionY"_s ).toDouble();
    const double width = itemElem.attribute( u"sectionWidth"_s ).toDouble();
    const double height = itemElem.attribute( u"sectionHeight"_s ).toDouble();
    newFrame->setContentSection( QRectF( x, y, width, height ) );
    newFrame->setHidePageIfEmpty( itemElem.attribute( u"hidePageIfEmpty"_s, u"0"_s ).toInt() );
    newFrame->setHideBackgroundIfEmpty( itemElem.attribute( u"hideBackgroundIfEmpty"_s, u"0"_s ).toInt() );
    layoutItem->addFrame( newFrame, false );
  }

  bool contentModeOK;
  layoutItem->setContentMode( static_cast< QgsLayoutItemHtml::ContentMode >( itemElem.attribute( u"contentMode"_s ).toInt( &contentModeOK ) ) );
  if ( !contentModeOK )
  {
    layoutItem->setContentMode( QgsLayoutItemHtml::ContentMode::Url );
  }
  layoutItem->setEvaluateExpressions( itemElem.attribute( u"evaluateExpressions"_s, u"true"_s ) == "true"_L1 );
  layoutItem->setUseSmartBreaks( itemElem.attribute( u"useSmartBreaks"_s, u"true"_s ) == "true"_L1 );
  layoutItem->setMaxBreakDistance( itemElem.attribute( u"maxBreakDistance"_s, u"10"_s ).toDouble() );
  layoutItem->setHtml( itemElem.attribute( u"html"_s ) );
  layoutItem->setUserStylesheet( itemElem.attribute( u"stylesheet"_s ) );
  layoutItem->setUserStylesheetEnabled( itemElem.attribute( u"stylesheetEnabled"_s, u"false"_s ) == "true"_L1 );

  //finally load the set url
  const QString urlString = itemElem.attribute( u"url"_s );
  if ( !urlString.isEmpty() )
  {
    layoutItem->setUrl( urlString );
  }
  layoutItem->loadHtml( true );

  return true;
}

bool QgsCompositionConverter::readTableXml( QgsLayoutItemAttributeTable *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{

  Q_UNUSED( project )
  readOldComposerObjectXml( layoutItem, itemElem );

  //first create the frames
  layoutItem->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( itemElem.attribute( u"resizeMode"_s, u"0"_s ).toInt() ) );
  const QDomNodeList frameList = itemElem.elementsByTagName( u"ComposerFrame"_s );
  for ( int i = 0; i < frameList.size(); ++i )
  {
    const QDomElement frameElem = frameList.at( i ).toElement();
    QgsLayoutFrame *newFrame = new QgsLayoutFrame( layoutItem->layout(), layoutItem );
    restoreGeneralComposeItemProperties( newFrame, frameElem );
    // Read frame XML
    const double x = itemElem.attribute( u"sectionX"_s ).toDouble();
    const double y = itemElem.attribute( u"sectionY"_s ).toDouble();
    const double width = itemElem.attribute( u"sectionWidth"_s ).toDouble();
    const double height = itemElem.attribute( u"sectionHeight"_s ).toDouble();
    newFrame->setContentSection( QRectF( x, y, width, height ) );
    newFrame->setHidePageIfEmpty( itemElem.attribute( u"hidePageIfEmpty"_s, u"0"_s ).toInt() );
    newFrame->setHideBackgroundIfEmpty( itemElem.attribute( u"hideBackgroundIfEmpty"_s, u"0"_s ).toInt() );
    layoutItem->addFrame( newFrame, false );
  }

  layoutItem->setEmptyTableBehavior( static_cast<QgsLayoutTable::EmptyTableMode>( itemElem.attribute( u"emptyTableMode"_s, u"0"_s ).toInt() ) );
  layoutItem->setEmptyTableMessage( itemElem.attribute( u"emptyTableMessage"_s, QObject::tr( "No matching records" ) ) );
  layoutItem->setShowEmptyRows( itemElem.attribute( u"showEmptyRows"_s, u"0"_s ).toInt() );
  QFont headerFont;
  if ( !QgsFontUtils::setFromXmlChildNode( headerFont, itemElem, u"headerFontProperties"_s ) )
  {
    headerFont.fromString( itemElem.attribute( u"headerFont"_s, QString() ) );
  }
  QgsTextFormat headerFormat = layoutItem->headerTextFormat();
  headerFormat.setFont( headerFont );
  if ( headerFont.pointSizeF() > 0 )
  {
    headerFormat.setSize( headerFont.pointSizeF() );
    headerFormat.setSizeUnit( Qgis::RenderUnit::Points );
  }
  else if ( headerFont.pixelSize() > 0 )
  {
    headerFormat.setSize( headerFont.pixelSize() );
    headerFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
  }
  headerFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"headerFontColor"_s, u"0,0,0,255"_s ) ) );
  layoutItem->setHeaderTextFormat( headerFormat );
  layoutItem->setHeaderHAlignment( static_cast<QgsLayoutTable::HeaderHAlignment>( itemElem.attribute( u"headerHAlignment"_s, u"0"_s ).toInt() ) ) ;
  layoutItem->setHeaderMode( static_cast<QgsLayoutTable::HeaderMode>( itemElem.attribute( u"headerMode"_s, u"0"_s ).toInt() ) );

  QFont contentFont;
  if ( !QgsFontUtils::setFromXmlChildNode( contentFont, itemElem, u"contentFontProperties"_s ) )
  {
    contentFont.fromString( itemElem.attribute( u"contentFont"_s, QString() ) );
  }
  QgsTextFormat contentFormat = layoutItem->contentTextFormat();
  contentFormat.setFont( contentFont );
  if ( contentFont.pointSizeF() > 0 )
  {
    contentFormat.setSize( contentFont.pointSizeF() );
    contentFormat.setSizeUnit( Qgis::RenderUnit::Points );
  }
  else if ( contentFont.pixelSize() > 0 )
  {
    contentFormat.setSize( contentFont.pixelSize() );
    contentFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
  }
  contentFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"contentFontColor"_s, u"0,0,0,255"_s ) ) );
  layoutItem->setContentTextFormat( contentFormat );

  layoutItem->setCellMargin( itemElem.attribute( u"cellMargin"_s, u"1.0"_s ).toDouble() );
  layoutItem->setGridStrokeWidth( itemElem.attribute( u"gridStrokeWidth"_s, u"0.5"_s ).toDouble() );
  layoutItem->setHorizontalGrid( itemElem.attribute( u"horizontalGrid"_s, u"1"_s ).toInt() );
  layoutItem->setVerticalGrid( itemElem.attribute( u"verticalGrid"_s, u"1"_s ).toInt() );
  layoutItem->setShowGrid( itemElem.attribute( u"showGrid"_s, u"1"_s ).toInt() );
  layoutItem->setGridColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"gridColor"_s, u"0,0,0,255"_s ) ) );
  layoutItem->setBackgroundColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( u"backgroundColor"_s, u"255,255,255,0"_s ) ) );
  layoutItem->setWrapBehavior( static_cast<QgsLayoutTable::WrapBehavior>( itemElem.attribute( u"wrapBehavior"_s, u"0"_s ).toInt() ) );

  //restore column specifications
  layoutItem->mColumns.clear();
  layoutItem->mSortColumns.clear();

  const QDomNodeList columnsList = itemElem.elementsByTagName( u"displayColumns"_s );
  if ( !columnsList.isEmpty() )
  {
    const QDomElement columnsElem = columnsList.at( 0 ).toElement();
    const QDomNodeList columnEntryList = columnsElem.elementsByTagName( u"column"_s );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      const QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsLayoutTableColumn column;
      column.mHAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( u"hAlignment"_s, QString::number( Qt::AlignLeft ) ).toInt() );
      column.mVAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( u"vAlignment"_s, QString::number( Qt::AlignVCenter ) ).toInt() );
      column.mHeading = columnElem.attribute( u"heading"_s, QString() );
      column.mAttribute = columnElem.attribute( u"attribute"_s, QString() );
      column.mSortByRank = columnElem.attribute( u"sortByRank"_s, u"0"_s ).toInt();
      column.mSortOrder = static_cast< Qt::SortOrder >( columnElem.attribute( u"sortOrder"_s, QString::number( Qt::AscendingOrder ) ).toInt() );
      column.mWidth = columnElem.attribute( u"width"_s, u"0.0"_s ).toDouble();

      const QDomNodeList bgColorList = columnElem.elementsByTagName( u"backgroundColor"_s );
      if ( !bgColorList.isEmpty() )
      {
        const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
        bool redOk, greenOk, blueOk, alphaOk;
        int bgRed, bgGreen, bgBlue, bgAlpha;
        bgRed = bgColorElem.attribute( u"red"_s ).toDouble( &redOk );
        bgGreen = bgColorElem.attribute( u"green"_s ).toDouble( &greenOk );
        bgBlue = bgColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
        bgAlpha = bgColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );
        if ( redOk && greenOk && blueOk && alphaOk )
        {
          column.mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
        }
      }
      layoutItem->mColumns.append( column );

      // sorting columns are now (QGIS 3.14+) handled in a dedicated list
      // copy the display columns if sortByRank > 0 and then, sort them by rank
      Q_NOWARN_DEPRECATED_PUSH
      std::copy_if( layoutItem->mColumns.begin(), layoutItem->mColumns.end(), std::back_inserter( layoutItem->mSortColumns ), []( const QgsLayoutTableColumn & col ) {return col.sortByRank() > 0;} );
      std::sort( layoutItem->mSortColumns.begin(), layoutItem->mSortColumns.end(), []( const QgsLayoutTableColumn & a, const QgsLayoutTableColumn & b ) {return a.sortByRank() < b.sortByRank();} );
      Q_NOWARN_DEPRECATED_POP
    }
  }

  //restore cell styles
  const QDomNodeList stylesList = itemElem.elementsByTagName( u"cellStyles"_s );
  if ( !stylesList.isEmpty() )
  {
    const QDomElement stylesElem = stylesList.at( 0 ).toElement();

    QMap< QgsLayoutTable::CellStyleGroup, QString >::const_iterator it = layoutItem->mCellStyleNames.constBegin();
    for ( ; it != layoutItem->mCellStyleNames.constEnd(); ++it )
    {
      const QString styleName = it.value();
      const QDomNodeList styleList = stylesElem.elementsByTagName( styleName );
      if ( !styleList.isEmpty() )
      {
        const QDomElement styleElem = styleList.at( 0 ).toElement();
        QgsLayoutTableStyle *style = layoutItem->mCellStyles.value( it.key() );
        if ( style )
          style->readXml( styleElem );
      }
    }
  }

  // look for stored layer name
  const QString layerId = itemElem.attribute( u"vectorLayer"_s );
  const QString layerName = itemElem.attribute( u"vectorLayerName"_s );
  const QString layerSource = itemElem.attribute( u"vectorLayerSource"_s );
  const QString layerProvider = itemElem.attribute( u"vectorLayerProvider"_s );

  QgsVectorLayerRef layerRef( layerId, layerName, layerSource, layerProvider );
  layoutItem->setVectorLayer( layerRef.resolveWeakly( project ) );

  return true;
}

bool QgsCompositionConverter::readGroupXml( QgsLayoutItemGroup *layoutItem, const QDomElement &itemElem, const QgsProject *project, const QList< QgsLayoutObject * > &items )
{
  Q_UNUSED( project )

  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  const QDomNodeList nodes = itemElem.elementsByTagName( "ComposerItemGroupElement" );
  for ( int i = 0, n = nodes.size(); i < n; ++i )
  {
    const QDomElement groupElement = nodes.at( i ).toElement();
    const QString elementUuid = groupElement.attribute( "uuid" );

    for ( QgsLayoutObject *item : items )
    {
      if ( dynamic_cast<QgsLayoutItem *>( item ) && static_cast<QgsLayoutItem *>( item )->uuid() == elementUuid )
      {
        layoutItem->addItem( static_cast<QgsLayoutItem *>( item ) );
        break;
      }
    }
  }

  return true;
}

template <class T, class T2>
bool QgsCompositionConverter::readPolyXml( T *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  restoreGeneralComposeItemProperties( layoutItem, itemElem );
  const QDomNodeList nodeList = itemElem.elementsByTagName( u"node"_s );
  if ( !nodeList.isEmpty() )
  {
    QPolygonF polygon;
    for ( int i = 0; i < nodeList.length(); i++ )
    {
      const QDomElement node = nodeList.at( i ).toElement();
      polygon.append( QPointF( node.attribute( u"x"_s ).toDouble( ), node.attribute( u"y"_s ).toDouble() ) );
    }
    layoutItem->setNodes( polygon );
  }
  if ( itemElem.elementsByTagName( u"symbol"_s ).size() )
  {
    const QDomElement symbolElement = itemElem.elementsByTagName( u"symbol"_s ).at( 0 ).toElement();
    QgsReadWriteContext context;
    if ( project )
      context.setPathResolver( project->pathResolver( ) );
    std::unique_ptr< T2 > styleSymbol = QgsSymbolLayerUtils::loadSymbol<T2>( symbolElement, context );
    if ( styleSymbol )
      layoutItem->setSymbol( styleSymbol.release() );
  }
  // Disable frame for shapes
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
  layoutItem->mUuid = itemElem.attribute( u"uuid"_s, QUuid::createUuid().toString() );

  // temporary for groups imported from templates
  layoutItem->mTemplateUuid = itemElem.attribute( u"templateUuid"_s );

  //id
  const QString id = itemElem.attribute( u"id"_s, QString() );
  layoutItem->setId( id );

  //frame
  const QString frame = itemElem.attribute( u"frame"_s );
  layoutItem->setFrameEnabled( frame.compare( "true"_L1, Qt::CaseInsensitive ) == 0 ) ;

  //frame
  const QString background = itemElem.attribute( u"background"_s );
  layoutItem->setBackgroundEnabled( background.compare( "true"_L1, Qt::CaseInsensitive ) == 0 );

  //position lock for mouse moves/resizes
  const QString positionLock = itemElem.attribute( u"positionLock"_s );
  layoutItem->setLocked( positionLock.compare( "true"_L1, Qt::CaseInsensitive ) == 0 );

  //visibility
  layoutItem->setVisibility( itemElem.attribute( u"visibility"_s, u"1"_s ) != "0"_L1 );

  layoutItem->mParentGroupUuid = itemElem.attribute( u"groupUuid"_s );
  if ( !layoutItem->mParentGroupUuid.isEmpty() )
  {
    if ( QgsLayoutItemGroup *group = layoutItem->parentGroup() )
    {
      group->addItem( layoutItem );
    }
  }
  layoutItem->mTemplateUuid = itemElem.attribute( u"templateUuid"_s );


  const QRectF position = itemPosition( layoutItem, itemElem );

  // TODO: missing?
  // mLastValidViewScaleFactor = itemElem.attribute( u"lastValidViewScaleFactor"_s, u"-1"_s ).toDouble();

  layoutItem->setZValue( itemElem.attribute( u"zValue"_s ).toDouble() );

  //pen
  const QDomNodeList frameColorList = itemElem.elementsByTagName( u"FrameColor"_s );
  if ( !frameColorList.isEmpty() )
  {
    const QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk, widthOk;
    int penRed, penGreen, penBlue, penAlpha;
    double penWidth;

    penWidth = itemElem.attribute( u"outlineWidth"_s ).toDouble( &widthOk );
    penRed = frameColorElem.attribute( u"red"_s ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );
    layoutItem->setFrameJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( u"frameJoinStyle"_s, u"miter"_s ) ) );

    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      layoutItem->setFrameStrokeColor( QColor( penRed, penGreen, penBlue, penAlpha ) );
      layoutItem->setFrameStrokeWidth( QgsLayoutMeasurement( penWidth ) );
      QPen framePen( layoutItem->frameStrokeColor() );
      framePen.setWidthF( layoutItem->frameStrokeWidth( ).length() );
      framePen.setJoinStyle( layoutItem->frameJoinStyle( ) );
      layoutItem->setPen( framePen );
      //apply any data defined settings
      layoutItem->refreshFrame( false );
    }
  }

  //brush
  const QDomNodeList bgColorList = itemElem.elementsByTagName( u"BackgroundColor"_s );
  if ( !bgColorList.isEmpty() )
  {
    const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( u"red"_s ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
      layoutItem->setBrush( QBrush( layoutItem->mBackgroundColor, Qt::SolidPattern ) );
    }
    //apply any data defined settings
    layoutItem->refreshBackgroundColor( false );
  }

  //blend mode
  layoutItem->setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( itemElem.attribute( u"blendMode"_s, u"0"_s ).toUInt() ) ) );

  //opacity
  if ( itemElem.hasAttribute( u"opacity"_s ) )
  {
    layoutItem->setItemOpacity( itemElem.attribute( u"opacity"_s, u"1"_s ).toDouble() );
  }
  else
  {
    layoutItem->setItemOpacity( 1.0 - itemElem.attribute( u"transparency"_s, u"0"_s ).toInt() / 100.0 );
  }

  layoutItem->mExcludeFromExports = itemElem.attribute( u"excludeFromExports"_s, u"0"_s ).toInt();
  layoutItem->mEvaluatedExcludeFromExports = layoutItem->mExcludeFromExports;

  // positioning
  layoutItem->attemptSetSceneRect( position );
  //rotation
  layoutItem->setItemRotation( itemElem.attribute( u"itemRotation"_s, u"0"_s ).toDouble(), false );

  layoutItem->mBlockUndoCommands = false;

  return true;
}



bool QgsCompositionConverter::readOldComposerObjectXml( QgsLayoutObject *layoutItem,
    const QDomElement &itemElem )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //old (pre 3.0) data defined properties
  QgsCompositionConverter::readOldDataDefinedPropertyMap( itemElem, layoutItem->mDataDefinedProperties );

  const QDomNode propsNode = itemElem.namedItem( u"dataDefinedProperties"_s );
  if ( !propsNode.isNull() )
  {
    layoutItem->mDataDefinedProperties.readXml( propsNode.toElement(), sPropertyDefinitions );
  }
  if ( layoutItem->mDataDefinedProperties.isActive( QgsCompositionConverter::DataDefinedProperty::Transparency ) )
  {
    // upgrade transparency -> opacity
    QString exp = layoutItem->mDataDefinedProperties.property( QgsCompositionConverter::DataDefinedProperty::Transparency ).asExpression();
    exp = u"100.0 - (%1)"_s.arg( exp );
    layoutItem->mDataDefinedProperties.setProperty( QgsCompositionConverter::DataDefinedProperty::Opacity, QgsProperty::fromExpression( exp ) );
    layoutItem->mDataDefinedProperties.setProperty( QgsCompositionConverter::DataDefinedProperty::Transparency, QgsProperty() );
  }

  //custom properties
  layoutItem->mCustomProperties.readXml( itemElem );

  return true;
}


void QgsCompositionConverter::readOldDataDefinedPropertyMap( const QDomElement &itemElem, QgsPropertyCollection &dataDefinedProperties )
{
  const QgsPropertiesDefinition defs = QgsCompositionConverter::propertyDefinitions();
  QgsPropertiesDefinition::const_iterator i = defs.constBegin();
  for ( ; i != defs.constEnd(); ++i )
  {
    const QString elemName = i.value().name();
    const QDomNodeList ddNodeList = itemElem.elementsByTagName( elemName );
    if ( !ddNodeList.isEmpty() )
    {
      const QDomElement ddElem = ddNodeList.at( 0 ).toElement();
      const QgsProperty prop = readOldDataDefinedProperty( static_cast< QgsCompositionConverter::DataDefinedProperty >( i.key() ), ddElem );
      if ( prop )
        dataDefinedProperties.setProperty( i.key(), prop );
    }
  }
}

QgsProperty QgsCompositionConverter::readOldDataDefinedProperty( const QgsCompositionConverter::DataDefinedProperty property, const QDomElement &ddElem )
{
  if ( property == QgsCompositionConverter::DataDefinedProperty::AllProperties || property == QgsCompositionConverter::DataDefinedProperty::NoProperty )
  {
    //invalid property
    return QgsProperty();
  }

  const QString active = ddElem.attribute( u"active"_s );
  bool isActive = false;
  if ( active.compare( "true"_L1, Qt::CaseInsensitive ) == 0 )
  {
    isActive = true;
  }
  const QString field = ddElem.attribute( u"field"_s );
  const QString expr = ddElem.attribute( u"expr"_s );

  const QString useExpr = ddElem.attribute( u"useExpr"_s );
  bool isExpression = false;
  if ( useExpr.compare( "true"_L1, Qt::CaseInsensitive ) == 0 )
  {
    isExpression = true;
  }

  if ( isExpression )
    return QgsProperty::fromExpression( expr, isActive );
  else
    return QgsProperty::fromField( field, isActive );
}
