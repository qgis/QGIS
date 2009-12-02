// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////

// qgis::plugin::ogrconv
#include "dialog.h"
#include "format.h"
#include "translator.h"
// QGIS includes
#include <qgsapplication.h>
#include <qgslogger.h>
#include <qgscontexthelp.h>
// TODO: Add support of QGIS projection selector
//#include <qgsprojectionselector.h>
// Qt4
#include <QtAlgorithms>
#include <QtDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <ogr_api.h>

Dialog::Dialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  populateFormats();
  resetSrcUi();
  resetDstUi();
}

Dialog::~Dialog()
{
}

void Dialog::resetSrcUi()
{
  // Clear all settings and states
  inputSrcDataset->clear();
  // TODO: Transformation support
  //inputSrcSrs->clear();
  comboSrcLayer->clear();
  radioSrcFile->setDisabled( true );
  radioSrcFile->setChecked( false );
  radioSrcDirectory->setDisabled( true );
  radioSrcDirectory->setChecked( false );
  radioSrcProtocol->setDisabled( true );
  radioSrcProtocol->setChecked( false );

  // Configure types of input sources
  unsigned char const& type = mSrcFormat.type();

  if ( isFormatType( type, Format::eFile ) )
  {
    radioSrcFile->setDisabled( false );
    radioSrcFile->setChecked( true );
  }

  if ( isFormatType( type, Format::eDirectory ) )
  {
    radioSrcDirectory->setDisabled( false );
    if ( !radioSrcFile->isEnabled() )
      radioSrcDirectory->setChecked( true );
  }

  if ( isFormatType( type, Format::eProtocol ) )
  {
    radioSrcProtocol->setDisabled( false );

    if ( !radioSrcFile->isEnabled() && !radioSrcDirectory->isEnabled() )
    {
      radioSrcProtocol->setChecked( true );
      inputSrcDataset->setText( mSrcFormat.protocol() );
    }
  }

  setButtonState( buttonSelectSrc, isFormatType( type, Format::eProtocol ) );
}

void Dialog::resetDstUi()
{
  inputDstDataset->clear();
  // TODO: Transformation support
  //inputDstSrs->clear();

  unsigned char const& type = mDstFormat.type();
  bool isProtocol = isFormatType( type, Format::eProtocol );

  if ( isProtocol )
  {
    inputDstDataset->setText( mDstFormat.protocol() );
  }

  setButtonState( buttonSelectDst, isProtocol );
}

void Dialog::setButtonState( QPushButton* btn, bool isProtocol )
{
  Q_CHECK_PTR( btn );

  if ( isProtocol )
  {
    btn->setText( tr( "Connect" ) );
  }
  else
  {
    btn->setText( tr( "Browse" ) );
  }
}

void Dialog::populateFormats()
{
  comboSrcFormats->clear();
  comboDstFormats->clear();

  QStringList drvSrcList;
  QStringList drvDstList;
  QString drvName;

  QgsApplication::registerOgrDrivers();
  int const drvCount = OGRGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    OGRSFDriverH drv = OGRGetDriver( i );
    Q_CHECK_PTR( drv );
    if ( 0 != drv )
    {
      drvName = OGR_Dr_GetName( drv );
      drvSrcList.append( drvName );

      if ( 0 != OGR_Dr_TestCapability( drv, ODrCCreateDataSource ) )
      {
        drvDstList.append( drvName );
      }
    }
  }

  qSort( drvSrcList.begin(), drvSrcList.end() );
  qSort( drvDstList.begin(), drvDstList.end() );
  comboSrcFormats->addItems( drvSrcList );
  comboDstFormats->addItems( drvDstList );
}

void Dialog::populateLayers( QString const& url )
{
  comboSrcLayer->clear();

  OGRDataSourceH ds = OGROpen( url.toAscii().constData(), 0, 0 );
  if ( 0 != ds )
  {
    QString lyrName;
    QString lyrType;

    int const size = OGR_DS_GetLayerCount( ds );
    for ( int i = 0; i < size; ++i )
    {
      OGRLayerH lyr = OGR_DS_GetLayer( ds, i );
      if ( 0 != lyr )
      {
        OGRFeatureDefnH lyrDef = OGR_L_GetLayerDefn( lyr );
        Q_ASSERT( 0 != lyrDef );

        lyrName = OGR_FD_GetName( lyrDef );

        OGRwkbGeometryType const geomType = OGR_FD_GetGeomType( lyrDef );
        lyrType = OGRGeometryTypeToName( geomType );

        // FIXME: Appending type to layer name prevents from layer finding
        //comboSrcLayer->addItem(lyrName + " (" + lyrType.toUpper() + ")");
        comboSrcLayer->addItem( lyrName );
      }
    }

    OGR_DS_Destroy( ds );
  }
  else
  {
    QMessageBox::warning( this,
                          tr( "OGR Converter" ),
                          tr( "Could not establish connection to: '%1'" ).arg( url ),
                          QMessageBox::Close );
  }
}

bool Dialog::testConnection( QString const& url )
{
  bool success = false;

  OGRDataSourceH ds = OGROpen( url.toAscii().constData(), 0, 0 );
  if ( 0 != ds )
  {
    success = true;
    OGR_DS_Destroy( ds );
  }

  return success;
}

