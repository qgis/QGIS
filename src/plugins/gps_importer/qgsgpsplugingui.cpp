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
#include "qgslogger.h"
#include "qgsgpsdetector.h"
#include "qgssettings.h"

//qt includes
#include <QFileDialog>

//standard includes
#include <cassert>
#include <cstdlib>

QgsGPSPluginGui::QgsGPSPluginGui( const BabelMap &importers,
                                  std::map<QString, QgsGPSDevice *> &devices,
                                  const std::vector<QgsVectorLayer *> &gpxMapLayers,
                                  QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mGPXLayers( gpxMapLayers )
  , mImporters( importers )
  , mDevices( devices )
{
  setupUi( this );
  connect( pbnIMPInput, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnIMPInput_clicked );
  connect( pbnIMPOutput, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnIMPOutput_clicked );
  connect( pbnCONVInput, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnCONVInput_clicked );
  connect( pbnCONVOutput, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnCONVOutput_clicked );
  connect( pbnDLOutput, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnDLOutput_clicked );
  connect( pbnRefresh, &QPushButton::clicked, this, &QgsGPSPluginGui::pbnRefresh_clicked );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsGPSPluginGui::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsGPSPluginGui::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsGPSPluginGui::showHelp );

  // restore size, position and active tab
  restoreState();

  populatePortComboBoxes();
  populateULLayerComboBox();
  populateIMPBabelFormats();
  populateCONVDialog();

  connect( pbULEditDevices, &QAbstractButton::clicked, this, &QgsGPSPluginGui::openDeviceEditor );
  connect( pbDLEditDevices, &QAbstractButton::clicked, this, &QgsGPSPluginGui::openDeviceEditor );

  // make sure that the OK button is enabled only when it makes sense to
  // click it
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );
  pbnOK->setEnabled( false );
  connect( mFileWidget, &QgsFileWidget::fileChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leIMPInput, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leIMPOutput, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leIMPLayer, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leCONVInput, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leCONVOutput, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leCONVLayer, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leDLOutput, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( leDLBasename, &QLineEdit::textChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( cmbULLayer, &QComboBox::editTextChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );
  connect( tabWidget, &QTabWidget::currentChanged,
           this, &QgsGPSPluginGui::enableRelevantControls );

  // drag and drop filter
  mFileWidget->setFilter( tr( "GPX files (*.gpx)" ) );
}

QgsGPSPluginGui::~QgsGPSPluginGui()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Plugin-GPS/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Plugin-GPS/lastTab" ), tabWidget->currentIndex() );
}

void QgsGPSPluginGui::buttonBox_accepted()
{
  // what should we do?
  switch ( tabWidget->currentIndex() )
  {
    case 0:
      // add a GPX layer?
      emit loadGPXFile( mFileWidget->filePath(), cbGPXWaypoints->isChecked(),
                        cbGPXRoutes->isChecked(), cbGPXTracks->isChecked() );
      break;

    case 1:
    {
      // or import other file?
      const QString &typeString( cmbIMPFeature->currentText() );
      emit importGPSFile( leIMPInput->text(),
                          mImporters.find( mImpFormat )->second,
                          typeString == tr( "Waypoints" ),
                          typeString == tr( "Routes" ),
                          typeString == tr( "Tracks" ),
                          leIMPOutput->text(),
                          leIMPLayer->text() );
      break;
    }

    case 2:
    {
      // or download GPS data from a device?
      int featureType = cmbDLFeatureType->currentIndex();

      QString fileName = leDLOutput->text();
      if ( !fileName.endsWith( QLatin1String( ".gpx" ), Qt::CaseInsensitive ) )
      {
        fileName += QLatin1String( ".gpx" );
      }

      emit downloadFromGPS( cmbDLDevice->currentText(),
                            cmbDLPort->currentData().toString(),
                            featureType == 0, featureType == 1, featureType == 2,
                            fileName, leDLBasename->text() );
      break;
    }

    case 3:
    {
      // or upload GPS data to a device?
      emit uploadToGPS( mGPXLayers[cmbULLayer->currentIndex()],
                        cmbULDevice->currentText(),
                        cmbULPort->currentData().toString() );
      break;
    }

    case 4:
    {
      // or convert between waypoints/tracks=
      int convertType = cmbCONVType->currentData().toInt();

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

void QgsGPSPluginGui::pbnDLOutput_clicked()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this,
                                  tr( "Choose a file name to save under" ),
                                  dir,
                                  tr( "GPS eXchange format" ) + " (*.gpx)" );
  if ( !myFileNameQString.isEmpty() )
  {
    if ( !myFileNameQString.endsWith( QLatin1String( ".gpx" ), Qt::CaseInsensitive ) )
    {
      myFileNameQString += QLatin1String( ".gpx" );
    }
    leDLOutput->setText( myFileNameQString );
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QFileInfo( myFileNameQString ).absolutePath() );
  }
}

