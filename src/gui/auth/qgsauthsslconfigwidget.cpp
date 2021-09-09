/***************************************************************************
    qgsauthsslconfigwidget.cpp
    ---------------------
    begin                : May 17, 2015
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

#include "qgsauthsslconfigwidget.h"
#include "qgsauthcertificateinfo.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QUrl>

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"


static void setItemBold_( QTreeWidgetItem *item )
{
  item->setFirstColumnSpanned( true );
  QFont secf( item->font( 0 ) );
  secf.setBold( true );
  item->setFont( 0, secf );
}

static const QString configFoundText_() { return QObject::tr( "Configuration loaded from database" ); }
static const QString configNotFoundText_() { return QObject::tr( "Configuration not found in database" ); }

QgsAuthSslConfigWidget::QgsAuthSslConfigWidget( QWidget *parent,
    const QSslCertificate &cert,
    const QString &hostport,
    const QList<QSslCertificate> &connectionCAs )
  : QWidget( parent )
  , mCert( nullptr )
  , mConnectionCAs( connectionCAs )
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
    connect( btnCertInfo, &QToolButton::clicked, this, &QgsAuthSslConfigWidget::btnCertInfo_clicked );

    connect( grpbxSslConfig, &QGroupBox::toggled, this, &QgsAuthSslConfigWidget::configEnabledChanged );
    connect( this, &QgsAuthSslConfigWidget::configEnabledChanged, this, &QgsAuthSslConfigWidget::readyToSave );
    connect( this, &QgsAuthSslConfigWidget::hostPortValidityChanged, this, &QgsAuthSslConfigWidget::readyToSave );

    setUpSslConfigTree();

    lblLoadedConfig->setVisible( false );
    lblLoadedConfig->clear();

    connect( leHost, &QLineEdit::textChanged,
             this, &QgsAuthSslConfigWidget::validateHostPortText );

    if ( !cert.isNull() )
    {
      setSslCertificate( cert, hostport );
    }
  }
}

QGroupBox *QgsAuthSslConfigWidget::certificateGroupBox()
{
  if ( mDisabled )
  {
    return nullptr;
  }
  return grpbxCert;
}

QGroupBox *QgsAuthSslConfigWidget::sslConfigGroupBox()
{
  if ( mDisabled )
  {
    return nullptr;
  }
  return grpbxSslConfig;
}

// private
QTreeWidgetItem *QgsAuthSslConfigWidget::addRootItem( const QString &label )
{
  QTreeWidgetItem *item = new QTreeWidgetItem(
    QStringList() << label,
    static_cast<int>( ConfigParent ) );
  setItemBold_( item );
  item->setTextAlignment( 0, Qt::AlignVCenter );
  item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->insertTopLevelItem( treeSslConfig->topLevelItemCount(), item );

  return item;
}

void QgsAuthSslConfigWidget::setUpSslConfigTree()
{
  treeSslConfig->setColumnCount( 1 );

  // add config field names
  mProtocolItem = addRootItem( tr( "Protocol" ) );
  mProtocolCmbBx = new QComboBox( treeSslConfig );
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SecureProtocols ),
                           static_cast<int>( QSsl::SecureProtocols ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::TlsV1SslV3 ),
                           static_cast<int>( QSsl::TlsV1SslV3 ) );
#endif
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::TlsV1_0 ),
                           static_cast<int>( QSsl::TlsV1_0 ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SslV3 ),
                           static_cast<int>( QSsl::SslV3 ) );
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SslV2 ),
                           static_cast<int>( QSsl::SslV2 ) );
#endif
  mProtocolCmbBx->setMaximumWidth( 300 );
  mProtocolCmbBx->setCurrentIndex( 0 );
  QTreeWidgetItem *protocolitem = new QTreeWidgetItem(
    mProtocolItem,
    QStringList() << QString(),
    static_cast<int>( ConfigItem ) );
  protocolitem->setFlags( protocolitem->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->setItemWidget( protocolitem, 0, mProtocolCmbBx );
  mProtocolItem->setExpanded( true );

  mVerifyModeItem = addRootItem( tr( "Peer verification" ) );
  mVerifyPeerCmbBx = new QComboBox( treeSslConfig );
  mVerifyPeerCmbBx->addItem( tr( "Verify Peer Certs" ),
                             static_cast<int>( QSslSocket::VerifyPeer ) );
  mVerifyPeerCmbBx->addItem( tr( "Do Not Verify Peer Certs" ),
                             static_cast<int>( QSslSocket::VerifyNone ) );
  mVerifyPeerCmbBx->setMaximumWidth( 300 );
  mVerifyPeerCmbBx->setCurrentIndex( 0 );
  QTreeWidgetItem *peerverifycmbxitem = new QTreeWidgetItem(
    mVerifyModeItem,
    QStringList() << QString(),
    static_cast<int>( ConfigItem ) );
  peerverifycmbxitem->setFlags( peerverifycmbxitem->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->setItemWidget( peerverifycmbxitem, 0, mVerifyPeerCmbBx );
  mVerifyModeItem->setExpanded( true );

  mVerifyDepthItem = addRootItem( tr( "Peer verification depth (0 = complete cert chain)" ) );
  mVerifyDepthSpnBx = new QSpinBox( treeSslConfig );
  mVerifyDepthSpnBx->setMinimum( 0 );
  mVerifyDepthSpnBx->setMaximum( 10 );
  mVerifyDepthSpnBx->setMaximumWidth( 200 );
  mVerifyDepthSpnBx->setAlignment( Qt::AlignHCenter );
  QTreeWidgetItem *peerverifyspnbxitem = new QTreeWidgetItem(
    mVerifyDepthItem,
    QStringList() << QString(),
    static_cast<int>( ConfigItem ) );
  peerverifyspnbxitem->setFlags( peerverifyspnbxitem->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->setItemWidget( peerverifyspnbxitem, 0, mVerifyDepthSpnBx );
  mVerifyDepthItem->setExpanded( true );

  mIgnoreErrorsItem = addRootItem( tr( "Ignore errors" ) );

  const QList<QPair<QSslError::SslError, QString> > errenums = QgsAuthCertUtils::sslErrorEnumStrings();
  for ( int i = 0; i < errenums.size(); i++ )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem(
      mIgnoreErrorsItem,
      QStringList() << errenums.at( i ).second,
      static_cast<int>( ConfigItem ) );
    item->setCheckState( 0, Qt::Unchecked );
    item->setTextAlignment( 0, Qt::AlignVCenter );
    item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
    item->setData( 0, Qt::UserRole, errenums.at( i ).first );
  }
  mIgnoreErrorsItem->setExpanded( true );
}

const QgsAuthConfigSslServer QgsAuthSslConfigWidget::sslCustomConfig()
{
  QgsAuthConfigSslServer config;
  if ( mDisabled )
  {
    return config;
  }
  config.setSslCertificate( mCert );
  config.setSslHostPort( leHost->text() );
  config.setSslProtocol( sslProtocol() );
  config.setSslIgnoredErrorEnums( sslIgnoreErrorEnums() );
  config.setSslPeerVerifyMode( sslPeerVerifyMode() );
  config.setSslPeerVerifyDepth( sslPeerVerifyDepth() );
  return config;
}

const QSslCertificate QgsAuthSslConfigWidget::sslCertificate()
{
  if ( mDisabled )
  {
    return QSslCertificate();
  }
  return mCert;
}

const QString QgsAuthSslConfigWidget::sslHost()
{
  if ( mDisabled )
  {
    return QString();
  }
  return leHost->text();
}

void QgsAuthSslConfigWidget::enableSslCustomOptions( bool enable )
{
  if ( mDisabled )
  {
    return;
  }
  if ( grpbxSslConfig->isCheckable() )
  {
    grpbxSslConfig->setChecked( enable );
  }
}

void QgsAuthSslConfigWidget::setSslCertificate( const QSslCertificate &cert, const QString &hostport )
{
  if ( mDisabled )
  {
    return;
  }
  if ( cert.isNull() )
  {
    return;
  }
  mCert = cert;

  if ( !hostport.isEmpty() )
  {
    setSslHost( hostport );
  }

  const QString sha( QgsAuthCertUtils::shaHexForCert( cert ) );
  const QgsAuthConfigSslServer config(
    QgsApplication::authManager()->sslCertCustomConfig( sha, hostport.isEmpty() ? sslHost() : hostport ) );

  emit certFoundInAuthDatabase( !config.isNull() );

  lblLoadedConfig->setVisible( true );
  if ( !config.isNull() )
  {
    loadSslCustomConfig( config );
    leCommonName->setStyleSheet( QgsAuthGuiUtils::greenTextStyleSheet() );
  }
  else
  {
    lblLoadedConfig->setText( configNotFoundText_() );
    leCommonName->setText( QgsAuthCertUtils::resolvedCertName( mCert ) );
    leCommonName->setStyleSheet( QgsAuthGuiUtils::orangeTextStyleSheet() );
  }
  validateHostPortText( leHost->text() );
}

void QgsAuthSslConfigWidget::loadSslCustomConfig( const QgsAuthConfigSslServer &config )
{
  if ( mDisabled )
  {
    return;
  }
  resetSslCertConfig();
  if ( config.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Passed-in SSL custom config is null" ) );
    return;
  }

  const QSslCertificate cert( config.sslCertificate() );
  if ( cert.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "SSL custom config's cert is null" ) );
    return;
  }

  enableSslCustomOptions( true );
  mCert = cert;
  leCommonName->setText( QgsAuthCertUtils::resolvedCertName( cert ) );
  leHost->setText( config.sslHostPort() );
  setSslIgnoreErrorEnums( config.sslIgnoredErrorEnums() );
  setSslProtocol( config.sslProtocol() );
  setSslPeerVerify( config.sslPeerVerifyMode(), config.sslPeerVerifyDepth() );

  lblLoadedConfig->setVisible( true );
  lblLoadedConfig->setText( configFoundText_() );
}

void QgsAuthSslConfigWidget::saveSslCertConfig()
{
  if ( mDisabled )
  {
    return;
  }
  if ( !QgsApplication::authManager()->storeSslCertCustomConfig( sslCustomConfig() ) )
  {
    QgsDebugMsg( QStringLiteral( "SSL custom config FAILED to store in authentication database" ) );
  }
}

void QgsAuthSslConfigWidget::resetSslCertConfig()
{
  if ( mDisabled )
  {
    return;
  }
  mCert.clear();
  mConnectionCAs.clear();
  leCommonName->clear();
  leCommonName->setStyleSheet( QString() );
  leHost->clear();

  lblLoadedConfig->setVisible( false );
  lblLoadedConfig->clear();
  resetSslProtocol();
  resetSslIgnoreErrors();
  resetSslPeerVerify();
  enableSslCustomOptions( false );
}

QSsl::SslProtocol QgsAuthSslConfigWidget::sslProtocol()
{
  if ( mDisabled )
  {
    return QSsl::UnknownProtocol;
  }
  return ( QSsl::SslProtocol )mProtocolCmbBx->currentData().toInt();
}

void QgsAuthSslConfigWidget::setSslProtocol( QSsl::SslProtocol protocol )
{
  if ( mDisabled )
  {
    return;
  }
  const int indx( mProtocolCmbBx->findData( static_cast<int>( protocol ) ) );
  mProtocolCmbBx->setCurrentIndex( indx );
}

void QgsAuthSslConfigWidget::resetSslProtocol()
{
  if ( mDisabled )
  {
    return;
  }
  mProtocolCmbBx->setCurrentIndex( 0 );
}

void QgsAuthSslConfigWidget::appendSslIgnoreErrors( const QList<QSslError> &errors )
{
  if ( mDisabled )
  {
    return;
  }
  enableSslCustomOptions( true );

  QList<QSslError::SslError> errenums;
  const auto constErrors = errors;
  for ( const QSslError &err : constErrors )
  {
    errenums << err.error();
  }

  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    QTreeWidgetItem *item( mIgnoreErrorsItem->child( i ) );
    if ( errenums.contains( ( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() ) )
    {
      item->setCheckState( 0, Qt::Checked );
    }
  }
}

void QgsAuthSslConfigWidget::setSslIgnoreErrorEnums( const QList<QSslError::SslError> &errorenums )
{
  if ( mDisabled )
  {
    return;
  }
  QList<QSslError> errors;
  const auto constErrorenums = errorenums;
  for ( const QSslError::SslError errorenum : constErrorenums )
  {
    errors << QSslError( errorenum );
  }
  setSslIgnoreErrors( errors );
}

void QgsAuthSslConfigWidget::setSslIgnoreErrors( const QList<QSslError> &errors )
{
  if ( mDisabled )
  {
    return;
  }
  if ( errors.isEmpty() )
  {
    return;
  }

  enableSslCustomOptions( true );

  QList<QSslError::SslError> errenums;
  const auto constErrors = errors;
  for ( const QSslError &err : constErrors )
  {
    errenums << err.error();
  }

  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    QTreeWidgetItem *item( mIgnoreErrorsItem->child( i ) );
    const bool enable( errenums.contains( ( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() ) );
    item->setCheckState( 0, enable ? Qt::Checked : Qt::Unchecked );
  }
}

void QgsAuthSslConfigWidget::resetSslIgnoreErrors()
{
  if ( mDisabled )
  {
    return;
  }
  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    mIgnoreErrorsItem->child( i )->setCheckState( 0, Qt::Unchecked );
  }
}

const QList<QSslError::SslError> QgsAuthSslConfigWidget::sslIgnoreErrorEnums()
{
  QList<QSslError::SslError> errs;
  if ( mDisabled )
  {
    return errs;
  }
  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    QTreeWidgetItem *item( mIgnoreErrorsItem->child( i ) );
    if ( item->checkState( 0 ) == Qt::Checked )
    {
      errs.append( ( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() );
    }
  }
  return errs;
}

QSslSocket::PeerVerifyMode QgsAuthSslConfigWidget::sslPeerVerifyMode()
{
  if ( mDisabled )
  {
    return QSslSocket::AutoVerifyPeer;
  }
  return ( QSslSocket::PeerVerifyMode )mVerifyPeerCmbBx->currentData().toInt();
}

int QgsAuthSslConfigWidget::sslPeerVerifyDepth()
{
  if ( mDisabled )
  {
    return 0;
  }
  return mVerifyDepthSpnBx->value();
}

void QgsAuthSslConfigWidget::setSslPeerVerify( QSslSocket::PeerVerifyMode mode, int modedepth )
{
  if ( mDisabled )
  {
    return;
  }
  enableSslCustomOptions( true );

  const int indx( mVerifyPeerCmbBx->findData( static_cast<int>( mode ) ) );
  mVerifyPeerCmbBx->setCurrentIndex( indx );

  mVerifyDepthSpnBx->setValue( modedepth );
}

void QgsAuthSslConfigWidget::resetSslPeerVerify()
{
  if ( mDisabled )
  {
    return;
  }
  mVerifyPeerCmbBx->setCurrentIndex( 0 );
  mVerifyDepthSpnBx->setValue( 0 );
}

bool QgsAuthSslConfigWidget::readyToSave()
{
  if ( mDisabled )
  {
    return false;
  }
  const bool cansave = ( isEnabled()
                         && ( grpbxSslConfig->isCheckable() ? grpbxSslConfig->isChecked() : true )
                         && validateHostPort( leHost->text() ) );
  if ( mCanSave != cansave )
  {
    mCanSave = cansave;
    emit readyToSaveChanged( cansave );
  }
  return cansave;
}

void QgsAuthSslConfigWidget::setSslHost( const QString &host )
{
  if ( mDisabled )
  {
    return;
  }
  leHost->setText( host );
}

bool QgsAuthSslConfigWidget::validateHostPort( const QString &txt )
{
  const QString hostport( txt );
  if ( hostport.isEmpty() )
  {
    return false;
  }

  // TODO: add QRegex checks against valid IP and domain.tld input
  //       i.e., currently accepts unlikely (though maybe valid) host:port combo, like 'a:1'
  const QString urlbase( QStringLiteral( "https://%1" ).arg( hostport ) );
  const QUrl url( urlbase );
  return ( !url.host().isEmpty() && QString::number( url.port() ).size() > 0
           && QStringLiteral( "https://%1:%2" ).arg( url.host() ).arg( url.port() ) == urlbase );
}

void QgsAuthSslConfigWidget::validateHostPortText( const QString &txt )
{
  if ( mDisabled )
  {
    return;
  }
  const bool valid = validateHostPort( txt );
  leHost->setStyleSheet( valid ? QgsAuthGuiUtils::greenTextStyleSheet()
                         : QgsAuthGuiUtils::redTextStyleSheet() );
  emit hostPortValidityChanged( valid );
}

void QgsAuthSslConfigWidget::setConfigCheckable( bool checkable )
{
  if ( mDisabled )
  {
    return;
  }
  grpbxSslConfig->setCheckable( checkable );
  if ( !checkable )
  {
    grpbxSslConfig->setEnabled( true );
  }
}

void QgsAuthSslConfigWidget::btnCertInfo_clicked()
{
  if ( mCert.isNull() )
  {
    return;
  }

  QgsAuthCertInfoDialog *dlg = new QgsAuthCertInfoDialog( mCert, false, this, mConnectionCAs );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}


//////////////// Embed in dialog ///////////////////

QgsAuthSslConfigDialog::QgsAuthSslConfigDialog( QWidget *parent, const QSslCertificate &cert, const QString &hostport )
  : QDialog( parent )

{
  setWindowTitle( tr( "Custom Certificate Configuration" ) );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 6, 6, 6, 6 );

  mSslConfigWdgt = new QgsAuthSslConfigWidget( this, cert, hostport );
  connect( mSslConfigWdgt, &QgsAuthSslConfigWidget::readyToSaveChanged,
           this, &QgsAuthSslConfigDialog::checkCanSave );
  layout->addWidget( mSslConfigWdgt );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Close | QDialogButtonBox::Save, Qt::Horizontal, this );

  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );
  mSaveButton = buttonBox->button( QDialogButtonBox::Save );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAuthSslConfigDialog::accept );
  layout->addWidget( buttonBox );

  setLayout( layout );
  mSaveButton->setEnabled( mSslConfigWdgt->readyToSave() );
}

void QgsAuthSslConfigDialog::accept()
{
  mSslConfigWdgt->saveSslCertConfig();
  QDialog::accept();
}

void QgsAuthSslConfigDialog::checkCanSave( bool cansave )
{
  mSaveButton->setEnabled( cansave );
}