QString Dialog::openFile()
{
  QSettings sets;
  QString path = QFileDialog::getOpenFileName( this,
                 tr( "Open OGR file" ),
                 sets.value( "/Plugin-OGR/ogr-file", "./" ).toString(),
                 tr( "OGR File Data Source (*.*)" ) );

  return path;
}

QString Dialog::openDirectory()
{
  QString path = QFileDialog::getExistingDirectory( this,
                 tr( "Open Directory" ), "",
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  return path;
}

void Dialog::on_buttonBox_accepted()
{
  // Validate input settings
  QString srcUrl( inputSrcDataset->text() );
  QString srcLayer( comboSrcLayer->currentText() );

  if ( srcUrl.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Input OGR dataset is missing!" ) );
    return;
  }

  if ( srcLayer.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Input OGR layer name is missing!" ) );
    return;
  }

  // Validate output settings
  QString dstFormat( comboDstFormats->currentText() );
  QString dstUrl( inputDstDataset->text() );
  QString dstLayer( inputDstLayer->text() );
  if ( dstLayer.isEmpty() )
    dstLayer = srcLayer;

  if ( dstFormat.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Target OGR format not selected!" ) );
    return;
  }

  if ( dstUrl.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Output OGR dataset is missing!" ) );
    return;
  }

  if ( dstLayer.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Output OGR layer name is missing!" ) );
    return;
  }

  // TODO: SRS transformation support
  //QString srcSrs("EPSG:");
  //QString dstSrs("EPSG:");
  //srcSrs += inputSrcSrs->text();
  //dstSrs += inputDstSrs->text();

  // Execute layer translation
  bool success = false;

  // TODO: Use try-catch to display more meaningful error messages from Translator
  Translator worker( srcUrl, dstUrl, dstFormat );
  worker.setSourceLayer( srcLayer );
  worker.setTargetLayer( dstLayer );
  success = worker.translate();

  if ( success )
  {
    QMessageBox::information( this,
                              tr( "OGR Layer Converter" ),
                              tr( "Successfully translated layer '%1'" ).arg( srcLayer ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "OGR Layer Converter" ),
                              tr( "Failed to translate layer '%1'" ).arg( srcLayer ) );
  }

  // Close dialog box
  accept();
}

void Dialog::on_buttonBox_rejected()
{
  reject();
}

void Dialog::on_radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    unsigned char const& type = mSrcFormat.type();
    Q_ASSERT( isFormatType( type, Format::eFile ) );

    inputSrcDataset->clear();
    setButtonState( buttonSelectSrc, isFormatType( type, Format::eProtocol ) );
  }
}

void Dialog::on_radioSrcDirectory_toggled( bool checked )
{
  if ( checked )
  {
    unsigned char const& type = mSrcFormat.type();
    Q_ASSERT( isFormatType( type, Format::eDirectory ) );

    inputSrcDataset->clear();
    setButtonState( buttonSelectSrc, isFormatType( type, Format::eProtocol ) );
  }
}

void Dialog::on_radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    unsigned char const& type = mSrcFormat.type();
    Q_ASSERT( isFormatType( type, Format::eProtocol ) );

    inputSrcDataset->setText( mSrcFormat.protocol() );
    setButtonState( buttonSelectSrc, isFormatType( type, Format::eProtocol ) );
  }
}

void Dialog::on_comboSrcFormats_currentIndexChanged( int index )
{
  // Select source data format
  QString frmtCode = comboSrcFormats->currentText();
  mSrcFormat = mFrmts.find( frmtCode );

  resetSrcUi();
}

void Dialog::on_comboDstFormats_currentIndexChanged( int index )
{
  // Select destination data format
  QString frmtCode = comboDstFormats->currentText();
  mDstFormat = mFrmts.find( frmtCode );

  resetDstUi();
}

void Dialog::on_buttonSelectSrc_clicked()
{
  QSettings settings;
  QString src;

  if ( radioSrcFile->isChecked() )
  {
    src = openFile();
  }
  else if ( radioSrcDirectory->isChecked() )
  {
    src = openDirectory();
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    src = inputSrcDataset->text();
  }
  else
  {
    Q_ASSERT( !"SHOULD NEVER GET HERE" );
  }

  inputSrcDataset->setText( src );

  if ( !src.isEmpty() )
  {
    populateLayers( src );
  }
}

void Dialog::on_buttonSelectDst_clicked()
{
  QSettings settings;
  QString dst;
  QString msg;

  unsigned char const& type = mDstFormat.type();
  if ( isFormatType( type, Format::eProtocol ) )
  {
    dst = inputDstDataset->text();

    if ( testConnection( dst ) )
    {
      msg = tr( "Successfully connected to: '%1'" ).arg( dst );
    }
    else
    {
      msg = tr( "Could not establish connection to: '%1'" ).arg( dst );
    }

    QMessageBox::information( this, tr( "OGR Converter" ), msg, QMessageBox::Close );
  }
  else if ( isFormatType( type, Format::eDirectory ) )
  {
    dst = openDirectory();
  }
  else if ( isFormatType( type, Format::eFile ) )
  {
    dst = QFileDialog::getSaveFileName( this,
                                        tr( "Choose a file name to save to" ),
                                        "output", tr( "OGR File Data Source (*.*)" ) );
  }
  else
  {
    Q_ASSERT( !"SHOULD NEVER GET HERE" );
  }

  inputDstDataset->setText( dst );
}
