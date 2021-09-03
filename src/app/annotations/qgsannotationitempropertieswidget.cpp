/***************************************************************************
    qgsannotationitempropertieswidget.cpp
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitempropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsannotationlayer.h"
#include "qgsannotationitemwidget.h"
#include "qgsannotationitem.h"
#include "qgsgui.h"
#include "qgsannotationitemguiregistry.h"
#include <QStackedWidget>
#include <QHBoxLayout>

QgsAnnotationItemPropertiesWidget::QgsAnnotationItemPropertiesWidget( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  mStack = new QStackedWidget();

  setDockMode( true );

  QHBoxLayout *l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  l->addWidget( mStack );
  setLayout( l );

  syncToLayer( layer );
}

void QgsAnnotationItemPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  if ( layer == mLayer )
    return;

  mLayer = qobject_cast< QgsAnnotationLayer * >( layer );
  if ( !mLayer )
    return;

  // check context
  setItemId( mContext.annotationId() );
}

void QgsAnnotationItemPropertiesWidget::setContext( const QgsMapLayerConfigWidgetContext &context )
{
  QgsMapLayerConfigWidget::setContext( context );
  setItemId( context.annotationId() );
}

void QgsAnnotationItemPropertiesWidget::setDockMode( bool dockMode )
{
  QgsMapLayerConfigWidget::setDockMode( dockMode );
  if ( mItemWidget )
    mItemWidget->setDockMode( dockMode );
}

void QgsAnnotationItemPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  mLayer->triggerRepaint();
}

void QgsAnnotationItemPropertiesWidget::onChanged()
{
  // set the annotation layer's item's properties to match the widget
  std::unique_ptr< QgsAnnotationItem > newItem( mItemWidget->createItem() );

  mLayer->replaceItem( mContext.annotationId(), newItem.release() );
}

void QgsAnnotationItemPropertiesWidget::setItemId( const QString &itemId )
{
  // try to retrieve matching item
  bool setItem = false;
  if ( QgsAnnotationItem *item = mLayer->item( itemId ) )
  {
    if ( mItemWidget )
    {
      setItem = mItemWidget->setItem( item );
    }

    if ( !setItem )
    {
      // create new item
      mItemWidget = QgsGui::annotationItemGuiRegistry()->createItemWidget( item );

      if ( mItemWidget )
      {
        setItem = true;

        QWidget *prevWidget = mStack->currentWidget();
        mStack->removeWidget( prevWidget );
        delete prevWidget;

        mStack->addWidget( mItemWidget );
        connect( mItemWidget, &QgsAnnotationItemBaseWidget::itemChanged, this, &QgsAnnotationItemPropertiesWidget::onChanged );
        mItemWidget->setDockMode( dockMode() );
        connect( mItemWidget, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
      }
    }
  }

  if ( !setItem )
  {
    // show a "no item" widget
  }
}

//
// QgsAnnotationItemPropertiesWidgetFactory
//

QgsAnnotationItemPropertiesWidgetFactory::QgsAnnotationItemPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ) );
  setTitle( tr( "Annotation" ) );
}

QgsMapLayerConfigWidget *QgsAnnotationItemPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsAnnotationItemPropertiesWidget( qobject_cast< QgsAnnotationLayer * >( layer ), canvas, parent );
}

bool QgsAnnotationItemPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return false;
}

bool QgsAnnotationItemPropertiesWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsAnnotationItemPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::AnnotationLayer;
}

