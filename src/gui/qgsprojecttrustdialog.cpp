/***************************************************************************
                          qgsprojecttrustdialog.cpp
                             -------------------
    begin                : October 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojecttrustdialog.h"
#include "moc_qgsprojecttrustdialog.cpp"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

#include <QFileInfo>
#include <QPushButton>
#include <QSvgRenderer>


QgsProjectTrustDialog::QgsProjectTrustDialog( QgsProject *project, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mButtonBox->button( QDialogButtonBox::StandardButton::YesToAll )->setText( tr( "Always Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::Yes )->setText( tr( "Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::No )->setText( tr( "Deny" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::NoToAll )->setText( tr( "Always Deny" ) );

  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsProjectTrustDialog::buttonBoxClicked );

  QSvgRenderer svg( QStringLiteral( ":/images/themes/default/mIconPythonFile.svg" ) );
  if ( svg.isValid() )
  {
    const double maxLength = 64.0;
    QSize size( maxLength, maxLength );
    const QRectF viewBox = svg.viewBoxF();
    if ( viewBox.height() > viewBox.width() )
    {
      size.setWidth( maxLength * viewBox.width() / viewBox.height() );
    }
    else
    {
      size.setHeight( maxLength * viewBox.height() / viewBox.width() );
    }

    QPixmap pixmap( maxLength, maxLength );
    pixmap.fill( Qt::transparent );

    QPainter painter;
    painter.begin( &pixmap );
    painter.setRenderHint( QPainter::SmoothPixmapTransform );
    painter.translate( ( maxLength - size.width() ) / 2, ( maxLength - size.height() ) / 2 );
    svg.render( &painter, QRectF( 0, 0, size.width(), size.height() ) );
    painter.end();

    mIconLabel->setPixmap( pixmap );
  }

  if ( project )
  {
    QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( project->fileName() );
    if ( storage )
    {
      if ( !storage->filePath( project->fileName() ).isEmpty() )
      {
        QFileInfo projectFileInfo( storage->filePath( project->fileName() ) );
        mProjectAbsoluteFilePath = projectFileInfo.absoluteFilePath();
        mProjectAbsolutePath = projectFileInfo.absolutePath();
        mProjectIsFile = true;
      }
      else
      {
        mProjectAbsolutePath = project->fileName();
        mProjectIsFile = false;
      }
    }
    else
    {
      QFileInfo projectFileInfo( project->fileName() );
      mProjectAbsoluteFilePath = projectFileInfo.absoluteFilePath();
      mProjectAbsolutePath = projectFileInfo.absolutePath();
      mProjectIsFile = true;
    }

    if ( mProjectIsFile )
    {
      mProjectDetailsLabel->setText( tr( "The current project file path is %1." ).arg( mProjectAbsoluteFilePath ) );
    }
    else
    {
      mProjectDetailsLabel->setText( tr( "The current project URI is %1." ).arg( mProjectAbsoluteFilePath ) );
      mTrustProjectFolderCheckBox->setVisible( false );
    }
  }
}

void QgsProjectTrustDialog::buttonBoxClicked( QAbstractButton *button )
{
  QDialogButtonBox::StandardButton buttonType = mButtonBox->standardButton( button );
  if ( buttonType == QDialogButtonBox::StandardButton::Help )
  {
    showHelp();
    return;
  }

  bool accepted = false;
  QString path = !mProjectIsFile || mTrustProjectFolderCheckBox->isChecked() ? mProjectAbsolutePath : mProjectAbsoluteFilePath;
  if ( !path.isEmpty() )
  {
    QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->value();
    trustedProjectsFolders.removeAll( path );
    QStringList temporarilyTrustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyTrustedProjectsFolders->value();
    temporarilyTrustedProjectsFolders.removeAll( path );
    QStringList deniedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionDeniedProjectsFolders->value();
    deniedProjectsFolders.removeAll( path );
    QStringList temporarilyDeniedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyDeniedProjectsFolders->value();
    temporarilyDeniedProjectsFolders.removeAll( path );

    if ( buttonType == QDialogButtonBox::StandardButton::YesToAll )
    {
      trustedProjectsFolders << path;
      accepted = true;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::Yes )
    {
      temporarilyTrustedProjectsFolders << path;
      accepted = true;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::NoToAll )
    {
      deniedProjectsFolders << path;
      accepted = false;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::No )
    {
      temporarilyDeniedProjectsFolders << path;
      accepted = false;
    }

    trustedProjectsFolders.sort();
    temporarilyTrustedProjectsFolders.sort();
    deniedProjectsFolders.sort();
    temporarilyDeniedProjectsFolders.sort();

    QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->setValue( trustedProjectsFolders );
    QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyTrustedProjectsFolders->setValue( temporarilyTrustedProjectsFolders );
    QgsSettingsRegistryCore::settingsCodeExecutionDeniedProjectsFolders->setValue( deniedProjectsFolders );
    QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyDeniedProjectsFolders->setValue( temporarilyDeniedProjectsFolders );
  }

  done( accepted ? QDialog::Accepted : QDialog::Rejected );
}

void QgsProjectTrustDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/getting_started.html" ) );
}
