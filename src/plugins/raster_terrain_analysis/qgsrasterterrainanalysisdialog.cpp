/***************************************************************************
                          qgsrasterterrainanalysisdialog.cpp  -  description
                             -------------------------------
    begin                : August 8th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
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

#include "qgsrasterterrainanalysisdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "cpl_string.h"
#include "gdal.h"
#include <QFileDialog>
#include <QSettings>

QgsRasterTerrainAnalysisDialog::QgsRasterTerrainAnalysisDialog( QgisInterface* iface, QWidget* parent ): QDialog( parent ), mIface( iface )
{
  setupUi( this );

  //insert available methods
  mAnalysisComboBox->addItem( tr( "Slope" ) );
  mAnalysisComboBox->addItem( tr( "Aspect" ) );
  mAnalysisComboBox->addItem( tr( "Ruggedness index" ) );
  mAnalysisComboBox->addItem( tr( "Total curvature" ) );

  //insert available raster layers
  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsRasterLayer* rl = qobject_cast<QgsRasterLayer *>( layer_it.value() );
    if ( rl )
    {
      mInputLayerComboBox->addItem( rl->name(), QVariant( rl->id() ) );
    }
  }

  //insert available drivers that support the create() operation
  GDALAllRegister();

  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; ++i )
  {
    GDALDriverH driver = GDALGetDriver( i );
    if ( driver != NULL )
    {
      char** driverMetadata = GDALGetMetadata( driver, NULL );
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
      {
        mOutputFormatComboBox->addItem( GDALGetDriverLongName( driver ), QVariant( GDALGetDriverShortName( driver ) ) );

        //store the driver shortnames and the corresponding extensions
        //(just in case the user does not give an extension for the output file name)
        int index = 0;
        while (( driverMetadata ) && driverMetadata[index] != 0 )
        {
          QStringList metadataTokens = QString( driverMetadata[index] ).split( "=", QString::SkipEmptyParts );
          if ( metadataTokens.size() < 1 )
          {
            break;
          }

          if ( metadataTokens[0] == "DMD_EXTENSION" )
          {
            if ( metadataTokens.size() < 2 )
            {
              ++index;
              continue;
            }
            mDriverExtensionMap.insert( QString( GDALGetDriverShortName( driver ) ), metadataTokens[1] );
            break;
          }
          ++index;
        }

      }
    }
  }

  //and set last used driver in combo box
  QSettings s;
  QString lastUsedDriver = s.value( "/RasterTerrainAnalysis/lastOutputFormat", "GeoTIFF" ).toString();
  int lastDriverIndex = mOutputFormatComboBox->findText( lastUsedDriver );
  if ( lastDriverIndex != -1 )
  {
    mOutputFormatComboBox->setCurrentIndex( lastDriverIndex );
  }

  QPushButton*  okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( false );
  }
}

QgsRasterTerrainAnalysisDialog::~QgsRasterTerrainAnalysisDialog()
{

}

QString QgsRasterTerrainAnalysisDialog::selectedInputLayerId() const
{
  int index = mInputLayerComboBox->currentIndex();
  if ( index == -1 )
  {
    return "";
  }
  return mInputLayerComboBox->itemData( index ).toString();
}

QString QgsRasterTerrainAnalysisDialog::selectedDriverKey() const
{
  int index = mOutputFormatComboBox->currentIndex();
  if ( index == -1 )
  {
    return "";
  }
  return mOutputFormatComboBox->itemData( index ).toString();
}

QString QgsRasterTerrainAnalysisDialog::selectedOuputFilePath() const
{
  QString outputFileName = mOutputLayerLineEdit->text();
  QFileInfo fileInfo( outputFileName );
  QString suffix = fileInfo.suffix();
  if ( !suffix.isEmpty() )
  {
    return outputFileName;
  }

  //add the file format extension if the user did not specify it
  int index = mOutputFormatComboBox->currentIndex();
  if ( index == -1 )
  {
    return outputFileName;
  }

  QString driverShortName = mOutputFormatComboBox->itemData( index ).toString();
  QMap<QString, QString>::const_iterator it = mDriverExtensionMap.find( driverShortName );
  if ( it == mDriverExtensionMap.constEnd() )
  {
    return outputFileName;
  }

  return ( outputFileName + "." + it.value() );
}

bool QgsRasterTerrainAnalysisDialog::addLayerToProject() const
{
  return mAddResultToProjectCheckBox->checkState() == Qt::Checked;
}

void QgsRasterTerrainAnalysisDialog::on_mOutputLayerPushButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/RasterTerrainAnalysis/lastOutputDir" ).toString();
  QString saveFileName = QFileDialog::getSaveFileName( 0, tr( "Enter result file" ), lastDir );
  if ( !saveFileName.isNull() )
  {
    mOutputLayerLineEdit->setText( saveFileName );
  }
}

QString QgsRasterTerrainAnalysisDialog::selectedAnalysisMethod() const
{
  return mAnalysisComboBox->currentText();
}

void QgsRasterTerrainAnalysisDialog::on_mButtonBox_accepted()
{
  //save last output format
  QSettings s;
  s.setValue( "/RasterTerrainAnalysis/lastOutputFormat", QVariant( mOutputFormatComboBox->currentText() ) );

  //save last output directory
  QFileInfo outputFileInfo( mOutputLayerLineEdit->text() );
  if ( outputFileInfo.exists() )
  {
    s.setValue( "/RasterTerrainAnalysis/lastOutputDir", QVariant( outputFileInfo.absolutePath() ) );
  }
}

void QgsRasterTerrainAnalysisDialog::on_mOutputLayerLineEdit_textChanged( const QString& text )
{
  QPushButton*  okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( !okButton )
  {
    return;
  }

  QString outputPath = QFileInfo( text ).absolutePath();
  if ( mInputLayerComboBox->count() > 0 && QFileInfo( outputPath ).isWritable() )
  {
    okButton->setEnabled( true );
  }
  else
  {
    okButton->setEnabled( false );
  }
}
