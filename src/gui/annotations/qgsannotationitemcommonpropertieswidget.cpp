/***************************************************************************
                    qgsannotationitemcommonpropertieswidget.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitemcommonpropertieswidget.h"
#include "moc_qgsannotationitemcommonpropertieswidget.cpp"
#include "qgsannotationitem.h"
#include "qgscalloutpanelwidget.h"
#include "qgsapplication.h"
#include "qgscalloutsregistry.h"

QgsAnnotationItemCommonPropertiesWidget::QgsAnnotationItemCommonPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mSpinZIndex, qOverload<int>( &QSpinBox::valueChanged ), this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  mSpinZIndex->setClearValue( 0 );

  connect( mReferenceScaleGroup, &QGroupBox::toggled, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mReferenceScaleWidget, &QgsScaleWidget::scaleChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mCalloutCheckBox, &QCheckBox::toggled, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mCalloutPropertiesButton, &QToolButton::clicked, this, &QgsAnnotationItemCommonPropertiesWidget::openCalloutProperties );
}

QgsAnnotationItemCommonPropertiesWidget::~QgsAnnotationItemCommonPropertiesWidget() = default;

void QgsAnnotationItemCommonPropertiesWidget::setItem( QgsAnnotationItem *item )
{
  mSpinZIndex->setValue( item->zIndex() );
  mReferenceScaleGroup->setChecked( item->useSymbologyReferenceScale() );
  mReferenceScaleWidget->setScale( item->symbologyReferenceScale() );
  mReferenceScaleGroup->setVisible( item->flags() & Qgis::AnnotationItemFlag::SupportsReferenceScale );
  mCalloutCheckBox->setChecked( item->callout() );
  mCallout.reset( item->callout() ? item->callout()->clone() : nullptr );
}

void QgsAnnotationItemCommonPropertiesWidget::updateItem( QgsAnnotationItem *item )
{
  item->setZIndex( mSpinZIndex->value() );
  item->setUseSymbologyReferenceScale( mReferenceScaleGroup->isChecked() );
  item->setSymbologyReferenceScale( mReferenceScaleWidget->scale() );
  item->setCallout( mCallout && mCalloutCheckBox->isChecked() ? mCallout->clone() : nullptr );
}

void QgsAnnotationItemCommonPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  mReferenceScaleWidget->setMapCanvas( context.mapCanvas() );
  if ( context.mapCanvas() )
    mReferenceScaleWidget->setShowCurrentScaleButton( true );
}

QgsSymbolWidgetContext QgsAnnotationItemCommonPropertiesWidget::context() const
{
  return mContext;
}

void QgsAnnotationItemCommonPropertiesWidget::openCalloutProperties()
{
  QgsCalloutPanelWidget *widget = new QgsCalloutPanelWidget();
  if ( !mCallout )
    mCallout.reset( QgsApplication::calloutRegistry()->defaultCallout() );
  widget->setCallout( mCallout.get() );

  connect( widget, &QgsCalloutPanelWidget::calloutChanged, this, [this, widget] {
    mCallout.reset( widget->callout()->clone() );
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    panel->openPanel( widget );
  }
}