void QgsGPSPluginGui::enableRelevantControls()
{
  // load GPX
  if ( tabWidget->currentIndex() == 0 )
  {
    if ( !mFileWidget->filePath().isEmpty() )
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
    if ( ( leIMPInput->text().isEmpty() ) || ( leIMPOutput->text().isEmpty() ) ||
         ( leIMPLayer->text().isEmpty() ) )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // download from device
  else if ( tabWidget->currentIndex() == 2 )
  {
    if ( cmbDLDevice->currentText().isEmpty() || leDLBasename->text().isEmpty() ||
         leDLOutput->text().isEmpty() )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // upload to device
  else if ( tabWidget->currentIndex() == 3 )
  {
    if ( cmbULDevice->currentText().isEmpty() || cmbULLayer->currentText().isEmpty() )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }

  // convert between waypoint/routes
  else if ( tabWidget->currentIndex() == 4 )
  {
    if ( ( leCONVInput->text().isEmpty() ) || ( leCONVOutput->text().isEmpty() ) ||
         ( leCONVLayer->text().isEmpty() ) )
      pbnOK->setEnabled( false );
    else
      pbnOK->setEnabled( true );
  }
}

void QgsGPSPluginGui::buttonBox_rejected()
{
  reject();
}

void QgsGPSPluginGui::on_pbnGPXSelectFile_clicked()
{
  QgsLogger::debug( QStringLiteral( " GPS File Importer::pbnGPXSelectFile_clicked() " ) );
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString myFileNameQString = QFileDialog::getOpenFileName(
                                this,
                                tr( "Select GPX file" ),
                                dir,
                                tr( "GPS eXchange format" ) + " (*.gpx)" );
  if ( !myFileNameQString.isEmpty() )
  {
    mFileWidget->setFilePath( myFileNameQString );
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QFileInfo( myFileNameQString ).absolutePath() );
  }
}

void QgsGPSPluginGui::pbnIMPInput_clicked()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/importdirectory" ), QDir::homePath() ).toString();
  QString tf = mBabelFilter.split( QStringLiteral( ";;" ) ).first();
  QString myFileType = settings.value( QStringLiteral( "Plugin-GPS/lastImportFilter" ), tf ).toString();
  QString myFileName = QFileDialog::getOpenFileName(
                         this,
                         tr( "Select file and format to import" ),
                         dir,
                         mBabelFilter,
                         &myFileType );
  if ( !myFileName.isEmpty() )
  {
    // save directory and file type
    settings.setValue( QStringLiteral( "Plugin-GPS/importdirectory" ), QFileInfo( myFileName ).absolutePath() );
    settings.setValue( QStringLiteral( "Plugin-GPS/lastImportFilter" ), myFileType );

    mImpFormat = myFileType.left( myFileType.length() - 6 );
    std::map<QString, QgsBabelFormat *>::const_iterator iter;
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

void QgsGPSPluginGui::pbnIMPOutput_clicked()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this,
                                  tr( "Choose a file name to save under" ),
                                  dir,
                                  tr( "GPS eXchange format" ) + " (*.gpx)" );
  if ( !myFileNameQString.isEmpty() )
  {
    if ( !myFileNameQString.endsWith( QLatin1String( ".gpx" ), Qt::CaseInsensitive ) )
    {
      myFileNameQString += QLatin1String( ".gpx" );
    }
    leIMPOutput->setText( myFileNameQString );
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QFileInfo( myFileNameQString ).absolutePath() );
  }
}

void QgsGPSPluginGui::pbnRefresh_clicked()
{
  populatePortComboBoxes();
}

