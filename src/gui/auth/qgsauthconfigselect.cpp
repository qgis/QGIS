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
#include <QTimer>

#include "qgsauthconfig.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgslogger.h"


QgsAuthConfigSelect::QgsAuthConfigSelect( QWidget *parent, const QString &dataprovider )
    : QWidget( parent )
    , mAuthCfg( QString() )
    , mDataProvider( dataprovider )
    , mConfigs( QgsAuthMethodConfigsMap() )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );

    leConfigMsg->setStyleSheet( QString( "QLineEdit{background-color: %1}" )
                                .arg( QgsAuthGuiUtils::yellowColor().name() ) );

    clearConfig();
    clearMessage();
    populateConfigSelector();
  }
}

QgsAuthConfigSelect::~QgsAuthConfigSelect()
{
}

void QgsAuthConfigSelect::setConfigId( const QString& authcfg )
{
  if ( mDisabled && mAuthNotify )
  {
    mAuthNotify->setText( QgsAuthManager::instance()->disabledMessage() + "\n\n" +
                          tr( "Authentication config id not loaded: %1" ).arg( authcfg ) );
  }
  else
  {
    if ( mAuthCfg != authcfg )
    {
      mAuthCfg = authcfg;
    }
    populateConfigSelector();
    loadConfig();
  }
}

void QgsAuthConfigSelect::setDataProviderKey( const QString &key )
{
  if ( mDisabled )
  {
    return;
  }

  mDataProvider = key;
  populateConfigSelector();
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
  emit selectedConfigIdChanged( mAuthCfg );
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
    showMessage( tr( "Configuration '%1' not in database" ).arg( mAuthCfg ) );
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
  QgsAuthMethodConfigsMap::const_iterator cit = mConfigs.constBegin();
  for ( cit = mConfigs.constBegin(); cit != mConfigs.constEnd(); ++cit )
  {
    QgsAuthMethodConfig config = cit.value();
    sortmap.insert( config.name(), cit.key() );
  }

  QgsStringMap::const_iterator sm = sortmap.constBegin();
  for ( sm = sortmap.constBegin(); sm != sortmap.constEnd(); ++sm )
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

void QgsAuthConfigSelect::showMessage( const QString &msg )
{
  if ( mDisabled )
  {
    return;
  }
  leConfigMsg->setText( msg );
  frConfigMsg->setVisible( true );
}

void QgsAuthConfigSelect::clearMessage()
{
  if ( mDisabled )
  {
    return;
  }
  leConfigMsg->clear();
  frConfigMsg->setVisible( false );
}

void QgsAuthConfigSelect::loadAvailableConfigs()
{
  mConfigs.clear();
  mConfigs = QgsAuthManager::instance()->availableAuthMethodConfigs( mDataProvider );
}

void QgsAuthConfigSelect::on_cmbConfigSelect_currentIndexChanged( int index )
{
  QString authcfg = cmbConfigSelect->itemData( index ).toString();
  mAuthCfg = ( !authcfg.isEmpty() && authcfg != QLatin1String( "0" ) ) ? authcfg : QString();
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
    //qDebug( "Edit returned config Id: %s", ace->configId().toAscii().constData() );
    setConfigId( ace->configId() );
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
    emit selectedConfigIdRemoved( mAuthCfg );
    setConfigId( QString() );
  }
}

void QgsAuthConfigSelect::on_btnConfigMsgClear_clicked()
{
  clearMessage();
}


//////////////// Embed in dialog ///////////////////

#include <QPushButton>

QgsAuthConfigUriEdit::QgsAuthConfigUriEdit( QWidget *parent, const QString &datauri, const QString &dataprovider )
    : QDialog( parent )
    , mAuthCfg( QString() )
    , mDataUri( QString() )
    , mDataUriOrig( QString() )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );

    setWindowTitle( tr( "Authentication Config ID String Editor" ) );

    buttonBox->button( QDialogButtonBox::Close )->setDefault( true );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveChanges() ) );

    connect( buttonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( resetChanges() ) );

    connect( wdgtAuthSelect, SIGNAL( selectedConfigIdChanged( QString ) ), this , SLOT( authCfgUpdated( QString ) ) );
    connect( wdgtAuthSelect, SIGNAL( selectedConfigIdRemoved( QString ) ), this , SLOT( authCfgRemoved( QString ) ) );

    wdgtAuthSelect->setDataProviderKey( dataprovider );
    setDataSourceUri( datauri );
  }
}

