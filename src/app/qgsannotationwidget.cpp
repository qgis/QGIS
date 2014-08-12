/***************************************************************************
                              qgsannotationwidget.cpp
                              ------------------------
  begin                : February 25, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationwidget.h"
#include "qgsannotationitem.h"
#include "qgsstylev2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"
#include "qgssymbolv2selectordialog.h"
#include <QColorDialog>


QgsAnnotationWidget::QgsAnnotationWidget( QgsAnnotationItem* item, QWidget * parent, Qt::WindowFlags f ): QWidget( parent, f ), mItem( item ), mMarkerSymbol( 0 )
{
  setupUi( this );

  if ( mItem )
  {
    blockAllSignals( true );

    if ( mItem->mapPositionFixed() )
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Unchecked );
    }
    mFrameWidthSpinBox->setValue( mItem->frameBorderWidth() );
    mFrameColorButton->setColor( mItem->frameColor() );
    mFrameColorButton->setColorDialogTitle( tr( "Select frame color" ) );
    mFrameColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
    mFrameColorButton->setContext( "symbology" );
    mFrameColorButton->setNoColorString( tr( "Transparent frame" ) );
    mFrameColorButton->setShowNoColor( true );
    mBackgroundColorButton->setColor( mItem->frameBackgroundColor() );
    mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
    mBackgroundColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
    mBackgroundColorButton->setContext( "symbology" );
    mBackgroundColorButton->setNoColorString( tr( "Transparent" ) );
    mBackgroundColorButton->setShowNoColor( true );

    const QgsMarkerSymbolV2* symbol = mItem->markerSymbol();
    if ( symbol )
    {
      mMarkerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( symbol->clone() );
      updateCenterIcon();
    }

    blockAllSignals( false );
  }
}

QgsAnnotationWidget::~QgsAnnotationWidget()
{
  delete mMarkerSymbol;
}

void QgsAnnotationWidget::apply()
{
  if ( mItem )
  {
    mItem->setMapPositionFixed( mMapPositionFixedCheckBox->checkState() == Qt::Checked );
    mItem->setFrameBorderWidth( mFrameWidthSpinBox->value() );
    mItem->setFrameColor( mFrameColorButton->color() );
    mItem->setFrameBackgroundColor( mBackgroundColorButton->color() );
    mItem->setMarkerSymbol( mMarkerSymbol );
    mMarkerSymbol = 0; //item takes ownership
    mItem->update();
  }
}

void QgsAnnotationWidget::blockAllSignals( bool block )
{
  mMapPositionFixedCheckBox->blockSignals( block );
  mMapMarkerButton->blockSignals( block );
  mFrameWidthSpinBox->blockSignals( block );
  mFrameColorButton->blockSignals( block );
}

void QgsAnnotationWidget::on_mMapMarkerButton_clicked()
{
  if ( !mMarkerSymbol )
  {
    return;
  }
  QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( mMarkerSymbol->clone() );
  QgsSymbolV2SelectorDialog dlg( markerSymbol, QgsStyleV2::defaultStyle(), 0, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete markerSymbol;
  }
  else
  {
    delete mMarkerSymbol;
    mMarkerSymbol = markerSymbol;
    updateCenterIcon();
  }
}

void QgsAnnotationWidget::on_mFrameColorButton_colorChanged( const QColor &color )
{
  if ( !mItem )
  {
    return;
  }

  mItem->setFrameColor( color );
}

void QgsAnnotationWidget::updateCenterIcon()
{
  if ( !mMarkerSymbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mMarkerSymbol, mMapMarkerButton->iconSize() );
  mMapMarkerButton->setIcon( icon );
}

void QgsAnnotationWidget::on_mBackgroundColorButton_colorChanged( const QColor &color )
{
  if ( !mItem )
  {
    return;
  }

  mItem->setFrameBackgroundColor( color );
}

