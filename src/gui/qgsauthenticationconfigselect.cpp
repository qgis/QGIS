/***************************************************************************
    qgsauthenticationconfigselect.cpp
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#include "qgsauthenticationconfigselect.h"
#include "ui_qgsauthenticationconfigselect.h"

#include <QHash>
#include <QMessageBox>

#include "qgsauthenticationconfig.h"
#include "qgsauthenticationmanager.h"
#include "qgsauthenticationconfigwidget.h"


QgsAuthConfigSelect::QgsAuthConfigSelect( QWidget *parent, bool keypasssupported )
    : QWidget( parent )
    , mKeyPassSupported( keypasssupported )
    , mConfigId( QString() )
    , mConfigs( QHash<QString, QgsAuthConfigBase>() )
    , mAuthNotifyLayout( 0 )
    , mAuthNotify( 0 )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    lblPassUnsupported->setVisible( !mKeyPassSupported );

    clearConfig();
    populateConfigSelector();
  }
}

QgsAuthConfigSelect::~QgsAuthConfigSelect()
{
}

void QgsAuthConfigSelect::setKeyPassSupported( bool supported )
{
  if ( !QgsAuthManager::instance()->isDisabled() )
  {
    mKeyPassSupported = supported;
    lblPassUnsupported->setVisible( !mKeyPassSupported );
  }
}

void QgsAuthConfigSelect::setConfigId( const QString& authid )
{
  if ( QgsAuthManager::instance()->isDisabled() && mAuthNotify )
  {
    mAuthNotify->setText( QgsAuthManager::instance()->disabledMessage() + "\n\n" +
                          tr( "Authentication config id not loaded: %1" ).arg( authid ) );
  }
  else
  {
    if ( mConfigId != authid )
      mConfigId = authid;
    populateConfigSelector();
    loadConfig();
  }
}

void QgsAuthConfigSelect::loadConfig()
{
  clearConfig();
  if ( !mConfigId.isEmpty() && mConfigs.contains( mConfigId ) )
  {
    QgsAuthConfigBase config = mConfigs.value( mConfigId );
    leConfigProviderType->setText( QgsAuthType::typeDescription( config.type() ) );
    leConfigProviderType->setCursorPosition( 0 ); // left justify
    leConfigId->setText( config.id() );
    btnConfigEdit->setEnabled( true );
    btnConfigRemove->setEnabled( true );
  }
}

void QgsAuthConfigSelect::clearConfig()
{
  leConfigProviderType->clear();
  leConfigId->clear();
  btnConfigEdit->setEnabled( false );
  btnConfigRemove->setEnabled( false );
}

void QgsAuthConfigSelect::validateConfig()
{
  if ( !mConfigId.isEmpty() && !mConfigs.contains( mConfigId ) )
  {
    mConfigId.clear();
  }
}

void QgsAuthConfigSelect::populateConfigSelector()
{
  loadAvailableConfigs();
  validateConfig();

  cmbConfigSelect->blockSignals( true );
  cmbConfigSelect->clear();
  cmbConfigSelect->addItem( tr( "No authentication" ), "0" );

  QHash<QString, QgsAuthConfigBase>::iterator cit = mConfigs.begin();
  for ( cit = mConfigs.begin(); cit != mConfigs.end(); ++cit )
  {
    QgsAuthConfigBase config = cit.value();
    cmbConfigSelect->addItem( config.name(), cit.key() );
  }
  cmbConfigSelect->blockSignals( false );

  int indx = 0;
  if ( !mConfigId.isEmpty() )
  {
    indx = cmbConfigSelect->findData( mConfigId );
  }
  cmbConfigSelect->setCurrentIndex( indx > 0 ? indx : 0 );
}

void QgsAuthConfigSelect::loadAvailableConfigs()
{
  mConfigs.clear();
  mConfigs = QgsAuthManager::instance()->availableConfigs();
}

void QgsAuthConfigSelect::on_cmbConfigSelect_currentIndexChanged( int index )
{
  QString authid = cmbConfigSelect->itemData( index ).toString();
  mConfigId = ( !authid.isEmpty() && authid != QString( "0" ) ) ? authid : QString();
  loadConfig();
}

void QgsAuthConfigSelect::on_btnConfigAdd_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigWidget * aw = new QgsAuthConfigWidget( this );
  aw->setWindowModality( Qt::WindowModal );
  if ( aw->exec() )
  {
    setConfigId( aw->configId() );
  }
}

void QgsAuthConfigSelect::on_btnConfigEdit_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigWidget * aw = new QgsAuthConfigWidget( this, mConfigId );
  aw->setWindowModality( Qt::WindowModal );
  if ( aw->exec() )
  {
    setConfigId( mConfigId );
  }
}

void QgsAuthConfigSelect::on_btnConfigRemove_clicked()
{
  if ( QMessageBox::warning( this, tr( "Remove Authentication" ),
                             tr( "Are you sure that you want to permanently remove this configuration right now?\n\n"
                                 "Operation can NOT be undone!" ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  if ( QgsAuthManager::instance()->removeAuthenticationConfig( mConfigId ) )
  {
    setConfigId( QString() );
  }
}
