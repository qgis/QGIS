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
#include "qgsdatadefinedsymboldialog.h"
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

  btnChangeColorBorder->setColor( mLayer->outlineColor() );
  btnChangeColorFill->setColor( mLayer->fillColor() );

  QList<QListWidgetItem *> symbolItemList = mShapeListWidget->findItems( mLayer->symbolName(), Qt::MatchExactly );
  if ( symbolItemList.size() > 0 )
  {
    mShapeListWidget->setCurrentItem( symbolItemList.at( 0 ) );
  }

  //set combo entries to current values
  blockComboSignals( true );
  if ( mLayer )
  {
    mSymbolWidthUnitComboBox->setCurrentIndex( mLayer->symbolWidthUnit() );
    mOutlineWidthUnitComboBox->setCurrentIndex( mLayer->outlineWidthUnit() );
    mSymbolHeightUnitComboBox->setCurrentIndex( mLayer->symbolHeightUnit() );
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
    QColor newColor = QColorDialog::getColor( mLayer->outlineColor(), this, "", QColorDialog::ShowAlphaChannel );
    if ( newColor.isValid() )
    {
      mLayer->setOutlineColor( newColor );
      btnChangeColorBorder->setColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorFill_clicked()
{
  if ( mLayer )
  {
    QColor newColor = QColorDialog::getColor( mLayer->fillColor(), this, "", QColorDialog::ShowAlphaChannel );
    if ( newColor.isValid() )
    {
      mLayer->setFillColor( newColor );
      btnChangeColorFill->setColor( newColor );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mSymbolWidthUnitComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setSymbolWidthUnit(( QgsSymbolV2::OutputUnit ) index );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthUnitComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit(( QgsSymbolV2::OutputUnit ) index );
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mSymbolHeightUnitComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setSymbolHeightUnit(( QgsSymbolV2::OutputUnit ) index );
  }
}

void QgsEllipseSymbolLayerV2Widget::blockComboSignals( bool block )
{
  mSymbolWidthUnitComboBox->blockSignals( block );
  mOutlineWidthUnitComboBox->blockSignals( block );
  mSymbolHeightUnitComboBox->blockSignals( block );
}

void QgsEllipseSymbolLayerV2Widget::on_mDataDefinedPropertiesButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QMap<QString, QPair< QString, QString> > dataDefinedProperties;
  dataDefinedProperties.insert( "width", qMakePair( tr( "Symbol width" ), mLayer->dataDefinedPropertyString( "width" ) ) );
  dataDefinedProperties.insert( "height", qMakePair( tr( "Symbol height" ), mLayer->dataDefinedPropertyString( "height" ) ) );
  dataDefinedProperties.insert( "rotation", qMakePair( tr( "Rotation" ), mLayer->dataDefinedPropertyString( "rotation" ) ) );
  dataDefinedProperties.insert( "outline_width", qMakePair( tr( "Outline width" ), mLayer->dataDefinedPropertyString( "outline_width" ) ) );
  dataDefinedProperties.insert( "fill_color", qMakePair( tr( "Fill color" ), mLayer->dataDefinedPropertyString( "fill_color" ) ) );
  dataDefinedProperties.insert( "outline_color", qMakePair( tr( "Border color" ), mLayer->dataDefinedPropertyString( "outline_color" ) ) );
  dataDefinedProperties.insert( "symbol_name", qMakePair( tr( "Symbol name" ), mLayer->dataDefinedPropertyString( "symbol_name" ) ) );

  QgsDataDefinedSymbolDialog d( dataDefinedProperties, mVectorLayer );
  if ( d.exec() == QDialog::Accepted )
  {
    //empty all existing properties first
    mLayer->removeDataDefinedProperties();

    QMap<QString, QString> properties = d.dataDefinedProperties();
    QMap<QString, QString>::const_iterator it = properties.constBegin();
    for ( ; it != properties.constEnd(); ++it )
    {
      if ( !it.value().isEmpty() )
      {
        mLayer->setDataDefinedProperty( it.key(), it.value() );
      }
    }
    emit changed();
  }
}
