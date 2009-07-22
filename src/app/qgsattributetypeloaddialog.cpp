/***************************************************************************
                         qgsattributetypeloaddialog.cpp
                             -------------------
    begin                : June 2009
    copyright            : (C) 2000 by Richard Kostecky
    email                : cSf.Kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsattributetypeloaddialog.h"

#include "qgsmaplayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"

#include <QTableWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QCompleter>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

QgsAttributeTypeLoadDialog::QgsAttributeTypeLoadDialog( QgsVectorLayer *vl )
    : QDialog(),
    mLayer( vl )
{
  setupUi( this );

  connect( layerComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( fillComboBoxes( int ) ) );
  connect( keyComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( createPreview( int ) ) );
  connect( valueComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( createPreview( int ) ) );
  connect( previewButton, SIGNAL( pressed( ) ), this, SLOT( previewButtonPushed( ) ) );

  fillLayerList();
}


QgsAttributeTypeLoadDialog::~QgsAttributeTypeLoadDialog()
{

}


void QgsAttributeTypeLoadDialog::setVectorLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}



void QgsAttributeTypeLoadDialog::previewButtonPushed()
{
  createPreview( valueComboBox->currentIndex(), true);
}

void QgsAttributeTypeLoadDialog::fillLayerList()
{
  layerComboBox->clear();
  int i = 0;
  QgsMapLayer* dataLayer;
  QMap<QString, QgsMapLayer*>::iterator layer_it = QgsMapLayerRegistry::instance()->mapLayers().begin();
  for ( ; layer_it != QgsMapLayerRegistry::instance()->mapLayers().end(); layer_it++ )
  {
    layerComboBox->addItem( layer_it.key() );
  }
}

void QgsAttributeTypeLoadDialog::fillComboBoxes( int layerIndex )
{
  //clear comboboxes first
  keyComboBox->clear();
  valueComboBox->clear();

  if (layerIndex < 0)
  {
    return;
  }

  QgsMapLayer* dataLayer = QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->currentText() );
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer *>( dataLayer );
  if (vLayer == NULL)
  {
      return;
  }
  QMap<QString, int> fieldMap = vLayer->dataProvider()->fieldNameMap();
  QMap<QString, int>::iterator it = fieldMap.begin();
  for (; it != fieldMap.end(); it++)
  {
    keyComboBox->addItem(it.key(), it.value());
    valueComboBox->addItem(it.key(), it.value());
  }

}

void QgsAttributeTypeLoadDialog::createPreview( int fieldIndex, bool full)
{
  previewTableWidget->clearContents();

  for (int i = previewTableWidget->rowCount() -1; i > 0; i--)
  {
    previewTableWidget->removeRow(i);
  }
  if (layerComboBox->currentIndex() < 0 || fieldIndex < 0 )
  {
    //when nothing is selected there is no reason for preview
    return;
  }
  int idx = keyComboBox->itemData(keyComboBox->currentIndex()).toInt();
  int idx2 = valueComboBox->itemData(valueComboBox->currentIndex()).toInt();
  QgsMapLayer* dataLayer = QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->currentText() );
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer *>( dataLayer );
  if (vLayer == NULL)
  {
      return;
  }

  QgsVectorDataProvider* dataProvider = vLayer->dataProvider();
  dataProvider->enableGeometrylessFeatures( true );

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );
  vLayer->select( attributeList, QgsRectangle(), false );

  QgsFeature f;
  QMap<QString, QVariant> valueMap;
  while ( vLayer->nextFeature( f ) )
  {
    QVariant val1 = f.attributeMap()[idx];
    QVariant val2 = f.attributeMap()[idx2];
    if ( val1.isValid() && !val1.isNull() && !val1.toString().isEmpty()
      && val2.isValid() && !val2.isNull() && !val2.toString().isEmpty() )
    {
      valueMap.insert(val1.toString(), val2.toString() );
    }
    if (!full && valueMap.size() > 8)
        break; //just first entries all on button
  }
  int row = 0;
  for ( QMap<QString, QVariant>::iterator mit = valueMap.begin(); mit != valueMap.end(); mit++, row++ )
  {
    previewTableWidget->insertRow( row );
    previewTableWidget->setItem( row, 0, new QTableWidgetItem( mit.value().toString() ) );
    previewTableWidget->setItem( row, 1, new QTableWidgetItem( mit.key() ) );
  }

  dataProvider->enableGeometrylessFeatures( false );
}

QMap<QString, QVariant> &QgsAttributeTypeLoadDialog::valueMap()
{
  return mValueMap;
}

void QgsAttributeTypeLoadDialog::loadDataToValueMap()
{
  mValueMap.clear();
  int idx = keyComboBox->itemData(keyComboBox->currentIndex()).toInt();
  int idx2 = valueComboBox->itemData(valueComboBox->currentIndex()).toInt();
  QgsMapLayer* dataLayer = QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->currentText() );
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer *>( dataLayer );
  if (vLayer == NULL)
  {
      return;
  }

  QgsVectorDataProvider* dataProvider = vLayer->dataProvider();
  dataProvider->enableGeometrylessFeatures( true );

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );
  vLayer->select( attributeList, QgsRectangle(), false );

  QgsFeature f;
  while ( vLayer->nextFeature( f ) )
  {
    QVariant val = f.attributeMap()[idx];
    if ( val.isValid() && !val.isNull() && !val.toString().isEmpty() )
    {
      mValueMap.insert(f.attributeMap()[idx2].toString(), val );
    }
  }
  dataProvider->enableGeometrylessFeatures( false );
}



void QgsAttributeTypeLoadDialog::accept()
{
  //store data to output variable
  loadDataToValueMap();
  QDialog::accept();
}