void QgsGPSPluginGui::populatePortComboBoxes()
{
  QList< QPair<QString, QString> > devs = QgsGPSDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );

  cmbDLPort->clear();
  cmbULPort->clear();
  for ( int i = 0; i < devs.size(); i++ )
  {
    cmbDLPort->addItem( devs[i].second, devs[i].first );
    cmbULPort->addItem( devs[i].second, devs[i].first );
  }

  // remember the last ports used
  QgsSettings settings;
  QString lastDLPort = settings.value( QStringLiteral( "Plugin-GPS/lastdlport" ), "" ).toString();
  QString lastULPort = settings.value( QStringLiteral( "Plugin-GPS/lastulport" ), "" ).toString();

  int idx = cmbDLPort->findData( lastDLPort );
  cmbDLPort->setCurrentIndex( idx < 0 ? 0 : idx );
  idx = cmbULPort->findData( lastULPort );
  cmbULPort->setCurrentIndex( idx < 0 ? 0 : idx );
}

void QgsGPSPluginGui::populateCONVDialog()
{
  cmbCONVType->addItem( tr( "Waypoints from a route" ), QVariant( int( 0 ) ) );
  cmbCONVType->addItem( tr( "Waypoints from a track" ), QVariant( int( 3 ) ) );
  cmbCONVType->addItem( tr( "Route from waypoints" ), QVariant( int( 1 ) ) );
  cmbCONVType->addItem( tr( "Track from waypoints" ), QVariant( int( 2 ) ) );
}

void QgsGPSPluginGui::populateULLayerComboBox()
{
  for ( std::vector<QgsVectorLayer *>::size_type i = 0; i < mGPXLayers.size(); ++i )
    cmbULLayer->addItem( mGPXLayers[i]->name() );
}

void QgsGPSPluginGui::populateIMPBabelFormats()
{
  mBabelFilter.clear();
  cmbULDevice->clear();
  cmbDLDevice->clear();
  QgsSettings settings;
  QString lastDLDevice = settings.value( QStringLiteral( "Plugin-GPS/lastdldevice" ), "" ).toString();
  QString lastULDevice = settings.value( QStringLiteral( "Plugin-GPS/lastuldevice" ), "" ).toString();
  BabelMap::const_iterator iter;
  for ( iter = mImporters.begin(); iter != mImporters.end(); ++iter )
    mBabelFilter.append( iter->first ).append( " (*.*);;" );
  mBabelFilter.chop( 2 ); // Remove the trailing ;;, which otherwise leads to an empty filetype
  int u = -1, d = -1;
  std::map<QString, QgsGPSDevice *>::const_iterator iter2;
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

void QgsGPSPluginGui::pbnCONVInput_clicked()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString myFileNameQString = QFileDialog::getOpenFileName(
                                this,
                                tr( "Select GPX file" ),
                                dir,
                                tr( "GPS eXchange format (*.gpx)" ) );
  if ( !myFileNameQString.isEmpty() )
  {
    leCONVInput->setText( myFileNameQString );
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QFileInfo( myFileNameQString ).absolutePath() );
  }
}

void QgsGPSPluginGui::pbnCONVOutput_clicked()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString myFileNameQString =
    QFileDialog::getSaveFileName( this,
                                  tr( "Choose a file name to save under" ),
                                  dir,
                                  tr( "GPS eXchange format" ) + " (*.gpx)" );
  if ( !myFileNameQString.isEmpty() )
  {
    if ( !myFileNameQString.endsWith( QLatin1String( ".gpx" ), Qt::CaseInsensitive ) )
    {
      myFileNameQString += QLatin1String( ".gpx" );
    }
    leCONVOutput->setText( myFileNameQString );
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QFileInfo( myFileNameQString ).absolutePath() );
  }
}

void QgsGPSPluginGui::openDeviceEditor()
{
  QgsGPSDeviceDialog *dlg = new QgsGPSDeviceDialog( mDevices );
  dlg->show();
  connect( dlg, &QgsGPSDeviceDialog::devicesChanged, this, &QgsGPSPluginGui::devicesUpdated );
}

void QgsGPSPluginGui::devicesUpdated()
{
  populateIMPBabelFormats();
}

void QgsGPSPluginGui::restoreState()
{
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Plugin-GPS/geometry" ) ).toByteArray() );
  tabWidget->setCurrentIndex( settings.value( QStringLiteral( "Plugin-GPS/lastTab" ), 4 ).toInt() );
}

void QgsGPSPluginGui::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_gps/plugins_gps.html" ) );
}
