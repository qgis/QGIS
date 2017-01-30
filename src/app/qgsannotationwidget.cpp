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

    whileBlocking( mSpinTopMargin )->setValue( annotation->contentsMargin().top() );
    whileBlocking( mSpinLeftMargin )->setValue( annotation->contentsMargin().left() );
    whileBlocking( mSpinRightMargin )->setValue( annotation->contentsMargin().right() );
    whileBlocking( mSpinBottomMargin )->setValue( annotation->contentsMargin().bottom() );

    mLayerComboBox->setLayer( annotation->mapLayer() );

    const QgsMarkerSymbol* symbol = annotation->markerSymbol();
    if ( symbol )
    {
      mMarkerSymbol.reset( symbol->clone() );
      updateCenterIcon();
    }
    const QgsFillSymbol* fill = annotation->fillSymbol();
    if ( fill )
    {
      mFillSymbol.reset( fill->clone() );
      updateFillIcon();
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
      annotation->setFillSymbol( mFillSymbol->clone() );
      annotation->setMarkerSymbol( mMarkerSymbol->clone() );
      annotation->setMapLayer( mLayerComboBox->currentLayer() );
      annotation->setContentsMargin( QgsMargins( mSpinLeftMargin->value(),
                                     mSpinTopMargin->value(),
                                     mSpinRightMargin->value(),
                                     mSpinBottomMargin->value() ) );
    }
    mItem->update();
  }
}

void QgsAnnotationWidget::blockAllSignals( bool block )
{
  mMapPositionFixedCheckBox->blockSignals( block );
  mMapMarkerButton->blockSignals( block );
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

void QgsAnnotationWidget::on_mFrameStyleButton_clicked()
{
  if ( !mFillSymbol )
  {
    return;
  }
  QgsFillSymbol* fillSymbol = mFillSymbol->clone();
  QgsSymbolSelectorDialog dlg( fillSymbol, QgsStyle::defaultStyle(), nullptr, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete fillSymbol;
  }
  else
  {
    mFillSymbol.reset( fillSymbol );
    updateFillIcon();
    backgroundColorChanged( fillSymbol->color() );
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

void QgsAnnotationWidget::updateFillIcon()
{
  if ( !mFillSymbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mFillSymbol.data(), mFrameStyleButton->iconSize() );
  mFrameStyleButton->setIcon( icon );
}

