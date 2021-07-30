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
#include "qgsmaplayer.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsgpsdetector.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsgui.h"
#include "qgsbabelformatregistry.h"
#include "qgsapplication.h"
//qt includes
#include <QFileDialog>

//standard includes
#include <cassert>
#include <cstdlib>

QgsGpsPluginGui::QgsGpsPluginGui( const std::vector<QgsVectorLayer *> &gpxMapLayers,
                                  QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mGPXLayers( gpxMapLayers )
{
  setupUi( this );
  QgsGui::instance()->enableAutoGeometryRestore( this );
  connect( pbnIMPInput, &QPushButton::clicked, this, &QgsGpsPluginGui::pbnIMPInput_clicked );
  connect( pbnIMPOutput, &QPushButton::clicked, this, &QgsGpsPluginGui::pbnIMPOutput_clicked );
  connect( pbnDLOutput, &QPushButton::clicked, this, &QgsGpsPluginGui::pbnDLOutput_clicked );
  connect( pbnRefresh, &QPushButton::clicked, this, &QgsGpsPluginGui::pbnRefresh_clicked );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsGpsPluginGui::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsGpsPluginGui::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsGpsPluginGui::showHelp );

  // restore size, position and active tab
  restoreState();

  populatePortComboBoxes();
  populateULLayerComboBox();
  populateIMPBabelFormats();

  // make sure that the OK button is enabled only when it makes sense to
  // click it
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );
  pbnOK->setEnabled( false );
  connect( mFileWidget, &QgsFileWidget::fileChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( leIMPInput, &QLineEdit::textChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( leIMPOutput, &QLineEdit::textChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( leIMPLayer, &QLineEdit::textChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( leDLOutput, &QLineEdit::textChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( leDLBasename, &QLineEdit::textChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( cmbULLayer, &QComboBox::editTextChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );
  connect( tabWidget, &QTabWidget::currentChanged,
           this, &QgsGpsPluginGui::enableRelevantControls );

  // drag and drop filter
  mFileWidget->setFilter( tr( "GPX files (*.gpx)" ) );
}

QgsGpsPluginGui::~QgsGpsPluginGui()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Plugin-GPS/lastTab" ), tabWidget->currentIndex() );
}

void QgsGpsPluginGui::buttonBox_accepted()
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
                          QgsApplication::gpsBabelFormatRegistry()->importFormat( mImpFormat ),
                          typeString == tr( "Waypoints" ) ? Qgis::GpsFeatureType::Waypoint
                          : typeString == tr( "Routes" ) ? Qgis::GpsFeatureType::Route : Qgis::GpsFeatureType::Track,
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
                            featureType == 0 ? Qgis::GpsFeatureType::Waypoint
                            : featureType == 1 ? Qgis::GpsFeatureType::Route : Qgis::GpsFeatureType::Track,
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
  }

  // The slots that are called above will emit closeGui() when successful.
  // If not successful, the user will get another shot without starting from scratch
  // accept();
}

void QgsGpsPluginGui::pbnDLOutput_clicked()
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

void QgsGpsPluginGui::enableRelevantControls()
{
  // load GPX
  if ( tabWidget->currentIndex() == 0 )
  {
    bool enabled = !mFileWidget->filePath().isEmpty();
    pbnOK->setEnabled( enabled );
    cbGPXWaypoints->setEnabled( enabled );
    cbGPXRoutes->setEnabled( enabled );
    cbGPXTracks->setEnabled( enabled );
    cbGPXWaypoints->setChecked( enabled );
    cbGPXRoutes->setChecked( enabled );
    cbGPXTracks->setChecked( enabled );
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
}

void QgsGpsPluginGui::buttonBox_rejected()
{
  reject();
}

void QgsGpsPluginGui::on_pbnGPXSelectFile_clicked()
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

void QgsGpsPluginGui::pbnIMPInput_clicked()
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

    const QgsAbstractBabelFormat *format = QgsApplication::gpsBabelFormatRegistry()->importFormat( mImpFormat );
    if ( !format )
    {
      QgsLogger::warning( "Unknown file format selected: " +
                          myFileType.left( myFileType.length() - 6 ) );
    }
    else
    {
      leIMPInput->setText( myFileName );
      cmbIMPFeature->clear();
      if ( format->capabilities() & Qgis::BabelFormatCapability::Waypoints )
        cmbIMPFeature->addItem( tr( "Waypoints" ) );
      if ( format->capabilities() & Qgis::BabelFormatCapability::Routes )
        cmbIMPFeature->addItem( tr( "Routes" ) );
      if ( format->capabilities() & Qgis::BabelFormatCapability::Tracks )
        cmbIMPFeature->addItem( tr( "Tracks" ) );
    }
  }
}

void QgsGpsPluginGui::pbnIMPOutput_clicked()
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

void QgsGpsPluginGui::pbnRefresh_clicked()
{
  populatePortComboBoxes();
}

void QgsGpsPluginGui::populatePortComboBoxes()
{
  QList< QPair<QString, QString> > devs = QgsGpsDetector::availablePorts() << QPair<QString, QString>( QStringLiteral( "usb:" ), QStringLiteral( "usb:" ) );

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

void QgsGpsPluginGui::populateULLayerComboBox()
{
  for ( std::vector<QgsVectorLayer *>::size_type i = 0; i < mGPXLayers.size(); ++i )
    cmbULLayer->addItem( mGPXLayers[i]->name() );
}

void QgsGpsPluginGui::populateIMPBabelFormats()
{
  mBabelFilter.clear();
  cmbULDevice->clear();
  cmbDLDevice->clear();
  QgsSettings settings;
  QString lastDLDevice = settings.value( QStringLiteral( "Plugin-GPS/lastdldevice" ), "" ).toString();
  QString lastULDevice = settings.value( QStringLiteral( "Plugin-GPS/lastuldevice" ), "" ).toString();

  const QStringList importers = QgsApplication::gpsBabelFormatRegistry()->importFormatNames();
  for ( const QString &format : importers )
    mBabelFilter.append( format ).append( " (*.*);;" );
  mBabelFilter.chop( 2 ); // Remove the trailing ;;, which otherwise leads to an empty filetype
  int u = -1, d = -1;
  const QStringList devices = QgsApplication::gpsBabelFormatRegistry()->deviceNames();
  for ( const QString &device : devices )
  {
    cmbULDevice->addItem( device );
    if ( device == lastULDevice )
      u = cmbULDevice->count() - 1;
    cmbDLDevice->addItem( device );
    if ( device == lastDLDevice )
      d = cmbDLDevice->count() - 1;
  }
  if ( u != -1 )
    cmbULDevice->setCurrentIndex( u );
  if ( d != -1 )
    cmbDLDevice->setCurrentIndex( d );
}

void QgsGpsPluginGui::devicesUpdated()
{
  populateIMPBabelFormats();
}

void QgsGpsPluginGui::restoreState()
{
  QgsSettings settings;
  tabWidget->setCurrentIndex( settings.value( QStringLiteral( "Plugin-GPS/lastTab" ), 0 ).toInt() );
}

void QgsGpsPluginGui::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_gps/plugins_gps.html" ) );
}
