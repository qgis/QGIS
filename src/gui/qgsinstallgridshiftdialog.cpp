/***************************************************************************
                         qgsinstallgridshiftdialog.cpp
                             -------------------
    begin                : September 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinstallgridshiftdialog.h"
#include "qgsgui.h"
#include "qgssettings.h"
#include "qgsapplication.h"

#include <QFileDialog>
#include <QMessageBox>

///@cond PRIVATE

QgsInstallGridShiftFileDialog::QgsInstallGridShiftFileDialog( const QString &gridName, QWidget *parent )
  : QDialog( parent )
  , mGridName( gridName )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mInstallButton->setText( tr( "Install %1 from Folder…" ).arg( mGridName ) );

  connect( mInstallButton, &QPushButton::clicked, this, &QgsInstallGridShiftFileDialog::installFromFile );
}

void QgsInstallGridShiftFileDialog::setDescription( const QString &html )
{
  mSummaryLabel->setHtml( html );
}

void QgsInstallGridShiftFileDialog::setDownloadMessage( const QString &message )
{
  mDownloadLabel->setText( message );
}

void QgsInstallGridShiftFileDialog::installFromFile()
{
  QgsSettings settings;
  const QString initialDir = settings.value( QStringLiteral( "lastTransformGridFolder" ), QDir::homePath(), QgsSettings::App ).toString();
  const QString gridFilePath = QFileDialog::getOpenFileName( nullptr, tr( "Install %1" ).arg( mGridName ), initialDir, QStringLiteral( "%1 (%1);;" ).arg( mGridName ) + tr( "Grid Shift Files" ) + QStringLiteral( " (*.gsb *.GSB *.tif);;" ) + QObject::tr( "All files" ) + " (*)" );

  if ( gridFilePath.isEmpty() )
  {
    return; //canceled by the user
  }

  const QFileInfo fi( gridFilePath );
  settings.setValue( QStringLiteral( "lastTransformGridFolder" ), fi.absolutePath(), QgsSettings::App );

  const QString baseGridPath = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "proj" );

  const QString destFilePath = baseGridPath + '/' + mGridName;
  const QString destPath = QFileInfo( destFilePath ).absolutePath();

  if ( !QDir( destPath ).exists() )
    QDir().mkpath( destPath );

  if ( QFile::copy( gridFilePath, destFilePath ) )
  {
    QMessageBox::information( this, tr( "Install Grid File" ), tr( "The %1 grid shift file has been successfully installed. Please restart QGIS for this change to take effect." ).arg( mGridName ) );
    accept();
  }
  else
  {
    QMessageBox::critical( this, tr( "Install Grid File" ), tr( "Could not copy %1 to %2. Please check folder permissions and retry." ).arg( mGridName, destFilePath ) );
  }
}

///@endcond
