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
#include <QUuid>

#include "qgscompositionconverter.h"
#include "qgsreadwritecontext.h"
#include "qgslayertree.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutobject.h"
#include "qgsfontutils.h"
#include "qgspainting.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgssymbollayer.h"
#include "qgsproject.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayer.h"
#include "qgslinesymbollayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"

#include "qgslayoutundostack.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapgrid.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemhtml.h"
#include "qgslayouttable.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutframe.h"
#include "qgslayoutguidecollection.h"
#include "qgslayoutnortharrowhandler.h"

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
          "|<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
          "|<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
          "|<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
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


std::unique_ptr< QgsPrintLayout > QgsCompositionConverter::createLayoutFromCompositionXml( const QDomElement &composerElement, QgsProject *project )
{
  initPropertyDefinitions();

  const QDomElement parentElement = composerElement.parentNode().toElement();

  std::unique_ptr< QgsPrintLayout > layout = std::make_unique< QgsPrintLayout >( project );
  layout->undoStack()->blockCommands( true );

  layout->mCustomProperties.readXml( composerElement );

  // Guides
  layout->guides().setVisible( composerElement.attribute( QStringLiteral( "guidesVisible" ), QStringLiteral( "1" ) ).toInt() != 0 );

  const int printResolution = composerElement.attribute( QStringLiteral( "printResolution" ), QStringLiteral( "300" ) ).toInt();
  layout->renderContext().setDpi( printResolution );

  // Create pages
  const int pages = composerElement.attribute( QStringLiteral( "numPages" ) ).toInt( );
  const float paperHeight = composerElement.attribute( QStringLiteral( "paperHeight" ) ).toDouble( );
  const float paperWidth = composerElement.attribute( QStringLiteral( "paperWidth" ) ).toDouble( );

  QString name = composerElement.attribute( QStringLiteral( "name" ) );
  // Try title
  if ( name.isEmpty() )
    name = composerElement.attribute( QStringLiteral( "title" ) );
  // Try title on parent element
  if ( name.isEmpty() )
    name = parentElement.attribute( QStringLiteral( "title" ) );
  layout->setName( name );
  const QgsLayoutSize pageSize( paperWidth, paperHeight );
  for ( int j = 0; j < pages; j++ )
  {
    QgsLayoutItemPage *page = QgsLayoutItemPage::create( layout.get() );
    page->setPageSize( pageSize );
    layout->pageCollection()->addPage( page );
    //custom snap lines
    const QDomNodeList snapLineNodes = composerElement.elementsByTagName( QStringLiteral( "SnapLine" ) );
    for ( int i = 0; i < snapLineNodes.size(); ++i )
    {
      const QDomElement snapLineElem = snapLineNodes.at( i ).toElement();
      const double x1 = snapLineElem.attribute( QStringLiteral( "x1" ) ).toDouble();
      const double y1 = snapLineElem.attribute( QStringLiteral( "y1" ) ).toDouble();
      const double x2 = snapLineElem.attribute( QStringLiteral( "x2" ) ).toDouble();
      // Not necessary: double y2 = snapLineElem.attribute( QStringLiteral( "y2" ) ).toDouble();
      const Qt::Orientation orientation( x1 == x2 ? Qt::Orientation::Vertical : Qt::Orientation::Horizontal );
      const QgsLayoutMeasurement position( x1 == x2 ? x1 : y1 );
      std::unique_ptr< QgsLayoutGuide > guide = std::make_unique< QgsLayoutGuide >( orientation, position, page );
      layout->guides().addGuide( guide.release() );
    }
  }


  if ( composerElement.elementsByTagName( QStringLiteral( "symbol" ) ).size() )
  {
    const QDomElement symbolElement = composerElement.elementsByTagName( QStringLiteral( "symbol" ) ).at( 0 ).toElement();
    QgsReadWriteContext context;
    if ( project )
      context.setPathResolver( project->pathResolver() );
    const std::unique_ptr< QgsFillSymbol > symbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context ) );
    if ( symbol )
      layout->pageCollection()->setPageStyleSymbol( symbol.get() );
  }

  addItemsFromCompositionXml( layout.get(), composerElement );

  // Read atlas from the parent element (Composer)
  if ( parentElement.elementsByTagName( QStringLiteral( "Atlas" ) ).size() )
  {
    const QDomElement atlasElement = parentElement.elementsByTagName( QStringLiteral( "Atlas" ) ).at( 0 ).toElement();
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
  const QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    const QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    //rotation
    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
    {
      //check for old (pre 2.1) rotation attribute
      layoutItem->setItemRotation( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), false );
    }
    QgsCompositionConverter::readXml( layoutItem, composerItemElem );
  }
}

QRectF QgsCompositionConverter::itemPosition( QgsLayoutItem *layoutItem, const QDomElement &itemElem )
{
  int page;
  double x, y, pagex, pagey, width, height;
  bool xOk, yOk, pageOk, pagexOk, pageyOk, widthOk, heightOk, positionModeOk;

  x = itemElem.attribute( QStringLiteral( "x" ) ).toDouble( &xOk );
  y = itemElem.attribute( QStringLiteral( "y" ) ).toDouble( &yOk );
  page = itemElem.attribute( QStringLiteral( "page" ) ).toInt( &pageOk );
  pagex = itemElem.attribute( QStringLiteral( "pagex" ) ).toDouble( &pagexOk );
  pagey = itemElem.attribute( QStringLiteral( "pagey" ) ).toDouble( &pageyOk );
  width = itemElem.attribute( QStringLiteral( "width" ) ).toDouble( &widthOk );
  height = itemElem.attribute( QStringLiteral( "height" ) ).toDouble( &heightOk );


  layoutItem->mReferencePoint = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( QStringLiteral( "positionMode" ) ).toInt( &positionModeOk ) );
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
  const QDomNodeList composerItemList = elem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  for ( int i = 0; i < composerItemList.size(); ++i )
  {
    const QDomElement currentComposerItemElem = composerItemList.at( i ).toElement();
    double x, y;
    bool xOk, yOk;
    x = currentComposerItemElem.attribute( QStringLiteral( "x" ) ).toDouble( &xOk );
    y = currentComposerItemElem.attribute( QStringLiteral( "y" ) ).toDouble( &yOk );
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
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerMap" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerMap" ) ).at( i ) );
    QgsLayoutItemMap *layoutItem = new QgsLayoutItemMap( layout );
    readMapXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Label
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerLabel" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerLabel" ) ).at( i ) );
    QgsLayoutItemLabel *layoutItem = new QgsLayoutItemLabel( layout );
    readLabelXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Shape
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerShape" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerShape" ) ).at( i ) );
    QgsLayoutItemShape *layoutItem = new QgsLayoutItemShape( layout );
    readShapeXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Picture
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerPicture" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerPicture" ) ).at( i ) );
    QgsLayoutItemPicture *layoutItem = new QgsLayoutItemPicture( layout );
    readPictureXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Polygon
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerPolygon" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerPolygon" ) ).at( i ) );
    QgsLayoutItemPolygon *layoutItem = new QgsLayoutItemPolygon( layout );
    readPolyXml<QgsLayoutItemPolygon, QgsFillSymbol>( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Polyline
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerPolyline" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerPolyline" ) ).at( i ) );
    QgsLayoutItemPolyline *layoutItem = new QgsLayoutItemPolyline( layout );
    readPolyXml<QgsLayoutItemPolyline, QgsLineSymbol>( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Arrow
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerArrow" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerArrow" ) ).at( i ) );
    QgsLayoutItemPolyline *layoutItem = new QgsLayoutItemPolyline( layout );
    readArrowXml( layoutItem, itemNode.toElement(), layout->project() );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Scalebar
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerScaleBar" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerScaleBar" ) ).at( i ) );
    QgsLayoutItemScaleBar *layoutItem = new QgsLayoutItemScaleBar( layout );
    readScaleBarXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Legend
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerLegend" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerLegend" ) ).at( i ) );
    QgsLayoutItemLegend *layoutItem = new QgsLayoutItemLegend( layout );
    readLegendXml( layoutItem, itemNode.toElement(), layout->project(), mapIdUiidMap );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  // Html
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerHtml" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerHtml" ) ).at( i ) );
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
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerAttributeTableV2" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerAttributeTableV2" ) ).at( i ) );
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
  for ( int i = 0; i < parentElement.elementsByTagName( QStringLiteral( "ComposerItemGroup" ) ).size(); i++ )
  {
    const QDomNode itemNode( parentElement.elementsByTagName( QStringLiteral( "ComposerItemGroup" ) ).at( i ) );
    QgsLayoutItemGroup *layoutItem = new QgsLayoutItemGroup( layout );
    readGroupXml( layoutItem, itemNode.toElement(), layout->project(), newItems );
    adjustPos( layout, layoutItem, position, pasteInPlace, zOrderOffset, pasteShiftPos, pageNumber );
    newItems << layoutItem ;
  }

  return newItems;
}

