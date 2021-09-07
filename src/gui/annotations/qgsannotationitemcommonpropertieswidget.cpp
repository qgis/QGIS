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
#include "qgsannotationitem.h"

QgsAnnotationItemCommonPropertiesWidget::QgsAnnotationItemCommonPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mSpinZIndex, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  mSpinZIndex->setClearValue( 0 );

  connect( mReferenceScaleGroup, &QGroupBox::toggled, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mReferenceScaleWidget, &QgsScaleWidget::scaleChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

void QgsAnnotationItemCommonPropertiesWidget::setItem( QgsAnnotationItem *item )
{
  mSpinZIndex->setValue( item->zIndex() );
  mReferenceScaleGroup->setChecked( item->useSymbologyReferenceScale() );
  mReferenceScaleWidget->setScale( item->symbologyReferenceScale() );
}

void QgsAnnotationItemCommonPropertiesWidget::updateItem( QgsAnnotationItem *item )
{
  item->setZIndex( mSpinZIndex->value() );
  item->setUseSymbologyReferenceScale( mReferenceScaleGroup->isChecked() );
  item->setSymbologyReferenceScale( mReferenceScaleWidget->scale() );
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
