/***************************************************************************
                            qgsannotationitemguiregistry.cpp
                            --------------------------
    begin                : September 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsannotationitemguiregistry.h"

#include "qgsannotationitem.h"
#include "qgsannotationitemregistry.h"
#include "qgsannotationitemwidget_impl.h"
#include "qgscreateannotationitemmaptool_impl.h"

#include <QImageReader>

#include "moc_qgsannotationitemguiregistry.cpp"

//
// QgsAnnotationItemAbstractGuiMetadata
//

QIcon QgsAnnotationItemAbstractGuiMetadata::creationIcon() const
{
  return QgsApplication::getThemeIcon( u"/mActionAddBasicRectangle.svg"_s );
}

QgsAnnotationItemBaseWidget *QgsAnnotationItemAbstractGuiMetadata::createItemWidget( QgsAnnotationItem * )
{
  return nullptr;
}

QgsCreateAnnotationItemMapToolInterface *QgsAnnotationItemAbstractGuiMetadata::createMapTool( QgsMapCanvas *, QgsAdvancedDigitizingDockWidget * )
{
  return nullptr;
}

QgsAnnotationItem *QgsAnnotationItemAbstractGuiMetadata::createItem()
{
  return nullptr;
}

void QgsAnnotationItemAbstractGuiMetadata::newItemAddedToLayer( QgsAnnotationItem *, QgsAnnotationLayer * )
{
}

//
// QgsAnnotationItemGuiMetadata
//

QIcon QgsAnnotationItemGuiMetadata::creationIcon() const
{
  return mIcon.isNull() ? QgsAnnotationItemAbstractGuiMetadata::creationIcon() : mIcon;
}

QgsAnnotationItemBaseWidget *QgsAnnotationItemGuiMetadata::createItemWidget( QgsAnnotationItem *item )
{
  return mWidgetFunc ? mWidgetFunc( item ) : nullptr;
}

QgsAnnotationItem *QgsAnnotationItemGuiMetadata::createItem()
{
  return mCreateFunc ? mCreateFunc() : QgsAnnotationItemAbstractGuiMetadata::createItem();
}

void QgsAnnotationItemGuiMetadata::newItemAddedToLayer( QgsAnnotationItem *item, QgsAnnotationLayer *layer )
{
  if ( mAddedToLayerFunc )
    mAddedToLayerFunc( item, layer );
}

QgsCreateAnnotationItemMapToolInterface *QgsAnnotationItemGuiMetadata::createMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
{
  return mCreateMapToolFunc ? mCreateMapToolFunc( canvas, cadDockWidget ) : nullptr;
}


//
// QgsAnnotationItemGuiRegistry
//

QgsAnnotationItemGuiRegistry::QgsAnnotationItemGuiRegistry( QObject *parent )
  : QObject( parent )
{
}


QgsAnnotationItemGuiRegistry::~QgsAnnotationItemGuiRegistry()
{
  qDeleteAll( mMetadata );
}

QgsAnnotationItemAbstractGuiMetadata *QgsAnnotationItemGuiRegistry::itemMetadata( int metadataId ) const
{
  return mMetadata.value( metadataId );
}

int QgsAnnotationItemGuiRegistry::metadataIdForItemType( const QString &type ) const
{
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.key();
  }
  return -1;
}

bool QgsAnnotationItemGuiRegistry::addAnnotationItemGuiMetadata( QgsAnnotationItemAbstractGuiMetadata *metadata )
{
  if ( !metadata )
    return false;

  const int id = mMetadata.count();
  mMetadata[id] = metadata;
  emit typeAdded( id );
  return true;
}

bool QgsAnnotationItemGuiRegistry::addItemGroup( const QgsAnnotationItemGuiGroup &group )
{
  if ( mItemGroups.contains( group.id ) )
    return false;

  mItemGroups.insert( group.id, group );
  return true;
}

const QgsAnnotationItemGuiGroup &QgsAnnotationItemGuiRegistry::itemGroup( const QString &id ) const
{
  auto iter = mItemGroups.find( id );
  if ( iter == mItemGroups.end() )
  {
    static QgsAnnotationItemGuiGroup invalidGroup;
    return invalidGroup;
  }
  return *iter;
}

QgsAnnotationItem *QgsAnnotationItemGuiRegistry::createItem( int metadataId ) const
{
  auto it = mMetadata.constFind( metadataId );
  if ( it == mMetadata.constEnd() )
    return nullptr;

  std::unique_ptr<QgsAnnotationItem> item( it.value()->createItem() );
  if ( item )
    return item.release();

  const QString type = it.value()->type();
  return QgsApplication::annotationItemRegistry()->createItem( type );
}

void QgsAnnotationItemGuiRegistry::newItemAddedToLayer( int metadataId, QgsAnnotationItem *item, QgsAnnotationLayer *layer )
{
  auto it = mMetadata.constFind( metadataId );
  if ( it == mMetadata.constEnd() )
    return;

  it.value()->newItemAddedToLayer( item, layer );
}

QgsAnnotationItemBaseWidget *QgsAnnotationItemGuiRegistry::createItemWidget( QgsAnnotationItem *item ) const
{
  if ( !item )
    return nullptr;

  const QString &type = item->type();
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type )
      return it.value()->createItemWidget( item );
  }

  return nullptr;
}

QList<int> QgsAnnotationItemGuiRegistry::itemMetadataIds() const
{
  return mMetadata.keys();
}

void QgsAnnotationItemGuiRegistry::addDefaultItems()
{
  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"polygon"_s, QObject::tr( "Polygon Annotation" ), QgsApplication::getThemeIcon( u"/mActionAddPolygon.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationPolygonItemWidget *widget = new QgsAnnotationPolygonItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreatePolygonItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"linestring"_s, QObject::tr( "Line Annotation" ), QgsApplication::getThemeIcon( u"/mActionAddPolyline.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationLineItemWidget *widget = new QgsAnnotationLineItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreateLineItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"marker"_s, QObject::tr( "Marker Annotation" ), QgsApplication::getThemeIcon( u"/mActionAddMarker.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationMarkerItemWidget *widget = new QgsAnnotationMarkerItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreateMarkerItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"pointtext"_s, QObject::tr( "Text Annotation at Point" ), QgsApplication::getThemeIcon( u"/mActionText.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationPointTextItemWidget *widget = new QgsAnnotationPointTextItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreatePointTextItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"linetext"_s, QObject::tr( "Text Annotation along Line" ), QgsApplication::getThemeIcon( u"/mActionTextAlongLine.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationLineTextItemWidget *widget = new QgsAnnotationLineTextItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreateLineTextItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"recttext"_s, QObject::tr( "Text Annotation in Rectangle" ), QgsApplication::getThemeIcon( u"/mActionTextInsideRect.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationRectangleTextItemWidget *widget = new QgsAnnotationRectangleTextItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreateRectangleTextItemMapTool( canvas, cadDockWidget ); } ) );

  addAnnotationItemGuiMetadata( new QgsAnnotationItemGuiMetadata( u"picture"_s, QObject::tr( "Picture Annotation" ), QgsApplication::getThemeIcon( u"/mActionAddImage.svg"_s ), []( QgsAnnotationItem *item ) -> QgsAnnotationItemBaseWidget * {
    QgsAnnotationPictureItemWidget *widget = new QgsAnnotationPictureItemWidget( nullptr );
    widget->setItem( item );
    return widget; }, QString(), Qgis::AnnotationItemGuiFlags(), nullptr, []( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsCreateAnnotationItemMapToolInterface * { return new QgsCreatePictureItemMapTool( canvas, cadDockWidget ); } ) );
}
