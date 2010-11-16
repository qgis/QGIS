/*
 * $Id$ 
 */
/***************************************************************************
    globe_plugin_dialog.cpp - settings dialog for the globe plugin
     --------------------------------------
    Date                 : 11-Nov-2010
    Copyright            : (C) 2010 by Marco Bernasocchi
    Email                : marco at bernawebdesign.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "globe_plugin_dialog.h"

#include <qgsapplication.h>
#include <qgslogger.h>
#include <qgscontexthelp.h>

#include <QtAlgorithms>
#include <QtDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

//constructor
QgsGlobePluginDialog::QgsGlobePluginDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
}

//destructor
QgsGlobePluginDialog::~QgsGlobePluginDialog()
{
}

QString QgsGlobePluginDialog::openFile()
{
  QSettings sets;
  QString path = QFileDialog::getOpenFileName( this,
                 tr( "Open Earthfile" ),
                 "/home",
                 tr( "Earthfiles (*.earth)" ) );

  return path;
}

void QgsGlobePluginDialog::on_buttonBox_accepted()
{
  /*
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
  */
  accept();
}

void QgsGlobePluginDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsGlobePluginDialog::on_comboStereo_currentIndexChanged( int stereoMode )
{
   QMessageBox msgBox;
    msgBox.setText("stereo mode changed");
    msgBox.exec();
  //showText("stereo mode changed");
  /*
  // Select destination data format
  QString frmtCode = comboDstFormats->currentText();
  mDstFormat = mFrmts.find( frmtCode );

  resetDstUi();
  */
}

void QgsGlobePluginDialog::on_buttonSelectEarthfile_clicked()
{
  QMessageBox msgBox;
    msgBox.setText("select file");
    msgBox.exec();
    /*
  QSettings settings;
  QString src;

  src = openFile();

  inputSrcDataset->setText( src );

  if ( !src.isEmpty() )
  {
    QMessageBox msgBox;
    msgBox.setText(src);
    msgBox.exec();
    //showText( src.toString() );
  }
  */
}

/*void QgsGlobePluginDialog::showText(QString text)
{
  QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}
*/
