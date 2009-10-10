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
      mInputLayerComboBox->addItem( rl->name(), QVariant( rl->getLayerID() ) );
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
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, FALSE ) )
      {
        mOutputFormatComboBox->addItem( GDALGetDriverLongName( driver ), QVariant( GDALGetDriverShortName( driver ) ) );
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
  return mOutputLayerLineEdit->text();
}

bool QgsRasterTerrainAnalysisDialog::addLayerToProject() const
{
  if ( mAddResultToProjectCheckBox->checkState() == Qt::Checked )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void QgsRasterTerrainAnalysisDialog::on_mOutputLayerPushButton_clicked()
{
  QString saveFileName = QFileDialog::getSaveFileName( 0, tr( "Enter result file" ) );
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
}

void QgsRasterTerrainAnalysisDialog::on_mOutputLayerLineEdit_textChanged( const QString& text )
{
  QPushButton*  okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( !okButton )
  {
    return;
  }

  QFileInfo fileInfo( text );
  if ( mInputLayerComboBox->count() > 0 && fileInfo.dir().exists() )
  {
    okButton->setEnabled( true );
  }
  else
  {
    okButton->setEnabled( false );
  }
}
