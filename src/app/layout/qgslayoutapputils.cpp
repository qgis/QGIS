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

#include "qgslayoutapputils.h"
#include "qgsgui.h"
#include "qgslayout.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutmapwidget.h"
#include "qgslayoutshapewidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutpolygonwidget.h"
#include "qgslayoutpolylinewidget.h"
#include "qgslayoutpicturewidget.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutlabelwidget.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutlegendwidget.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgslayouthtmlwidget.h"
#include "qgslayoutscalebarwidget.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutattributetablewidget.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"

#ifdef HAVE_3D
#include "qgslayout3dmapwidget.h"
#endif

void QgsLayoutAppUtils::registerGuiForKnownItemTypes()
{
  QgsLayoutItemGuiRegistry *registry = QgsGui::layoutItemGuiRegistry();

  registry->addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "shapes" ), QObject::tr( "Shape" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) ) );
  registry->addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "nodes" ), QObject::tr( "Node Item" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) ) ) );

  auto createRubberBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  } );
  auto createEllipseBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewEllipticalRubberBand( view );
  } );
  auto createTriangleBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewTriangleRubberBand( view );
  } );

#if 0
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, QStringLiteral( "test" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), nullptr, createRubberBand ) );
#endif

  // map item

  auto mapItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutMap, QObject::tr( "Map" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) ),
                         [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutMapWidget( qobject_cast< QgsLayoutItemMap * >( item ) );
  }, createRubberBand );
  mapItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
    Q_ASSERT( map );

    //get the color for map canvas background and set map background color accordingly
    int bgRedInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
    int bgGreenInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
    int bgBlueInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
    map->setBackgroundColor( QColor( bgRedInt, bgGreenInt, bgBlueInt ) );

    if ( QgisApp::instance()->mapCanvas() )
    {
      map->zoomToExtent( QgisApp::instance()->mapCanvas()->mapSettings().visibleExtent() );
    }
  } );
  registry->addLayoutItemGuiMetadata( mapItemMetadata.release() );

  // 3D map item
#ifdef HAVE_3D
  std::unique_ptr< QgsLayoutItemGuiMetadata > map3dMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::Layout3DMap, QObject::tr( "3D Map" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAdd3DMap.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayout3DMapWidget( qobject_cast< QgsLayoutItem3DMap * >( item ) );
  }, createRubberBand );
  registry->addLayoutItemGuiMetadata( map3dMetadata.release() );
