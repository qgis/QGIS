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

#include "qgsauthconfig.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsauthmethodmetadata.h"

#include <QHash>
#include <QMessageBox>
#include <QTimer>
#include <QRegularExpression>


QgsAuthConfigSelect::QgsAuthConfigSelect( QWidget *parent, const QString &dataprovider )
  : QWidget( parent )
  , mDataProvider( dataprovider )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    connect( cmbConfigSelect, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAuthConfigSelect::cmbConfigSelect_currentIndexChanged );
    connect( btnConfigAdd, &QToolButton::clicked, this, &QgsAuthConfigSelect::btnConfigAdd_clicked );
    connect( btnConfigEdit, &QToolButton::clicked, this, &QgsAuthConfigSelect::btnConfigEdit_clicked );
    connect( btnConfigRemove, &QToolButton::clicked, this, &QgsAuthConfigSelect::btnConfigRemove_clicked );
    connect( btnConfigMsgClear, &QToolButton::clicked, this, &QgsAuthConfigSelect::btnConfigMsgClear_clicked );

    // Set icons and remove texts
    btnConfigAdd->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );
    btnConfigRemove->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyRemove.svg" ) ) );
    btnConfigEdit->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
    btnConfigMsgClear->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconClose.svg" ) ) );

    btnConfigAdd->setText( QString() );
    btnConfigRemove->setText( QString() );
    btnConfigEdit->setText( QString() );
    btnConfigMsgClear->setText( QString() );

    leConfigMsg->setStyleSheet( QStringLiteral( "QLineEdit{background-color: %1}" )
                                .arg( QgsAuthGuiUtils::yellowColor().name() ) );

    clearConfig();
    clearMessage();
    populateConfigSelector();
  }
}

void QgsAuthConfigSelect::setConfigId( const QString &authcfg )
{
  if ( mDisabled && mAuthNotify )
  {
    mAuthNotify->setText( QgsApplication::authManager()->disabledMessage() + "\n\n" +
                          tr( "Authentication config id not loaded: %1" ).arg( authcfg ) );
  }
  else
  {
    if ( mAuthCfg != authcfg )
    {
      mAuthCfg = authcfg;
    }
    // avoid duplicate call to loadConfig(), which may potentially be triggered by combo box index changes in the
    // call to populateConfigSelector(). We *always* call loadConfig() after this, so we don't want to do it twice.
    mTemporarilyBlockLoad = true;
    populateConfigSelector();
    mTemporarilyBlockLoad = false;
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
    const QgsAuthMethodConfig config = mConfigs.value( mAuthCfg );
    const QString authMethodKey = QgsApplication::authManager()->configAuthMethodKey( mAuthCfg );
    QString methoddesc = tr( "Missing authentication method description" );
    const QgsAuthMethodMetadata *meta = QgsApplication::authManager()->authMethodMetadata( authMethodKey );
    if ( meta )
    {
      methoddesc = meta->description();
    }
    cmbConfigSelect->setToolTip( tr( "<ul><li><b>Method type:</b> %1</li>"
                                     "<li><b>Configuration ID:</b> %2</li></ul>" ).arg( methoddesc, config.id( ) ) );
    btnConfigEdit->setEnabled( true );
    btnConfigRemove->setEnabled( true );
  }
  emit selectedConfigIdChanged( mAuthCfg );
}

void QgsAuthConfigSelect::clearConfig()
{
  cmbConfigSelect->setToolTip( QString() );
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
  cmbConfigSelect->addItem( tr( "No Authentication" ), "0" );

  QgsStringMap sortmap;
  QgsAuthMethodConfigsMap::const_iterator cit = mConfigs.constBegin();
  for ( cit = mConfigs.constBegin(); cit != mConfigs.constEnd(); ++cit )
  {
    const QgsAuthMethodConfig config = cit.value();
    sortmap.insert( QStringLiteral( "%1 (%2)" ).arg( config.name(), config.method() ), cit.key() );
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
  mConfigs = QgsApplication::authManager()->availableAuthMethodConfigs( mDataProvider );
}

void QgsAuthConfigSelect::cmbConfigSelect_currentIndexChanged( int index )
{
  const QString authcfg = cmbConfigSelect->itemData( index ).toString();
  mAuthCfg = ( !authcfg.isEmpty() && authcfg != QLatin1String( "0" ) ) ? authcfg : QString();
  if ( !mTemporarilyBlockLoad )
    loadConfig();
}

void QgsAuthConfigSelect::btnConfigAdd_clicked()
{
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this, QString(), mDataProvider );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    setConfigId( ace->configId() );
  }
  ace->deleteLater();
}

void QgsAuthConfigSelect::btnConfigEdit_clicked()
{
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this, mAuthCfg, mDataProvider );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    //qDebug( "Edit returned config Id: %s", ace->configId().toLatin1().constData() );
    setConfigId( ace->configId() );
  }
  ace->deleteLater();
}

void QgsAuthConfigSelect::btnConfigRemove_clicked()
{
  if ( QMessageBox::warning( this, tr( "Remove Authentication" ),
                             tr( "Are you sure that you want to permanently remove this configuration right now?\n\n"
                                 "Operation can NOT be undone!" ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  if ( QgsApplication::authManager()->removeAuthenticationConfig( mAuthCfg ) )
  {
    emit selectedConfigIdRemoved( mAuthCfg );
    setConfigId( QString() );
  }
}

void QgsAuthConfigSelect::btnConfigMsgClear_clicked()
{
  clearMessage();
}


//////////////// Embed in dialog ///////////////////

#include <QPushButton>

QgsAuthConfigUriEdit::QgsAuthConfigUriEdit( QWidget *parent, const QString &datauri, const QString &dataprovider )
  : QDialog( parent )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );

    setWindowTitle( tr( "Authentication Config ID String Editor" ) );

    buttonBox->button( QDialogButtonBox::Close )->setDefault( true );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
    connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAuthConfigUriEdit::saveChanges );

    connect( buttonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked, this, &QgsAuthConfigUriEdit::resetChanges );

    connect( wdgtAuthSelect, &QgsAuthConfigSelect::selectedConfigIdChanged, this, &QgsAuthConfigUriEdit::authCfgUpdated );
    connect( wdgtAuthSelect, &QgsAuthConfigSelect::selectedConfigIdRemoved, this, &QgsAuthConfigUriEdit::authCfgRemoved );

    wdgtAuthSelect->setDataProviderKey( dataprovider );
    setDataSourceUri( datauri );
  }
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

  QgsDebugMsg( QStringLiteral( "Parsed authcfg ID: %1" ).arg( mAuthCfg ) );

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

bool QgsAuthConfigUriEdit::hasConfigId( const QString &txt )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    return false;
  }
  return QgsApplication::authManager()->hasConfigId( txt );
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
  return mDataUri.indexOf( QRegularExpression( QgsApplication::authManager()->configIdRegex() ) );
}

QString QgsAuthConfigUriEdit::authCfgFromUri()
{
  const int startindex = authCfgIndex();
  if ( startindex == -1 )
    return QString();

  return mDataUri.mid( startindex + 8, 7 );
}

void QgsAuthConfigUriEdit::selectAuthCfgInUri()
{
  const int startindex = authCfgIndex();
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
  const int startindex = authCfgIndex();
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

