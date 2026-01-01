/***************************************************************************
                             qgslayoutapputils.cpp
                             ---------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutguiutils.h"

#include "qgsfontutils.h"
#include "qgsgui.h"
#include "qgslayoutattributetablewidget.h"
#include "qgslayoutchartwidget.h"
#include "qgslayoutelevationprofilewidget.h"
#include "qgslayoutframe.h"
#include "qgslayouthtmlwidget.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemchart.h"
#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmanualtable.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmarker.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutlabelwidget.h"
#include "qgslayoutlegendwidget.h"
#include "qgslayoutmanualtablewidget.h"
#include "qgslayoutmapwidget.h"
#include "qgslayoutmarkerwidget.h"
#include "qgslayoutpicturewidget.h"
#include "qgslayoutpolygonwidget.h"
#include "qgslayoutpolylinewidget.h"
#include "qgslayoutscalebarwidget.h"
#include "qgslayoutshapewidget.h"
#include "qgslayoutviewrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsplot.h"

/**
 * Attempts to find the best guess at a map item to link \a referenceItem to,
 * by:
 *
 * - Prioritizing a selected map
 * - If no selection, prioritizing the topmost map the item was drawn over
 * - If still none, use the layout's reference map (or biggest map)
 */
QgsLayoutItemMap *findSensibleDefaultLinkedMapItem( QgsLayoutItem *referenceItem )
{
  // start by trying to find a selected map
  QList<QgsLayoutItemMap *> mapItems;
  referenceItem->layout()->layoutItems( mapItems );

  QgsLayoutItemMap *targetMap = nullptr;
  for ( QgsLayoutItemMap *map : std::as_const( mapItems ) )
  {
    if ( map->isSelected() )
    {
      return map;
    }
  }

  // nope, no selection... hm, was the item drawn over a map? If so, use the topmost intersecting one
  double largestZValue = std::numeric_limits<double>::lowest();
  for ( QgsLayoutItemMap *map : std::as_const( mapItems ) )
  {
    if ( map->collidesWithItem( referenceItem ) && map->zValue() > largestZValue )
    {
      targetMap = map;
      largestZValue = map->zValue();
    }
  }
  if ( targetMap )
    return targetMap;

  // ah frick it, just use the reference (or biggest!) map
  return referenceItem->layout()->referenceMap();
}