#endif

  // picture item

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutPicture, QObject::tr( "Picture" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ),
                                      [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPictureWidget( qobject_cast< QgsLayoutItemPicture * >( item ) );
  }, createRubberBand ) );


  // label item

  auto labelItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutLabel, QObject::tr( "Label" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabel.svg" ) ),
                           [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutLabelWidget( qobject_cast< QgsLayoutItemLabel * >( item ) );
  }, createRubberBand );
  labelItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemLabel *label = qobject_cast< QgsLayoutItemLabel * >( item );
    Q_ASSERT( label );

    label->setText( QObject::tr( "Lorem ipsum" ) );
    QSizeF minSize = label->sizeForText();
    QSizeF currentSize = label->rect().size();

    //make sure label size is sufficient to fit text
    double labelWidth = std::max( minSize.width(), currentSize.width() );
    double labelHeight = std::max( minSize.height(), currentSize.height() );
    label->attemptSetSceneRect( QRectF( label->pos().x(), label->pos().y(), labelWidth, labelHeight ) );
  } );

  registry->addLayoutItemGuiMetadata( labelItemMetadata.release() );


  // legend item

  auto legendItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutLegend, QObject::tr( "Legend" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLegend.svg" ) ),
                            [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutLegendWidget( qobject_cast< QgsLayoutItemLegend * >( item ) );
  }, createRubberBand );
  legendItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemLegend *legend = qobject_cast< QgsLayoutItemLegend * >( item );
    Q_ASSERT( legend );

    QList<QgsLayoutItemMap *> mapItems;
    legend->layout()->layoutItems( mapItems );

    // try to find a good map to link the legend with by default
    // start by trying to find a selected map
    QgsLayoutItemMap *targetMap = nullptr;
    for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
    {
      if ( map->isSelected() )
      {
        targetMap = map;
        break;
      }
    }
    // otherwise just use first map
    if ( !targetMap && !mapItems.isEmpty() )
    {
      targetMap = mapItems.at( 0 );
    }
    if ( targetMap )
    {
      legend->setLinkedMap( targetMap );
    }

    legend->updateLegend();
  } );

  registry->addLayoutItemGuiMetadata( legendItemMetadata.release() );

  // scalebar item

  auto scalebarItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutScaleBar, QObject::tr( "Scale Bar" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionScaleBar.svg" ) ),
                              [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutScaleBarWidget( qobject_cast< QgsLayoutItemScaleBar * >( item ) );
  }, createRubberBand );
  scalebarItemMetadata->setItemAddedToLayoutFunction( [ = ]( QgsLayoutItem * item )
  {
    QgsLayoutItemScaleBar *scalebar = qobject_cast< QgsLayoutItemScaleBar * >( item );
    Q_ASSERT( scalebar );

    QList<QgsLayoutItemMap *> mapItems;
    scalebar->layout()->layoutItems( mapItems );

    // try to find a good map to link the scalebar with by default
    // start by trying to find a selected map
    QgsLayoutItemMap *targetMap = nullptr;
    for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
    {
      if ( map->isSelected() )
      {
        targetMap = map;
        break;
      }
    }
    // otherwise just use first map
    if ( !targetMap && !mapItems.isEmpty() )
    {
      targetMap = mapItems.at( 0 );
    }
    if ( targetMap )
    {
      scalebar->setLinkedMap( targetMap );
      scalebar->applyDefaultSize( scalebar->guessUnits() );
    }
  } );

  registry->addLayoutItemGuiMetadata( scalebarItemMetadata.release() );

  // shape items

  auto createShapeWidget =
    []( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutShapeWidget( qobject_cast< QgsLayoutItemShape * >( item ) );
  };

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Rectangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ), createShapeWidget, createRubberBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Rectangle );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Ellipse" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ), createShapeWidget, createEllipseBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Ellipse );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Triangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ), createShapeWidget, createTriangleBand, QStringLiteral( "shapes" ), false, nullptr, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Triangle );
    return shape.release();
  } ) );

  // arrow
  std::unique_ptr< QgsLayoutItemGuiMetadata > arrowMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Arrow" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddArrow.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolylineWidget( qobject_cast< QgsLayoutItemPolyline * >( item ) );
  }, createRubberBand, QString(), true );
  arrowMetadata->setItemCreationFunction( []( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemPolyline > arrow = qgis::make_unique< QgsLayoutItemPolyline >( layout );
    arrow->setEndMarker( QgsLayoutItemPolyline::ArrowHead );
    return arrow.release();
  } );
  arrowMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPathItem*
  {
    std::unique_ptr< QGraphicsPathItem > band = qgis::make_unique< QGraphicsPathItem >();
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( arrowMetadata.release() );


  // node items

  std::unique_ptr< QgsLayoutItemGuiMetadata > polygonMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >(
        QgsLayoutItemRegistry::LayoutPolygon, QObject::tr( "Polygon" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolygon.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolygonWidget( qobject_cast< QgsLayoutItemPolygon * >( item ) );
  }, createRubberBand, QStringLiteral( "nodes" ), true );
  polygonMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPolygonItem*
  {
    std::unique_ptr< QGraphicsPolygonItem > band = qgis::make_unique< QGraphicsPolygonItem >();
    band->setBrush( Qt::NoBrush );
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polygonMetadata.release() );

  std::unique_ptr< QgsLayoutItemGuiMetadata > polylineMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata>(
        QgsLayoutItemRegistry::LayoutPolyline, QObject::tr( "Polyline" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolyline.svg" ) ),
        [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPolylineWidget( qobject_cast< QgsLayoutItemPolyline * >( item ) );
  }, createRubberBand, QStringLiteral( "nodes" ), true );
  polylineMetadata->setNodeRubberBandCreationFunction( []( QgsLayoutView * )->QGraphicsPathItem*
  {
    std::unique_ptr< QGraphicsPathItem > band = qgis::make_unique< QGraphicsPathItem >();
    band->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
    band->setZValue( QgsLayout::ZViewTool );
    return band.release();
  } );
  registry->addLayoutItemGuiMetadata( polylineMetadata.release() );


  // html item

  auto htmlItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutHtml, QObject::tr( "HTML" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddHtml.svg" ) ),
                          [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutHtmlWidget( qobject_cast< QgsLayoutFrame * >( item ) );
  }, createRubberBand );
  htmlItemMetadata->setItemCreationFunction( [ = ]( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemHtml > htmlMultiFrame = qgis::make_unique< QgsLayoutItemHtml >( layout );
    QgsLayoutItemHtml *html = htmlMultiFrame.get();
    layout->addMultiFrame( htmlMultiFrame.release() );
    std::unique_ptr< QgsLayoutFrame > frame = qgis::make_unique< QgsLayoutFrame >( layout, html );
    QgsLayoutFrame *f = frame.get();
    html->addFrame( frame.release() );
    return f;
  } );
  registry->addLayoutItemGuiMetadata( htmlItemMetadata.release() );

  // attribute table item

  auto attributeTableItemMetadata = qgis::make_unique< QgsLayoutItemGuiMetadata >( QgsLayoutItemRegistry::LayoutAttributeTable, QObject::tr( "Attribute Table" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) ),
                                    [ = ]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutAttributeTableWidget( qobject_cast< QgsLayoutFrame * >( item ) );
  }, createRubberBand );
  attributeTableItemMetadata->setItemCreationFunction( [ = ]( QgsLayout * layout )->QgsLayoutItem *
  {
    std::unique_ptr< QgsLayoutItemAttributeTable > tableMultiFrame = qgis::make_unique< QgsLayoutItemAttributeTable >( layout );
    QgsLayoutItemAttributeTable *table = tableMultiFrame.get();

    //set first vector layer from layer registry as table source
    QMap<QString, QgsMapLayer *> layerMap = layout->project()->mapLayers();
    for ( auto it = layerMap.constBegin() ; it != layerMap.constEnd(); ++it )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() ) )
      {
        table->setVectorLayer( vl );
        break;
      }
    }

    layout->addMultiFrame( tableMultiFrame.release() );
    std::unique_ptr< QgsLayoutFrame > frame = qgis::make_unique< QgsLayoutFrame >( layout, table );
    QgsLayoutFrame *f = frame.get();
    table->addFrame( frame.release() );
    return f;
  } );
  registry->addLayoutItemGuiMetadata( attributeTableItemMetadata .release() );
}
