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


static void setItemBold_( QTreeWidgetItem* item )
{
  item->setFirstColumnSpanned( true );
  QFont secf( item->font( 0 ) );
  secf.setBold( true );
  item->setFont( 0, secf );
}

static const QString configFoundText_() { return QObject::tr( "Configuration loaded from database" ); }
static const QString configNotFoundText_() { return QObject::tr( "Configuration not found in database" ); }

QgsAuthSslConfigWidget::QgsAuthSslConfigWidget( QWidget *parent,
    const QSslCertificate& cert,
    const QString &hostport,
    const QList<QSslCertificate> &connectionCAs )
    : QWidget( parent )
    , mCert( nullptr )
    , mConnectionCAs( connectionCAs )
    , mProtocolItem( nullptr )
    , mProtocolCmbBx( nullptr )
    , mIgnoreErrorsItem( nullptr )
    , mVerifyModeItem( nullptr )
    , mVerifyPeerCmbBx( nullptr )
    , mVerifyDepthItem( nullptr )
    , mVerifyDepthSpnBx( nullptr )
    , mCanSave( false )
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

    connect( grpbxSslConfig, SIGNAL( toggled( bool ) ), this, SIGNAL( configEnabledChanged( bool ) ) );
    connect( this, SIGNAL( configEnabledChanged( bool ) ), this, SLOT( readyToSave() ) );
    connect( this, SIGNAL( hostPortValidityChanged( bool ) ), this, SLOT( readyToSave() ) );

    setUpSslConfigTree();

    lblLoadedConfig->setVisible( false );
    lblLoadedConfig->setText( "" );

    connect( leHost, SIGNAL( textChanged( QString ) ),
             this, SLOT( validateHostPortText( QString ) ) );

    if ( !cert.isNull() )
    {
      setSslCertificate( cert, hostport );
    }
  }
}

QgsAuthSslConfigWidget::~QgsAuthSslConfigWidget()
{
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
QTreeWidgetItem* QgsAuthSslConfigWidget::addRootItem( const QString &label )
{
  QTreeWidgetItem *item = new QTreeWidgetItem(
    QStringList() << label,
    ( int )ConfigParent );
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
#if QT_VERSION >= 0x040800
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SecureProtocols ),
                           ( int )QSsl::SecureProtocols );
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::TlsV1SslV3 ),
                           ( int )QSsl::TlsV1SslV3 );
