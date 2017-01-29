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
#include "qgsmapcanvasannotationitem.h"
#include "qgsannotation.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include <QColorDialog>


QgsAnnotationWidget::QgsAnnotationWidget( QgsMapCanvasAnnotationItem* item, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , mItem( item )
    , mMarkerSymbol( nullptr )
{
  setupUi( this );
  mLayerComboBox->setAllowEmptyLayer( true );

  if ( mItem && mItem->annotation() )
  {
    QgsAnnotation* annotation = mItem->annotation();
    blockAllSignals( true );

    if ( annotation->hasFixedMapPosition() )
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Unchecked );
    }
    mFrameWidthSpinBox->setValue( annotation->frameBorderWidth() );
    mFrameColorButton->setColor( annotation->frameColor() );
    mFrameColorButton->setColorDialogTitle( tr( "Select frame color" ) );
    mFrameColorButton->setAllowAlpha( true );
    mFrameColorButton->setContext( QStringLiteral( "symbology" ) );
    mFrameColorButton->setNoColorString( tr( "Transparent frame" ) );
    mFrameColorButton->setShowNoColor( true );
    mBackgroundColorButton->setColor( annotation->frameBackgroundColor() );
    mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
    mBackgroundColorButton->setAllowAlpha( true );
    mBackgroundColorButton->setContext( QStringLiteral( "symbology" ) );
    mBackgroundColorButton->setNoColorString( tr( "Transparent" ) );
    mBackgroundColorButton->setShowNoColor( true );

    mLayerComboBox->setLayer( annotation->mapLayer() );

    connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, &QgsAnnotationWidget::backgroundColorChanged );

    const QgsMarkerSymbol* symbol = annotation->markerSymbol();
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
    QgsAnnotation* annotation = mItem->annotation();
    if ( annotation )
    {
      annotation->setHasFixedMapPosition( mMapPositionFixedCheckBox->checkState() == Qt::Checked );
      annotation->setFrameBorderWidth( mFrameWidthSpinBox->value() );
      annotation->setFrameColor( mFrameColorButton->color() );
      annotation->setFrameBackgroundColor( mBackgroundColorButton->color() );
      annotation->setMarkerSymbol( mMarkerSymbol->clone() );
      annotation->setMapLayer( mLayerComboBox->currentLayer() );
    }
    mItem->update();
  }
}

void QgsAnnotationWidget::blockAllSignals( bool block )
{
  mMapPositionFixedCheckBox->blockSignals( block );
  mMapMarkerButton->blockSignals( block );
  mFrameWidthSpinBox->blockSignals( block );
  mFrameColorButton->blockSignals( block );
  mLayerComboBox->blockSignals( block );
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

