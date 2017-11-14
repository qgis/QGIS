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
#include "qgslayoutlegendwidget.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"

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

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, QStringLiteral( "test" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), nullptr, createRubberBand ) );


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
    if ( QgisApp::instance()->mapCanvas() )
    {
      map->zoomToExtent( QgisApp::instance()->mapCanvas()->mapSettings().visibleExtent() );
    }
  } );
  registry->addLayoutItemGuiMetadata( mapItemMetadata.release() );


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

    if ( !mapItems.isEmpty() )
    {
      legend->setMap( mapItems.at( 0 ) );
    }

    legend->updateLegend();
  } );

  registry->addLayoutItemGuiMetadata( legendItemMetadata.release() );


  // shape items

  auto createShapeWidget =
    []( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutShapeWidget( qobject_cast< QgsLayoutItemShape * >( item ) );
  };

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Rectangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ), createShapeWidget, createRubberBand, QStringLiteral( "shapes" ), false, 0, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Rectangle );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Ellipse" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ), createShapeWidget, createEllipseBand, QStringLiteral( "shapes" ), false, 0, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Ellipse );
    return shape.release();
  } ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutShape, QObject::tr( "Triangle" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ), createShapeWidget, createTriangleBand, QStringLiteral( "shapes" ), false, 0, []( QgsLayout * layout )->QgsLayoutItem*
  {
    std::unique_ptr< QgsLayoutItemShape > shape = qgis::make_unique< QgsLayoutItemShape >( layout );
    shape->setShapeType( QgsLayoutItemShape::Triangle );
    return shape.release();
  } ) );


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
}