bool QgsCompositionConverter::isCompositionTemplate( const QDomDocument &document )
{
  return document.elementsByTagName( QStringLiteral( "Composition" ) ).count() > 0;
}

QDomDocument QgsCompositionConverter::convertCompositionTemplate( const QDomDocument &document, QgsProject *project )
{
  QDomDocument doc;
  QgsReadWriteContext context;
  if ( project )
    context.setPathResolver( project->pathResolver() );
  if ( document.elementsByTagName( QStringLiteral( "Composition" ) ).count( ) > 0 )
  {
    const QDomElement composerElem = document.elementsByTagName( QStringLiteral( "Composition" ) ).at( 0 ).toElement( );

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
  layoutItem->setText( itemElem.attribute( QStringLiteral( "labelText" ) ) );

  //html state
  layoutItem->setMode( itemElem.attribute( QStringLiteral( "htmlState" ) ).toInt() == Qt::Checked ? QgsLayoutItemLabel::Mode::ModeHtml : QgsLayoutItemLabel::Mode::ModeFont );

  //margin
  bool marginXOk = false;
  bool marginYOk = false;
  double marginX = itemElem.attribute( QStringLiteral( "marginX" ) ).toDouble( &marginXOk );
  double marginY = itemElem.attribute( QStringLiteral( "marginY" ) ).toDouble( &marginYOk );
  if ( !marginXOk || !marginYOk )
  {
    //upgrade old projects where margins where stored in a single attribute
    const double margin = itemElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "1.0" ) ).toDouble();
    marginX = margin;
    marginY = margin;
  }
  layoutItem->setMarginX( marginX );
  layoutItem->setMarginY( marginY );

  //Horizontal alignment
  layoutItem->setHAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "halign" ) ).toInt() ) );

  //Vertical alignment
  layoutItem->setVAlign( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "valign" ) ).toInt() ) );


  QFont font;
  //font
  QgsFontUtils::setFromXmlChildNode( font, itemElem, QStringLiteral( "LabelFont" ) );
  QgsTextFormat format;
  format.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    format.setSize( font.pointSizeF() );
    format.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    format.setSize( font.pixelSize() );
    format.setSizeUnit( QgsUnitTypes::RenderPixels );
  }

  //font color
  const QDomNodeList fontColorList = itemElem.elementsByTagName( QStringLiteral( "FontColor" ) );
  if ( !fontColorList.isEmpty() )
  {
    const QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
    const int red = fontColorElem.attribute( QStringLiteral( "red" ), QStringLiteral( "0" ) ).toInt();
    const int green = fontColorElem.attribute( QStringLiteral( "green" ), QStringLiteral( "0" ) ).toInt();
    const int blue = fontColorElem.attribute( QStringLiteral( "blue" ), QStringLiteral( "0" ) ).toInt();
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
  layoutItem->setShapeType( static_cast<QgsLayoutItemShape::Shape>( itemElem.attribute( QStringLiteral( "shapeType" ), QStringLiteral( "0" ) ).toInt() ) );
  layoutItem->setCornerRadius( QgsLayoutMeasurement( itemElem.attribute( QStringLiteral( "cornerRadius" ), QStringLiteral( "0" ) ).toDouble() ) );

  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  QgsReadWriteContext context;
  if ( project )
    context.setPathResolver( project->pathResolver() );

  if ( itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).size() )
  {
    const QDomElement symbolElement = itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).at( 0 ).toElement();
    const std::unique_ptr< QgsFillSymbol > shapeStyleSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElement, context ) );
    if ( shapeStyleSymbol )
      layoutItem->setSymbol( shapeStyleSymbol.get() );
  }
  else
  {
    //upgrade project file from 2.0 to use symbol styling
    QVariantMap properties;
    properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( layoutItem->brush().color() ) );
    if ( layoutItem->hasBackground() )
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
    }
    else
    {
      properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
    }
    if ( layoutItem->frameEnabled() )
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
    const QDomNodeList outlineColorList = itemElem.elementsByTagName( QStringLiteral( "OutlineColor" ) );
    if ( !outlineColorList.isEmpty() )
    {
      const QDomElement frameColorElem = outlineColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk, widthOk;
      int penRed, penGreen, penBlue, penAlpha;
      double penWidth;

      penWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
      penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toInt( &redOk );
      penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toInt( &greenOk );
      penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toInt( &blueOk );
      penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toInt( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk && widthOk )
      {
        properties.insert( QStringLiteral( "color_border" ), QgsSymbolLayerUtils::encodeColor( QColor( penRed, penGreen, penBlue, penAlpha ) ) );
        properties.insert( QStringLiteral( "width_border" ), QString::number( penWidth ) );
      }
    }
    const QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "FillColor" ) );
    if ( !fillColorList.isEmpty() )
    {
      const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toInt( &redOk );
      fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toInt( &greenOk );
      fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toInt( &blueOk );
      fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toInt( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        properties.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) ) );
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
      }
    }
    if ( itemElem.hasAttribute( QStringLiteral( "transparentFill" ) ) )
    {
      //old style (pre 2.0) of specifying that shapes had no fill
      const bool hasOldTransparentFill = itemElem.attribute( QStringLiteral( "transparentFill" ), QStringLiteral( "0" ) ).toInt();
      if ( hasOldTransparentFill )
      {
        properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
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

  layoutItem->mResizeMode = QgsLayoutItemPicture::ResizeMode( itemElem.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() );
  //when loading from xml, default to anchor point of middle to match pre 2.4 behavior
  bool positionModeOk = false;
  layoutItem->mReferencePoint = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( QStringLiteral( "positionMode" ) ).toInt( &positionModeOk ) );
  if ( !positionModeOk )
  {
    layoutItem->mReferencePoint = QgsLayoutItem::ReferencePoint::UpperLeft;
  }
  bool anchorPointOk = false;

  layoutItem->setPictureAnchor( static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( QStringLiteral( "anchorPoint" ), QString::number( QgsLayoutItem::ReferencePoint::Middle ) ).toInt( &anchorPointOk ) ) );
  if ( !anchorPointOk )
  {
    layoutItem->mPictureAnchor = QgsLayoutItem::ReferencePoint::UpperLeft;
  }
  layoutItem->mSvgFillColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "svgFillColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) );
  layoutItem->mSvgStrokeColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "svgBorderColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 0, 0, 0 ) ) ) );
  layoutItem->mSvgStrokeWidth = itemElem.attribute( QStringLiteral( "svgBorderWidth" ), QStringLiteral( "0.2" ) ).toDouble();

  QString imagePath = itemElem.attribute( QStringLiteral( "file" ) );
  if ( project )
  {
    // convert from relative path to absolute. For SVG we also need to consider system SVG paths
    const QgsPathResolver pathResolver = project->pathResolver();
    if ( imagePath.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
      imagePath = QgsSymbolLayerUtils::svgSymbolNameToPath( imagePath, pathResolver );
    else
      imagePath = pathResolver.readPath( imagePath );
  }
  layoutItem->setPicturePath( imagePath );
  layoutItem->mPictureHeight = itemElem.attribute( QStringLiteral( "pictureHeight" ), QStringLiteral( "10" ) ).toDouble();
  layoutItem->mPictureWidth = itemElem.attribute( QStringLiteral( "pictureWidth" ), QStringLiteral( "10" ) ).toDouble();

  //picture rotation
  if ( !qgsDoubleNear( itemElem.attribute( QStringLiteral( "pictureRotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
  {
    layoutItem->mPictureRotation = itemElem.attribute( QStringLiteral( "pictureRotation" ), QStringLiteral( "0" ) ).toDouble();
  }

  //rotation map
  layoutItem->mNorthArrowHandler->setNorthMode( static_cast< QgsLayoutNorthArrowHandler::NorthMode >( itemElem.attribute( QStringLiteral( "northMode" ), QStringLiteral( "0" ) ).toInt() ) );
  layoutItem->mNorthArrowHandler->setNorthOffset( itemElem.attribute( QStringLiteral( "northOffset" ), QStringLiteral( "0" ) ).toDouble() );

  const QString rotationMapId = itemElem.attribute( QStringLiteral( "mapId" ), QStringLiteral( "-1" ) );
  if ( rotationMapId != QLatin1String( "-1" ) )
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
  const QDomNodeList startPointList = itemElem.elementsByTagName( QStringLiteral( "StartPoint" ) );
  if ( ! startPointList.isEmpty() )
  {
    const QDomElement node = startPointList.at( 0 ).toElement();
    polygon.append( QPointF( node.attribute( QStringLiteral( "x" ) ).toDouble( ), node.attribute( QStringLiteral( "y" ) ).toDouble() ) );
  }
  const QDomNodeList stopPointList = itemElem.elementsByTagName( QStringLiteral( "StopPoint" ) );
  if ( ! stopPointList.isEmpty() )
  {
    const QDomElement node = stopPointList.at( 0 ).toElement();
    polygon.append( QPointF( node.attribute( QStringLiteral( "x" ) ).toDouble( ), node.attribute( QStringLiteral( "y" ) ).toDouble() ) );
  }

  const QgsCompositionConverter::MarkerMode markerMode = static_cast< QgsCompositionConverter::MarkerMode>( itemElem.attribute( QStringLiteral( "markerMode" ), QStringLiteral( "0" ) ).toInt( ) );

  if ( markerMode == QgsCompositionConverter::MarkerMode::DefaultMarker )
  {
    layoutItem->setEndMarker( QgsLayoutItemPolyline::MarkerMode::ArrowHead );
    layoutItem->setStartMarker( QgsLayoutItemPolyline::MarkerMode::NoMarker );
    layoutItem->setArrowHeadFillColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "arrowHeadFillColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) ) );
    layoutItem->setArrowHeadStrokeColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "arrowHeadOutlineColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) ) );
    layoutItem->setArrowHeadStrokeWidth( itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "1.0" ) ).toDouble( ) );
    layoutItem->setArrowHeadWidth( itemElem.attribute( QStringLiteral( "arrowHeadWidth" ), QStringLiteral( "1.0" ) ).toDouble( ) );
  }
  else if ( markerMode == QgsCompositionConverter::MarkerMode::SVGMarker )
  {
    QString endMarkerFile = itemElem.attribute( QStringLiteral( "endMarkerFile" ) );
    QString startMarkerFile = itemElem.attribute( QStringLiteral( "endMarkerFile" ) );

    // Fix the paths
    if ( project )
    {
      // convert from relative path to absolute. For SVG we also need to consider system SVG paths
      const QgsPathResolver pathResolver = project->pathResolver();
      if ( !endMarkerFile.isEmpty() )
      {
        if ( endMarkerFile.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
          endMarkerFile = QgsSymbolLayerUtils::svgSymbolNameToPath( endMarkerFile, pathResolver );
        else
          endMarkerFile = pathResolver.readPath( endMarkerFile );
      }
      if ( !startMarkerFile.isEmpty() )
      {
        if ( startMarkerFile.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
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

  mapId2Uuid[ itemElem.attribute( QStringLiteral( "id" ) ) ] = layoutItem->uuid();

  // TODO: Unused but all the layouts readXML require it (I'd suggest to remove it from the API)
  const QDomDocument doc;

  QgsReadWriteContext context;

  if ( project )
    context.setPathResolver( project->pathResolver() );

  //extent
  const QDomNodeList extentNodeList = itemElem.elementsByTagName( QStringLiteral( "Extent" ) );
  if ( !extentNodeList.isEmpty() )
  {
    const QDomElement extentElem = extentNodeList.at( 0 ).toElement();
    double xmin, xmax, ymin, ymax;
    xmin = extentElem.attribute( QStringLiteral( "xmin" ) ).toDouble();
    xmax = extentElem.attribute( QStringLiteral( "xmax" ) ).toDouble();
    ymin = extentElem.attribute( QStringLiteral( "ymin" ) ).toDouble();
    ymax = extentElem.attribute( QStringLiteral( "ymax" ) ).toDouble();
    layoutItem->setExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );
  }

  const QDomNodeList crsNodeList = itemElem.elementsByTagName( QStringLiteral( "crs" ) );
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
  if ( !qgsDoubleNear( itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
  {
    layoutItem->setMapRotation( itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble() );
  }

  // follow map theme
  layoutItem->setFollowVisibilityPreset( itemElem.attribute( QStringLiteral( "followPreset" ) ).compare( QLatin1String( "true" ) ) == 0 );
  layoutItem->setFollowVisibilityPresetName( itemElem.attribute( QStringLiteral( "followPresetName" ) ) );

  //mKeepLayerSet flag
  const QString keepLayerSetFlag = itemElem.attribute( QStringLiteral( "keepLayerSet" ) );
  if ( keepLayerSetFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    layoutItem->setKeepLayerSet( true );
  }
  else
  {
    layoutItem->setKeepLayerSet( false );
  }

  const QString drawCanvasItemsFlag = itemElem.attribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  if ( drawCanvasItemsFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
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

  const QDomNodeList layerSetNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerSet" ) );
  if ( !layerSetNodeList.isEmpty() )
  {
    const QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    const QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( QStringLiteral( "Layer" ) );
    layoutItem->mLayers.reserve( layerIdNodeList.size() );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      const QDomElement layerElem = layerIdNodeList.at( i ).toElement();
      const QString layerId = layerElem.text();
      const QString layerName = layerElem.attribute( QStringLiteral( "name" ) );
      const QString layerSource = layerElem.attribute( QStringLiteral( "source" ) );
      const QString layerProvider = layerElem.attribute( QStringLiteral( "provider" ) );

      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( project );
      layoutItem->mLayers << ref;
    }
  }

  // override styles
  const QDomNodeList layerStylesNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerStyles" ) );
  layoutItem->mKeepLayerStyles = !layerStylesNodeList.isEmpty();
  if ( layoutItem->mKeepLayerStyles )
  {
    const QDomElement layerStylesElem = layerStylesNodeList.at( 0 ).toElement();
    const QDomNodeList layerStyleNodeList = layerStylesElem.elementsByTagName( QStringLiteral( "LayerStyle" ) );
    for ( int i = 0; i < layerStyleNodeList.size(); ++i )
    {
      const QDomElement &layerStyleElement = layerStyleNodeList.at( i ).toElement();
      const QString layerId = layerStyleElement.attribute( QStringLiteral( "layerid" ) );
      const QString layerName = layerStyleElement.attribute( QStringLiteral( "name" ) );
      const QString layerSource = layerStyleElement.attribute( QStringLiteral( "source" ) );
      const QString layerProvider = layerStyleElement.attribute( QStringLiteral( "provider" ) );
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
  const QDomNodeList mapOverviewNodeList = itemElem.elementsByTagName( QStringLiteral( "ComposerMapOverview" ) );
  for ( int i = 0; i < mapOverviewNodeList.size(); ++i )
  {
    const QDomElement mapOverviewElem = mapOverviewNodeList.at( i ).toElement();
    std::unique_ptr<QgsLayoutItemMapOverview> mapOverview( new QgsLayoutItemMapOverview( mapOverviewElem.attribute( QStringLiteral( "name" ) ), layoutItem ) );
    mapOverview->readXml( mapOverviewElem, doc, context );
    const QString frameMapId = mapOverviewElem.attribute( QStringLiteral( "frameMap" ), QStringLiteral( "-1" ) );
    if ( frameMapId != QLatin1String( "-1" ) && mapId2Uuid.contains( frameMapId ) )
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
  const QDomNodeList gridNodeList = itemElem.elementsByTagName( QStringLiteral( "Grid" ) );
  if ( layoutItem->mGridStack->size() == 0 && !gridNodeList.isEmpty() )
  {
    const QDomElement gridElem = gridNodeList.at( 0 ).toElement();
    QgsLayoutItemMapGrid *mapGrid = new QgsLayoutItemMapGrid( QObject::tr( "Grid %1" ).arg( 1 ), layoutItem );
    mapGrid->setEnabled( gridElem.attribute( QStringLiteral( "show" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
    mapGrid->setStyle( QgsLayoutItemMapGrid::GridStyle( gridElem.attribute( QStringLiteral( "gridStyle" ), QStringLiteral( "0" ) ).toInt() ) );
    mapGrid->setIntervalX( gridElem.attribute( QStringLiteral( "intervalX" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setIntervalY( gridElem.attribute( QStringLiteral( "intervalY" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setOffsetX( gridElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setOffsetY( gridElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );
    mapGrid->setCrossLength( gridElem.attribute( QStringLiteral( "crossLength" ), QStringLiteral( "3" ) ).toDouble() );
    mapGrid->setFrameStyle( static_cast< QgsLayoutItemMapGrid::FrameStyle >( gridElem.attribute( QStringLiteral( "gridFrameStyle" ), QStringLiteral( "0" ) ).toInt() ) );
    mapGrid->setFrameWidth( gridElem.attribute( QStringLiteral( "gridFrameWidth" ), QStringLiteral( "2.0" ) ).toDouble() );
    mapGrid->setFramePenSize( gridElem.attribute( QStringLiteral( "gridFramePenThickness" ), QStringLiteral( "0.5" ) ).toDouble() );
    mapGrid->setFramePenColor( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "framePenColor" ), QStringLiteral( "0,0,0" ) ) ) );
    mapGrid->setFrameFillColor1( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "frameFillColor1" ), QStringLiteral( "255,255,255,255" ) ) ) );
    mapGrid->setFrameFillColor2( QgsSymbolLayerUtils::decodeColor( gridElem.attribute( QStringLiteral( "frameFillColor2" ), QStringLiteral( "0,0,0,255" ) ) ) );
    mapGrid->setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( itemElem.attribute( QStringLiteral( "gridBlendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );
    const QDomElement gridSymbolElem = gridElem.firstChildElement( QStringLiteral( "symbol" ) );
    QgsLineSymbol *lineSymbol = nullptr;
    if ( gridSymbolElem.isNull() )
    {
      //old project file, read penWidth /penColorRed, penColorGreen, penColorBlue
      lineSymbol = QgsLineSymbol::createSimple( QVariantMap() );
      lineSymbol->setWidth( gridElem.attribute( QStringLiteral( "penWidth" ), QStringLiteral( "0" ) ).toDouble() );
      lineSymbol->setColor( QColor( gridElem.attribute( QStringLiteral( "penColorRed" ), QStringLiteral( "0" ) ).toInt(),
                                    gridElem.attribute( QStringLiteral( "penColorGreen" ), QStringLiteral( "0" ) ).toInt(),
                                    gridElem.attribute( QStringLiteral( "penColorBlue" ), QStringLiteral( "0" ) ).toInt() ) );
    }
    else
    {
      lineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( gridSymbolElem, context );
    }
    mapGrid->setLineSymbol( lineSymbol );

    //annotation
    const QDomNodeList annotationNodeList = gridElem.elementsByTagName( QStringLiteral( "Annotation" ) );
    if ( !annotationNodeList.isEmpty() )
    {
      const QDomElement annotationElem = annotationNodeList.at( 0 ).toElement();
      mapGrid->setAnnotationEnabled( annotationElem.attribute( QStringLiteral( "show" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
      mapGrid->setAnnotationFormat( QgsLayoutItemMapGrid::AnnotationFormat( annotationElem.attribute( QStringLiteral( "format" ), QStringLiteral( "0" ) ).toInt() ) );
      mapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "leftPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Left );
      mapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "rightPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Right );
      mapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "topPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Top );
      mapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::AnnotationPosition( annotationElem.attribute( QStringLiteral( "bottomPosition" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Bottom );
      mapGrid->setAnnotationDirection( QgsLayoutItemMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "leftDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Left );
      mapGrid->setAnnotationDirection( QgsLayoutItemMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "rightDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Right );
      mapGrid->setAnnotationDirection( QgsLayoutItemMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "topDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Top );
      mapGrid->setAnnotationDirection( QgsLayoutItemMapGrid::AnnotationDirection( annotationElem.attribute( QStringLiteral( "bottomDirection" ), QStringLiteral( "0" ) ).toInt() ), QgsLayoutItemMapGrid::Bottom );
      mapGrid->setAnnotationFrameDistance( annotationElem.attribute( QStringLiteral( "frameDistance" ), QStringLiteral( "0" ) ).toDouble() );
      QFont annotationFont;
      annotationFont.fromString( annotationElem.attribute( QStringLiteral( "font" ), QString() ) );

      QgsTextFormat annotationFormat = mapGrid->annotationTextFormat();
      annotationFormat.setFont( annotationFont );
      if ( annotationFont.pointSizeF() > 0 )
      {
        annotationFormat.setSize( annotationFont.pointSizeF() );
        annotationFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
      }
      else if ( annotationFont.pixelSize() > 0 )
      {
        annotationFormat.setSize( annotationFont.pixelSize() );
        annotationFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
      }
      annotationFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "0,0,0,255" ) ) ) );
      mapGrid->setAnnotationTextFormat( annotationFormat );

      mapGrid->setAnnotationPrecision( annotationElem.attribute( QStringLiteral( "precision" ), QStringLiteral( "3" ) ).toInt() );
    }
    layoutItem->mGridStack->addGrid( mapGrid );
  }

  //atlas
  const QDomNodeList atlasNodeList = itemElem.elementsByTagName( QStringLiteral( "AtlasMap" ) );
  if ( !atlasNodeList.isEmpty() )
  {
    const QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    layoutItem->mAtlasDriven = ( atlasElem.attribute( QStringLiteral( "atlasDriven" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
    if ( atlasElem.hasAttribute( QStringLiteral( "fixedScale" ) ) ) // deprecated XML
    {
      layoutItem->setAtlasScalingMode( atlasElem.attribute( QStringLiteral( "fixedScale" ), QStringLiteral( "0" ) ) != QLatin1String( "0" )  ? QgsLayoutItemMap::AtlasScalingMode::Fixed : QgsLayoutItemMap::AtlasScalingMode::Auto );
    }
    else if ( atlasElem.hasAttribute( QStringLiteral( "scalingMode" ) ) )
    {
      layoutItem->setAtlasScalingMode( static_cast<QgsLayoutItemMap::AtlasScalingMode>( atlasElem.attribute( QStringLiteral( "scalingMode" ) ).toInt() ) );
    }
    layoutItem->setAtlasMargin( atlasElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "0.1" ) ).toDouble() );
  }

  layoutItem->updateBoundingRect();

  return true;
}

bool QgsCompositionConverter::readScaleBarXml( QgsLayoutItemScaleBar *layoutItem, const QDomElement &itemElem, const QgsProject *project, const QgsStringMap &mapId2Uuid )
{
  Q_UNUSED( project )
  restoreGeneralComposeItemProperties( layoutItem, itemElem );

  layoutItem->setHeight( itemElem.attribute( QStringLiteral( "height" ), QStringLiteral( "5.0" ) ).toDouble() );
  layoutItem->setHeight( itemElem.attribute( QStringLiteral( "height" ), QStringLiteral( "5.0" ) ).toDouble() );
  layoutItem->setLabelBarSpace( itemElem.attribute( QStringLiteral( "labelBarSpace" ), QStringLiteral( "3.0" ) ).toDouble() );
  layoutItem->setBoxContentSpace( itemElem.attribute( QStringLiteral( "boxContentSpace" ), QStringLiteral( "1.0" ) ).toDouble() );
  layoutItem->setNumberOfSegments( itemElem.attribute( QStringLiteral( "numSegments" ), QStringLiteral( "2" ) ).toInt() );
  layoutItem->setNumberOfSegmentsLeft( itemElem.attribute( QStringLiteral( "numSegmentsLeft" ), QStringLiteral( "0" ) ).toInt() );
  layoutItem->setUnitsPerSegment( itemElem.attribute( QStringLiteral( "numUnitsPerSegment" ), QStringLiteral( "1.0" ) ).toDouble() );
  layoutItem->setSegmentSizeMode( static_cast<QgsScaleBarSettings::SegmentSizeMode>( itemElem.attribute( QStringLiteral( "segmentSizeMode" ), QStringLiteral( "0" ) ).toInt() ) );
  layoutItem->setMinimumBarWidth( itemElem.attribute( QStringLiteral( "minBarWidth" ), QStringLiteral( "50" ) ).toDouble() );
  layoutItem->setMaximumBarWidth( itemElem.attribute( QStringLiteral( "maxBarWidth" ), QStringLiteral( "150" ) ).toDouble() );
  layoutItem->mSegmentMillimeters = itemElem.attribute( QStringLiteral( "segmentMillimeters" ), QStringLiteral( "0.0" ) ).toDouble();
  layoutItem->setMapUnitsPerScaleBarUnit( itemElem.attribute( QStringLiteral( "numMapUnitsPerScaleBarUnit" ), QStringLiteral( "1.0" ) ).toDouble() );
  layoutItem->setUnitLabel( itemElem.attribute( QStringLiteral( "unitLabel" ) ) );

  QFont f;
  if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, QStringLiteral( "scaleBarFont" ) ) )
  {
    f.fromString( itemElem.attribute( QStringLiteral( "font" ), QString() ) );
  }
  Q_NOWARN_DEPRECATED_PUSH
  layoutItem->setFont( f );
  Q_NOWARN_DEPRECATED_POP

  //colors
  //fill color
  const QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "fillColor" ) );
  if ( !fillColorList.isEmpty() )
  {
    const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->fillSymbol()->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    layoutItem->fillSymbol()->setColor( QColor( itemElem.attribute( QStringLiteral( "brushColor" ), QStringLiteral( "#000000" ) ) ) );
  }

  //fill color 2
  const QDomNodeList fillColor2List = itemElem.elementsByTagName( QStringLiteral( "fillColor2" ) );
  if ( !fillColor2List.isEmpty() )
  {
    const QDomElement fillColor2Elem = fillColor2List.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColor2Elem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    fillGreen = fillColor2Elem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    fillBlue = fillColor2Elem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    fillAlpha = fillColor2Elem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      layoutItem->alternateFillSymbol()->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    layoutItem->alternateFillSymbol()->setColor( QColor( itemElem.attribute( QStringLiteral( "brush2Color" ), QStringLiteral( "#ffffff" ) ) ) );
  }

  std::unique_ptr< QgsLineSymbol > lineSymbol = std::make_unique< QgsLineSymbol >();
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
  lineSymbolLayer->setWidth( itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "0.3" ) ).toDouble() );
  lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  lineSymbolLayer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( QStringLiteral( "lineJoinStyle" ), QStringLiteral( "miter" ) ) ) );
  lineSymbolLayer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( itemElem.attribute( QStringLiteral( "lineCapStyle" ), QStringLiteral( "square" ) ) ) );
  //stroke color
  const QDomNodeList strokeColorList = itemElem.elementsByTagName( QStringLiteral( "strokeColor" ) );
  if ( !strokeColorList.isEmpty() )
  {
    const QDomElement strokeColorElem = strokeColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int strokeRed, strokeGreen, strokeBlue, strokeAlpha;

    strokeRed = strokeColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    strokeGreen = strokeColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    strokeBlue = strokeColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    strokeAlpha = strokeColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      lineSymbolLayer->setColor( QColor( strokeRed, strokeGreen, strokeBlue, strokeAlpha ) );
    }
  }
  else
  {
    lineSymbolLayer->setColor( QColor( itemElem.attribute( QStringLiteral( "penColor" ), QStringLiteral( "#000000" ) ) ) );
  }
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
  layoutItem->setDivisionLineSymbol( lineSymbol->clone() );
  layoutItem->setSubdivisionLineSymbol( lineSymbol->clone() );
  layoutItem->setLineSymbol( lineSymbol.release() );

  //font color
  const QDomNodeList textColorList = itemElem.elementsByTagName( QStringLiteral( "textColor" ) );
  if ( !textColorList.isEmpty() )
  {
    const QDomElement textColorElem = textColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int textRed, textGreen, textBlue, textAlpha;

    textRed = textColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    textGreen = textColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    textBlue = textColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    textAlpha = textColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

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
    c.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
    Q_NOWARN_DEPRECATED_PUSH
    layoutItem->setFontColor( c );
    Q_NOWARN_DEPRECATED_POP
  }

  //style
  const QString styleString = itemElem.attribute( QStringLiteral( "style" ), QString() );
  layoutItem->setStyle( QObject::tr( styleString.toLocal8Bit().data() ) );

  if ( itemElem.attribute( QStringLiteral( "unitType" ) ).isEmpty() )
  {
    QgsUnitTypes::DistanceUnit u = QgsUnitTypes::DistanceUnknownUnit;
    switch ( itemElem.attribute( QStringLiteral( "units" ) ).toInt() )
    {
      case 0:
        u = QgsUnitTypes::DistanceUnknownUnit;
        break;
      case 1:
        u = QgsUnitTypes::DistanceMeters;
        break;
      case 2:
        u = QgsUnitTypes::DistanceFeet;
        break;
      case 3:
        u = QgsUnitTypes::DistanceNauticalMiles;
        break;
    }
    layoutItem->setUnits( u );
  }
  else
  {
    layoutItem->setUnits( QgsUnitTypes::decodeDistanceUnit( itemElem.attribute( QStringLiteral( "unitType" ) ) ) );
  }
  layoutItem->setAlignment( static_cast< QgsScaleBarSettings::Alignment >( itemElem.attribute( QStringLiteral( "alignment" ), QStringLiteral( "0" ) ).toInt() ) );

  //composer map: use uuid
  const QString mapId = itemElem.attribute( QStringLiteral( "mapId" ), QStringLiteral( "-1" ) );
  if ( mapId != QLatin1String( "-1" ) && mapId2Uuid.contains( mapId ) )
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
  const QString mapId = itemElem.attribute( QStringLiteral( "map" ), QStringLiteral( "-1" ) );
  if ( mapId != QLatin1String( "-1" ) && mapId2Uuid.contains( mapId ) )
  {
    QgsLayoutItemMap *mapInstance = qobject_cast<QgsLayoutItemMap *>( layoutItem->layout()->itemByUuid( mapId2Uuid[ mapId ] ) );
    if ( mapInstance )
    {
      layoutItem->setLinkedMap( mapInstance );
    }
  }

  //read general properties
  layoutItem->setTitle( itemElem.attribute( QStringLiteral( "title" ) ) );
  if ( !itemElem.attribute( QStringLiteral( "titleAlignment" ) ).isEmpty() )
  {
    layoutItem->setTitleAlignment( static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "titleAlignment" ) ).toInt() ) );
  }
  int colCount = itemElem.attribute( QStringLiteral( "columnCount" ), QStringLiteral( "1" ) ).toInt();
  if ( colCount < 1 ) colCount = 1;
  layoutItem->setColumnCount( colCount );
  layoutItem->setSplitLayer( itemElem.attribute( QStringLiteral( "splitLayer" ), QStringLiteral( "0" ) ).toInt() == 1 );
  layoutItem->setEqualColumnWidth( itemElem.attribute( QStringLiteral( "equalColumnWidth" ), QStringLiteral( "0" ) ).toInt() == 1 );

  const QDomNodeList stylesNodeList = itemElem.elementsByTagName( QStringLiteral( "styles" ) );
  if ( !stylesNodeList.isEmpty() )
  {
    const QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      const QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsLegendStyle style;
      style.readXml( styleElem, QDomDocument() );
      const QString name = styleElem.attribute( QStringLiteral( "name" ) );
      QgsLegendStyle::Style s;
      if ( name == QLatin1String( "title" ) ) s = QgsLegendStyle::Title;
      else if ( name == QLatin1String( "group" ) ) s = QgsLegendStyle::Group;
      else if ( name == QLatin1String( "subgroup" ) ) s = QgsLegendStyle::Subgroup;
      else if ( name == QLatin1String( "symbol" ) ) s = QgsLegendStyle::Symbol;
      else if ( name == QLatin1String( "symbolLabel" ) ) s = QgsLegendStyle::SymbolLabel;
      else continue;
      layoutItem->setStyle( s, style );
    }
  }

  //font color
  QColor fontClr;
  fontClr.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
  layoutItem->setFontColor( fontClr );

  //spaces
  layoutItem->setBoxSpace( itemElem.attribute( QStringLiteral( "boxSpace" ), QStringLiteral( "2.0" ) ).toDouble() );
  layoutItem->setColumnSpace( itemElem.attribute( QStringLiteral( "columnSpace" ), QStringLiteral( "2.0" ) ).toDouble() );

  layoutItem->setSymbolWidth( itemElem.attribute( QStringLiteral( "symbolWidth" ), QStringLiteral( "7.0" ) ).toDouble() );
  layoutItem->setSymbolHeight( itemElem.attribute( QStringLiteral( "symbolHeight" ), QStringLiteral( "14.0" ) ).toDouble() );
  layoutItem->setWmsLegendWidth( itemElem.attribute( QStringLiteral( "wmsLegendWidth" ), QStringLiteral( "50" ) ).toDouble() );
  layoutItem->setWmsLegendHeight( itemElem.attribute( QStringLiteral( "wmsLegendHeight" ), QStringLiteral( "25" ) ).toDouble() );
  layoutItem->setLineSpacing( itemElem.attribute( QStringLiteral( "lineSpacing" ), QStringLiteral( "1.0" ) ).toDouble() );

  layoutItem->setDrawRasterStroke( itemElem.attribute( QStringLiteral( "rasterBorder" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  layoutItem->setRasterStrokeColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "rasterBorderColor" ), QStringLiteral( "0,0,0" ) ) ) );
  layoutItem->setRasterStrokeWidth( itemElem.attribute( QStringLiteral( "rasterBorderWidth" ), QStringLiteral( "0" ) ).toDouble() );

  layoutItem->setWrapString( itemElem.attribute( QStringLiteral( "wrapChar" ) ) );

  layoutItem->mSizeToContents = itemElem.attribute( QStringLiteral( "resizeToContents" ), QStringLiteral( "1" ) ) != QLatin1String( "0" );
  layoutItem->mLegendFilterByMap = itemElem.attribute( QStringLiteral( "legendFilterByMap" ), QStringLiteral( "0" ) ).toInt();
  layoutItem->mFilterOutAtlas = itemElem.attribute( QStringLiteral( "legendFilterByAtlas" ), QStringLiteral( "0" ) ).toInt();

  // QGIS >= 2.6
  QDomElement layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree" ) );
  if ( layerTreeElem.isNull() )
    layerTreeElem = itemElem.firstChildElement( QStringLiteral( "layer-tree-group" ) );

  if ( !layerTreeElem.isNull() )
  {
    QgsLayerTree *tree( QgsLayerTree::readXml( layerTreeElem, context ) );
    if ( project )
      tree->resolveReferences( project, true );
    layoutItem->setCustomLayerTree( tree );
  }
  else
  {
    layoutItem->setCustomLayerTree( nullptr );
  }

  return true;
}

bool QgsCompositionConverter::readAtlasXml( QgsLayoutAtlas *atlasItem, const QDomElement &itemElem, const QgsProject *project )
{
  atlasItem->setEnabled( itemElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "false" ) ) == QLatin1String( "true" ) );

  // look for stored layer name
  const QString layerId = itemElem.attribute( QStringLiteral( "coverageLayer" ) );
  const QString layerName = itemElem.attribute( QStringLiteral( "coverageLayerName" ) );
  const QString layerSource = itemElem.attribute( QStringLiteral( "coverageLayerSource" ) );
  const QString layerProvider = itemElem.attribute( QStringLiteral( "coverageLayerProvider" ) );

  QgsVectorLayerRef layerRef( layerId, layerName, layerSource, layerProvider );
  atlasItem->setCoverageLayer( layerRef.resolveWeakly( project ) );

  atlasItem->setPageNameExpression( itemElem.attribute( QStringLiteral( "pageNameExpression" ), QString() ) );
  QString errorString;
  atlasItem->setFilenameExpression( itemElem.attribute( QStringLiteral( "filenamePattern" ), QString() ), errorString );
  // note: no error reporting for errorString
  atlasItem->setSortFeatures( itemElem.attribute( QStringLiteral( "sortFeatures" ), QStringLiteral( "false" ) ) == QLatin1String( "true" ) );
  if ( atlasItem->sortFeatures() )
  {
    atlasItem->setSortExpression( itemElem.attribute( QStringLiteral( "sortKey" ), QString() ) );
    atlasItem->setSortAscending( itemElem.attribute( QStringLiteral( "sortAscending" ), QStringLiteral( "true" ) ) == QLatin1String( "true" ) );
  }
  atlasItem->setFilterFeatures( itemElem.attribute( QStringLiteral( "filterFeatures" ), QStringLiteral( "false" ) ) == QLatin1String( "true" ) );
  if ( atlasItem->filterFeatures( ) )
  {
    QString expErrorString;
    atlasItem->setFilterExpression( itemElem.attribute( QStringLiteral( "featureFilter" ), QString() ), expErrorString );
    // note: no error reporting for errorString
  }

  atlasItem->setHideCoverage( itemElem.attribute( QStringLiteral( "hideCoverage" ), QStringLiteral( "false" ) ) == QLatin1String( "true" ) );

  return true;

}

bool QgsCompositionConverter::readHtmlXml( QgsLayoutItemHtml *layoutItem, const QDomElement &itemElem, const QgsProject *project )
{
  Q_UNUSED( project )
  readOldComposerObjectXml( layoutItem, itemElem );

  //first create the frames
  layoutItem->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( itemElem.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() ) );
  const QDomNodeList frameList = itemElem.elementsByTagName( QStringLiteral( "ComposerFrame" ) );
  for ( int i = 0; i < frameList.size(); ++i )
  {
    const QDomElement frameElem = frameList.at( i ).toElement();
    QgsLayoutFrame *newFrame = new QgsLayoutFrame( layoutItem->layout(), layoutItem );
    restoreGeneralComposeItemProperties( newFrame, frameElem );
    // Read frame XML
    const double x = itemElem.attribute( QStringLiteral( "sectionX" ) ).toDouble();
    const double y = itemElem.attribute( QStringLiteral( "sectionY" ) ).toDouble();
    const double width = itemElem.attribute( QStringLiteral( "sectionWidth" ) ).toDouble();
    const double height = itemElem.attribute( QStringLiteral( "sectionHeight" ) ).toDouble();
    newFrame->setContentSection( QRectF( x, y, width, height ) );
    newFrame->setHidePageIfEmpty( itemElem.attribute( QStringLiteral( "hidePageIfEmpty" ), QStringLiteral( "0" ) ).toInt() );
    newFrame->setHideBackgroundIfEmpty( itemElem.attribute( QStringLiteral( "hideBackgroundIfEmpty" ), QStringLiteral( "0" ) ).toInt() );
    layoutItem->addFrame( newFrame, false );
  }

  bool contentModeOK;
  layoutItem->setContentMode( static_cast< QgsLayoutItemHtml::ContentMode >( itemElem.attribute( QStringLiteral( "contentMode" ) ).toInt( &contentModeOK ) ) );
  if ( !contentModeOK )
  {
    layoutItem->setContentMode( QgsLayoutItemHtml::ContentMode::Url );
  }
  layoutItem->setEvaluateExpressions( itemElem.attribute( QStringLiteral( "evaluateExpressions" ), QStringLiteral( "true" ) ) == QLatin1String( "true" ) );
  layoutItem->setUseSmartBreaks( itemElem.attribute( QStringLiteral( "useSmartBreaks" ), QStringLiteral( "true" ) ) == QLatin1String( "true" ) );
  layoutItem->setMaxBreakDistance( itemElem.attribute( QStringLiteral( "maxBreakDistance" ), QStringLiteral( "10" ) ).toDouble() );
  layoutItem->setHtml( itemElem.attribute( QStringLiteral( "html" ) ) );
  layoutItem->setUserStylesheet( itemElem.attribute( QStringLiteral( "stylesheet" ) ) );
  layoutItem->setUserStylesheetEnabled( itemElem.attribute( QStringLiteral( "stylesheetEnabled" ), QStringLiteral( "false" ) ) == QLatin1String( "true" ) );

  //finally load the set url
  const QString urlString = itemElem.attribute( QStringLiteral( "url" ) );
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
  layoutItem->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( itemElem.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() ) );
  const QDomNodeList frameList = itemElem.elementsByTagName( QStringLiteral( "ComposerFrame" ) );
  for ( int i = 0; i < frameList.size(); ++i )
  {
    const QDomElement frameElem = frameList.at( i ).toElement();
    QgsLayoutFrame *newFrame = new QgsLayoutFrame( layoutItem->layout(), layoutItem );
    restoreGeneralComposeItemProperties( newFrame, frameElem );
    // Read frame XML
    const double x = itemElem.attribute( QStringLiteral( "sectionX" ) ).toDouble();
    const double y = itemElem.attribute( QStringLiteral( "sectionY" ) ).toDouble();
    const double width = itemElem.attribute( QStringLiteral( "sectionWidth" ) ).toDouble();
    const double height = itemElem.attribute( QStringLiteral( "sectionHeight" ) ).toDouble();
    newFrame->setContentSection( QRectF( x, y, width, height ) );
    newFrame->setHidePageIfEmpty( itemElem.attribute( QStringLiteral( "hidePageIfEmpty" ), QStringLiteral( "0" ) ).toInt() );
    newFrame->setHideBackgroundIfEmpty( itemElem.attribute( QStringLiteral( "hideBackgroundIfEmpty" ), QStringLiteral( "0" ) ).toInt() );
    layoutItem->addFrame( newFrame, false );
  }

  layoutItem->setEmptyTableBehavior( static_cast<QgsLayoutTable::EmptyTableMode>( itemElem.attribute( QStringLiteral( "emptyTableMode" ), QStringLiteral( "0" ) ).toInt() ) );
  layoutItem->setEmptyTableMessage( itemElem.attribute( QStringLiteral( "emptyTableMessage" ), QObject::tr( "No matching records" ) ) );
  layoutItem->setShowEmptyRows( itemElem.attribute( QStringLiteral( "showEmptyRows" ), QStringLiteral( "0" ) ).toInt() );
  QFont headerFont;
  if ( !QgsFontUtils::setFromXmlChildNode( headerFont, itemElem, QStringLiteral( "headerFontProperties" ) ) )
  {
    headerFont.fromString( itemElem.attribute( QStringLiteral( "headerFont" ), QString() ) );
  }
  QgsTextFormat headerFormat = layoutItem->headerTextFormat();
  headerFormat.setFont( headerFont );
  if ( headerFont.pointSizeF() > 0 )
  {
    headerFormat.setSize( headerFont.pointSizeF() );
    headerFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( headerFont.pixelSize() > 0 )
  {
    headerFormat.setSize( headerFont.pixelSize() );
    headerFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }
  headerFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "headerFontColor" ), QStringLiteral( "0,0,0,255" ) ) ) );
  layoutItem->setHeaderTextFormat( headerFormat );
  layoutItem->setHeaderHAlignment( static_cast<QgsLayoutTable::HeaderHAlignment>( itemElem.attribute( QStringLiteral( "headerHAlignment" ), QStringLiteral( "0" ) ).toInt() ) ) ;
  layoutItem->setHeaderMode( static_cast<QgsLayoutTable::HeaderMode>( itemElem.attribute( QStringLiteral( "headerMode" ), QStringLiteral( "0" ) ).toInt() ) );

  QFont contentFont;
  if ( !QgsFontUtils::setFromXmlChildNode( contentFont, itemElem, QStringLiteral( "contentFontProperties" ) ) )
  {
    contentFont.fromString( itemElem.attribute( QStringLiteral( "contentFont" ), QString() ) );
  }
  QgsTextFormat contentFormat = layoutItem->contentTextFormat();
  contentFormat.setFont( contentFont );
  if ( contentFont.pointSizeF() > 0 )
  {
    contentFormat.setSize( contentFont.pointSizeF() );
    contentFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( contentFont.pixelSize() > 0 )
  {
    contentFormat.setSize( contentFont.pixelSize() );
    contentFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }
  contentFormat.setColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "contentFontColor" ), QStringLiteral( "0,0,0,255" ) ) ) );
  layoutItem->setContentTextFormat( contentFormat );

  layoutItem->setCellMargin( itemElem.attribute( QStringLiteral( "cellMargin" ), QStringLiteral( "1.0" ) ).toDouble() );
  layoutItem->setGridStrokeWidth( itemElem.attribute( QStringLiteral( "gridStrokeWidth" ), QStringLiteral( "0.5" ) ).toDouble() );
  layoutItem->setHorizontalGrid( itemElem.attribute( QStringLiteral( "horizontalGrid" ), QStringLiteral( "1" ) ).toInt() );
  layoutItem->setVerticalGrid( itemElem.attribute( QStringLiteral( "verticalGrid" ), QStringLiteral( "1" ) ).toInt() );
  layoutItem->setShowGrid( itemElem.attribute( QStringLiteral( "showGrid" ), QStringLiteral( "1" ) ).toInt() );
  layoutItem->setGridColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "gridColor" ), QStringLiteral( "0,0,0,255" ) ) ) );
  layoutItem->setBackgroundColor( QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "backgroundColor" ), QStringLiteral( "255,255,255,0" ) ) ) );
  layoutItem->setWrapBehavior( static_cast<QgsLayoutTable::WrapBehavior>( itemElem.attribute( QStringLiteral( "wrapBehavior" ), QStringLiteral( "0" ) ).toInt() ) );

  //restore column specifications
  layoutItem->mColumns.clear();
  layoutItem->mSortColumns.clear();

  const QDomNodeList columnsList = itemElem.elementsByTagName( QStringLiteral( "displayColumns" ) );
  if ( !columnsList.isEmpty() )
  {
    const QDomElement columnsElem = columnsList.at( 0 ).toElement();
    const QDomNodeList columnEntryList = columnsElem.elementsByTagName( QStringLiteral( "column" ) );
    for ( int i = 0; i < columnEntryList.size(); ++i )
    {
      const QDomElement columnElem = columnEntryList.at( i ).toElement();
      QgsLayoutTableColumn column;
      column.mHAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( QStringLiteral( "hAlignment" ), QString::number( Qt::AlignLeft ) ).toInt() );
      column.mVAlignment = static_cast< Qt::AlignmentFlag >( columnElem.attribute( QStringLiteral( "vAlignment" ), QString::number( Qt::AlignVCenter ) ).toInt() );
      column.mHeading = columnElem.attribute( QStringLiteral( "heading" ), QString() );
      column.mAttribute = columnElem.attribute( QStringLiteral( "attribute" ), QString() );
      column.mSortByRank = columnElem.attribute( QStringLiteral( "sortByRank" ), QStringLiteral( "0" ) ).toInt();
      column.mSortOrder = static_cast< Qt::SortOrder >( columnElem.attribute( QStringLiteral( "sortOrder" ), QString::number( Qt::AscendingOrder ) ).toInt() );
      column.mWidth = columnElem.attribute( QStringLiteral( "width" ), QStringLiteral( "0.0" ) ).toDouble();

      const QDomNodeList bgColorList = columnElem.elementsByTagName( QStringLiteral( "backgroundColor" ) );
      if ( !bgColorList.isEmpty() )
      {
        const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
        bool redOk, greenOk, blueOk, alphaOk;
        int bgRed, bgGreen, bgBlue, bgAlpha;
        bgRed = bgColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
        bgGreen = bgColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
        bgBlue = bgColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
        bgAlpha = bgColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
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
  const QDomNodeList stylesList = itemElem.elementsByTagName( QStringLiteral( "cellStyles" ) );
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
  const QString layerId = itemElem.attribute( QStringLiteral( "vectorLayer" ) );
  const QString layerName = itemElem.attribute( QStringLiteral( "vectorLayerName" ) );
  const QString layerSource = itemElem.attribute( QStringLiteral( "vectorLayerSource" ) );
  const QString layerProvider = itemElem.attribute( QStringLiteral( "vectorLayerProvider" ) );

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
  const QDomNodeList nodeList = itemElem.elementsByTagName( QStringLiteral( "node" ) );
  if ( !nodeList.isEmpty() )
  {
    QPolygonF polygon;
    for ( int i = 0; i < nodeList.length(); i++ )
    {
      const QDomElement node = nodeList.at( i ).toElement();
      polygon.append( QPointF( node.attribute( QStringLiteral( "x" ) ).toDouble( ), node.attribute( QStringLiteral( "y" ) ).toDouble() ) );
    }
    layoutItem->setNodes( polygon );
  }
  if ( itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).size() )
  {
    const QDomElement symbolElement = itemElem.elementsByTagName( QStringLiteral( "symbol" ) ).at( 0 ).toElement();
    QgsReadWriteContext context;
    if ( project )
      context.setPathResolver( project->pathResolver( ) );
    T2 *styleSymbol = QgsSymbolLayerUtils::loadSymbol<T2>( symbolElement, context );
    if ( styleSymbol )
      layoutItem->setSymbol( styleSymbol );
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
  layoutItem->mUuid = itemElem.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );

  // temporary for groups imported from templates
  layoutItem->mTemplateUuid = itemElem.attribute( QStringLiteral( "templateUuid" ) );

  //id
  const QString id = itemElem.attribute( QStringLiteral( "id" ), QString() );
  layoutItem->setId( id );

  //frame
  const QString frame = itemElem.attribute( QStringLiteral( "frame" ) );
  layoutItem->setFrameEnabled( frame.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 ) ;

  //frame
  const QString background = itemElem.attribute( QStringLiteral( "background" ) );
  layoutItem->setBackgroundEnabled( background.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );

  //position lock for mouse moves/resizes
  const QString positionLock = itemElem.attribute( QStringLiteral( "positionLock" ) );
  layoutItem->setLocked( positionLock.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );

  //visibility
  layoutItem->setVisibility( itemElem.attribute( QStringLiteral( "visibility" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );

  layoutItem->mParentGroupUuid = itemElem.attribute( QStringLiteral( "groupUuid" ) );
  if ( !layoutItem->mParentGroupUuid.isEmpty() )
  {
    if ( QgsLayoutItemGroup *group = layoutItem->parentGroup() )
    {
      group->addItem( layoutItem );
    }
  }
  layoutItem->mTemplateUuid = itemElem.attribute( QStringLiteral( "templateUuid" ) );


  const QRectF position = itemPosition( layoutItem, itemElem );

  // TODO: missing?
  // mLastValidViewScaleFactor = itemElem.attribute( QStringLiteral( "lastValidViewScaleFactor" ), QStringLiteral( "-1" ) ).toDouble();

  layoutItem->setZValue( itemElem.attribute( QStringLiteral( "zValue" ) ).toDouble() );

  //pen
  const QDomNodeList frameColorList = itemElem.elementsByTagName( QStringLiteral( "FrameColor" ) );
  if ( !frameColorList.isEmpty() )
  {
    const QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
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
      layoutItem->refreshFrame( false );
    }
  }

  //brush
  const QDomNodeList bgColorList = itemElem.elementsByTagName( QStringLiteral( "BackgroundColor" ) );
  if ( !bgColorList.isEmpty() )
  {
    const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
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
  layoutItem->attemptSetSceneRect( position );
  //rotation
  layoutItem->setItemRotation( itemElem.attribute( QStringLiteral( "itemRotation" ), QStringLiteral( "0" ) ).toDouble(), false );

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

  const QDomNode propsNode = itemElem.namedItem( QStringLiteral( "dataDefinedProperties" ) );
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
  if ( property == QgsCompositionConverter::AllProperties || property == QgsCompositionConverter::NoProperty )
  {
    //invalid property
    return QgsProperty();
  }

  const QString active = ddElem.attribute( QStringLiteral( "active" ) );
  bool isActive = false;
  if ( active.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isActive = true;
  }
  const QString field = ddElem.attribute( QStringLiteral( "field" ) );
  const QString expr = ddElem.attribute( QStringLiteral( "expr" ) );

  const QString useExpr = ddElem.attribute( QStringLiteral( "useExpr" ) );
  bool isExpression = false;
  if ( useExpr.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isExpression = true;
  }

  if ( isExpression )
    return QgsProperty::fromExpression( expr, isActive );
  else
    return QgsProperty::fromField( field, isActive );
}
