/***************************************************************************
    qgsauthcertificateinfo.cpp
    ---------------------
    begin                : April 26, 2015
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


#include "qgsauthcertificateinfo.h"
#include "ui_qgsauthcertificateinfo.h"

#include <QtCrypto>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>

#include "qgsapplication.h"
#include "qgsauthcertutils.h"
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

static void removeChildren_( QTreeWidgetItem* item )
{
  Q_FOREACH ( QTreeWidgetItem* child, item->takeChildren() )
  {
    delete child;
  }
}

QgsAuthCertInfo::QgsAuthCertInfo( const QSslCertificate& cert,
                                  bool manageCertTrust,
                                  QWidget *parent ,
                                  const QList<QSslCertificate>& connectionCAs )
    : QWidget( parent )
    , mConnectionCAs( connectionCAs )
    , mDefaultItemForeground( QBrush() )
    , mManageTrust( manageCertTrust )
    , mTrustCacheRebuilt( false )
    , mDefaultTrustPolicy( QgsAuthCertUtils::DefaultTrust )
    , mCurrentTrustPolicy( QgsAuthCertUtils::DefaultTrust )
    , mSecGeneral( nullptr )
    , mSecDetails( nullptr )
    , mSecPemText( nullptr )
    , mGrpSubj( nullptr )
    , mGrpIssu( nullptr )
    , mGrpCert( nullptr )
    , mGrpPkey( nullptr )
    , mGrpExts( nullptr )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
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

    lblError->setHidden( true );

    treeHierarchy->setRootIsDecorated( false );

    connect( treeHierarchy, SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this, SLOT( currentCertItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

    mCaCertsCache = QgsAuthManager::instance()->getCaCertsCache();

    setUpCertDetailsTree();

    grpbxTrust->setVisible( mManageTrust );

    // trust policy is still queried, even if not managing the policy, so public getter will work
    mDefaultTrustPolicy = QgsAuthManager::instance()->defaultCertTrustPolicy();
    mCurrentTrustPolicy = QgsAuthCertUtils::DefaultTrust;

    bool res;
    res = populateQcaCertCollection();
    if ( res )
      res = setQcaCertificate( cert );
    if ( res )
      res = populateCertChain();
    if ( res )
      setCertHierarchy();

    connect( cmbbxTrust, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( currentPolicyIndexChanged( int ) ) );
  }
}

QgsAuthCertInfo::~QgsAuthCertInfo()
{
}

void QgsAuthCertInfo::setupError( const QString &msg )
{
  lblError->setVisible( true );
  QString out = tr( "<b>Setup ERROR:</b>\n\n" );
  out += msg;
  lblError->setText( out );
  lblError->setStyleSheet( QgsAuthGuiUtils::redTextStyleSheet() );
}

void QgsAuthCertInfo::currentCertItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  Q_UNUSED( previous );
  updateCurrentCert( current );
}

void QgsAuthCertInfo::updateCurrentCert( QTreeWidgetItem *item )
{
  if ( !item )
    item = treeHierarchy->currentItem();
  if ( !item )
    return;

  int indx( item->data( 0, Qt::UserRole ).toInt() );
  updateCurrentCertInfo( indx );
}

bool QgsAuthCertInfo::populateQcaCertCollection()
{
  const QList<QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> >& certpairs( mCaCertsCache.values() );
  for ( int i = 0; i < certpairs.size(); ++i )
  {
    QCA::ConvertResult res;
    QCA::Certificate acert = QCA::Certificate::fromPEM( certpairs.at( i ).second.toPem(), &res, QString( "qca-ossl" ) );
    if ( res == QCA::ConvertGood && !acert.isNull() )
    {
      mCaCerts.addCertificate( acert );
    }
  }
  if ( !mConnectionCAs.isEmpty() )
  {
    Q_FOREACH ( const QSslCertificate& cert, mConnectionCAs )
    {
      QCA::ConvertResult res;
      QCA::Certificate acert = QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) );
      if ( res == QCA::ConvertGood && !acert.isNull() )
      {
        mCaCerts.addCertificate( acert );
      }
    }
  }

  if ( mCaCerts.certificates().size() < 1 )
  {
    setupError( tr( "Could not populate QCA certificate collection" ) );
    return false;
  }
  return true;
}

bool QgsAuthCertInfo::setQcaCertificate( const QSslCertificate& cert )
{
  QCA::ConvertResult res;
  mCert = QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) );
  if ( res != QCA::ConvertGood || mCert.isNull() )
  {
    setupError( tr( "Could not set QCA certificate" ) );
    return false;
  }
  return true;
}

bool QgsAuthCertInfo::populateCertChain()
{
  QCA::CertificateChain certchain( mCert );
  QCA::Validity valid;
  mACertChain = certchain.complete( mCaCerts.certificates(), &valid );
  if ( valid != QCA::ValidityGood && valid != QCA::ErrorInvalidCA )
  {
    // invalid CAs are skipped to allow an incomplete chain
    setupError( tr( "Invalid population of QCA certificate chain.<br><br>"
                    "Validity message: %1" ).arg( QgsAuthCertUtils::qcaValidityMessage( valid ) ) );
    return false;
  }

  if ( mACertChain.isEmpty() )
  {
    QgsDebugMsg( "Could not populate QCA certificate chain" );
    mACertChain = certchain;
  }

  if ( !mACertChain.last().isSelfSigned() )
  {
    // chain is incomplete, add null certificate to signify local missing parent CA
    mACertChain.append( QCA::Certificate() );
  }

  // mirror chain to QSslCertificate
  Q_FOREACH ( QCA::Certificate cert, mACertChain )
  {
    QSslCertificate qcert;
    if ( !cert.isNull() )
    {
      qcert = QSslCertificate( cert.toPEM().toAscii() );
    }
    mQCertChain.append( qcert );
  }
  return true;
}

void QgsAuthCertInfo::setCertHierarchy()
{
  QListIterator<QSslCertificate> it( mQCertChain );
  it.toBack();
  int i = mQCertChain.size();
  QTreeWidgetItem * item = nullptr;
  QTreeWidgetItem * previtem = nullptr;
  while ( it.hasPrevious() )
  {
    QSslCertificate cert( it.previous() );
    bool missingCA = cert.isNull();
    QString cert_source( "" );
    if ( missingCA && it.hasPrevious() )
    {
      cert_source = QgsAuthCertUtils::resolvedCertName( it.peekPrevious(), true );
      cert_source += QString( " (%1)" ).arg( tr( "Missing CA" ) );
    }
    else
    {
      cert_source = QgsAuthCertUtils::resolvedCertName( cert );
      QString sha = QgsAuthCertUtils::shaHexForCert( cert );
      if ( mCaCertsCache.contains( sha ) )
      {
        const QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate >& certpair( mCaCertsCache.value( sha ) );
        cert_source += QString( " (%1)" ).arg( QgsAuthCertUtils::getCaSourceName( certpair.first, true ) );
      }
      else if ( mConnectionCAs.contains( cert ) )
      {
        cert_source += QString( " (%1)" )
                       .arg( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::Connection, true ) );
      }
    }

    if ( !previtem )
    {
      item = new QTreeWidgetItem( treeHierarchy, QStringList() << cert_source );
    }
    else
    {
      item = new QTreeWidgetItem( previtem, QStringList() << cert_source );
    }
    if ( missingCA && it.hasPrevious() )
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    }

    item->setData( 0, Qt::UserRole, --i );

    if ( mDefaultItemForeground.style() == Qt::NoBrush )
    {
      mDefaultItemForeground = item->foreground( 0 );
    }

    decorateCertTreeItem( cert, QgsAuthManager::instance()->getCertificateTrustPolicy( cert ), item );

    item->setFirstColumnSpanned( true );
    if ( !previtem )
      treeHierarchy->addTopLevelItem( item );
    previtem = item;
  }
  treeHierarchy->setCurrentItem( item, 0, QItemSelectionModel::ClearAndSelect );
  treeHierarchy->expandAll();
}

void QgsAuthCertInfo::updateCurrentCertInfo( int chainindx )
{
  btnSaveTrust->setEnabled( false );

  mCurrentQCert = mQCertChain.at( chainindx );
  mCurrentACert = mACertChain.at( chainindx );

  if ( mManageTrust )
  {
    grpbxTrust->setHidden( mCurrentQCert.isNull() );
  }

  if ( !mCurrentQCert.isNull() )
  {
    QgsAuthCertUtils::CertTrustPolicy trustpolicy( QgsAuthManager::instance()->getCertificateTrustPolicy( mCurrentQCert ) );
    mCurrentTrustPolicy = trustpolicy;

    cmbbxTrust->setTrustPolicy( trustpolicy );
    if ( !mCurrentQCert.isValid() )
    {
      cmbbxTrust->setDefaultTrustPolicy( QgsAuthCertUtils::Untrusted );
    }
  }

  populateCertInfo();
}

void QgsAuthCertInfo::setUpCertDetailsTree()
{
  treeDetails->setColumnCount( 2 );
  treeDetails->setHeaderLabels( QStringList() << tr( "Field" ) << tr( "Value" ) );
  treeDetails->setColumnWidth( 0, 200 );

  QTreeWidgetItem *headeritem = treeDetails->headerItem();
  headeritem->setTextAlignment( 0, Qt::AlignRight );
  headeritem->setTextAlignment( 1, Qt::AlignLeft );

  treeDetails->setRootIsDecorated( true );
  treeDetails->setWordWrap( true );

  // add root items
  mSecGeneral = new QTreeWidgetItem(
    treeDetails,
    QStringList( tr( "General" ) ),
    ( int )DetailsSection );
  setItemBold_( mSecGeneral );
  mSecGeneral->setFirstColumnSpanned( true );
  mSecGeneral->setFlags( Qt::ItemIsEnabled );
  mSecGeneral->setExpanded( true );
  treeDetails->insertTopLevelItem( 0, mSecGeneral );

  mSecDetails = new QTreeWidgetItem(
    treeDetails,
    QStringList( tr( "Details" ) ),
    ( int )DetailsSection );
  setItemBold_( mSecDetails );
  mSecDetails->setFirstColumnSpanned( true );
  mSecDetails->setFlags( Qt::ItemIsEnabled );
  mSecDetails->setExpanded( false );
  treeDetails->insertTopLevelItem( 0, mSecDetails );

  // add details groups
  mGrpSubj = addGroupItem( mSecDetails, tr( "Subject Info" ) );
  mGrpIssu = addGroupItem( mSecDetails, tr( "Issuer Info" ) ) ;
  mGrpCert = addGroupItem( mSecDetails, tr( "Certificate Info" ) );
  mGrpPkey = addGroupItem( mSecDetails, tr( "Public Key Info" ) );
  mGrpExts = addGroupItem( mSecDetails, tr( "Extensions" ) );

  mSecPemText = new QTreeWidgetItem(
    treeDetails,
    QStringList( tr( "PEM Text" ) ),
    ( int )DetailsSection );
  setItemBold_( mSecPemText );
  mSecPemText->setFirstColumnSpanned( true );
  mSecPemText->setFlags( Qt::ItemIsEnabled );
  mSecPemText->setExpanded( false );
  treeDetails->insertTopLevelItem( 0, mSecPemText );
}

void QgsAuthCertInfo::populateCertInfo()
{
  mSecDetails->setHidden( false );
  mSecPemText->setHidden( false );

  populateInfoGeneralSection();
  populateInfoDetailsSection();
  populateInfoPemTextSection();
}

QTreeWidgetItem * QgsAuthCertInfo::addGroupItem( QTreeWidgetItem *parent, const QString &group )
{
  QTreeWidgetItem *grpitem = new QTreeWidgetItem(
    parent,
    QStringList( group ),
    ( int )DetailsGroup );

  grpitem->setFirstColumnSpanned( true );
  grpitem->setFlags( Qt::ItemIsEnabled );
  grpitem->setExpanded( true );

  QBrush orgb( grpitem->foreground( 0 ) );
  orgb.setColor( QColor::fromRgb( 90, 90, 90 ) );
  grpitem->setForeground( 0, orgb );
  QFont grpf( grpitem->font( 0 ) );
  grpf.setItalic( true );
  grpitem->setFont( 0, grpf );

  return grpitem;
}

void QgsAuthCertInfo::addFieldItem( QTreeWidgetItem *parent, const QString &field, const QString &value,
                                    QgsAuthCertInfo::FieldWidget wdgt, const QColor& color )
{
  if ( value.isEmpty() )
    return;

  QTreeWidgetItem *item = new QTreeWidgetItem(
    parent,
    QStringList() << field << ( wdgt == NoWidget ? value : "" ),
    ( int )DetailsField );

  item->setTextAlignment( 0, Qt::AlignRight );
  item->setTextAlignment( 1, Qt::AlignLeft );

  QBrush fieldb( item->foreground( 0 ) );
  fieldb.setColor( QColor::fromRgb( 90, 90, 90 ) );
  item->setForeground( 0, fieldb );

  if ( wdgt == NoWidget )
  {
    if ( color.isValid() )
    {
      QBrush valueb( item->foreground( 1 ) );
      valueb.setColor( color );
      item->setForeground( 1, valueb );
    }
  }
  else if ( wdgt == LineEdit )
  {
    QLineEdit *le = new QLineEdit( value, treeDetails );
    le->setReadOnly( true );
    le->setAlignment( Qt::AlignLeft );
    le->setCursorPosition( 0 );
    if ( color.isValid() )
    {
      le->setStyleSheet( QString( "QLineEdit { color: %1; }" ).arg( color.name() ) );
    }
    item->treeWidget()->setItemWidget( item, 1, le );
  }
  else if ( wdgt == TextEdit )
  {
    QPlainTextEdit *pte = new QPlainTextEdit( value, treeDetails );
    pte->setReadOnly( true );
    pte->setMinimumHeight( 75 );
    pte->setMaximumHeight( 75 );
    pte->moveCursor( QTextCursor::Start );
    if ( color.isValid() )
    {
      pte->setStyleSheet( QString( "QPlainTextEdit { color: %1; }" ).arg( color.name() ) );
    }
    item->treeWidget()->setItemWidget( item, 1, pte );
  }

}

void QgsAuthCertInfo::populateInfoGeneralSection()
{
  removeChildren_( mSecGeneral );

  if ( mCurrentQCert.isNull() )
  {
    addFieldItem( mSecGeneral, tr( "Type" ),
                  tr( "Missing CA (incomplete local CA chain)" ),
                  LineEdit );
    mSecGeneral->setExpanded( true );
    mSecDetails->setHidden( true );
    mSecPemText->setHidden( true );
    return;
  }

  QString certype;
  bool isselfsigned = mCurrentACert.isSelfSigned();
  QString selfsigned( tr( "self-signed" ) );

  QList<QgsAuthCertUtils::CertUsageType> usagetypes(
    QgsAuthCertUtils::certificateUsageTypes( mCurrentQCert ) );
  bool isca = usagetypes.contains( QgsAuthCertUtils::CertAuthorityUsage );
  bool isissuer = usagetypes.contains( QgsAuthCertUtils::CertIssuerUsage );
  bool issslserver = usagetypes.contains( QgsAuthCertUtils::TlsServerUsage );
  bool issslclient = usagetypes.contains( QgsAuthCertUtils::TlsClientUsage );

  if ( issslclient )
  {
    certype = QgsAuthCertUtils::certificateUsageTypeString( QgsAuthCertUtils::TlsClientUsage );
  }
  if ( issslserver )
  {
    certype = QgsAuthCertUtils::certificateUsageTypeString( QgsAuthCertUtils::TlsServerUsage );
  }
  if ( isissuer || ( isca && !isselfsigned ) )
  {
    certype = QgsAuthCertUtils::certificateUsageTypeString( QgsAuthCertUtils::CertIssuerUsage );
  }
  if (( isissuer || isca ) && isselfsigned )
  {
    certype = QString( "%1 %2" )
              .arg( tr( "Root" ),
                    QgsAuthCertUtils::certificateUsageTypeString( QgsAuthCertUtils::CertAuthorityUsage ) );
  }
  if ( isselfsigned )
  {
    certype.append( certype.isEmpty() ? selfsigned : QString( " (%1)" ).arg( selfsigned ) );
  }

  addFieldItem( mSecGeneral, tr( "Usage type" ),
                certype,
                LineEdit );
  addFieldItem( mSecGeneral, tr( "Subject" ),
                QgsAuthCertUtils::resolvedCertName( mCurrentQCert ),
                LineEdit );
  addFieldItem( mSecGeneral, tr( "Issuer" ),
                QgsAuthCertUtils::resolvedCertName( mCurrentQCert, true ),
                LineEdit );
  addFieldItem( mSecGeneral, tr( "Not valid after" ),
                mCurrentQCert.expiryDate().toString(),
                LineEdit,
                mCurrentQCert.expiryDate() < QDateTime::currentDateTime() ? QgsAuthGuiUtils::redColor() : QColor() );

  QSslKey pubkey( mCurrentQCert.publicKey() );
  QString alg( pubkey.algorithm() == QSsl::Rsa ? "RSA" : "DSA" );
  int bitsize( pubkey.length() );
  addFieldItem( mSecGeneral, tr( "Public key" ),
                QString( "%1, %2 bits" ).arg( alg, bitsize == -1 ? QString( "?" ) : QString::number( bitsize ) ),
                LineEdit );
  addFieldItem( mSecGeneral, tr( "Signature algorithm" ),
                QgsAuthCertUtils::qcaSignatureAlgorithm( mCurrentACert.signatureAlgorithm() ),
                LineEdit );
}

void QgsAuthCertInfo::populateInfoDetailsSection()
{
  removeChildren_( mGrpSubj );
  removeChildren_( mGrpIssu );
  removeChildren_( mGrpCert );
  removeChildren_( mGrpPkey );
  removeChildren_( mGrpExts );

  if ( mCurrentQCert.isNull() )
    return;

  // Subject Info
  addFieldItem( mGrpSubj, tr( "Country (C)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::CountryName ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "State/Province (ST)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::StateOrProvinceName ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Locality (L)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::LocalityName ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Organization (O)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::Organization ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Organizational unit (OU)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::OrganizationalUnitName ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Common name (CN)" ),
                SSL_SUBJECT_INFO( mCurrentQCert, QSslCertificate::CommonName ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Email address (E)" ),
                mCurrentACert.subjectInfo().value( QCA::Email ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Distinguished name" ),
                QgsAuthCertUtils::getCertDistinguishedName( mCurrentQCert, mCurrentACert, false ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Email Legacy" ),
                mCurrentACert.subjectInfo().value( QCA::EmailLegacy ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Incorporation Country" ),
                mCurrentACert.subjectInfo().value( QCA::IncorporationCountry ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Incorporation State/Province" ),
                mCurrentACert.subjectInfo().value( QCA::IncorporationState ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "Incorporation Locality" ),
                mCurrentACert.subjectInfo().value( QCA::IncorporationLocality ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "URI" ),
                mCurrentACert.subjectInfo().value( QCA::URI ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "DNS" ),
                mCurrentACert.subjectInfo().value( QCA::DNS ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "IP Address" ),
                mCurrentACert.subjectInfo().value( QCA::IPAddress ),
                LineEdit );
  addFieldItem( mGrpSubj, tr( "XMPP" ),
                mCurrentACert.subjectInfo().value( QCA::XMPP ),
                LineEdit );

  QMultiMap<QSsl::AlternateNameEntryType, QString> alts( mCurrentQCert.alternateSubjectNames() );
  QStringList altslist;
  QString email( tr( "Email: " ) );
  QStringList emails( alts.values( QSsl::EmailEntry ) );
  if ( !emails.isEmpty() )
  {
    altslist << email + emails.join( '\n' + email );
  }
  QString dns( tr( "DNS: " ) );
  QStringList dnss( alts.values( QSsl::DnsEntry ) );
  if ( !dnss.isEmpty() )
  {
    altslist << dns + dnss.join( '\n' + dns );
  }
  addFieldItem( mGrpSubj, tr( "Alternate names" ),
                altslist.join( "\n" ),
                TextEdit );

  // Issuer Info
  addFieldItem( mGrpIssu, tr( "Country (C)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::CountryName ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "State/Province (ST)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::StateOrProvinceName ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Locality (L)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::LocalityName ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Organization (O)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::Organization ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Organizational unit (OU)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::OrganizationalUnitName ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Common name (CN)" ),
                SSL_ISSUER_INFO( mCurrentQCert, QSslCertificate::CommonName ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Email address (E)" ),
                mCurrentACert.issuerInfo().value( QCA::Email ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Distinguished name" ),
                QgsAuthCertUtils::getCertDistinguishedName( mCurrentQCert, mCurrentACert, true ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Email Legacy" ),
                mCurrentACert.issuerInfo().value( QCA::EmailLegacy ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Incorporation Country" ),
                mCurrentACert.issuerInfo().value( QCA::IncorporationCountry ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Incorporation State/Province" ),
                mCurrentACert.issuerInfo().value( QCA::IncorporationState ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "Incorporation Locality" ),
                mCurrentACert.issuerInfo().value( QCA::IncorporationLocality ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "URI" ),
                mCurrentACert.issuerInfo().value( QCA::URI ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "DNS" ),
                mCurrentACert.issuerInfo().value( QCA::DNS ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "IP Address" ),
                mCurrentACert.issuerInfo().value( QCA::IPAddress ),
                LineEdit );
  addFieldItem( mGrpIssu, tr( "XMPP" ),
                mCurrentACert.issuerInfo().value( QCA::XMPP ),
                LineEdit );

  // Certificate Info
  addFieldItem( mGrpCert, tr( "Version" ),
                mCurrentQCert.version(),
                LineEdit );
  addFieldItem( mGrpCert, tr( "Serial #" ),
                mCurrentQCert.serialNumber(),
                LineEdit );
  addFieldItem( mGrpCert, tr( "Not valid before" ),
                mCurrentQCert.effectiveDate().toString(),
                LineEdit,
                mCurrentQCert.effectiveDate() > QDateTime::currentDateTime() ? QgsAuthGuiUtils::redColor() : QColor() );
  addFieldItem( mGrpCert, tr( "Not valid after" ),
                mCurrentQCert.expiryDate().toString(),
                LineEdit,
                mCurrentQCert.expiryDate() < QDateTime::currentDateTime() ? QgsAuthGuiUtils::redColor() : QColor() );
  addFieldItem( mGrpCert, tr( "Signature algorithm" ),
                QgsAuthCertUtils::qcaSignatureAlgorithm( mCurrentACert.signatureAlgorithm() ),
                LineEdit );
  addFieldItem( mGrpCert, tr( "MD5 fingerprint" ),
                QgsAuthCertUtils::getColonDelimited( mCurrentQCert.digest().toHex().toUpper() ),
                LineEdit );
  addFieldItem( mGrpCert, tr( "SHA1 fingerprint" ),
                QgsAuthCertUtils::shaHexForCert( mCurrentQCert, true ).toUpper(),
                LineEdit );

  QStringList crllocs( mCurrentACert.crlLocations() );
  if ( !crllocs.isEmpty() )
  {
    addFieldItem( mGrpCert, tr( "CRL locations" ),
                  crllocs.join( "\n" ),
                  TextEdit );
  }
  QStringList issulocs( mCurrentACert.issuerLocations() );
  if ( !issulocs.isEmpty() )
  {
    addFieldItem( mGrpCert, tr( "Issuer locations" ),
                  issulocs.join( "\n" ),
                  TextEdit );
  }
  QStringList ocsplocs( mCurrentACert.ocspLocations() );
  if ( !ocsplocs.isEmpty() )
  {
    addFieldItem( mGrpCert, tr( "OCSP locations" ),
                  ocsplocs.join( "\n" ),
                  TextEdit );
  }

  // Public Key Info
  // TODO: handle ECC (Elliptic Curve) keys when Qt supports them
  QSslKey pubqkey( mCurrentQCert.publicKey() );
  QString alg( pubqkey.algorithm() == QSsl::Rsa ? "RSA" : "DSA" );
  int bitsize( pubqkey.length() );
  addFieldItem( mGrpPkey, tr( "Algorithm" ),
                bitsize == -1 ? QString( "Unknown (possibly Elliptic Curve)" ) : alg,
                LineEdit );
  addFieldItem( mGrpPkey, tr( "Key size" ),
                bitsize == -1 ? QString( "?" ) : QString::number( bitsize ),
                LineEdit );
  if ( bitsize > 0 ) // ECC keys unsupported by Qt/QCA, so returned key size is 0
  {
    QCA::PublicKey pubakey( mCurrentACert.subjectPublicKey() );

    if ( pubqkey.algorithm() == QSsl::Rsa )
    {
      QCA::RSAPublicKey rsakey( pubakey.toRSA() );
      QCA::BigInteger modulus = rsakey.n();
      QByteArray modarray( modulus.toArray().toByteArray().toHex() );
      if ( modarray.size() > 2 && modarray.mid( 0, 2 ) == QByteArray( "00" ) )
      {
        modarray = modarray.mid( 2 );
      }
      QCA::BigInteger exponent = rsakey.e();
      addFieldItem( mGrpPkey, tr( "Public key" ),
                    QgsAuthCertUtils::getColonDelimited( modarray ).toUpper(),
                    TextEdit );
      addFieldItem( mGrpPkey, tr( "Exponent" ),
                    exponent.toString(),
                    LineEdit );
    }
    // TODO: how is DSA textually represented using QCA?
    // QCA::DSAPublicKey dsakey( pubakey.toDSA() );

    // TODO: how to get the signature and show it as hex?
    //  addFieldItem( mGrpPkey, tr( "Signature" ),
    //                mCurrentACert.???.toHex(),
    //                TextEdit );

    // key usage
    QStringList usage;
    if ( pubakey.canVerify() )
    {
      usage.append( tr( "Verify" ) );
    }

    // TODO: these two seem to always show up, why?
    if ( pubakey.canEncrypt() )
    {
      usage.append( tr( "Encrypt" ) );
    }
#if QCA_VERSION >= 0x020100
    if ( pubakey.canDecrypt() )
    {
      usage.append( tr( "Decrypt" ) );
    }
#endif
    if ( pubakey.canKeyAgree() )
    {
      usage.append( tr( "Key agreement" ) );
    }
    if ( pubakey.canExport() )
    {
      usage.append( tr( "Export" ) );
    }
    if ( !usage.isEmpty() )
    {
      addFieldItem( mGrpPkey, tr( "Key usage" ),
                    usage.join( ", " ),
                    LineEdit );
    }
  }

  // Extensions
  QStringList basicconst;
  basicconst << tr( "Certificate Authority: %1" ).arg( mCurrentACert.isCA() ? tr( "Yes" ) : tr( "No" ) )
  << tr( "Chain Path Limit: %1" ).arg( mCurrentACert.pathLimit() );
  addFieldItem( mGrpExts, tr( "Basic constraints" ),
                basicconst.join( "\n" ),
                TextEdit );

  QStringList keyusage;
  QStringList extkeyusage;
  QList<QCA::ConstraintType> certconsts = mCurrentACert.constraints();
  Q_FOREACH ( const QCA::ConstraintType& certconst, certconsts )
  {
    if ( certconst.section() == QCA::ConstraintType::KeyUsage )
    {
      keyusage.append( QgsAuthCertUtils::qcaKnownConstraint( certconst.known() ) );
    }
    else if ( certconst.section() == QCA::ConstraintType::ExtendedKeyUsage )
    {
      extkeyusage.append( QgsAuthCertUtils::qcaKnownConstraint( certconst.known() ) );
    }
  }
  if ( !keyusage.isEmpty() )
  {
    addFieldItem( mGrpExts, tr( "Key usage" ),
                  keyusage.join( "\n" ),
                  TextEdit );
  }
  if ( !extkeyusage.isEmpty() )
  {
    addFieldItem( mGrpExts, tr( "Extended key usage" ),
                  extkeyusage.join( "\n" ),
                  TextEdit );
  }

  addFieldItem( mGrpExts, tr( "Subject key ID" ),
                QgsAuthCertUtils::getColonDelimited( mCurrentACert.subjectKeyId().toHex() ).toUpper(),
                LineEdit );
  addFieldItem( mGrpExts, tr( "Authority key ID" ),
                QgsAuthCertUtils::getColonDelimited( mCurrentACert.issuerKeyId().toHex() ).toUpper(),
                LineEdit );
}

void QgsAuthCertInfo::populateInfoPemTextSection()
{
  removeChildren_( mSecPemText );

  if ( mCurrentQCert.isNull() )
    return;

  QTreeWidgetItem *item = new QTreeWidgetItem(
    mSecPemText,
    QStringList( "" ),
    ( int )DetailsField );

  item->setFirstColumnSpanned( true );

  QPlainTextEdit *pte = new QPlainTextEdit( mCurrentQCert.toPem(), treeDetails );
  pte->setReadOnly( true );
  pte->setMinimumHeight( 150 );
  pte->setMaximumHeight( 150 );
  pte->moveCursor( QTextCursor::Start );
  item->treeWidget()->setItemWidget( item, 0, pte );
}

void QgsAuthCertInfo::on_btnSaveTrust_clicked()
{
  QgsAuthCertUtils::CertTrustPolicy newpolicy( cmbbxTrust->trustPolicy() );
  if ( !QgsAuthManager::instance()->storeCertTrustPolicy( mCurrentQCert, newpolicy ) )
  {
    QgsDebugMsg( "Could not set trust policy for certificate" );
  }
  mCurrentTrustPolicy = newpolicy;
  decorateCertTreeItem( mCurrentQCert, newpolicy, nullptr );
  btnSaveTrust->setEnabled( false );

  // rebuild trust cache
  QgsAuthManager::instance()->rebuildCertTrustCache();
  mTrustCacheRebuilt = true;
  QgsAuthManager::instance()->rebuildTrustedCaCertsCache();
}

void QgsAuthCertInfo::currentPolicyIndexChanged( int indx )
{
  QgsAuthCertUtils::CertTrustPolicy newpolicy( cmbbxTrust->trustPolicyForIndex( indx ) );
  btnSaveTrust->setEnabled( newpolicy != mCurrentTrustPolicy );
}

void QgsAuthCertInfo::decorateCertTreeItem( const QSslCertificate &cert,
    QgsAuthCertUtils::CertTrustPolicy trustpolicy,
    QTreeWidgetItem * item )
{
  if ( !item )
  {
    item = treeHierarchy->currentItem();
  }
  if ( !item )
  {
    return;
  }

  if ( cert.isNull() || trustpolicy == QgsAuthCertUtils::NoPolicy )
  {
    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateMissing.svg" ) );
    // missing CA, color gray and italicize
    QBrush b( item->foreground( 0 ) );
    b.setColor( QColor::fromRgb( 90, 90, 90 ) );
    item->setForeground( 0, b );
    QFont f( item->font( 0 ) );
    f.setItalic( true );
    item->setFont( 0, f );
    return;
  }

  if ( !cert.isValid() )
  {
    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
    return;
  }

  if ( trustpolicy == QgsAuthCertUtils::Trusted )
  {
    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateTrusted.svg" ) );
  }
  else if ( trustpolicy == QgsAuthCertUtils::Untrusted
            || ( trustpolicy == QgsAuthCertUtils::DefaultTrust
                 && mDefaultTrustPolicy == QgsAuthCertUtils::Untrusted ) )
  {
    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
  }
  else
  {
    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificate.svg" ) );
  }
}

//////////////// Embed in dialog ///////////////////

QgsAuthCertInfoDialog::QgsAuthCertInfoDialog( const QSslCertificate& cert,
    bool manageCertTrust,
    QWidget *parent ,
    const QList<QSslCertificate>& connectionCAs )
    : QDialog( parent )
    , mCertInfoWdgt( nullptr )
{
  setWindowTitle( tr( "Certificate Information" ) );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 6 );

  mCertInfoWdgt = new QgsAuthCertInfo( cert, manageCertTrust, this, connectionCAs );
  layout->addWidget( mCertInfoWdgt );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Close,
      Qt::Horizontal, this );
  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  layout->addWidget( buttonBox );

  setLayout( layout );
}

QgsAuthCertInfoDialog::~QgsAuthCertInfoDialog()
{
}