QgsAuthConfigUriEdit::~QgsAuthConfigUriEdit()
{
}

void QgsAuthConfigUriEdit::setDataSourceUri( const QString &datauri )
{
  if ( mDisabled )
  {
    return;
  }
  if ( datauri.isEmpty() )
    return;

  mDataUri = mDataUriOrig = datauri;

  teDataUri->setPlainText( mDataUri );

  if ( authCfgIndex() == -1 )
  {
    wdgtAuthSelect->showMessage( tr( "No authcfg in Data Source URI" ) );
    return;
  }

  selectAuthCfgInUri();

  mAuthCfg = authCfgFromUri();

  QgsDebugMsg( QString( "Parsed authcfg ID: %1" ).arg( mAuthCfg ) );

  wdgtAuthSelect->blockSignals( true );
  wdgtAuthSelect->setConfigId( mAuthCfg );
  wdgtAuthSelect->blockSignals( false );
}

QString QgsAuthConfigUriEdit::dataSourceUri()
{
  if ( mDisabled )
  {
    return QString();
  }
  return mDataUri;
}

bool QgsAuthConfigUriEdit::hasConfigID( const QString &txt )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    return false;
  }
  return QgsAuthManager::instance()->hasConfigId( txt );
}

void QgsAuthConfigUriEdit::saveChanges()
{
  this->accept();
}

void QgsAuthConfigUriEdit::resetChanges()
{
  wdgtAuthSelect->clearMessage();
  setDataSourceUri( mDataUriOrig );
}

void QgsAuthConfigUriEdit::authCfgUpdated( const QString &authcfg )
{
  mAuthCfg = authcfg;

  if ( mAuthCfg.size() != 7 )
  {
    mAuthCfg.clear();
    removeAuthCfgFromUri();
  }
  else
  {
    updateUriWithAuthCfg();
  }
  teDataUri->clear();
  teDataUri->setPlainText( mDataUri );
  selectAuthCfgInUri();
}

void QgsAuthConfigUriEdit::authCfgRemoved( const QString &authcfg )
{
  if ( authCfgFromUri() == authcfg )
  {
    removeAuthCfgFromUri();
  }
}

int QgsAuthConfigUriEdit::authCfgIndex()
{
  QRegExp rx( QgsAuthManager::instance()->configIdRegex() );
  return rx.indexIn( mDataUri );
}

QString QgsAuthConfigUriEdit::authCfgFromUri()
{
  int startindex = authCfgIndex();
  if ( startindex == -1 )
    return QString();

  return mDataUri.mid( startindex + 8, 7 );
}

void QgsAuthConfigUriEdit::selectAuthCfgInUri()
{
  int startindex = authCfgIndex();
  if ( startindex == -1 )
    return;

  // authcfg=.{7} will always be 15 chars
  QTextCursor tc = teDataUri->textCursor();
  tc.setPosition( startindex );
  tc.setPosition( startindex + 15, QTextCursor::KeepAnchor );
  teDataUri->setTextCursor( tc );
  teDataUri->setFocus();
}

void QgsAuthConfigUriEdit::updateUriWithAuthCfg()
{
  int startindex = authCfgIndex();
  if ( startindex == -1 )
  {
    if ( mAuthCfg.size() == 7 )
    {
      wdgtAuthSelect->showMessage( tr( "Adding authcfg to URI not supported" ) );
    }
    return;
  }

  mDataUri = mDataUri.replace( startindex + 8, 7, mAuthCfg );
}

void QgsAuthConfigUriEdit::removeAuthCfgFromUri()
{
  int startindex = authCfgIndex();
  if ( startindex == -1 )
    return;

  // add any preceding space so two spaces will not result after removal
  int rmvlen = 15;
  if ( startindex - 1 >= 0
       && ( mDataUri.at( startindex - 1 ).isSpace()
            || mDataUri.at( startindex - 1 ) == QChar( '&' ) ) )
  {
    startindex -= 1;
    rmvlen += 1;
  }

  // trim any leftover spaces or & from ends
  mDataUri = mDataUri.remove( startindex, rmvlen ).trimmed();
  if ( mDataUri.at( 0 ) == QChar( '&' ) )
    mDataUri = mDataUri.remove( 0, 1 );

  // trim any & from

  mAuthCfg.clear();
}

