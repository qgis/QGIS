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

//
// QgsUserProfileOptionsWidget
//

QgsUserProfileOptionsWidget::QgsUserProfileOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  // Disable combobox if default profile is not selected
  mDefaultProfileComboBox->setEnabled( false );
  connect( mDefaultProfile, &QRadioButton::toggled, mDefaultProfileComboBox, &QComboBox::setEnabled );

  // Init radio buttons
  auto manager = QgisApp::instance()->userProfileManager();
  if ( manager->userProfileSelectionPolicy() == QgsUserProfileManager::UserProfileSelectionPolicy::LastProfile )
  {
    mLastProfile->setChecked( true );
  }
  else if ( manager->userProfileSelectionPolicy() == QgsUserProfileManager::UserProfileSelectionPolicy::AskUser )
  {
    mAskUser->setChecked( true );
  }
  else if ( manager->userProfileSelectionPolicy() == QgsUserProfileManager::UserProfileSelectionPolicy::DefaultProfile )
  {
    mDefaultProfile->setChecked( true );
  }

  // Fill combobox with profiles
  mDefaultProfileComboBox->clear();
  for ( auto profile : manager->allProfiles() )
  {
    QIcon icon = manager->profileForName( profile )->icon();
    mDefaultProfileComboBox->addItem( icon, profile );
  }
  mDefaultProfileComboBox->setCurrentText( manager->defaultProfileName() );
}

void QgsUserProfileOptionsWidget::apply()
{
  auto manager = QgisApp::instance()->userProfileManager();
  if ( mLastProfile->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( QgsUserProfileManager::UserProfileSelectionPolicy::LastProfile );
  }
  else if ( mAskUser->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( QgsUserProfileManager::UserProfileSelectionPolicy::AskUser );
  }
  else if ( mDefaultProfile->isChecked() )
  {
    manager->setUserProfileSelectionPolicy( QgsUserProfileManager::UserProfileSelectionPolicy::DefaultProfile );
    manager->setDefaultProfileName( mDefaultProfileComboBox->currentText() );
  }
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
