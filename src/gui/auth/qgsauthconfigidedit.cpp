/***************************************************************************
    qgsauthconfigidedit.cpp
    ---------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthconfigidedit.h"
#include "ui_qgsauthconfigidedit.h"

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"


QgsAuthConfigIdEdit::QgsAuthConfigIdEdit( QWidget *parent, const QString &authcfg , bool allowEmpty )
    : QWidget( parent )
    , mAuthCfgOrig( authcfg )
    , mValid( false )
    , mAllowEmpty( allowEmpty )
{
  setupUi( this );

  leAuthCfg->setReadOnly( true );

  connect( this, SIGNAL( validityChanged( bool ) ), this, SLOT( updateValidityStyle( bool ) ) );

  leAuthCfg->setText( authcfg );
  updateValidityStyle( validate() );
}

QgsAuthConfigIdEdit::~QgsAuthConfigIdEdit()
{
}

const QString QgsAuthConfigIdEdit::configId()
{
  if ( validate() )
  {
    return leAuthCfg->text();
  }
  return QString();
}

bool QgsAuthConfigIdEdit::validate()
{
  QString authcfg( leAuthCfg->text() );
  bool curvalid = (( authcfg == mAuthCfgOrig && authcfg.size() == 7 )
                   || ( mAllowEmpty && authcfg.isEmpty() ) );

  if ( !QgsAuthManager::instance()->isDisabled() && !curvalid && authcfg.size() == 7 && isAlphaNumeric( authcfg ) )
  {
    curvalid = QgsAuthManager::instance()->configIdUnique( authcfg );
  }

  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }

  return curvalid;
}

void QgsAuthConfigIdEdit::setAuthConfigId( const QString &authcfg )
{
  if ( mAuthCfgOrig.isEmpty() )
  {
    mAuthCfgOrig = authcfg;
  }
  leAuthCfg->setText( authcfg );
  validate();
}

void QgsAuthConfigIdEdit::setAllowEmptyId( bool allowed )
{
  mAllowEmpty = allowed;
  validate();
}

void QgsAuthConfigIdEdit::clear()
{
  leAuthCfg->setText( mAuthCfgOrig );
  updateValidityStyle( true );
}

void QgsAuthConfigIdEdit::updateValidityStyle( bool valid )
{
  QString ss( "QLineEdit{" );
  ss += valid ? "" : QString( "color: %1;" ).arg( QgsAuthGuiUtils::redColor().name() );
  ss += !btnLock->isChecked() ? "" : QString( "background-color: %1;" ).arg( QgsAuthGuiUtils::yellowColor().name() );
  ss += '}';

  leAuthCfg->setStyleSheet( ss );
}

void QgsAuthConfigIdEdit::on_btnLock_toggled( bool checked )
{
  leAuthCfg->setReadOnly( !checked );
  if ( checked )
    leAuthCfg->setFocus();

  updateValidityStyle( validate() );
}

void QgsAuthConfigIdEdit::on_leAuthCfg_textChanged( const QString &txt )
{
  Q_UNUSED( txt );
  validate();
}

bool QgsAuthConfigIdEdit::isAlphaNumeric( const QString &authcfg )
{
  QRegExp rx( "([a-z]|[A-Z]|[0-9]){7}" );
  return rx.indexIn( authcfg ) != -1;
}
