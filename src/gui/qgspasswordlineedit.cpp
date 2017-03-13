/***************************************************************************
                              qgspasswordlineedit.cpp
                              ------------------------
  begin                : March 13, 2017
  copyright            : (C) 2017 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspasswordlineedit.h"
#include "qgsapplication.h"

QgsPasswordLineEdit::QgsPasswordLineEdit( QWidget *parent )
  : QLineEdit( parent )
  , mActionShowHidePassword( nullptr )
  , mActionLock( nullptr )
  , mLockIconVisible( false )
{
  mShowPasswordIcon = QgsApplication::getThemeIcon( "/mActionShowAllLayers.svg" );
  mHidePasswordIcon = QgsApplication::getThemeIcon( "/mActionHideAllLayers.svg" );

  mActionShowHidePassword = addAction( mShowPasswordIcon, QLineEdit::TrailingPosition );
  mActionShowHidePassword->setCheckable( true );

  if ( mLockIconVisible )
  {
    mActionLock = addAction( QgsApplication::getThemeIcon( "/lockedGray.svg" ), QLineEdit::LeadingPosition );
  }

  connect( mActionShowHidePassword, SIGNAL( triggered( bool ) ), this, SLOT( togglePasswordVisibility( bool ) ) );
}

void QgsPasswordLineEdit::togglePasswordVisibility( bool toggled )
{
  if ( toggled )
  {
    setEchoMode( QLineEdit::Normal );
    mActionShowHidePassword->setIcon( mHidePasswordIcon );
  }
  else
  {
    setEchoMode( QLineEdit::Password );
    mActionShowHidePassword->setIcon( mShowPasswordIcon );
  }
}

void QgsPasswordLineEdit::setShowLockIcon( bool visible )
{
  mLockIconVisible = visible;
  if ( mLockIconVisible )
  {
    if ( !mActionLock )
    {
      mActionLock = addAction( QgsApplication::getThemeIcon( "/lockedGray.svg" ), QLineEdit::LeadingPosition );
    }
  }
  else
  {
    if ( mActionLock )
    {
      removeAction( mActionLock );
      mActionLock = nullptr;
    }
  }
}
