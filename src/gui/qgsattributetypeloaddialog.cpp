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
    : QDialog()
    , mLayer( vl )
{
  setupUi( this );

  connect( layerComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( fillComboBoxes( int ) ) );
  connect( keyComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( createPreview( int ) ) );
  connect( valueComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( createPreview( int ) ) );
  connect( previewButton, SIGNAL( pressed() ), this, SLOT( previewButtonPushed() ) );

  fillLayerList();

  keyComboBox->setDisabled( true );
  valueComboBox->setDisabled( true );
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
  createPreview( valueComboBox->currentIndex(), true );
}

void QgsAttributeTypeLoadDialog::fillLayerList()
{
  layerComboBox->blockSignals( true );
  layerComboBox->clear();
  Q_FOREACH ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( l );
    if ( vl )
      layerComboBox->addItem( vl->name(), vl->id() );
  }
  layerComboBox->setCurrentIndex( -1 );
  layerComboBox->blockSignals( false );
}

void QgsAttributeTypeLoadDialog::fillComboBoxes( int layerIndex )
{
  keyComboBox->blockSignals( true );
  valueComboBox->blockSignals( true );

  //clear comboboxes first
  keyComboBox->clear();
  valueComboBox->clear();

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layerIndex < 0 ? 0 : QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->itemData( layerIndex ).toString() ) );
  if ( vLayer )
  {
    QMap<QString, int> fieldMap = vLayer->dataProvider()->fieldNameMap();
    QMap<QString, int>::iterator it = fieldMap.begin();
    for ( ; it != fieldMap.end(); ++it )
    {
      keyComboBox->addItem( it.key(), it.value() );
      valueComboBox->addItem( it.key(), it.value() );
    }
  }

  keyComboBox->setEnabled( vLayer != 0 );
  valueComboBox->setEnabled( vLayer != 0 );

  keyComboBox->setCurrentIndex( -1 );
  valueComboBox->setCurrentIndex( -1 );

  keyComboBox->blockSignals( false );
  valueComboBox->blockSignals( false );
}

void QgsAttributeTypeLoadDialog::createPreview( int fieldIndex, bool full )
{
  previewTableWidget->clearContents();

  for ( int i = previewTableWidget->rowCount() - 1; i > 0; i-- )
  {
    previewTableWidget->removeRow( i );
  }
  if ( layerComboBox->currentIndex() < 0 || fieldIndex < 0 )
  {
    //when nothing is selected there is no reason for preview
    return;
  }
  int idx = keyComboBox->itemData( keyComboBox->currentIndex() ).toInt();
  int idx2 = valueComboBox->itemData( valueComboBox->currentIndex() ).toInt();
  QgsMapLayer* dataLayer = QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->itemData( layerComboBox->currentIndex() ).toString() );
  QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer *>( dataLayer );
  if ( !vLayer )
    return;

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );

  QgsFeatureIterator fit = vLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( attributeList ) );

  QgsFeature f;
  QMap<QString, QVariant> valueMap;
  while ( fit.nextFeature( f ) )
  {
    QVariant val1 = f.attribute( idx );
    QVariant val2 = f.attribute( idx2 );
    if ( val1.isValid() && !val1.isNull() && !val1.toString().isEmpty()
         && val2.isValid() && !val2.isNull() && !val2.toString().isEmpty() )
    {
      valueMap.insert( val1.toString(), val2.toString() );
    }
    if ( !full && valueMap.size() > 8 )
      break; //just first entries all on button
  }
  int row = 0;
  for ( QMap<QString, QVariant>::iterator mit = valueMap.begin(); mit != valueMap.end(); ++mit, row++ )
  {
    previewTableWidget->insertRow( row );
    previewTableWidget->setItem( row, 0, new QTableWidgetItem( mit.value().toString() ) );
    previewTableWidget->setItem( row, 1, new QTableWidgetItem( mit.key() ) );
  }
}

QMap<QString, QVariant> &QgsAttributeTypeLoadDialog::valueMap()
{
  return mValueMap;
}

bool QgsAttributeTypeLoadDialog::insertNull()
{
  return nullCheckBox->isChecked();
}

void QgsAttributeTypeLoadDialog::loadDataToValueMap()
{
  mValueMap.clear();
  int idx = keyComboBox->itemData( keyComboBox->currentIndex() ).toInt();
  int idx2 = valueComboBox->itemData( valueComboBox->currentIndex() ).toInt();
  QgsMapLayer* dataLayer = QgsMapLayerRegistry::instance()->mapLayer( layerComboBox->itemData( layerComboBox->currentIndex() ).toString() );
  QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer *>( dataLayer );
  if ( !vLayer )
    return;

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );

  QgsFeatureIterator fit = vLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( attributeList ) );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    QVariant val = f.attribute( idx );
    if ( val.isValid() && !val.isNull() && !val.toString().isEmpty() )
    {
      mValueMap.insert( f.attribute( idx2 ).toString(), val );
    }
  }
}

void QgsAttributeTypeLoadDialog::accept()
{
  //store data to output variable
  loadDataToValueMap();
  QDialog::accept();
}
