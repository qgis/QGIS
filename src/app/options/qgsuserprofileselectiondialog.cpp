/***************************************************************************
  qgsuserprofileselectiondialog.cpp
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Yoann Quenach de Quivillic
  Email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "qgsapplication.h"
#include "qgsuserprofilemanager.h"
#include "qgsnewnamedialog.h"

#include "qgsuserprofileselectiondialog.h"
#include "moc_qgsuserprofileselectiondialog.cpp"

QgsUserProfileSelectionDialog::QgsUserProfileSelectionDialog( QgsUserProfileManager *manager, QWidget *parent )
  : QDialog( parent ), mManager( manager )

{
  setupUi( this );
  setWindowIcon( QIcon( QgsApplication::appIconPath() ) );

  // Select user profile on double-click
  connect( mProfileListWidget, &QListWidget::itemDoubleClicked, this, &QgsUserProfileSelectionDialog::accept );

  // Add a new profile on button click
  connect( mAddProfileButton, &QPushButton::clicked, this, &QgsUserProfileSelectionDialog::onAddProfile );

  const int iconSize = mManager->settings()->value( QStringLiteral( "/selector/iconSize" ), 24 ).toInt();
  mProfileListWidget->setIconSize( QSize( iconSize, iconSize ) );

  // Fill the list of profiles
  mProfileListWidget->clear(); // Clear bogus profiles in the Ui form
  for ( auto profile : mManager->allProfiles() )
  {
    auto item = new QListWidgetItem( mManager->profileForName( profile )->icon(), profile );
    mProfileListWidget->addItem( item );

    // If the profile is the last used one, select it
    if ( profile == mManager->lastProfileName() )
    {
      mProfileListWidget->setCurrentItem( item );
      item->setSelected( true );
    }
  }
}

QString QgsUserProfileSelectionDialog::selectedProfileName() const
{
  return mProfileListWidget->currentItem()->text();
}

void QgsUserProfileSelectionDialog::accept()
{
  // Accept only if an item is selected
  if ( mProfileListWidget->currentItem() && mProfileListWidget->currentItem()->isSelected() )
  {
    QDialog::accept();
  }
}

void QgsUserProfileSelectionDialog::onAddProfile()
{
  // Ask for a new profile name
  QgsNewNameDialog dlg( QString(), QString(), QStringList(), mManager->allProfiles(), Qt::CaseInsensitive, this );
  dlg.setConflictingNameWarning( tr( "A profile with this name already exists" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setHintString( tr( "New profile name" ) );
  dlg.setWindowTitle( tr( "New Profile Name" ) );

  // Prevent from entering slashes and backslashes
  dlg.setRegularExpression( "[^/\\\\]+" );

  if ( dlg.exec() != QDialog::Accepted )
    return;

  // Try to create the profile folder
  QString profileName = dlg.name();
  QgsError error = mManager->createUserProfile( profileName );
  if ( error.isEmpty() )
  {
    auto item = new QListWidgetItem( QgsApplication::getThemeIcon( "user.svg" ), profileName );
    mProfileListWidget->addItem( item );
    mProfileListWidget->setCurrentItem( item );
    item->setSelected( true );
    // Automatically accept the dialog
    accept();
  }
  else
  {
    QMessageBox::warning( this, tr( "New Profile" ), tr( "Cannot create folder '%1'" ).arg( profileName ) );
    return;
  }
}

QgsUserProfileSelectionDialog::~QgsUserProfileSelectionDialog() {}
