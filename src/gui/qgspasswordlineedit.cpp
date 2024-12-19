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

QgsPasswordLineEdit::QgsPasswordLineEdit( QWidget *parent, bool passwordVisible )
  : QLineEdit( parent )
{
  mShowPasswordIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) );
  mHidePasswordIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideAllLayers.svg" ) );

  mActionShowHidePassword = addAction( mShowPasswordIcon, QLineEdit::TrailingPosition );
  mActionShowHidePassword->setCheckable( true );

  if ( mLockIconVisible )
  {
    mActionLock = addAction( QgsApplication::getThemeIcon( QStringLiteral( "/lockedGray.svg" ) ), QLineEdit::LeadingPosition );
  }

  setPasswordVisibility( passwordVisible );
  connect( mActionShowHidePassword, &QAction::triggered, this, &QgsPasswordLineEdit::togglePasswordVisibility );
}

void QgsPasswordLineEdit::setPasswordVisibility( bool visible )
{
  togglePasswordVisibility( visible );
}

void QgsPasswordLineEdit::togglePasswordVisibility( bool toggled )
{
  if ( toggled )
  {
    setEchoMode( QLineEdit::Normal );
    mActionShowHidePassword->setIcon( mHidePasswordIcon );
    mActionShowHidePassword->setToolTip( tr( "Hide text" ) );
  }
  else
  {
    setEchoMode( QLineEdit::Password );
    mActionShowHidePassword->setIcon( mShowPasswordIcon );
    mActionShowHidePassword->setToolTip( tr( "Show text" ) );
  }
}

void QgsPasswordLineEdit::setShowLockIcon( bool visible )
{
  mLockIconVisible = visible;
  if ( mLockIconVisible )
  {
    if ( !mActionLock )
    {
      mActionLock = addAction( QgsApplication::getThemeIcon( QStringLiteral( "/lockedGray.svg" ) ), QLineEdit::LeadingPosition );
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
