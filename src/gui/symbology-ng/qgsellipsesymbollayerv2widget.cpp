/***************************************************************************
    qgsellipsesymbollayerv2widget.cpp
    ---------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>

QgsEllipseSymbolLayerV2Widget::QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  setupUi( this );
  QStringList names;
  names << "circle" << "rectangle" << "cross" << "triangle";
  QSize iconSize = mShapeListWidget->iconSize();

  QStringList::const_iterator nameIt = names.constBegin();
  for ( ; nameIt != names.constEnd(); ++nameIt )
  {
    QgsEllipseSymbolLayerV2* lyr = new QgsEllipseSymbolLayerV2();
    lyr->setSymbolName( *nameIt );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth( 4 );
    lyr->setSymbolHeight( 2 );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, iconSize );
    QListWidgetItem* item = new QListWidgetItem( icon, "", mShapeListWidget );
    item->setToolTip( *nameIt );
    item->setData( Qt::UserRole, *nameIt );
    delete lyr;
  }

  blockComboSignals( true );
  fillDataDefinedComboBoxes();
  blockComboSignals( false );
}

void QgsEllipseSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "EllipseMarker" )
  {
    return;
  }

  mLayer = static_cast<QgsEllipseSymbolLayerV2*>( layer );
  mWidthSpinBox->setValue( mLayer->symbolWidth() );
  mHeightSpinBox->setValue( mLayer->symbolHeight() );
  mRotationSpinBox->setValue( mLayer->angle() );
  mOutlineWidthSpinBox->setValue( mLayer->outlineWidth() );

  QList<QListWidgetItem *> symbolItemList = mShapeListWidget->findItems( mLayer->symbolName(), Qt::MatchExactly );
  if ( symbolItemList.size() > 0 )
  {
    mShapeListWidget->setCurrentItem( symbolItemList.at( 0 ) );
  }

  //set combo entries to current values
  blockComboSignals( true );
  if ( mLayer )
  {

    if ( mLayer->widthField().isEmpty() )
    {
      mDDSymbolWidthComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDSymbolWidthComboBox->setCurrentIndex( mDDSymbolWidthComboBox->findText( mLayer->widthField() ) );
    }
    if ( mLayer->heightField().isEmpty() )
    {
      mDDSymbolHeightComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDSymbolHeightComboBox->setCurrentIndex( mDDSymbolHeightComboBox->findText( mLayer->heightField() ) );
    }
    if ( mLayer->rotationField().isEmpty() )
    {
      mDDRotationComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDRotationComboBox->setCurrentIndex( mDDRotationComboBox->findText( mLayer->rotationField() ) );
    }
    if ( mLayer->outlineWidthField().isEmpty() )
    {
      mDDOutlineWidthComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDOutlineWidthComboBox->setCurrentIndex( mDDOutlineWidthComboBox->findText( mLayer->outlineWidthField() ) );
    }
    if ( mLayer->fillColorField().isEmpty() )
    {
      mDDFillColorComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDFillColorComboBox->setCurrentIndex( mDDFillColorComboBox->findText( mLayer->fillColorField() ) );
    }
    if ( mLayer->outlineColorField().isEmpty() )
    {
      mDDOutlineColorComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDOutlineColorComboBox->setCurrentIndex( mDDOutlineColorComboBox->findText( mLayer->outlineColorField() ) );
    }
    if ( mLayer->symbolNameField().isEmpty() )
    {
      mDDShapeComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDDShapeComboBox->setCurrentIndex( mDDShapeComboBox->findText( mLayer->symbolNameField() ) );
    }
  }
  blockComboSignals( false );
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsEllipseSymbolLayerV2Widget::on_mShapeListWidget_itemSelectionChanged()
{
  if ( mLayer )
  {
    QListWidgetItem* item = mShapeListWidget->currentItem();
    if ( item )
    {
      mLayer->setSymbolName( item->data( Qt::UserRole ).toString() );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mHeightSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolHeight( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorBorder_clicked()
{
  if ( mLayer )
  {
    QColor newColor = QColorDialog::getColor( mLayer->outlineColor() );
    if ( newColor.isValid() )
    {
      mLayer->setOutlineColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorFill_clicked()
{
  if ( mLayer )
  {
    QColor newColor = QColorDialog::getColor( mLayer->fillColor() );
    if ( newColor.isValid() )
    {
      mLayer->setFillColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::fillDataDefinedComboBoxes()
{
  mDDSymbolWidthComboBox->clear();
  mDDSymbolWidthComboBox->addItem( "", -1 );
  mDDSymbolHeightComboBox->clear();
  mDDSymbolHeightComboBox->addItem( "", -1 );
  mDDRotationComboBox->clear();
  mDDRotationComboBox->addItem( "", -1 );
  mDDOutlineWidthComboBox->clear();
  mDDOutlineWidthComboBox->addItem( "", -1 );
  mDDFillColorComboBox->clear();
  mDDFillColorComboBox->addItem( "", -1 );
  mDDOutlineColorComboBox->clear();
  mDDOutlineColorComboBox->addItem( "", -1 );
  mDDShapeComboBox->clear();
  mDDShapeComboBox->addItem( "", -1 );

  if ( mVectorLayer )
  {
    const QgsFieldMap& fm = mVectorLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fm.constBegin();
    for ( ; fieldIt != fm.constEnd(); ++fieldIt )
    {
      QString fieldName = fieldIt.value().name();
      int index = fieldIt.key();

      mDDSymbolWidthComboBox->addItem( fieldName, index );
      mDDSymbolHeightComboBox->addItem( fieldName, index );
      mDDRotationComboBox->addItem( fieldName, index );
      mDDOutlineWidthComboBox->addItem( fieldName, index );
      mDDFillColorComboBox->addItem( fieldName, index );
      mDDOutlineColorComboBox->addItem( fieldName, index );
      mDDShapeComboBox->addItem( fieldName, index );
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDSymbolWidthComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setWidthField( mDDSymbolWidthComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDSymbolHeightComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setHeightField( mDDSymbolHeightComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDRotationComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setRotationField( mDDRotationComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDOutlineWidthComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthField( mDDOutlineWidthComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDFillColorComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setFillColorField( mDDFillColorComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDOutlineColorComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setOutlineColorField( mDDOutlineColorComboBox->itemText( idx ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mDDShapeComboBox_currentIndexChanged( int idx )
{
  if ( mLayer )
  {
    mLayer->setSymbolNameField( mDDShapeComboBox->itemText( idx ) );
  }
}

void QgsEllipseSymbolLayerV2Widget::blockComboSignals( bool block )
{
  mDDSymbolWidthComboBox->blockSignals( block );
  mDDSymbolHeightComboBox->blockSignals( block );
  mDDRotationComboBox->blockSignals( block );
  mDDOutlineWidthComboBox->blockSignals( block );
  mDDFillColorComboBox->blockSignals( block );
  mDDOutlineColorComboBox->blockSignals( block );
  mDDShapeComboBox->blockSignals( block );
}
