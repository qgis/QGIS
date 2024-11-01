/***************************************************************************
    qgsuserprofileoptions.cpp
    -----------------
    begin                : February 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsuserprofilemanager.h"

#include "qgsuserprofileoptions.h"
#include "moc_qgsuserprofileoptions.cpp"

//
// QgsUserProfileOptionsWidget
//

QgsUserProfileOptionsWidget::QgsUserProfileOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsUserProfileManager *manager = QgisApp::instance()->userProfileManager();

  // Disable combobox if default profile is not selected
  mDefaultProfileComboBox->setEnabled( false );
  connect( mDefaultProfile, &QRadioButton::toggled, mDefaultProfileComboBox, &QComboBox::setEnabled );

  // Disable Profile selector items if Ask user is not selected
  mIconSizeLabel->setEnabled( false );
  mIconSize->setEnabled( false );
  connect( mAskUser, &QRadioButton::toggled, this, &QgsUserProfileOptionsWidget::onAskUserChanged );

  // Connect icon size and allow profile creation
  mIconSize->setCurrentText( QString::number( manager->settings()->value( QStringLiteral( "/selector/iconSize" ), 24 ).toInt() ) );
  connect( mIconSize, &QComboBox::currentTextChanged, this, [manager]( const QString &text ) {
    manager->settings()->setValue( QStringLiteral( "/selector/iconSize" ), text.toInt() );
    manager->settings()->sync();
  } );

  // Connect change icon button
  connect( mActiveProfileIconButton, &QToolButton::clicked, this, &QgsUserProfileOptionsWidget::onChangeIconClicked );
  connect( mResetIconButton, &QToolButton::clicked, this, &QgsUserProfileOptionsWidget::onResetIconClicked );

  // Init radio buttons
  if ( manager->userProfileSelectionPolicy() == Qgis::UserProfileSelectionPolicy::LastProfile )
  {
    mLastProfile->setChecked( true );
  }
  else if ( manager->userProfileSelectionPolicy() == Qgis::UserProfileSelectionPolicy::AskUser )
  {
    mAskUser->setChecked( true );
  }
  else if ( manager->userProfileSelectionPolicy() == Qgis::UserProfileSelectionPolicy::DefaultProfile )
  {
    mDefaultProfile->setChecked( true );
  }

  // Fill combobox with profiles
  mDefaultProfileComboBox->clear();
  for ( const QString &profile : manager->allProfiles() )
  {
    QIcon icon = manager->profileForName( profile )->icon();
    mDefaultProfileComboBox->addItem( icon, profile );
  }
  mDefaultProfileComboBox->setCurrentText( manager->defaultProfileName() );

  // Init Active profile name and icon
  mActiveProfileIconButton->setIcon( manager->userProfile()->icon() );
  mActiveProfileIconLabel->setText( tr( "Active Profile (%1) icon", "Active profile icon" ).arg( manager->userProfile()->name() ) );
}

QString QgsUserProfileOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#user-profiles" );
}

void QgsUserProfileOptionsWidget::apply()
{
  QgsUserProfileManager *manager = QgisApp::instance()->userProfileManager();
  if ( mLastProfile->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( Qgis::UserProfileSelectionPolicy::LastProfile );
  }
  else if ( mAskUser->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( Qgis::UserProfileSelectionPolicy::AskUser );
  }
  else if ( mDefaultProfile->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( Qgis::UserProfileSelectionPolicy::DefaultProfile );
    manager->setDefaultProfileName( mDefaultProfileComboBox->currentText() );
  }
}

void QgsUserProfileOptionsWidget::onChangeIconClicked()
{
  const QgsUserProfile *activeProfile = QgisApp::instance()->userProfileManager()->userProfile();
  const QString iconPath = QFileDialog::getOpenFileName( this, tr( "Select Icon" ), "", tr( "Images (*.png *.jpg *.jpeg *.gif *.bmp *.svg)" ) );
  if ( !iconPath.isEmpty() )
  {
    // Remove existing icon files
    QDir dir( activeProfile->folder(), "icon.*", QDir::Name, QDir::Files );
    for ( const QString &file : dir.entryList() )
    {
      dir.remove( file );
    }
    // Copy the icon file to the profile folder
    const QString extension = QFileInfo( iconPath ).suffix();
    const QString dstPath = activeProfile->folder() + QDir::separator() + "icon." + extension;
    QFile::copy( iconPath, dstPath );

    // Update the button icon
    mActiveProfileIconButton->setIcon( QIcon( iconPath ) );
    mDefaultProfileComboBox->setItemIcon( mDefaultProfileComboBox->findText( activeProfile->name() ), activeProfile->icon() );
  }
}

void QgsUserProfileOptionsWidget::onResetIconClicked()
{
  const QgsUserProfile *activeProfile = QgisApp::instance()->userProfileManager()->userProfile();
  // Remove existing icon files
  QDir dir( activeProfile->folder(), "icon.*", QDir::Name, QDir::Files );
  for ( const QString &file : dir.entryList() )
  {
    dir.remove( file );
  }
  // Update the button icon
  mActiveProfileIconButton->setIcon( activeProfile->icon() );
  mDefaultProfileComboBox->setItemIcon( mDefaultProfileComboBox->findText( activeProfile->name() ), activeProfile->icon() );
}

void QgsUserProfileOptionsWidget::onAskUserChanged()
{
  mIconSizeLabel->setEnabled( mAskUser->isChecked() );
  mIconSize->setEnabled( mAskUser->isChecked() );
}

//
// QgsUserProfileOptionsFactory
//
QgsUserProfileOptionsFactory::QgsUserProfileOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "User Profiles" ), QgsApplication::getThemeIcon( QStringLiteral( "/user.svg" ) ) )
{
}

QgsOptionsPageWidget *QgsUserProfileOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsUserProfileOptionsWidget( parent );
}

QString QgsUserProfileOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsPageCRS" );
}
