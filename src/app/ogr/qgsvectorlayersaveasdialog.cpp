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
#include "qgsvectorlayersaveasdialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgscoordinatereferencesystem.h"

#include <QSettings>
#include <QFileDialog>
#include <QTextCodec>

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
    , mCRS( srsid )
{
  setup();
}

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, int options, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
    , mCRS( srsid )
{
  setup();
  if ( !( options & Symbology ) )
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleSpinBox->hide();
  }
}

void QgsVectorLayerSaveAsDialog::setup()
{
  setupUi( this );
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/VectorLayerSaveAs/geometry" ).toByteArray() );
  QMap<QString, QString> map = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for ( QMap< QString, QString>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    mFormatComboBox->addItem( it.key(), it.value() );
  }

  QString format = settings.value( "/UI/lastVectorFormat", "ESRI Shapefile" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  QString enc = settings.value( "/UI/encoding", "System" ).toString();
  int idx = mEncodingComboBox->findText( enc );
  if ( idx < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    idx = 0;
  }

  mCRSSelection->clear();
  mCRSSelection->addItems( QStringList() << tr( "Layer CRS" ) << tr( "Project CRS" ) << tr( "Selected CRS" ) );

  QgsCoordinateReferenceSystem srs( mCRS, QgsCoordinateReferenceSystem::InternalCrsId );
  leCRS->setText( srs.description() );

  mEncodingComboBox->setCurrentIndex( idx );
  on_mFormatComboBox_currentIndexChanged( mFormatComboBox->currentIndex() );

  //symbology export combo box
  mSymbologyExportComboBox->addItem( tr( "No symbology" ), QgsVectorFileWriter::NoSymbology );
  mSymbologyExportComboBox->addItem( tr( "Feature symbology" ), QgsVectorFileWriter::FeatureSymbology );
  mSymbologyExportComboBox->addItem( tr( "Symbol layer symbology" ), QgsVectorFileWriter::SymbolLayerSymbology );
  on_mSymbologyExportComboBox_currentIndexChanged( mSymbologyExportComboBox->currentText() );
}

QgsVectorLayerSaveAsDialog::~QgsVectorLayerSaveAsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/VectorLayerSaveAs/geometry", saveGeometry() );
}

void QgsVectorLayerSaveAsDialog::accept()
{
  QSettings settings;
  settings.setValue( "/UI/lastVectorFileFilterDir", QFileInfo( filename() ).absolutePath() );
  settings.setValue( "/UI/lastVectorFormat", format() );
  settings.setValue( "/UI/encoding", encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::on_mCRSSelection_currentIndexChanged( int idx )
{
  leCRS->setEnabled( idx == 2 );
}

void QgsVectorLayerSaveAsDialog::on_mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx );

  browseFilename->setEnabled( true );
  leFilename->setEnabled( true );

  if ( format() == "KML" )
  {
    mEncodingComboBox->setCurrentIndex( mEncodingComboBox->findText( "UTF-8" ) );
    mEncodingComboBox->setDisabled( true );
    mSkipAttributeCreation->setEnabled( true );
  }
  else if ( format() == "DXF" )
  {
    mSkipAttributeCreation->setChecked( true );
    mSkipAttributeCreation->setDisabled( true );
  }
  else
  {
    mEncodingComboBox->setEnabled( true );
    mSkipAttributeCreation->setEnabled( true );
  }
}

void QgsVectorLayerSaveAsDialog::on_browseFilename_clicked()
{
  QSettings settings;
  QString dirName = leFilename->text().isEmpty() ? settings.value( "/UI/lastVectorFileFilterDir", "." ).toString() : leFilename->text();
  QString filterString = QgsVectorFileWriter::filterForDriver( format() );
  QString outputFile = QFileDialog::getSaveFileName( 0, tr( "Save layer as..." ), dirName, filterString );
  if ( !outputFile.isNull() )
  {
    leFilename->setText( outputFile );
  }
}

void QgsVectorLayerSaveAsDialog::on_browseCRS_clicked()
{
  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector();
  if ( mCRS >= 0 )
    mySelector->setSelectedCrsId( mCRS );
  mySelector->setMessage( tr( "Select the coordinate reference system for the vector file. "
                              "The data points will be transformed from the layer coordinate reference system." ) );

  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mCRS = srs.srsid();
    leCRS->setText( srs.description() );
    mCRSSelection->setCurrentIndex( 2 );
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
  if ( mCRSSelection->currentIndex() == 0 )
  {
    return -1; // Layer CRS
  }
  else if ( mCRSSelection->currentIndex() == 1 )
  {
    return -2; // Project CRS
  }
  else
  {
    return mCRS;
  }
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

bool QgsVectorLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked();
}

int QgsVectorLayerSaveAsDialog::symbologyExport() const
{
  return mSymbologyExportComboBox->itemData( mSymbologyExportComboBox->currentIndex() ).toInt();
}

double QgsVectorLayerSaveAsDialog::scaleDenominator() const
{
  return mScaleSpinBox->value();
}

void QgsVectorLayerSaveAsDialog::on_mSymbologyExportComboBox_currentIndexChanged( const QString& text )
{
  bool scaleEnabled = true;
  if ( text == tr( "No symbology" ) )
  {
    scaleEnabled = false;
  }
  mScaleSpinBox->setEnabled( scaleEnabled );
  mScaleLabel->setEnabled( scaleEnabled );
}
