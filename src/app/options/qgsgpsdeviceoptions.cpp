/***************************************************************************
    qgsgpsdeviceoptions.cpp
    -------------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsdeviceoptions.h"
#include "qgsbabelgpsdevice.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsbabelformatregistry.h"
#include "qgssettingsregistrycore.h"

#include <QMessageBox>

//
// QgsGpsDeviceOptionsWidget
//

QgsGpsDeviceOptionsWidget::QgsGpsDeviceOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  connect( mButtonAddDevice, &QToolButton::clicked, this, &QgsGpsDeviceOptionsWidget::addNewDevice );
  connect( mButtonRemoveDevice, &QToolButton::clicked, this, &QgsGpsDeviceOptionsWidget::removeCurrentDevice );
  connect( mListDevices, &QListWidget::currentItemChanged, this, &QgsGpsDeviceOptionsWidget::selectedDeviceChanged );

  mDescriptionBrowser->setHtml( QStringLiteral( "<p>%1</p><ul>"
                                "<li><code>%babel</code> - %2</li>"
                                "<li><code>%in</code> - %3</li>"
                                "<li><code>%out</code> - %4</li>"
                                "<li><code>%type</code> - %5</li>"
                                "</ul>" ).arg( tr( "In the download and upload commands there can be special words that will be replaced by "
                                    "QGIS when the commands are used. These words are:" ),
                                    tr( "the path to GPSBabel" ),
                                    tr( "the GPX filename when uploading or the port when downloading" ),
                                    tr( "the port when uploading or the GPX filename when downloading" ),
                                    tr( "GPSBabel feature type argument matching selected feature type (e.g. '-w' for waypoints, '-t' for tracks, and '-r' for routes)" ) ) );

  const QMap< QString, QgsBabelGpsDeviceFormat * > registeredDevices = QgsApplication::gpsBabelFormatRegistry()->devices();
  for ( auto it = registeredDevices.constBegin(); it != registeredDevices.constEnd(); ++it )
  {
    if ( !it.value() )
      continue;

    const QString waypointDownloadCommand =
      it.value()->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    const QString waypointUploadCommand =
      it.value()->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    const QString routeDownloadCommand =
      it.value()->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    const QString routeUploadCommand =
      it.value()->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    const QString trackDownloadCommand =
      it.value()->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    const QString trackUploadCommand =
      it.value()->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );

    mDevices.insert( it.key(), {waypointDownloadCommand,
                                waypointUploadCommand,
                                routeDownloadCommand,
                                routeUploadCommand,
                                trackDownloadCommand,
                                trackUploadCommand
                               } );
  }

  updateDeviceList();

  connect( leWptDown, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leWptUp, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leRteDown, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leRteUp, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leTrkDown, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leTrkUp, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::updateCurrentDevice );
  connect( leDeviceName, &QLineEdit::textEdited, this, &QgsGpsDeviceOptionsWidget::renameCurrentDevice );

  mGpsBabelFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mGpsBabelFileWidget->setDialogTitle( tr( "Select GPSBabel Executable" ) );
  mGpsBabelFileWidget->setFilePath( QgsSettingsRegistryCore::settingsGpsBabelPath.value() );
}

void QgsGpsDeviceOptionsWidget::apply()
{
  QStringList deviceNames;
  QgsSettings settings;
  const QString devPath = QStringLiteral( "babelDevices/%1" );
  settings.remove( QStringLiteral( "babelDevices" ), QgsSettings::Gps );

  for ( auto iter = mDevices.constBegin(); iter != mDevices.constEnd(); ++iter )
  {
    const QString name = iter.key();
    deviceNames << name;
    const QStringList commands = iter.value();
    settings.setValue( devPath.arg( name ) + "/wptdownload", commands.value( 0 ), QgsSettings::Gps );
    settings.setValue( devPath.arg( name ) + "/wptupload", commands.value( 1 ), QgsSettings::Gps );
    settings.setValue( devPath.arg( name ) + "/rtedownload", commands.value( 2 ), QgsSettings::Gps );
    settings.setValue( devPath.arg( name ) + "/rteupload", commands.value( 3 ), QgsSettings::Gps );
    settings.setValue( devPath.arg( name ) + "/trkdownload", commands.value( 4 ), QgsSettings::Gps );
    settings.setValue( devPath.arg( name ) + "/trkupload", commands.value( 5 ), QgsSettings::Gps );
  }
  settings.setValue( QStringLiteral( "babelDeviceList" ), deviceNames, QgsSettings::Gps );

  QgsSettingsRegistryCore::settingsGpsBabelPath.setValue( mGpsBabelFileWidget->filePath() );

  QgsApplication::gpsBabelFormatRegistry()->reloadFromSettings();
}

void QgsGpsDeviceOptionsWidget::addNewDevice()
{
  QString deviceName = tr( "New device %1" );
  int i = 1;
  for ( auto iter = mDevices.constBegin(); iter != mDevices.constEnd(); ++i )
    iter = mDevices.constFind( deviceName.arg( i ) );
  deviceName = deviceName.arg( i - 1 );
  mDevices[deviceName] = QStringList() << QString() << QString() << QString() << QString() << QString() << QString();
  updateDeviceList( deviceName );
}

void QgsGpsDeviceOptionsWidget::removeCurrentDevice()
{
  if ( QMessageBox::warning( this, tr( "Delete Device" ),
                             tr( "Are you sure that you want to delete this device?" ),
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    const auto iter = mDevices.find( mListDevices->currentItem()->text() );
    if ( iter != mDevices.end() )
    {
      mDevices.erase( iter );
      updateDeviceList();
    }
  }
}

void QgsGpsDeviceOptionsWidget::updateDeviceList( const QString &selection )
{
  QString selected;
  if ( selection.isEmpty() )
  {
    QListWidgetItem *item = mListDevices->currentItem();
    selected = ( item ? item->text() : QString() );
  }
  else
  {
    selected = selection;
  }

  // We're going to be changing the selected item, so disable our
  // notification of that.
  disconnect( mListDevices, &QListWidget::currentItemChanged,
              this, &QgsGpsDeviceOptionsWidget::selectedDeviceChanged );

  mListDevices->clear();
  for ( auto iter = mDevices.constBegin(); iter != mDevices.constEnd(); ++iter )
  {
    QListWidgetItem *item = new QListWidgetItem( iter.key(), mListDevices );
    if ( iter.key() == selected )
    {
      mListDevices->setCurrentItem( item );
    }
  }

  if ( !mListDevices->currentItem() && mListDevices->count() > 0 )
    mListDevices->setCurrentRow( 0 );

  // Update the display and reconnect the selection changed signal
  selectedDeviceChanged( mListDevices->currentItem() );
  connect( mListDevices, &QListWidget::currentItemChanged,
           this, &QgsGpsDeviceOptionsWidget::selectedDeviceChanged );
}

void QgsGpsDeviceOptionsWidget::selectedDeviceChanged( QListWidgetItem *current )
{
  if ( mListDevices->count() > 0 )
  {
    const QString devName = current->text();

    mBlockStoringChanges = true;
    leDeviceName->setText( devName );
    const QStringList commands = mDevices.value( devName );
    leWptDown->setText( commands.value( 0 ) );
    leWptUp->setText( commands.value( 1 ) );
    leRteDown->setText( commands.value( 2 ) );
    leRteUp->setText( commands.value( 3 ) );
    leTrkDown->setText( commands.value( 4 ) );
    leTrkUp->setText( commands.value( 5 ) );
    mBlockStoringChanges = false;
  }
}

void QgsGpsDeviceOptionsWidget::updateCurrentDevice()
{
  if ( mBlockStoringChanges )
    return;

  const QString name = mListDevices->currentItem()->text();
  mDevices.insert( name, {leWptDown->text(),
                          leWptUp->text(),
                          leRteDown->text(),
                          leRteUp->text(),
                          leTrkDown->text(),
                          leTrkUp->text()
                         } );
}

void QgsGpsDeviceOptionsWidget::renameCurrentDevice()
{
  if ( mBlockStoringChanges )
    return;

  const QString prevName = mListDevices->currentItem()->text();
  const QString newName = leDeviceName->text();
  mDevices.remove( prevName );

  mDevices.insert( newName, {leWptDown->text(),
                             leWptUp->text(),
                             leRteDown->text(),
                             leRteUp->text(),
                             leTrkDown->text(),
                             leTrkUp->text()
                            } );

  mListDevices->currentItem()->setText( newName );
}

//
// QgsGpsDeviceOptionsFactory
//
QgsGpsDeviceOptionsFactory::QgsGpsDeviceOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "GPSBabel" ), QIcon() )
{

}

QIcon QgsGpsDeviceOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QgsOptionsPageWidget *QgsGpsDeviceOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsGpsDeviceOptionsWidget( parent );
}

QStringList QgsGpsDeviceOptionsFactory::path() const
{
  return {QStringLiteral( "gps" ) };
}

