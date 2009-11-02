/***************************************************************************
                              qgssnappingdialog.cpp
                              ---------------------
  begin                : June 11, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnappingdialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include <QCheckBox>
#include <QDoubleValidator>
#include <QComboBox>
#include <QLineEdit>

QgsSnappingDialog::QgsSnappingDialog( QgsMapCanvas* canvas, const QMap<QString, LayerEntry >& settings ): mMapCanvas( canvas )
{
  setupUi( this );
  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  //an entry for each layer
  int nLayers = mMapCanvas->layerCount();
  int nVectorLayers = 0;
  //mLayerTableWidget->setRowCount(nLayers);

  QgsMapLayer* currentLayer = 0;
  QgsVectorLayer* currentVectorLayer = 0;
  QString currentLayerName;
  QMap<QString, LayerEntry >::const_iterator settingIt;
  QTreeWidgetItem* newItem = 0;

  if ( mMapCanvas )
  {
    for ( int i = 0; i < nLayers; ++i )
    {
      currentLayer = mMapCanvas->layer( i );
      if ( currentLayer )
      {
        currentVectorLayer = qobject_cast<QgsVectorLayer *>( currentLayer );
        if ( currentVectorLayer )
        {
          //snap to layer yes/no
          newItem = new QTreeWidgetItem( mLayerTreeWidget );
          newItem->setText( 0, currentLayer->name() );
          mLayerIds << currentLayer->getLayerID(); //store also the layer id
          newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
          newItem->setCheckState( 0, Qt::Unchecked );

          //snap to vertex/ snap to segment
          QComboBox* snapToComboBox = new QComboBox( mLayerTreeWidget );
          snapToComboBox->insertItem( 0, tr( "to vertex" ) );
          snapToComboBox->insertItem( 1, tr( "to segment" ) );
          snapToComboBox->insertItem( 2, tr( "to vertex and segment" ) );
          mLayerTreeWidget->setItemWidget( newItem, 1, snapToComboBox );

          //snapping tolerance
          QLineEdit* snappingToleranceEdit = new QLineEdit( mLayerTreeWidget );
          QDoubleValidator* validator = new QDoubleValidator( snappingToleranceEdit );
          snappingToleranceEdit->setValidator( validator );
          mLayerTreeWidget->setItemWidget( newItem, 2, snappingToleranceEdit );

          //snap to vertex/ snap to segment
          QComboBox* toleranceUnitsComboBox = new QComboBox( mLayerTreeWidget );
          toleranceUnitsComboBox->insertItem( 0, tr( "map units" ) );
          toleranceUnitsComboBox->insertItem( 1, tr( "pixels" ) );
          mLayerTreeWidget->setItemWidget( newItem, 3, toleranceUnitsComboBox );

          settingIt = settings.find( currentVectorLayer->getLayerID() );
          if ( settingIt != settings.constEnd() )
          {
            snappingToleranceEdit->setText( QString::number( settingIt.value().tolerance ) );
            int index;
            if ( settingIt.value().snapTo == 0 )//to segment
            {
              index = snapToComboBox->findText( tr( "to vertex" ) );
            }
            else if ( settingIt.value().snapTo == 1 ) //to vertex
            {
              index = snapToComboBox->findText( tr( "to segment" ) );
            }
            else //to vertex and segment
            {
              index = snapToComboBox->findText( tr( "to vertex and segment" ) );
            }
            snapToComboBox->setCurrentIndex( index );
            if ( settingIt.value().toleranceUnit == 0 )//map units
            {
              index = toleranceUnitsComboBox->findText( tr( "map units" ) );
            }
            else
            {
              index = toleranceUnitsComboBox->findText( tr( "pixels" ) );
            }
            toleranceUnitsComboBox->setCurrentIndex( index );
            if ( settingIt.value().checked )
            {
              newItem->setCheckState( 0, Qt::Checked );
            }
          }
          else //insert the default values
          {
            snappingToleranceEdit->setText( "0" );
          }
          ++nVectorLayers;
        }
      }
    }
    mLayerTreeWidget->resizeColumnToContents( 0 );
    mLayerTreeWidget->setColumnWidth( 1, 200 );  //hardcoded for now
    mLayerTreeWidget->resizeColumnToContents( 2 );
    mLayerTreeWidget->resizeColumnToContents( 3 );
  }
}

QgsSnappingDialog::QgsSnappingDialog()
{

}

QgsSnappingDialog::~QgsSnappingDialog()
{

}

void QgsSnappingDialog::layerSettings( QMap<QString, LayerEntry>& settings ) const
{
  settings.clear();

  int nRows = mLayerTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem = 0;
  QString layerId;
  QString layerName;
  QString snapToItemText;
  QString toleranceItemText;
  int snapTo;
  int toleranceUnit;
  double tolerance;
  bool checked = false;

  for ( int i = 0; i < nRows; ++i )
  {
    currentItem = mLayerTreeWidget->topLevelItem( i );
    if ( !currentItem )
    {
      continue;
    }

    //get layer id, to vertex/to segment and tolerance
    layerName = currentItem->text( 0 );
    layerId = mLayerIds.at( i );
    checked = ( currentItem->checkState( 0 ) == Qt::Checked );
    snapToItemText = (( QComboBox* )( mLayerTreeWidget->itemWidget( currentItem, 1 ) ) )->currentText();
    toleranceItemText = (( QComboBox* )( mLayerTreeWidget->itemWidget( currentItem, 3 ) ) )->currentText();
    if ( snapToItemText == tr( "to vertex" ) )
    {
      snapTo = 0;
    }
    else if ( snapToItemText == tr( "to segment" ) )
    {
      snapTo = 1;
    }
    else //to vertex and segment
    {
      snapTo = 2;
    }
    if ( toleranceItemText == tr( "map units" ) )
    {
      toleranceUnit = 0;
    }
    else //to vertex and segment
    {
      toleranceUnit = 1;
    }
    tolerance = (( QLineEdit* )( mLayerTreeWidget->itemWidget( currentItem, 2 ) ) )->text().toDouble();
    LayerEntry newEntry;
    newEntry.checked = checked; newEntry.snapTo = snapTo; newEntry.layerName = layerName;
    newEntry.tolerance = tolerance; newEntry.toleranceUnit = toleranceUnit;
    settings.insert( layerId, newEntry );
  }
}
