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
#include "qgshelp.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

#include <QFileInfo>
#include <QPushButton>


QgsProjectTrustDialog::QgsProjectTrustDialog( QgsProject *project, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mCodePreviewEditor->setReadOnly( true );

  mButtonBox->button( QDialogButtonBox::StandardButton::YesToAll )->setText( tr( "Always Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::Yes )->setText( tr( "Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::No )->setText( tr( "Deny" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::NoToAll )->setText( tr( "Always Deny" ) );

  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsProjectTrustDialog::buttonBoxClicked );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectTrustDialog::showHelp );

  if ( project )
  {
    QFileInfo fi( project->fileName() );
    mProjectDetailsLabel->setText( tr( "Project filename: %1" ).arg( fi.absoluteFilePath() ) );

    QFileInfo projectFileInfo( project->absoluteFilePath() );
    mProjectAbsoluteFilePath = projectFileInfo.absoluteFilePath();
    mProjectAbsolutePath = projectFileInfo.absolutePath();
  }
}

bool QgsProjectTrustDialog::applyTrustToProjectFolder() const
{
  return mTrustProjectFolderCheckBox->isChecked();
}

void QgsProjectTrustDialog::buttonBoxClicked( QAbstractButton *button )
{
  const QString path = applyTrustToProjectFolder() ? mProjectAbsolutePath : mProjectAbsoluteFilePath;

  if ( mButtonBox->buttonRole( button ) != QDialogButtonBox::ButtonRole::YesToAll )
  {
    QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->value();
    if ( !path.isEmpty() && !trustedProjectsFolders.contains( path ) )
    {
      trustedProjectsFolders << path;
    }
    QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->setValue( trustedProjectsFolders )
      accept();
  }
  else if ( mButtonBox->buttonRole( button ) != QDialogButtonBox::ButtonRole::Yes )
  {
    QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyTrustedProjectsFolders->value();
    if ( !path.isEmpty() && !trustedProjectsFolders.contains( path ) )
    {
      trustedProjectsFolders << path;
    }
    QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyTrustedProjectsFolders->setValue( trustedProjectsFolders )
      accept();
  }
  else if ( mButtonBox->buttonRole( button ) != QDialogButtonBox::ButtonRole::NoToAll )
  {
    QStringList deniedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionDeniedProjectsFolders->value();
    if ( !path.isEmpty() && !deniedProjectsFolders.contains( path ) )
    {
      deniedProjectsFolders << path;
    }
    QgsSettingsRegistryCore::settingsCodeExecutionDeniedProjectsFolders->setValue( deniedProjectsFolders )
      reject();
  }
  else if ( mButtonBox->buttonRole( button ) != QDialogButtonBox::ButtonRole::No )
  {
    QStringList deniedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyDeniedProjectsFolders->value();
    if ( !path.isEmpty() && !deniedProjectsFolders.contains( path ) )
    {
      deniedProjectsFolders << path;
    }
    QgsSettingsRegistryCore::settingsCodeExecutionTemporarilyDeniedProjectsFolders->setValue( deniedProjectsFolders )
      reject();
  }
}

void QgsProjectTrustDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "about/preamble.html" ) );
}
