/***************************************************************************
                          qgsvectorlayersaveasdialog.h
 Dialog to select destination, type and crs for ogr layers
                             -------------------
    begin                : Mon Mar 22 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id:$ */
#include "qgsvectorlayersaveasdialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"

#include <QSettings>
#include <QFileDialog>
#include <QTextCodec>

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( QWidget* parent, Qt::WFlags fl )
  : QDialog( parent, fl )
  , mCRS( -1 )
{
  setupUi( this );

  QSettings settings;
  QMap<QString, QString> map = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for( QMap< QString, QString>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    mFormatComboBox->addItem( it.key(), it.value() );
  }

  QString format = settings.value( "/UI/lastVectorFormat", "ESRI Shapefile" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  QString enc = settings.value( "/UI/encoding", QString( "System" ) ).toString();
  int idx = mEncodingComboBox->findText( enc );
  if( idx < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    idx = 0;
  }

  mEncodingComboBox->setCurrentIndex( idx );

  leCRS->setText( tr( "Original CRS" ) );
  on_mFormatComboBox_currentIndexChanged( mFormatComboBox->currentIndex() );
}

QgsVectorLayerSaveAsDialog::~QgsVectorLayerSaveAsDialog()
{
}

void QgsVectorLayerSaveAsDialog::accept()
{
  QSettings settings;
  settings.setValue( "/UI/lastVectorFileFilterDir", QFileInfo( filename() ).absolutePath() );
  settings.setValue( "/UI/lastVectorFormat", format() );
  settings.setValue( "/UI/encoding", encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::on_mFormatComboBox_currentIndexChanged( int idx )
{
  browseFilename->setEnabled( true );
  leFilename->setEnabled( true );

  if( format() == "KML" )
  {
    mEncodingComboBox->setCurrentIndex( mEncodingComboBox->findText( "UTF-8" ) );
    mEncodingComboBox->setDisabled( true );
  }
  else
  {
    mEncodingComboBox->setEnabled( true );
  }
}

void QgsVectorLayerSaveAsDialog::on_browseFilename_clicked()
{
  QSettings settings;
  QString dirName = leFilename->text().isEmpty() ? settings.value( "/UI/lastVectorFileFilterDir", "." ).toString() : leFilename->text();
  QString filterString = QgsVectorFileWriter::filterForDriver( format() );
  QString outputFile = QFileDialog::getSaveFileName( 0, tr( "Save layer as..." ), dirName, filterString );
  if( !outputFile.isNull() )
  {
    leFilename->setText( outputFile );
  }
}

void QgsVectorLayerSaveAsDialog::on_browseCRS_clicked()
{
  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector();
  if( mCRS >= 0 )
    mySelector->setSelectedCrsId( mCRS );
  mySelector->setMessage( tr( "Select the coordinate reference system for the vector file. "
                              "The data points will be transformed from the layer coordinate reference system." ) );

  if( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mCRS = srs.srsid();
    leCRS->setText( srs.description() );
  }

  delete mySelector;
}

QString QgsVectorLayerSaveAsDialog::filename() const
{
  return leFilename->text();
}

QString QgsVectorLayerSaveAsDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

QString QgsVectorLayerSaveAsDialog::format() const
{
  return mFormatComboBox->itemData( mFormatComboBox->currentIndex() ).toString();
}

long QgsVectorLayerSaveAsDialog::crs() const
{
  return mCRS;
}

QStringList QgsVectorLayerSaveAsDialog::datasourceOptions() const
{
  return mOgrDatasourceOptions->toPlainText().split( "\n" );
}

QStringList QgsVectorLayerSaveAsDialog::layerOptions() const
{
  return mOgrLayerOptions->toPlainText().split( "\n" );
}

bool QgsVectorLayerSaveAsDialog::skipAttributeCreation() const
{
  return mSkipAttributeCreation->isChecked();
}
