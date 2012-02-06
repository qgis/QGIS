/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
// qgis includes
#include "qgis.h"
#include "heatmapgui.h"
#include "qgscontexthelp.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

// GDAL includes
#include "gdal_priv.h"
#include "cpl_string.h"
#include "cpl_conv.h"

//qt includes
#include <QComboBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

//standard includes

HeatmapGui::HeatmapGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  // Adding point layers to the mInputVectorCombo
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator<QString, QgsMapLayer*> layers(mapLayers);

  while( layers.hasNext() )
  {
    layers.next();
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>(layers.value());
    if( ( vl ) && ( vl->geometryType() == QGis::Point ) )
    {
      mInputVectorCombo->addItem( vl->name(), QVariant( vl->id() ) );
    }
  }

  // Adding GDAL drivers with CREATE to the mFormatCombo
  GDALAllRegister();
  int nDrivers = GDALGetDriverCount();
  for( int i = 0; i < nDrivers; i +=1 )
  {
    GDALDriver* nthDriver = GetGDALDriverManager()->GetDriver( i );
    char** driverMetadata = nthDriver->GetMetadata();
    if( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
    {
      // Add LongName text, shortname variant; GetDescription actually gets the shortname
      mFormatCombo->addItem( nthDriver->GetMetadataItem( GDAL_DMD_LONGNAME ), QVariant( nthDriver->GetDescription() ) );
      // Add the drivers and their extensions to a map for filename correction
      mExtensionMap.insert( nthDriver->GetDescription(), nthDriver->GetMetadataItem( GDAL_DMD_EXTENSION ) );
    }
  }

  //finally set right the ok button
  enableOrDisableOkButton();
}

HeatmapGui::~HeatmapGui()
{
}

void HeatmapGui::on_mButtonBox_accepted()
{
  // Variables to be emitted with the createRaster signal
  QgsVectorLayer* inputLayer;
  int bufferDistance;
  float decayRatio;
  QString outputFileName;
  QString outputFormat;

  QString dummyText;

  // The input vector layer
  int myLayerId = mInputVectorCombo->itemData( mInputVectorCombo->currentIndex() ).toInt();

  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator<QString, QgsMapLayer*> layers(mapLayers);
  
  while( layers.hasNext() )
  {
    layers.next();
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>(layers.value());
    if ( vl )
    {
      dummyText = vl->id();
      if( dummyText.toInt() == myLayerId )
      {
        inputLayer = vl;
      }
    }
  }

  // The buffer distance
  dummyText = mBufferLineEdit->text();
  bufferDistance = dummyText.toInt();
  // The decay ratio
  dummyText = mDecayLineEdit->text();
  decayRatio = dummyText.toFloat();

  // The output filename
  outputFileName = mOutputRasterLineEdit->text();
  QFileInfo myFileInfo( outputFileName );
  if( outputFileName.isEmpty() || !myFileInfo.dir().exists() )
  {
    QMessageBox::information( 0, tr("Output filename is invalid!"), tr("Kindly enter a valid output file path and name.") );
  }

  // The output format
  outputFormat = mFormatCombo->itemData( mFormatCombo->currentIndex() ).toString();

  // append the file format if the suffix is empty
  QString suffix = myFileInfo.suffix();
  if( suffix.isEmpty() )
  {
    QMap<QString, QString>::const_iterator it = mExtensionMap.find( outputFormat );
    if( it != mExtensionMap.end() && it.key() == outputFormat )
    {
      // making sure that there is really a extension value available
      // Some drivers donot seem to have any extension at all
      if( it.value() != NULL || it.value() != "" )
      {
        outputFileName.append(".");
        outputFileName.append( it.value() );
      }
    }
  }

  emit createRaster( inputLayer, bufferDistance, decayRatio, outputFileName, outputFormat );
  //and finally
  accept();
}

void HeatmapGui::on_mButtonBox_rejected()
{
  reject();
}

void HeatmapGui::on_mButtonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void HeatmapGui::on_mBrowseButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/Heatmap/lastOutputDir", "" ).toString();

  QString outputFilename = QFileDialog::getSaveFileName( 0, tr( "Save Heatmap as: "), lastDir );
  if( !outputFilename.isEmpty() )
  {
    mOutputRasterLineEdit->setText( outputFilename );
    QFileInfo outputFileInfo( outputFilename );
    QDir outputDir = outputFileInfo.absoluteDir();
    if( outputDir.exists() )
    {
      s.setValue( "/Heatmap/lastOutputDir", outputFileInfo.absolutePath() );
    }
  }

  enableOrDisableOkButton();
}

void HeatmapGui::enableOrDisableOkButton()
{
  bool enabled = true;
  QString filename = mOutputRasterLineEdit->text();
  QFileInfo theFileInfo( filename );
  if( filename.isEmpty() || !theFileInfo.dir().exists() )
  {
    enabled = false;
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}
