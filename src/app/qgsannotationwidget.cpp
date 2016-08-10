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
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include <QColorDialog>


QgsAnnotationWidget::QgsAnnotationWidget( QgsAnnotationItem* item, QWidget * parent, Qt::WindowFlags f ): QWidget( parent, f ), mItem( item ), mMarkerSymbol( nullptr )
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
    mFrameColorButton->setAllowAlpha( true );
    mFrameColorButton->setContext( "symbology" );
    mFrameColorButton->setNoColorString( tr( "Transparent frame" ) );
    mFrameColorButton->setShowNoColor( true );
    mBackgroundColorButton->setColor( mItem->frameBackgroundColor() );
    mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
    mBackgroundColorButton->setAllowAlpha( true );
    mBackgroundColorButton->setContext( "symbology" );
    mBackgroundColorButton->setNoColorString( tr( "Transparent" ) );
    mBackgroundColorButton->setShowNoColor( true );

    connect( mBackgroundColorButton, SIGNAL( colorChanged( QColor ) ), this, SIGNAL( backgroundColorChanged( QColor ) ) );

    const QgsMarkerSymbol* symbol = mItem->markerSymbol();
    if ( symbol )
    {
      mMarkerSymbol.reset( symbol->clone() );
      updateCenterIcon();
    }

    blockAllSignals( false );
  }
}

QgsAnnotationWidget::~QgsAnnotationWidget()
{
}

void QgsAnnotationWidget::apply()
{
  if ( mItem )
  {
    mItem->setMapPositionFixed( mMapPositionFixedCheckBox->checkState() == Qt::Checked );
    mItem->setFrameBorderWidth( mFrameWidthSpinBox->value() );
    mItem->setFrameColor( mFrameColorButton->color() );
    mItem->setFrameBackgroundColor( mBackgroundColorButton->color() );
    mItem->setMarkerSymbol( mMarkerSymbol->clone() );
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
  QgsMarkerSymbol* markerSymbol = mMarkerSymbol->clone();
  QgsSymbolSelectorDialog dlg( markerSymbol, QgsStyle::defaultStyle(), nullptr, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete markerSymbol;
  }
  else
  {
    mMarkerSymbol.reset( markerSymbol );
    updateCenterIcon();
  }
}

void QgsAnnotationWidget::updateCenterIcon()
{
  if ( !mMarkerSymbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mMarkerSymbol.data(), mMapMarkerButton->iconSize() );
  mMapMarkerButton->setIcon( icon );
}

