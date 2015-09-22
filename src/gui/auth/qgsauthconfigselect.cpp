/***************************************************************************
    qgsauthconfigselect.cpp
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

#include "qgsauthconfigselect.h"
#include "ui_qgsauthconfigselect.h"

#include <QHash>
#include <QMessageBox>

#include "qgsauthconfig.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"


QgsAuthConfigSelect::QgsAuthConfigSelect( QWidget *parent, const QString &dataprovider )
    : QWidget( parent )
    , mAuthCfg( QString() )
    , mDataProvider( dataprovider )
    , mConfigs( QgsAuthMethodConfigsMap() )
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

    clearConfig();
    populateConfigSelector();
  }
}

QgsAuthConfigSelect::~QgsAuthConfigSelect()
{
}

void QgsAuthConfigSelect::setConfigId( const QString& authcfg )
{
  if ( QgsAuthManager::instance()->isDisabled() && mAuthNotify )
  {
    mAuthNotify->setText( QgsAuthManager::instance()->disabledMessage() + "\n\n" +
                          tr( "Authentication config id not loaded: %1" ).arg( authcfg ) );
  }
  else
  {
    if ( mAuthCfg != authcfg )
      mAuthCfg = authcfg;
    populateConfigSelector();
    loadConfig();
  }
}

void QgsAuthConfigSelect::loadConfig()
{
  clearConfig();
  if ( !mAuthCfg.isEmpty() && mConfigs.contains( mAuthCfg ) )
  {
    QgsAuthMethodConfig config = mConfigs.value( mAuthCfg );
    QgsAuthMethod * authmethod = QgsAuthManager::instance()->configAuthMethod( mAuthCfg );
    QString methoddesc = tr( "Missing authentication method description" );
    if ( authmethod )
    {
      methoddesc = authmethod->description();
    }
    leConfigMethodDesc->setText( methoddesc );
    leConfigMethodDesc->setCursorPosition( 0 ); // left justify
    leConfigId->setText( config.id() );
    btnConfigEdit->setEnabled( true );
    btnConfigRemove->setEnabled( true );
  }
}

void QgsAuthConfigSelect::clearConfig()
{
  leConfigMethodDesc->clear();
  leConfigId->clear();
  btnConfigEdit->setEnabled( false );
  btnConfigRemove->setEnabled( false );
}

void QgsAuthConfigSelect::validateConfig()
{
  if ( !mAuthCfg.isEmpty() && !mConfigs.contains( mAuthCfg ) )
  {
    mAuthCfg.clear();
  }
}

void QgsAuthConfigSelect::populateConfigSelector()
{
  loadAvailableConfigs();
  validateConfig();

  cmbConfigSelect->blockSignals( true );
  cmbConfigSelect->clear();
  cmbConfigSelect->addItem( tr( "No authentication" ), "0" );

  QgsStringMap sortmap;
  QgsAuthMethodConfigsMap::iterator cit = mConfigs.begin();
  for ( cit = mConfigs.begin(); cit != mConfigs.end(); ++cit )
  {
    QgsAuthMethodConfig config = cit.value();
    sortmap.insert( config.name(), cit.key() );
  }

  QgsStringMap::iterator sm = sortmap.begin();
  for ( sm = sortmap.begin(); sm != sortmap.end(); ++sm )
  {
    cmbConfigSelect->addItem( sm.key(), sm.value() );
  }
  cmbConfigSelect->blockSignals( false );

  int indx = 0;
  if ( !mAuthCfg.isEmpty() )
  {
    indx = cmbConfigSelect->findData( mAuthCfg );
  }
  cmbConfigSelect->setCurrentIndex( indx > 0 ? indx : 0 );
}

void QgsAuthConfigSelect::loadAvailableConfigs()
{
  mConfigs.clear();
  mConfigs = QgsAuthManager::instance()->availableAuthMethodConfigs( mDataProvider );
}

void QgsAuthConfigSelect::on_cmbConfigSelect_currentIndexChanged( int index )
{
  QString authcfg = cmbConfigSelect->itemData( index ).toString();
  mAuthCfg = ( !authcfg.isEmpty() && authcfg != QString( "0" ) ) ? authcfg : QString();
  loadConfig();
}

void QgsAuthConfigSelect::on_btnConfigAdd_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this, QString(), mDataProvider );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    setConfigId( ace->configId() );
  }
  ace->deleteLater();
}

void QgsAuthConfigSelect::on_btnConfigEdit_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this, mAuthCfg, mDataProvider );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    setConfigId( mAuthCfg );
  }
  ace->deleteLater();
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

  if ( QgsAuthManager::instance()->removeAuthenticationConfig( mAuthCfg ) )
  {
    setConfigId( QString() );
  }
}