#endif
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::TlsV1 ),
                           ( int )QSsl::TlsV1 );
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SslV3 ),
                           ( int )QSsl::SslV3 );
  mProtocolCmbBx->addItem( QgsAuthCertUtils::getSslProtocolName( QSsl::SslV2 ),
                           ( int )QSsl::SslV2 );
  mProtocolCmbBx->setMaximumWidth( 300 );
  mProtocolCmbBx->setCurrentIndex( 0 );
  QTreeWidgetItem *protocolitem = new QTreeWidgetItem(
    mProtocolItem,
    QStringList() << "",
    ( int )ConfigItem );
  protocolitem->setFlags( protocolitem->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->setItemWidget( protocolitem, 0, mProtocolCmbBx );
  mProtocolItem->setExpanded( true );

  mVerifyModeItem = addRootItem( tr( "Peer verification" ) );
  mVerifyPeerCmbBx = new QComboBox( treeSslConfig );
  mVerifyPeerCmbBx->addItem( tr( "Verify peer certs" ),
                             ( int )QSslSocket::VerifyPeer );
  mVerifyPeerCmbBx->addItem( tr( "Do not verify peer certs" ),
                             ( int )QSslSocket::VerifyNone );
  mVerifyPeerCmbBx->setMaximumWidth( 300 );
  mVerifyPeerCmbBx->setCurrentIndex( 0 );
  QTreeWidgetItem *peerverifycmbxitem = new QTreeWidgetItem(
    mVerifyModeItem,
    QStringList() << "",
    ( int )ConfigItem );
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
    QStringList() << "",
    ( int )ConfigItem );
  peerverifyspnbxitem->setFlags( peerverifyspnbxitem->flags() & ~Qt::ItemIsSelectable );
  treeSslConfig->setItemWidget( peerverifyspnbxitem, 0, mVerifyDepthSpnBx );
  mVerifyDepthItem->setExpanded( true );

  mIgnoreErrorsItem = addRootItem( tr( "Ignore errors" ) );

  QList<QPair<QSslError::SslError, QString> > errenums = QgsAuthCertUtils::sslErrorEnumStrings();
  for ( int i = 0; i < errenums.size(); i++ )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem(
      mIgnoreErrorsItem,
      QStringList() << errenums.at( i ).second,
      ( int )ConfigItem );
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

  QString sha( QgsAuthCertUtils::shaHexForCert( cert ) );
  QgsAuthConfigSslServer config(
    QgsAuthManager::instance()->getSslCertCustomConfig( sha, hostport.isEmpty() ? sslHost() : hostport ) );

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
    QgsDebugMsg( "Passed-in SSL custom config is null" );
    return;
  }

  QSslCertificate cert( config.sslCertificate() );
  if ( cert.isNull() )
  {
    QgsDebugMsg( "SSL custom config's cert is null" );
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
  if ( !QgsAuthManager::instance()->storeSslCertCustomConfig( sslCustomConfig() ) )
  {
    QgsDebugMsg( "SSL custom config FAILED to store in authentication database" );
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
  leCommonName->setStyleSheet( "" );
  leHost->clear();

  lblLoadedConfig->setVisible( false );
  lblLoadedConfig->setText( "" );
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
  return ( QSsl::SslProtocol )mProtocolCmbBx->itemData( mProtocolCmbBx->currentIndex() ).toInt();
}

void QgsAuthSslConfigWidget::setSslProtocol( QSsl::SslProtocol protocol )
{
  if ( mDisabled )
  {
    return;
  }
  int indx( mProtocolCmbBx->findData(( int )protocol ) );
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
  Q_FOREACH ( const QSslError& err, errors )
  {
    errenums << err.error();
  }

  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    QTreeWidgetItem *item( mIgnoreErrorsItem->child( i ) );
    if ( errenums.contains(( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() ) )
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
  Q_FOREACH ( QSslError::SslError errorenum, errorenums )
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
  Q_FOREACH ( const QSslError& err, errors )
  {
    errenums << err.error();
  }

  for ( int i = 0; i < mIgnoreErrorsItem->childCount(); i++ )
  {
    QTreeWidgetItem *item( mIgnoreErrorsItem->child( i ) );
    bool enable( errenums.contains(( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() ) );
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
      errs.append(( QSslError::SslError )item->data( 0, Qt::UserRole ).toInt() );
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
  return ( QSslSocket::PeerVerifyMode )mVerifyPeerCmbBx->itemData( mVerifyPeerCmbBx->currentIndex() ).toInt();
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

  int indx( mVerifyPeerCmbBx->findData(( int )mode ) );
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
  bool cansave = ( isEnabled()
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
  QString hostport( txt );
  if ( hostport.isEmpty() )
  {
    return false;
  }

  // TODO: add QRegex checks against valid IP and domain.tld input
  //       i.e., currently accepts unlikely (though maybe valid) host:port combo, like 'a:1'
  QString urlbase( QString( "https://%1" ).arg( hostport ) );
  QUrl url( urlbase );
  return ( !url.host().isEmpty() && QString::number( url.port() ).size() > 0
           && QString( "https://%1:%2" ).arg( url.host() ).arg( url.port() ) == urlbase );
}

void QgsAuthSslConfigWidget::validateHostPortText( const QString &txt )
{
  if ( mDisabled )
  {
    return;
  }
  bool valid = validateHostPort( txt );
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

void QgsAuthSslConfigWidget::on_btnCertInfo_clicked()
{
  if ( mCert.isNull() )
  {
    return;
  }

  QgsAuthCertInfoDialog * dlg = new QgsAuthCertInfoDialog( mCert, false, this, mConnectionCAs );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}


//////////////// Embed in dialog ///////////////////

QgsAuthSslConfigDialog::QgsAuthSslConfigDialog( QWidget *parent , const QSslCertificate& cert , const QString &hostport )
    : QDialog( parent )
    , mSslConfigWdgt( nullptr )
{
  setWindowTitle( tr( "Custom Certificate Configuration" ) );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 6 );

  mSslConfigWdgt = new QgsAuthSslConfigWidget( this, cert, hostport );
  connect( mSslConfigWdgt, SIGNAL( readyToSaveChanged( bool ) ),
           this, SLOT( checkCanSave( bool ) ) );
  layout->addWidget( mSslConfigWdgt );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Close | QDialogButtonBox::Save, Qt::Horizontal, this );

  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );
  mSaveButton = buttonBox->button( QDialogButtonBox::Save );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  layout->addWidget( buttonBox );

  setLayout( layout );
  mSaveButton->setEnabled( mSslConfigWdgt->readyToSave() );
}

QgsAuthSslConfigDialog::~QgsAuthSslConfigDialog()
{
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
