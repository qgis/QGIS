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
  bool accepted = false;
  const QString path = applyTrustToProjectFolder() ? mProjectAbsolutePath : mProjectAbsoluteFilePath;
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

    QDialogButtonBox::StandardButton buttonType = mButtonBox->standardButton( button );
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
  QgsHelp::openHelp( QStringLiteral( "about/preamble.html" ) );
}
