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
#include "moc_qgsattributetypeloaddialog.cpp"

#include "qgsmaplayer.h"
#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QTableWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QCompleter>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

QgsAttributeTypeLoadDialog::QgsAttributeTypeLoadDialog( QgsVectorLayer *vl )
  : mLayer( vl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  layerComboBox->setFilters( Qgis::LayerFilter::VectorLayer );
  layerComboBox->setCurrentIndex( -1 );

  connect( layerComboBox, &QgsMapLayerComboBox::layerChanged, keyComboBox, &QgsFieldComboBox::setLayer );
  connect( layerComboBox, &QgsMapLayerComboBox::layerChanged, valueComboBox, &QgsFieldComboBox::setLayer );
  connect( keyComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [=]( int index ) { createPreview( index ); } );
  connect( valueComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [=]( int index ) { createPreview( index ); } );
  connect( previewButton, &QAbstractButton::pressed, this, &QgsAttributeTypeLoadDialog::previewButtonPushed );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#edit-widgets" ) );
  } );
}

void QgsAttributeTypeLoadDialog::setVectorLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsAttributeTypeLoadDialog::previewButtonPushed()
{
  createPreview( valueComboBox->currentIndex(), true );
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
  const int idx = keyComboBox->currentIndex();
  const int idx2 = valueComboBox->currentIndex();
  QgsMapLayer *dataLayer = layerComboBox->currentLayer();
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( dataLayer );
  if ( !vLayer )
    return;

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );

  QgsFeatureIterator fit = vLayer->getFeatures( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( attributeList ) );

  QgsFeature f;
  QMap<QString, QVariant> valueMap;
  while ( fit.nextFeature( f ) )
  {
    const QVariant val1 = f.attribute( idx );
    const QVariant val2 = f.attribute( idx2 );
    if ( val1.isValid() && !QgsVariantUtils::isNull( val1 ) && !val1.toString().isEmpty()
         && val2.isValid() && !QgsVariantUtils::isNull( val2 ) && !val2.toString().isEmpty() )
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
  const int idx = keyComboBox->currentIndex();
  const int idx2 = valueComboBox->currentIndex();
  QgsMapLayer *dataLayer = layerComboBox->currentLayer();
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( dataLayer );
  if ( !vLayer )
    return;

  QgsAttributeList attributeList = QgsAttributeList();
  attributeList.append( idx );
  attributeList.append( idx2 );

  QgsFeatureIterator fit = vLayer->getFeatures( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( attributeList ) );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    const QVariant val = f.attribute( idx );
    if ( val.isValid() && !QgsVariantUtils::isNull( val ) && !val.toString().isEmpty() )
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
