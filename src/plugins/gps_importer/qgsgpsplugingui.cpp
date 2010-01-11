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
#include "qgsgpsplugingui.h"
#include "qgsgpsdevicedialog.h"
#include "qgsmaplayer.h"
#include "qgsdataprovider.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"
#include "qgsgpsconnection.h"

//qt includes
#include <QFileDialog>
#include <QSettings>

//standard includes
#include <cassert>
#include <cstdlib>
#include <iostream>


QgsGPSPluginGui::QgsGPSPluginGui( const BabelMap& importers,
                                  std::map<QString, QgsGPSDevice*>& devices,
                                  std::vector<QgsVectorLayer*> gpxMapLayers,
                                  QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mGPXLayers( gpxMapLayers ),
    mImporters( importers ), mDevices( devices )
{
  setupUi( this );
  populatePortComboBoxes();
  populateULLayerComboBox();
  populateIMPBabelFormats();

  connect( pbULEditDevices, SIGNAL( clicked() ), this, SLOT( openDeviceEditor() ) );
  connect( pbDLEditDevices, SIGNAL( clicked() ), this, SLOT( openDeviceEditor() ) );

  // make sure that the OK button is enabled only when it makes sense to
  // click it
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );
  pbnOK->setEnabled( false );
  connect( leGPXFile, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leIMPInput, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leIMPOutput, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leIMPLayer, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leCONVInput, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leCONVOutput, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leCONVLayer, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leDLOutput, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( leDLBasename, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( cmbULLayer, SIGNAL( textChanged( QString ) ),
           this, SLOT( enableRelevantControls() ) );
  connect( tabWidget, SIGNAL( currentChanged( int ) ),
           this, SLOT( enableRelevantControls() ) );

  // drag and drop filter
  leGPXFile->setSuffixFilter( "gpx" );
}

QgsGPSPluginGui::~QgsGPSPluginGui()
{
}

void QgsGPSPluginGui::on_buttonBox_accepted()
{

  // what should we do?
  switch ( tabWidget->currentIndex() )
  {
      // add a GPX layer?
    case 0:
      emit loadGPXFile( leGPXFile->text(), cbGPXWaypoints->isChecked(),
                        cbGPXRoutes->isChecked(), cbGPXTracks->isChecked() );
      break;

      // or import other file?
    case 1:
    {
      const QString& typeString( cmbIMPFeature->currentText() );
      emit importGPSFile( leIMPInput->text(),
                          mImporters.find( mImpFormat )->second,
                          typeString == tr( "Waypoints" ),
                          typeString == tr( "Routes" ),
                          typeString == tr( "Tracks" ),
                          leIMPOutput->text(),
                          leIMPLayer->text() );
      break;
    }
    // or download GPS data from a device?
    case 2:
    {
      int featureType = cmbDLFeatureType->currentIndex();

      QString fileName = leDLOutput->text();
      if ( fileName.right( 4 ) != ".gpx" )
      {
        fileName += ".gpx";
      }

      emit downloadFromGPS( cmbDLDevice->currentText(), cmbDLPort->currentText(),
                            featureType == 0, featureType == 1, featureType == 2,
                            fileName, leDLBasename->text() );
      break;
    }
    // or upload GPS data to a device?
    case 3:
    {
      emit uploadToGPS( mGPXLayers[cmbULLayer->currentIndex()],
                        cmbULDevice->currentText(), cmbULPort->currentText() );
      break;
    }
    // or convert between waypoints/tracks=
    case 4:
    {
      int convertType = cmbCONVType->currentIndex();
      emit convertGPSFile( leCONVInput->text(),
                           convertType,
                           leCONVOutput->text(),
                           leCONVLayer->text() );
      break;
    }
  }
  // The slots that are called above will emit closeGui() when successful.
  // If not successful, the user will get another shot without starting from scratch
  // accept();
}


void QgsGPSPluginGui::on_pbnDLOutput_clicked()
{
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this, //parent dialog
                                  tr( "Choose a file name to save under" ),
                                  ".", //initial dir
                                  tr( "GPS eXchange format (*.gpx)" ) );
  if ( !myFileNameQString.isEmpty() )
    leDLOutput->setText( myFileNameQString );
}


void QgsGPSPluginGui::enableRelevantControls()
{
  // load GPX
  if ( tabWidget->currentIndex() == 0 )
  {
    if (( leGPXFile->text() == "" ) )
    {
      pbnOK->setEnabled( false );
      cbGPXWaypoints->setEnabled( false );
      cbGPXRoutes->setEnabled( false );
      cbGPXTracks->setEnabled( false );
      cbGPXWaypoints->setChecked( false );
      cbGPXRoutes->setChecked( false );
      cbGPXTracks->setChecked( false );
    }
    else
    {
      pbnOK->setEnabled( true );
      cbGPXWaypoints->setEnabled( true );
      cbGPXWaypoints->setChecked( true );
      cbGPXRoutes->setEnabled( true );
      cbGPXTracks->setEnabled( true );
      cbGPXRoutes->setChecked( true );
      cbGPXTracks->setChecked( true );
    }
  }

  // import other file
  else if ( tabWidget->currentIndex() == 1 )
  {

    if (( leIMPInput->text() == "" ) || ( leIMPOutput->text() == "" ) ||
        ( leIMPLayer->text() == "" ) )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // download from device
  else if ( tabWidget->currentIndex() == 2 )
  {
    if ( cmbDLDevice->currentText() == "" || leDLBasename->text() == "" ||
         leDLOutput->text() == "" )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // upload to device
  else if ( tabWidget->currentIndex() == 3 )
  {
    if ( cmbULDevice->currentText() == "" || cmbULLayer->currentText() == "" )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // convert between waypoint/routes
  else if ( tabWidget->currentIndex() == 4 )
  {

    if (( leCONVInput->text() == "" ) || ( leCONVOutput->text() == "" ) ||
        ( leCONVLayer->text() == "" ) )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }
}


void QgsGPSPluginGui::on_buttonBox_rejected()
{
  reject();
}


void QgsGPSPluginGui::on_pbnGPXSelectFile_clicked()
{
  QgsLogger::debug( " Gps File Importer::pbnGPXSelectFile_clicked() " );
  QString myFileTypeQString;
  QString myFilterString = tr( "GPS eXchange format (*.gpx)" );
  QSettings settings;
  QString dir = settings.value( "/Plugin-GPS/gpxdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";
  QString myFileNameQString = QFileDialog::getOpenFileName(
                                this, //parent dialog
                                tr( "Select GPX file" ), //caption
                                dir, //initial dir
                                myFilterString, //filters to select
                                &myFileTypeQString ); //the pointer to store selected filter
  QgsLogger::debug( "Selected filetype filter is : " + myFileTypeQString );
  if ( !myFileNameQString.isEmpty() )
    leGPXFile->setText( myFileNameQString );
}


void QgsGPSPluginGui::on_pbnIMPInput_clicked()
{
  QString myFileType;
  QString myFileName = QFileDialog::getOpenFileName(
                         this, //parent dialog
                         tr( "Select file and format to import" ), //caption
                         ".", //initial dir
                         mBabelFilter,
                         &myFileType ); //the pointer to store selected filter
  if ( !myFileName.isEmpty() )
  {
    mImpFormat = myFileType.left( myFileType.length() - 6 );
    std::map<QString, QgsBabelFormat*>::const_iterator iter;
    iter = mImporters.find( mImpFormat );
    if ( iter == mImporters.end() )
    {
      QgsLogger::warning( "Unknown file format selected: " +
                          myFileType.left( myFileType.length() - 6 ) );
    }
    else
    {
      QgsLogger::debug( iter->first + " selected" );
      leIMPInput->setText( myFileName );
      cmbIMPFeature->clear();
      if ( iter->second->supportsWaypoints() )
        cmbIMPFeature->addItem( tr( "Waypoints" ) );
      if ( iter->second->supportsRoutes() )
        cmbIMPFeature->addItem( tr( "Routes" ) );
      if ( iter->second->supportsTracks() )
        cmbIMPFeature->addItem( tr( "Tracks" ) );
    }
  }
}


void QgsGPSPluginGui::on_pbnIMPOutput_clicked()
{
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this, //parent dialog
                                  tr( "Choose a file name to save under" ),
                                  ".", //initial dir
                                  tr( "GPS eXchange format (*.gpx)" ) );
  if ( !myFileNameQString.isEmpty() )
    leIMPOutput->setText( myFileNameQString );
}

void QgsGPSPluginGui::on_pbnRefresh_clicked()
{
  populatePortComboBoxes();
}

void QgsGPSPluginGui::populatePortComboBoxes()
{
  QStringList devs = QgsGPSConnection::availablePorts() << "usb:";
  cmbDLPort->clear();
  cmbDLPort->addItems( devs );
  cmbULPort->clear();
  cmbULPort->addItems( devs );

  // remember the last ports used
  QSettings settings;
  QString lastDLPort = settings.value( "/Plugin-GPS/lastdlport", "" ).toString();
  QString lastULPort = settings.value( "/Plugin-GPS/lastulport", "" ).toString();
  for ( int i = 0; i < cmbDLPort->count(); ++i )
  {
    if ( cmbDLPort->itemText( i ) == lastDLPort )
    {
      cmbDLPort->setCurrentIndex( i );
      break;
    }
  }
  for ( int i = 0; i < cmbULPort->count(); ++i )
  {
    if ( cmbULPort->itemText( i ) == lastULPort )
    {
      cmbULPort->setCurrentIndex( i );
      break;
    }
  }
}


void QgsGPSPluginGui::populateULLayerComboBox()
{
  for ( std::vector<QgsVectorLayer*>::size_type i = 0; i < mGPXLayers.size(); ++i )
    cmbULLayer->addItem( mGPXLayers[i]->name() );
}


void QgsGPSPluginGui::populateIMPBabelFormats()
{
  mBabelFilter = "";
  cmbULDevice->clear();
  cmbDLDevice->clear();
  QSettings settings;
  QString lastDLDevice = settings.value( "/Plugin-GPS/lastdldevice", "" ).toString();
  QString lastULDevice = settings.value( "/Plugin-GPS/lastuldevice", "" ).toString();
  BabelMap::const_iterator iter;
  for ( iter = mImporters.begin(); iter != mImporters.end(); ++iter )
    mBabelFilter.append( iter->first ).append( " (*.*);;" );
  mBabelFilter.chop( 2 ); // Remove the trailing ;;, which otherwise leads to an empty filetype
  int u = -1, d = -1;
  std::map<QString, QgsGPSDevice*>::const_iterator iter2;
  for ( iter2 = mDevices.begin(); iter2 != mDevices.end(); ++iter2 )
  {
    cmbULDevice->addItem( iter2->first );
    if ( iter2->first == lastULDevice )
      u = cmbULDevice->count() - 1;
    cmbDLDevice->addItem( iter2->first );
    if ( iter2->first == lastDLDevice )
      d = cmbDLDevice->count() - 1;
  }
  if ( u != -1 )
    cmbULDevice->setCurrentIndex( u );
  if ( d != -1 )
    cmbDLDevice->setCurrentIndex( d );
}

void QgsGPSPluginGui::on_pbnCONVInput_clicked()
{
  QString myFileTypeQString;
  QString myFilterString = tr( "GPS eXchange format (*.gpx)" );
  QSettings settings;
  QString dir = settings.value( "/Plugin-GPS/gpxdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";
  QString myFileNameQString = QFileDialog::getOpenFileName(
                                this, //parent dialog
                                tr( "Select GPX file" ), //caption
                                dir, //initial dir
                                myFilterString, //filters to select
                                &myFileTypeQString ); //the pointer to store selected filter
  if ( !myFileNameQString.isEmpty() )
    leCONVInput->setText( myFileNameQString );
}

void QgsGPSPluginGui::on_pbnCONVOutput_clicked()
{
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this, //parent dialog
                                  tr( "Choose a file name to save under" ),
                                  ".", //initial dir
                                  tr( "GPS eXchange format (*.gpx)" ) );
  if ( !myFileNameQString.isEmpty() )
    leCONVOutput->setText( myFileNameQString );
}

void QgsGPSPluginGui::openDeviceEditor()
{
  QgsGPSDeviceDialog* dlg = new QgsGPSDeviceDialog( mDevices );
  dlg->show();
  connect( dlg, SIGNAL( devicesChanged() ), this, SLOT( devicesUpdated() ) );
}

void QgsGPSPluginGui::devicesUpdated()
{
  populateIMPBabelFormats();
}
