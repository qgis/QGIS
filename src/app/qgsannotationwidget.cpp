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
  QgsSymbolV2SelectorDialog dlg( markerSymbol, QgsStyleV2::defaultStyle(), this );
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

void QgsAnnotationWidget::on_mFrameColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

#if QT_VERSION >= 0x040500
  QColor c = QColorDialog::getColor( mFrameColorButton->color(), 0, tr( "Select frame color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor c = QColorDialog::getColor( mFrameColorButton->color() );
#endif
  if ( c.isValid() )
  {
    mFrameColorButton->setColor( c );
    mItem->setFrameColor( c );
  }
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

