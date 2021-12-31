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
#include "qgspainteffect.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>

QgsAnnotationItemPropertiesWidget::QgsAnnotationItemPropertiesWidget( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );

  mPageNoItem = new QWidget();
  QSizePolicy sizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  sizePolicy.setHorizontalStretch( 0 );
  sizePolicy.setVerticalStretch( 0 );
  sizePolicy.setHeightForWidth( mPageNoItem->sizePolicy().hasHeightForWidth() );
  mPageNoItem->setSizePolicy( sizePolicy );
  QVBoxLayout *verticalLayout = new QVBoxLayout();
  verticalLayout->setContentsMargins( 0, 0, 0, 0 );
  QLabel *label = new QLabel();
  label->setText( tr( "No item selected." ) );
  verticalLayout->addWidget( label );
  mPageNoItem->setLayout( verticalLayout );
  mStack->addWidget( mPageNoItem );
  mStack->setCurrentWidget( mPageNoItem );

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsAnnotationItemPropertiesWidget::onLayerPropertyChanged );
  connect( mBlendModeComboBox, qOverload< int >( &QgsBlendModeComboBox::currentIndexChanged ), this, &QgsAnnotationItemPropertiesWidget::onLayerPropertyChanged );
  connect( mEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsAnnotationItemPropertiesWidget::onLayerPropertyChanged );

  setDockMode( true );

  syncToLayer( layer );
}

QgsAnnotationItemPropertiesWidget::~QgsAnnotationItemPropertiesWidget() = default;

void QgsAnnotationItemPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  if ( layer == mLayer )
    return;

  mLayer = qobject_cast< QgsAnnotationLayer * >( layer );
  if ( !mLayer )
    return;

  // opacity and blend modes
  mBlockLayerUpdates = true;
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );
  mOpacityWidget->setOpacity( mLayer->opacity() );
  if ( mLayer->paintEffect() )
  {
    mPaintEffect.reset( mLayer->paintEffect()->clone() );
    mEffectWidget->setPaintEffect( mPaintEffect.get() );
  }
  mBlockLayerUpdates = false;

  // check context
  setItemId( mMapLayerConfigWidgetContext.annotationId() );
}

void QgsAnnotationItemPropertiesWidget::setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context )
{
  QgsMapLayerConfigWidget::setMapLayerConfigWidgetContext( context );
  setItemId( context.annotationId() );

  if ( mItemWidget )
  {
    QgsSymbolWidgetContext symbolWidgetContext;
    symbolWidgetContext.setMapCanvas( context.mapCanvas() );
    symbolWidgetContext.setMessageBar( context.messageBar() );
    mItemWidget->setContext( symbolWidgetContext );
  }
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

  // set the blend mode and opacity for the layer
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  mLayer->setOpacity( mOpacityWidget->opacity() );

  if ( mPaintEffect )
    mLayer->setPaintEffect( mPaintEffect->clone() );

  mLayer->triggerRepaint();
}

void QgsAnnotationItemPropertiesWidget::focusDefaultWidget()
{
  if ( mItemWidget )
    mItemWidget->focusDefaultWidget();
}

void QgsAnnotationItemPropertiesWidget::onChanged()
{
  if ( !mLayer )
    return;

  // we refetch the item from the layer and update it, as the item's geometry (or some other property)
  // may have changed and we always want to use the current properties

  if ( QgsAnnotationItem *existingItem = mLayer->item( mMapLayerConfigWidgetContext.annotationId() ) )
  {
    std::unique_ptr< QgsAnnotationItem > newItem( existingItem->clone() );
    mItemWidget->updateItem( newItem.get() );

    mLayer->replaceItem( mMapLayerConfigWidgetContext.annotationId(), newItem.release() );
  }
}

void QgsAnnotationItemPropertiesWidget::onLayerPropertyChanged()
{
  if ( mBlockLayerUpdates )
    return;

  emit widgetChanged();
}

void QgsAnnotationItemPropertiesWidget::setItemId( const QString &itemId )
{
  if ( !mLayer )
    return;

  // try to retrieve matching item
  bool setItem = false;
  QgsAnnotationItem *item = !itemId.isEmpty() ? mLayer->item( itemId ) : nullptr;
  if ( item )
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
        if ( prevWidget != mPageNoItem )
        {
          mStack->removeWidget( prevWidget );
          delete prevWidget;
        }

        mStack->addWidget( mItemWidget );
        mStack->setCurrentWidget( mItemWidget );
        connect( mItemWidget, &QgsAnnotationItemBaseWidget::itemChanged, this, &QgsAnnotationItemPropertiesWidget::onChanged );
        mItemWidget->setDockMode( dockMode() );
        connect( mItemWidget, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

        QgsSymbolWidgetContext symbolWidgetContext;
        symbolWidgetContext.setMapCanvas( mMapLayerConfigWidgetContext.mapCanvas() );
        symbolWidgetContext.setMessageBar( mMapLayerConfigWidgetContext.messageBar() );
        mItemWidget->setContext( symbolWidgetContext );
      }
    }
  }

  if ( !setItem )
  {
    // show the "no item" widget
    QWidget *prevWidget = mStack->currentWidget();
    if ( prevWidget != mPageNoItem )
    {
      mStack->removeWidget( prevWidget );
      delete prevWidget;
    }
    mStack->setCurrentWidget( mPageNoItem );
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