void QgsLayoutGuiUtils::registerGuiForKnownItemTypes( QgsMapCanvas *mapCanvas )
{
  QgsLayoutItemGuiRegistry *registry = QgsGui::layoutItemGuiRegistry();

  registry->addItemGroup( QgsLayoutItemGuiGroup( u"shapes"_s, QObject::tr( "Shape" ), QgsApplication::getThemeIcon( u"/mActionAddBasicShape.svg"_s ) ) );
  registry->addItemGroup( QgsLayoutItemGuiGroup( u"nodes"_s, QObject::tr( "Node Item" ), QgsApplication::getThemeIcon( u"/mActionAddNodesItem.svg"_s ) ) );

  auto createRubberBand = ( []( QgsLayoutView *view ) -> QgsLayoutViewRubberBand * {
    return new QgsLayoutViewRectangularRubberBand( view );
  } );
  auto createEllipseBand = ( []( QgsLayoutView *view ) -> QgsLayoutViewRubberBand * {
    return new QgsLayoutViewEllipticalRubberBand( view );
  } );
  auto createTriangleBand = ( []( QgsLayoutView *view ) -> QgsLayoutViewRubberBand * {
    return new QgsLayoutViewTriangleRubberBand( view );
  } );

#if 0
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, u"test"_s, QgsApplication::getThemeIcon( u"/mActionAddLabel.svg"_s ), nullptr, createRubberBand ) );
#endif

  // map item

  auto mapItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutMap, QObject::tr( "Map" ), QgsApplication::getThemeIcon( u"/mActionAddMap.svg"_s ), [mapCanvas]( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutMapWidget( qobject_cast<QgsLayoutItemMap *>( item ), mapCanvas ); }, createRubberBand );
  mapItemMetadata->setItemAddedToLayoutFunction( [mapCanvas]( QgsLayoutItem *item, const QVariantMap & ) {
    QgsLayoutItemMap *map = qobject_cast<QgsLayoutItemMap *>( item );
    Q_ASSERT( map );

    //get the color for map canvas background and set map background color accordingly
    map->setBackgroundColor( QgsProject::instance()->backgroundColor() );

    if ( mapCanvas )
    {
      map->setMapRotation( mapCanvas->rotation() );
      map->zoomToExtent( mapCanvas->mapSettings().visibleExtent() );
    }

    // auto assign a unique id to map items
    QList<QgsLayoutItemMap *> mapsList;
    if ( map->layout() )
      map->layout()->layoutItems( mapsList );

    int counter = mapsList.size() + 1;
    bool existing = false;
    while ( true )
    {
      existing = false;
      for ( QgsLayoutItemMap *otherMap : std::as_const( mapsList ) )
      {
        if ( map == otherMap )
          continue;

        if ( otherMap->id() == QObject::tr( "Map %1" ).arg( counter ) )
        {
          existing = true;
          break;
        }
      }
      if ( existing )
        counter++;
      else
        break;
    }
    map->setId( QObject::tr( "Map %1" ).arg( counter ) );
  } );
  registry->addLayoutItemGuiMetadata( mapItemMetadata.release() );

  // picture item

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutPicture, QObject::tr( "Picture" ), QgsApplication::getThemeIcon( u"/mActionAddImage.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutPictureWidget( qobject_cast<QgsLayoutItemPicture *>( item ) ); }, createRubberBand ) );

  // label item

  auto labelItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutLabel, QObject::tr( "Label" ), QgsApplication::getThemeIcon( u"/mActionLabel.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutLabelWidget( qobject_cast<QgsLayoutItemLabel *>( item ) ); }, createRubberBand );
  labelItemMetadata->setItemAddedToLayoutFunction( []( QgsLayoutItem *item, const QVariantMap &properties ) {
    QgsLayoutItemLabel *label = qobject_cast<QgsLayoutItemLabel *>( item );
    Q_ASSERT( label );

    label->setText( properties.value( u"expression"_s ).toString().isEmpty() ? QObject::tr( "Lorem ipsum" ) : u"[% %1 %]"_s.arg( properties.value( u"expression"_s ).toString() ) );
    if ( QApplication::isRightToLeft() )
    {
      label->setHAlign( Qt::AlignRight );
    }
    QSizeF minSize = label->sizeForText();
    QSizeF currentSize = label->rect().size();

    //make sure label size is sufficient to fit text
    double labelWidth = std::max( minSize.width(), currentSize.width() );
    double labelHeight = std::max( minSize.height(), currentSize.height() );
    label->attemptSetSceneRect( QRectF( label->pos().x(), label->pos().y(), labelWidth, labelHeight ) );
  } );

  labelItemMetadata->setItemDoubleClickedFunction( []( QgsLayoutItem *item, Qgis::MouseHandlesAction action ) {
    QgsLayoutItemLabel *label = qobject_cast<QgsLayoutItemLabel *>( item );

    // size to text doesn't have any real meaning for HTML content, skip it
    if ( label->mode() == QgsLayoutItemLabel::ModeHtml )
      return;

    Q_ASSERT( label );
    QgsLayoutItem::ReferencePoint reference = QgsLayoutItem::ReferencePoint::UpperLeft;
    switch ( action )
    {
      case Qgis::MouseHandlesAction::MoveItem:
      case Qgis::MouseHandlesAction::NoAction:
      case Qgis::MouseHandlesAction::SelectItem:
      case Qgis::MouseHandlesAction::RotateTopLeft:
      case Qgis::MouseHandlesAction::RotateTopRight:
      case Qgis::MouseHandlesAction::RotateBottomLeft:
      case Qgis::MouseHandlesAction::RotateBottomRight:
        return;

      case Qgis::MouseHandlesAction::ResizeUp:
        reference = QgsLayoutItem::ReferencePoint::LowerMiddle;
        break;

      case Qgis::MouseHandlesAction::ResizeDown:
        reference = QgsLayoutItem::ReferencePoint::UpperMiddle;
        break;

      case Qgis::MouseHandlesAction::ResizeLeft:
        reference = QgsLayoutItem::ReferencePoint::MiddleRight;
        break;

      case Qgis::MouseHandlesAction::ResizeRight:
        reference = QgsLayoutItem::ReferencePoint::MiddleLeft;
        break;

      case Qgis::MouseHandlesAction::ResizeLeftUp:
        reference = QgsLayoutItem::ReferencePoint::LowerRight;
        break;

      case Qgis::MouseHandlesAction::ResizeRightUp:
        reference = QgsLayoutItem::ReferencePoint::LowerLeft;
        break;

      case Qgis::MouseHandlesAction::ResizeLeftDown:
        reference = QgsLayoutItem::ReferencePoint::UpperRight;
        break;

      case Qgis::MouseHandlesAction::ResizeRightDown:
        reference = QgsLayoutItem::ReferencePoint::UpperLeft;
        break;
    }

    label->beginCommand( QObject::tr( "Resize to Text" ) );
    label->adjustSizeToText( reference );
    label->endCommand();
  } );

  registry->addLayoutItemGuiMetadata( labelItemMetadata.release() );


  // legend item

  auto legendItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutLegend, QObject::tr( "Legend" ), QgsApplication::getThemeIcon( u"/mActionAddLegend.svg"_s ), [mapCanvas]( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutLegendWidget( qobject_cast<QgsLayoutItemLegend *>( item ), mapCanvas ); }, createRubberBand );
  legendItemMetadata->setItemAddedToLayoutFunction( []( QgsLayoutItem *item, const QVariantMap & ) {
    QgsLayoutItemLegend *legend = qobject_cast<QgsLayoutItemLegend *>( item );
    Q_ASSERT( legend );

    // try to find a good map to link the legend with by default
    legend->setLinkedMap( findSensibleDefaultLinkedMapItem( legend ) );

    if ( QApplication::isRightToLeft() )
    {
      // for right-to-left locales, use an appropriate default layout
      legend->setSymbolAlignment( Qt::AlignRight );
      legend->rstyle( Qgis::LegendComponent::Group ).setAlignment( Qt::AlignRight );
      legend->rstyle( Qgis::LegendComponent::Subgroup ).setAlignment( Qt::AlignRight );
      legend->rstyle( Qgis::LegendComponent::SymbolLabel ).setAlignment( Qt::AlignRight );
      legend->setTitleAlignment( Qt::AlignRight );
    }

    //set default legend font from settings
    QgsSettings settings;
    const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
    if ( !defaultFontString.isEmpty() )
    {
      QFont font;
      QgsFontUtils::setFontFamily( font, defaultFontString );

      QgsTextFormat f = legend->rstyle( Qgis::LegendComponent::Title ).textFormat();
      f.setFont( font );
      legend->rstyle( Qgis::LegendComponent::Title ).setTextFormat( f );

      f = legend->rstyle( Qgis::LegendComponent::Group ).textFormat();
      f.setFont( font );
      legend->rstyle( Qgis::LegendComponent::Group ).setTextFormat( f );

      f = legend->rstyle( Qgis::LegendComponent::Subgroup ).textFormat();
      f.setFont( font );
      legend->rstyle( Qgis::LegendComponent::Subgroup ).setTextFormat( f );

      f = legend->rstyle( Qgis::LegendComponent::SymbolLabel ).textFormat();
      f.setFont( font );
      legend->rstyle( Qgis::LegendComponent::SymbolLabel ).setTextFormat( f );
    }

    legend->updateLegend();
  } );

  registry->addLayoutItemGuiMetadata( legendItemMetadata.release() );

  // scalebar item

  auto scalebarItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutScaleBar, QObject::tr( "Scale Bar" ), QgsApplication::getThemeIcon( u"/mActionScaleBar.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutScaleBarWidget( qobject_cast<QgsLayoutItemScaleBar *>( item ) ); }, createRubberBand );
  scalebarItemMetadata->setItemAddedToLayoutFunction( []( QgsLayoutItem *item, const QVariantMap & ) {
    QgsLayoutItemScaleBar *scalebar = qobject_cast<QgsLayoutItemScaleBar *>( item );
    Q_ASSERT( scalebar );

    // default to project's scale calculation method
    scalebar->setMethod( scalebar->layout()->project()->scaleMethod() );

    // try to find a good map to link the scalebar with by default
    if ( QgsLayoutItemMap *targetMap = findSensibleDefaultLinkedMapItem( scalebar ) )
    {
      scalebar->setLinkedMap( targetMap );
      scalebar->applyDefaultSize( scalebar->guessUnits() );
    }
  } );

  registry->addLayoutItemGuiMetadata( scalebarItemMetadata.release() );


  // north arrow
  auto northArrowMetadata = std::make_unique<QgsLayoutItemGuiMetadata>(
    QgsLayoutItemRegistry::LayoutPicture, QObject::tr( "North Arrow" ), QgsApplication::getThemeIcon( u"/north_arrow.svg"_s ),
    []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * {
      return new QgsLayoutPictureWidget( qobject_cast<QgsLayoutItemPicture *>( item ) );
    },
    createRubberBand
  );
  northArrowMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    // count how many existing north arrows are already in layout
    QList<QgsLayoutItemPicture *> pictureItems;
    layout->layoutItems( pictureItems );
    int northArrowCount = 0;

    QgsSettings settings;
    const QString defaultPath = settings.value( u"LayoutDesigner/defaultNorthArrow"_s, u":/images/north_arrows/layout_default_north_arrow.svg"_s, QgsSettings::Gui ).toString();

    for ( QgsLayoutItemPicture *p : std::as_const( pictureItems ) )
    {
      // look for pictures which use the default north arrow svg
      if ( p->picturePath() == defaultPath )
        northArrowCount++;
    }

    auto picture = std::make_unique<QgsLayoutItemPicture>( layout );
    picture->setNorthMode( QgsLayoutItemPicture::GridNorth );
    picture->setPicturePath( defaultPath );
    // set an id by default, so that north arrows are discernible in layout item lists
    picture->setId( northArrowCount > 0 ? QObject::tr( "North Arrow %1" ).arg( northArrowCount + 1 ) : QObject::tr( "North Arrow" ) );
    return picture.release();
  } );
  northArrowMetadata->setItemAddedToLayoutFunction( []( QgsLayoutItem *item, const QVariantMap & ) {
    QgsLayoutItemPicture *picture = qobject_cast<QgsLayoutItemPicture *>( item );
    Q_ASSERT( picture );

    QList<QgsLayoutItemMap *> mapItems;
    picture->layout()->layoutItems( mapItems );

    // try to find a good map to link the north arrow with by default
    picture->setLinkedMap( findSensibleDefaultLinkedMapItem( picture ) );
  } );
  registry->addLayoutItemGuiMetadata( northArrowMetadata.release() );

  // shape items

  auto createShapeWidget =
    []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * {
    return new QgsLayoutShapeWidget( qobject_cast<QgsLayoutItemShape *>( item ) );
  };

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Rectangle" ), QgsApplication::getThemeIcon( u"/mActionAddBasicRectangle.svg"_s ), createShapeWidget, createRubberBand, u"shapes"_s, false, QgsLayoutItemAbstractGuiMetadata::Flags(), []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto shape = std::make_unique<QgsLayoutItemShape>( layout );
    shape->setShapeType( QgsLayoutItemShape::Rectangle );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Ellipse" ), QgsApplication::getThemeIcon( u"/mActionAddBasicCircle.svg"_s ), createShapeWidget, createEllipseBand, u"shapes"_s, false, QgsLayoutItemAbstractGuiMetadata::Flags(), []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto shape = std::make_unique<QgsLayoutItemShape>( layout );
    shape->setShapeType( QgsLayoutItemShape::Ellipse );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Triangle" ), QgsApplication::getThemeIcon( u"/mActionAddBasicTriangle.svg"_s ), createShapeWidget, createTriangleBand, u"shapes"_s, false, QgsLayoutItemAbstractGuiMetadata::Flags(), []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto shape = std::make_unique<QgsLayoutItemShape>( layout );
    shape->setShapeType( QgsLayoutItemShape::Triangle );
    return shape.release();
  } ) );

  // marker
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutMarker, QObject::tr( "Marker" ), QgsApplication::getThemeIcon( u"/mActionAddMarker.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutMarkerWidget( qobject_cast<QgsLayoutItemMarker *>( item ) ); }, nullptr ) );

  // arrow
  auto arrowMetadata = std::make_unique<QgsLayoutItemGuiMetadata>(
    QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Arrow" ), QgsApplication::getThemeIcon( u"/mActionAddArrow.svg"_s ),
    []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * {
      return new QgsLayoutPolylineWidget( qobject_cast<QgsLayoutItemPolyline *>( item ) );
    },
    createRubberBand, QString(), true
  );
  arrowMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto arrow = std::make_unique<QgsLayoutItemPolyline>( layout );
    arrow->setEndMarker( QgsLayoutItemPolyline::ArrowHead );
    return arrow.release();
  } );
  arrowMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * ) -> QGraphicsItemGroup * {
    auto band = std::make_unique<QGraphicsItemGroup>();
    QGraphicsPathItem *poly = new QGraphicsPathItem( band.get() );
    poly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );

    QGraphicsPathItem *tempPoly = new QGraphicsPathItem( band.get() );
    tempPoly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0, Qt::DotLine ) );

    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( arrowMetadata.release() );

  // node items

  auto polygonMetadata = std::make_unique<QgsLayoutItemGuiMetadata>(
    QgsLayoutItemRegistry::LayoutPolygon, QObject::tr( "Polygon" ), QgsApplication::getThemeIcon( u"/mActionAddPolygon.svg"_s ),
    []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * {
      return new QgsLayoutPolygonWidget( qobject_cast<QgsLayoutItemPolygon *>( item ) );
    },
    createRubberBand, u"nodes"_s, true
  );
  polygonMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * ) -> QGraphicsItemGroup * {
    auto band = std::make_unique<QGraphicsItemGroup>();
    QGraphicsPolygonItem *poly = new QGraphicsPolygonItem( band.get() );
    poly->setBrush( QBrush( QColor( 227, 22, 22, 20 ) ) );
    poly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );

    QGraphicsPolygonItem *tempPoly = new QGraphicsPolygonItem( band.get() );
    tempPoly->setBrush( Qt::NoBrush );
    tempPoly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0, Qt::DotLine ) );

    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polygonMetadata.release() );

  auto polylineMetadata = std::make_unique<QgsLayoutItemGuiMetadata>(
    QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Polyline" ), QgsApplication::getThemeIcon( u"/mActionAddPolyline.svg"_s ),
    []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * {
      return new QgsLayoutPolylineWidget( qobject_cast<QgsLayoutItemPolyline *>( item ) );
    },
    createRubberBand, u"nodes"_s, true
  );
  polylineMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * ) -> QGraphicsItemGroup * {
    auto band = std::make_unique<QGraphicsItemGroup>();
    QGraphicsPathItem *poly = new QGraphicsPathItem( band.get() );
    poly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );

    QGraphicsPathItem *tempPoly = new QGraphicsPathItem( band.get() );
    tempPoly->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0, Qt::DotLine ) );

    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polylineMetadata.release() );


  // html item

  auto htmlItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutHtml, QObject::tr( "HTML" ), QgsApplication::getThemeIcon( u"/mActionAddHtml.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutHtmlWidget( qobject_cast<QgsLayoutFrame *>( item ) ); }, createRubberBand );
  htmlItemMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto htmlMultiFrame = std::make_unique<QgsLayoutItemHtml>( layout );
    QgsLayoutItemHtml *html = htmlMultiFrame.get();
    layout->addMultiFrame( htmlMultiFrame.release() );
    auto frame = std::make_unique<QgsLayoutFrame>( layout, html );
    QgsLayoutFrame *f = frame.get();
    html->addFrame( frame.release() );
    // cppcheck-suppress returnDanglingLifetime
    return f;
  } );
  registry->addLayoutItemGuiMetadata( htmlItemMetadata.release() );

  // attribute table item

  auto attributeTableItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutAttributeTable, QObject::tr( "Attribute Table" ), QgsApplication::getThemeIcon( u"/mActionAddTable.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutAttributeTableWidget( qobject_cast<QgsLayoutFrame *>( item ) ); }, createRubberBand );
  attributeTableItemMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto tableMultiFrame = std::make_unique<QgsLayoutItemAttributeTable>( layout );
    QgsLayoutItemAttributeTable *table = tableMultiFrame.get();

    //set first vector layer from layer registry as table source
    QMap<QString, QgsMapLayer *> layerMap = layout->project()->mapLayers();
    for ( auto it = layerMap.constBegin(); it != layerMap.constEnd(); ++it )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() ) )
      {
        table->setVectorLayer( vl );
        break;
      }
    }

    //set default table fonts from settings
    QgsSettings settings;
    const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
    if ( !defaultFontString.isEmpty() )
    {
      QgsTextFormat format;
      QFont f = format.font();
      QgsFontUtils::setFontFamily( f, defaultFontString );
      format.setFont( f );
      tableMultiFrame->setContentTextFormat( format );
      f.setBold( true );
      format.setFont( f );
      tableMultiFrame->setHeaderTextFormat( format );
    }

    layout->addMultiFrame( tableMultiFrame.release() );
    auto frame = std::make_unique<QgsLayoutFrame>( layout, table );
    QgsLayoutFrame *f = frame.get();
    table->addFrame( frame.release() );
    // cppcheck-suppress returnDanglingLifetime
    return f;
  } );
  registry->addLayoutItemGuiMetadata( attributeTableItemMetadata.release() );

  // manual table item

  auto manualTableItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutManualTable, QObject::tr( "Fixed Table" ), QgsApplication::getThemeIcon( u"/mActionAddManualTable.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutManualTableWidget( qobject_cast<QgsLayoutFrame *>( item ) ); }, createRubberBand );
  manualTableItemMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto tableMultiFrame = std::make_unique<QgsLayoutItemManualTable>( layout );
    QgsLayoutItemManualTable *table = tableMultiFrame.get();

    // initially start with a 2x2 empty table
    QgsTableContents contents;
    contents << ( QgsTableRow() << QgsTableCell() << QgsTableCell() );
    contents << ( QgsTableRow() << QgsTableCell() << QgsTableCell() );
    table->setTableContents( contents );

    //set default table fonts from settings
    QgsSettings settings;
    const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
    if ( !defaultFontString.isEmpty() )
    {
      QgsTextFormat format;
      QFont f = format.font();
      QgsFontUtils::setFontFamily( f, defaultFontString );
      format.setFont( f );
      tableMultiFrame->setContentTextFormat( format );
      f.setBold( true );
      format.setFont( f );
      tableMultiFrame->setHeaderTextFormat( format );
    }

    layout->addMultiFrame( tableMultiFrame.release() );

    auto frame = std::make_unique<QgsLayoutFrame>( layout, table );
    QgsLayoutFrame *f = frame.get();
    table->addFrame( frame.release() );
    // cppcheck-suppress returnDanglingLifetime
    return f;
  } );
  manualTableItemMetadata->setItemDoubleClickedFunction( []( QgsLayoutItem *item, Qgis::MouseHandlesAction ) {
    QgsLayoutManualTableWidget::openTableDesigner( qobject_cast<QgsLayoutFrame *>( item ) );
  } );
  registry->addLayoutItemGuiMetadata( manualTableItemMetadata.release() );


  // elevation profile item

  auto elevationProfileItemMetadata = std::make_unique<QgsLayoutItemGuiMetadata>( QgsLayoutItemRegistry::LayoutElevationProfile, QObject::tr( "Elevation Profile" ), QgsApplication::getThemeIcon( u"/mActionElevationProfile.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutElevationProfileWidget( qobject_cast<QgsLayoutItemElevationProfile *>( item ) ); }, createRubberBand );
  elevationProfileItemMetadata->setItemCreationFunction( []( QgsLayout *layout ) -> QgsLayoutItem * {
    auto profileItem = std::make_unique<QgsLayoutItemElevationProfile>( layout );

    //set default fonts from settings
    QgsSettings settings;
    const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
    if ( !defaultFontString.isEmpty() )
    {
      QgsTextFormat format = profileItem->plot()->xAxis().textFormat();
      QFont f = format.font();
      QgsFontUtils::setFontFamily( f, defaultFontString );
      format.setFont( f );
      profileItem->plot()->xAxis().setTextFormat( format );

      format = profileItem->plot()->yAxis().textFormat();
      f = format.font();
      QgsFontUtils::setFontFamily( f, defaultFontString );
      format.setFont( f );
      profileItem->plot()->yAxis().setTextFormat( format );
    }
    return profileItem.release();
  } );
  registry->addLayoutItemGuiMetadata( elevationProfileItemMetadata.release() );

  // chart item

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutChart, QObject::tr( "Chart" ), QgsApplication::getThemeIcon( u"/mActionAddChart.svg"_s ), []( QgsLayoutItem *item ) -> QgsLayoutItemBaseWidget * { return new QgsLayoutChartWidget( qobject_cast<QgsLayoutItemChart *>( item ) ); }, createRubberBand ) );
}
