/***************************************************************************
    qgsauthauthoritieseditor.cpp
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

#include "qgsauthauthoritieseditor.h"
#include "ui_qgsauthauthoritieseditor.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPair>
#include <QPushButton>
#include <QSettings>
#include <QSslConfiguration>

#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthimportcertdialog.h"
#include "qgsauthmanager.h"
#include "qgsauthtrustedcasdialog.h"
#include "qgslogger.h"

QgsAuthAuthoritiesEditor::QgsAuthAuthoritiesEditor( QWidget *parent )
    : QWidget( parent )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
    , mRootCaSecItem( nullptr )
    , mFileCaSecItem( nullptr )
    , mDbCaSecItem( nullptr )
    , mDefaultTrustPolicy( QgsAuthCertUtils::DefaultTrust )
    , mUtilitiesMenu( nullptr )
    , mDisabled( false )
    , mActionDefaultTrustPolicy( nullptr )
    , mActionShowTrustedCAs( nullptr )
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

    connect( QgsAuthManager::instance(), SIGNAL( messageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ),
             this, SLOT( authMessageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ) );

    connect( QgsAuthManager::instance(), SIGNAL( authDatabaseChanged() ),
             this, SLOT( refreshCaCertsView() ) );

    setupCaCertsTree();

    connect( treeWidgetCAs->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    connect( treeWidgetCAs, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( handleDoubleClick( QTreeWidgetItem *, int ) ) );

    connect( btnViewRefresh, SIGNAL( clicked() ), this, SLOT( refreshCaCertsView() ) );

    QVariant cafileval = QgsAuthManager::instance()->getAuthSetting( QString( "cafile" ) );
    if ( !cafileval.isNull() )
    {
      leCaFile->setText( cafileval.toString() );
    }

    btnGroupByOrg->setChecked( false );
    QVariant sortbyval = QgsAuthManager::instance()->getAuthSetting( QString( "casortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    mDefaultTrustPolicy = QgsAuthManager::instance()->defaultCertTrustPolicy();
    populateCaCertsView();
    checkSelection();

    populateUtilitiesMenu();
  }
}

QgsAuthAuthoritiesEditor::~QgsAuthAuthoritiesEditor()
{
}

static void setItemBold_( QTreeWidgetItem* item )
{
  item->setFirstColumnSpanned( true );
  QFont secf( item->font( 0 ) );
  secf.setBold( true );
  item->setFont( 0, secf );
}

void QgsAuthAuthoritiesEditor::setupCaCertsTree()
{
  treeWidgetCAs->setColumnCount( 4 );
  treeWidgetCAs->setHeaderLabels(
    QStringList() << tr( "Common Name" )
    << tr( "Serial #" )
    << tr( "Expiry Date" )
    << tr( "Trust Policy" ) );
  treeWidgetCAs->setColumnWidth( 0, 300 );
  treeWidgetCAs->setColumnWidth( 1, 75 );
  treeWidgetCAs->setColumnWidth( 2, 200 );

  // add root sections
  mDbCaSecItem = new QTreeWidgetItem(
    treeWidgetCAs,
    QStringList( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::InDatabase ) ),
    ( int )QgsAuthAuthoritiesEditor::Section );
  setItemBold_( mDbCaSecItem );
  mDbCaSecItem->setFlags( Qt::ItemIsEnabled );
  mDbCaSecItem->setExpanded( true );
  treeWidgetCAs->insertTopLevelItem( 0, mDbCaSecItem );

  mFileCaSecItem = new QTreeWidgetItem(
    treeWidgetCAs,
    QStringList( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::FromFile ) ),
    ( int )QgsAuthAuthoritiesEditor::Section );
  setItemBold_( mFileCaSecItem );
  mFileCaSecItem->setFlags( Qt::ItemIsEnabled );
  mFileCaSecItem->setExpanded( true );
  treeWidgetCAs->insertTopLevelItem( 0, mFileCaSecItem );

  mRootCaSecItem = new QTreeWidgetItem(
    treeWidgetCAs,
    QStringList( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::SystemRoot ) ),
    ( int )QgsAuthAuthoritiesEditor::Section );
  setItemBold_( mRootCaSecItem );
  mRootCaSecItem->setFlags( Qt::ItemIsEnabled );
  mRootCaSecItem->setExpanded( false );
  treeWidgetCAs->insertTopLevelItem( 0, mRootCaSecItem );
}

void QgsAuthAuthoritiesEditor::populateCaCertsView()
{
  updateCertTrustPolicyCache();
  populateDatabaseCaCerts();
  populateFileCaCerts();
  populateRootCaCerts();
}

void QgsAuthAuthoritiesEditor::refreshCaCertsView()
{
//  QgsAuthManager::instance()->rebuildCaCertsCache();
  populateCaCertsView();
}

static void removeChildren_( QTreeWidgetItem* item )
{
  Q_FOREACH ( QTreeWidgetItem* child, item->takeChildren() )
  {
    delete child;
  }
}

void QgsAuthAuthoritiesEditor::populateDatabaseCaCerts()
{
  removeChildren_( mDbCaSecItem );

  bool expanded = mDbCaSecItem->isExpanded();
  populateCaCertsSection( mDbCaSecItem,
                          QgsAuthManager::instance()->getDatabaseCAs(),
                          QgsAuthAuthoritiesEditor::DbCaCert );
  mDbCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateFileCaCerts()
{
  removeChildren_( mFileCaSecItem );

  bool expanded = mFileCaSecItem->isExpanded();
  populateCaCertsSection( mFileCaSecItem,
                          QgsAuthManager::instance()->getExtraFileCAs(),
                          QgsAuthAuthoritiesEditor::FileCaCert );
  mFileCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateRootCaCerts()
{
  removeChildren_( mRootCaSecItem );

  bool expanded = mRootCaSecItem->isExpanded();
  populateCaCertsSection( mRootCaSecItem,
                          QgsAuthManager::instance()->getSystemRootCAs(),
                          QgsAuthAuthoritiesEditor::RootCaCert );
  mRootCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateCaCertsSection( QTreeWidgetItem* item, const QList<QSslCertificate>& certs,
    QgsAuthAuthoritiesEditor::CaType catype )
{
  if ( btnGroupByOrg->isChecked() )
  {
    appendCertsToGroup( certs, catype, item );
  }
  else
  {
    appendCertsToItem( certs, catype, item );
  }
}

void QgsAuthAuthoritiesEditor::appendCertsToGroup( const QList<QSslCertificate>& certs,
    QgsAuthAuthoritiesEditor::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeWidgetCAs->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem * grpitem( new QTreeWidgetItem( parent,
                               QStringList() << it.key(),
                               ( int )QgsAuthAuthoritiesEditor::OrgName ) );
    grpitem->setFirstColumnSpanned( true );
    grpitem->setFlags( Qt::ItemIsEnabled );
    grpitem->setExpanded( true );

    QBrush orgb( grpitem->foreground( 0 ) );
    orgb.setColor( QColor::fromRgb( 90, 90, 90 ) );
    grpitem->setForeground( 0, orgb );
    QFont grpf( grpitem->font( 0 ) );
    grpf.setItalic( true );
    grpitem->setFont( 0, grpf );

    appendCertsToItem( it.value(), catype, grpitem );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthAuthoritiesEditor::appendCertsToItem( const QList<QSslCertificate>& certs,
    QgsAuthAuthoritiesEditor::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeWidgetCAs->currentItem();
  }

  QBrush greenb( QgsAuthGuiUtils::greenColor() );
  QBrush redb( QgsAuthGuiUtils::redColor() );

  QStringList trustedids = mCertTrustCache.value( QgsAuthCertUtils::Trusted );
  QStringList untrustedids = mCertTrustCache.value( QgsAuthCertUtils::Untrusted );

  // Columns: Common Name, Serial #, Expiry Date
  Q_FOREACH ( const QSslCertificate& cert, certs )
  {
    QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

    QStringList coltxts;
    coltxts << QgsAuthCertUtils::resolvedCertName( cert );
    coltxts << QString( cert.serialNumber() );
    coltxts << cert.expiryDate().toString();

    // trust policy
    QString policy( QgsAuthCertUtils::getCertTrustName( mDefaultTrustPolicy ) );
    if ( trustedids.contains( id ) )
    {
      policy = QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Trusted );
    }
    else if ( untrustedids.contains( id ) || !cert.isValid() )
    {
      policy = QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Untrusted );
    }
    coltxts << policy;

    QTreeWidgetItem * item( new QTreeWidgetItem( parent, coltxts, ( int )catype ) );

    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificate.svg" ) );
    if ( !cert.isValid() )
    {
      item->setForeground( 2, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
    }

    if ( trustedids.contains( id ) )
    {
      item->setForeground( 3, greenb );
      if ( cert.isValid() )
      {
        item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateTrusted.svg" ) );
      }
    }
    else if ( untrustedids.contains( id ) )
    {
      item->setForeground( 3, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
    }
    else if ( mDefaultTrustPolicy == QgsAuthCertUtils::Untrusted )
    {
      item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
    }

    item->setData( 0, Qt::UserRole, id );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthAuthoritiesEditor::updateCertTrustPolicyCache()
{
  mCertTrustCache = QgsAuthManager::instance()->getCertTrustCache();
}

void QgsAuthAuthoritiesEditor::populateUtilitiesMenu()
{
  mActionDefaultTrustPolicy = new QAction( "Change default trust policy", this );
  connect( mActionDefaultTrustPolicy, SIGNAL( triggered() ), this, SLOT( editDefaultTrustPolicy() ) );

  mActionShowTrustedCAs = new QAction( "Show trusted authorities/issuers", this );
  connect( mActionShowTrustedCAs, SIGNAL( triggered() ), this, SLOT( showTrustedCertificateAuthorities() ) );

  mUtilitiesMenu = new QMenu( this );
  mUtilitiesMenu->addAction( mActionDefaultTrustPolicy );
  mUtilitiesMenu->addSeparator();
  mUtilitiesMenu->addAction( mActionShowTrustedCAs );

  btnUtilities->setMenu( mUtilitiesMenu );
}

void QgsAuthAuthoritiesEditor::showCertInfo( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  QString digest( item->data( 0, Qt::UserRole ).toString() );

  QMap<QString, QPair<QgsAuthCertUtils::CaCertSource , QSslCertificate> > cacertscache(
    QgsAuthManager::instance()->getCaCertsCache() );

  if ( !cacertscache.contains( digest ) )
  {
    QgsDebugMsg( "Certificate Authority not in CA certs cache" );
    return;
  }

  QSslCertificate cert( cacertscache.value( digest ).second );

  QgsAuthCertInfoDialog * dlg = new QgsAuthCertInfoDialog( cert, true, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  if ( dlg->trustCacheRebuilt() )
  {
    // QgsAuthManager::instance()->rebuildTrustedCaCertsCache() already called in dlg
    populateCaCertsView();
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::selectionChanged( const QItemSelection& selected , const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  checkSelection();
}

void QgsAuthAuthoritiesEditor::checkSelection()
{
  bool iscert = false;
  bool isdbcert = false;
  if ( treeWidgetCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeWidgetCAs->currentItem() );

    switch (( QgsAuthAuthoritiesEditor::CaType )item->type() )
    {
      case QgsAuthAuthoritiesEditor::RootCaCert:
        iscert = true;
        break;
      case QgsAuthAuthoritiesEditor::FileCaCert:
        iscert = true;
        break;
      case QgsAuthAuthoritiesEditor::DbCaCert:
        iscert = true;
        isdbcert = true;
        break;
      default:
        break;
    }
  }

  btnRemoveCa->setEnabled( isdbcert );
  btnInfoCa->setEnabled( iscert );
}

void QgsAuthAuthoritiesEditor::handleDoubleClick( QTreeWidgetItem *item, int col )
{
  Q_UNUSED( col );
  bool iscert = true;

  switch (( QgsAuthAuthoritiesEditor::CaType )item->type() )
  {
    case QgsAuthAuthoritiesEditor::Section:
      iscert = false;
      break;
    case QgsAuthAuthoritiesEditor::OrgName:
      iscert = false;
      break;
    default:
      break;
  }

  if ( iscert )
  {
    showCertInfo( item );
  }
}

void QgsAuthAuthoritiesEditor::on_btnAddCa_clicked()
{
  QgsAuthImportCertDialog *dlg = new QgsAuthImportCertDialog( this, QgsAuthImportCertDialog::CaFilter );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, 450 );
  if ( dlg->exec() )
  {
    const QList<QSslCertificate>& certs( dlg->certificatesToImport() );
    if ( !QgsAuthManager::instance()->storeCertAuthorities( certs ) )
    {
      messageBar()->pushMessage( tr( "ERROR storing CA(s) in authentication database" ),
                                 QgsMessageBar::CRITICAL );
    }

    QgsAuthManager::instance()->rebuildCaCertsCache();

    if ( dlg->certTrustPolicy() != QgsAuthCertUtils::DefaultTrust )
    {
      Q_FOREACH ( const QSslCertificate& cert, certs )
      {
        if ( !QgsAuthManager::instance()->storeCertTrustPolicy( cert, dlg->certTrustPolicy() ) )
        {
          authMessageOut( QObject::tr( "Could not set trust policy for imported certificates" ),
                          QObject::tr( "Authorities Manager" ),
                          QgsAuthManager::WARNING );
        }
      }
      QgsAuthManager::instance()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }

    QgsAuthManager::instance()->rebuildTrustedCaCertsCache();
    populateDatabaseCaCerts();
    mDbCaSecItem->setExpanded( true );
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::on_btnRemoveCa_clicked()
{
  QTreeWidgetItem* item( treeWidgetCAs->currentItem() );

  if ( !item )
  {
    QgsDebugMsg( "Current tree widget item not set" );
    return;
  }

  QString digest( item->data( 0, Qt::UserRole ).toString() );

  if ( digest.isEmpty() )
  {
    messageBar()->pushMessage( tr( "Certificate id missing" ),
                               QgsMessageBar::WARNING );
    return;
  }

  QMap<QString, QSslCertificate> mappedcerts(
    QgsAuthManager::instance()->getMappedDatabaseCAs() );

  if ( !mappedcerts.contains( digest ) )
  {
    QgsDebugMsg( "Certificate Authority not in mapped database CAs" );
    return;
  }

  if ( QMessageBox::warning(
         this, tr( "Remove Certificate Authority" ),
         tr( "Are you sure you want to remove the selected "
             "Certificate Authority from the database?\n\n"
             "Operation can NOT be undone!" ),
         QMessageBox::Ok | QMessageBox::Cancel,
         QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QSslCertificate cert( mappedcerts.value( digest ) );

  if ( cert.isNull() )
  {
    messageBar()->pushMessage( tr( "Certificate could not found in database for id %1:" ).arg( digest ),
                               QgsMessageBar::WARNING );
    return;
  }

  if ( !QgsAuthManager::instance()->removeCertAuthority( cert ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing CA from authentication database for id %1:" ).arg( digest ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  if ( !QgsAuthManager::instance()->removeCertTrustPolicy( cert ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing cert trust policy from authentication database for id %1:" ).arg( digest ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  QgsAuthManager::instance()->rebuildCaCertsCache();
  QgsAuthManager::instance()->rebuildTrustedCaCertsCache();
  updateCertTrustPolicyCache();

  item->parent()->removeChild( item );
  delete item;

//  populateDatabaseCaCerts();
  mDbCaSecItem->setExpanded( true );
}

void QgsAuthAuthoritiesEditor::on_btnInfoCa_clicked()
{
  if ( treeWidgetCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeWidgetCAs->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthAuthoritiesEditor::on_btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "casortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
  }
  populateCaCertsView();
}

void QgsAuthAuthoritiesEditor::editDefaultTrustPolicy()
{
  QDialog * dlg = new QDialog( this );
  dlg->setWindowTitle( tr( "Default Trust Policy" ) );
  QVBoxLayout *layout = new QVBoxLayout( dlg );

  QHBoxLayout *hlayout = new QHBoxLayout();

  QLabel * lblwarn = new QLabel( dlg );
  QStyle *style = QApplication::style();
  lblwarn->setPixmap( style->standardIcon( QStyle::SP_MessageBoxWarning ).pixmap( 48, 48 ) );
  lblwarn->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  hlayout->addWidget( lblwarn );

  QLabel *lbltxt = new QLabel( dlg );
  lbltxt->setText( tr( "Changing the default certificate authority trust policy to 'Untrusted' "
                       "can cause unexpected SSL network connection results." ) );
  lbltxt->setWordWrap( true );
  hlayout->addWidget( lbltxt );

  layout->addLayout( hlayout );

  QHBoxLayout *hlayout2 = new QHBoxLayout();

  QLabel *lblpolicy = new QLabel( tr( "Default policy" ), dlg );
  lblpolicy->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  hlayout2->addWidget( lblpolicy );

  QComboBox *cmbpolicy = new QComboBox( dlg );
  QList < QPair<QgsAuthCertUtils::CertTrustPolicy, QString> > policies;
  policies << qMakePair( QgsAuthCertUtils::Trusted,
                         QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Trusted ) )
  << qMakePair( QgsAuthCertUtils::Untrusted,
                QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Untrusted ) );

  for ( int i = 0; i < policies.size(); i++ )
  {
    cmbpolicy->addItem( policies.at( i ).second, QVariant(( int )policies.at( i ).first ) );
  }

  int idx = cmbpolicy->findData( QVariant(( int )mDefaultTrustPolicy ) );
  cmbpolicy->setCurrentIndex( idx == -1 ? 0 : idx );
  hlayout2->addWidget( cmbpolicy );

  layout->addLayout( hlayout2 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Close | QDialogButtonBox::Ok,
      Qt::Horizontal, dlg );
  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );

  layout->addWidget( buttonBox );

  connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), dlg, SLOT( close() ) );

  dlg->setLayout( layout );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, 200 );
  dlg->setMinimumSize( 400, 200 );
  dlg->setMaximumSize( 500, 300 );
  if ( dlg->exec() )
  {
    QgsAuthCertUtils::CertTrustPolicy trustpolicy(
      ( QgsAuthCertUtils::CertTrustPolicy )cmbpolicy->itemData( cmbpolicy->currentIndex() ).toInt() );
    if ( mDefaultTrustPolicy != trustpolicy )
    {
      defaultTrustPolicyChanged( trustpolicy );
    }
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::defaultTrustPolicyChanged( QgsAuthCertUtils::CertTrustPolicy trustpolicy )
{
  if ( !QgsAuthManager::instance()->setDefaultCertTrustPolicy( trustpolicy ) )
  {
    authMessageOut( QObject::tr( "Could not store default trust policy" ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::CRITICAL );
  }
  mDefaultTrustPolicy = trustpolicy;
  QgsAuthManager::instance()->rebuildCertTrustCache();
  QgsAuthManager::instance()->rebuildTrustedCaCertsCache();
  populateCaCertsView();
}

void QgsAuthAuthoritiesEditor::on_btnCaFile_clicked()
{
  QgsAuthImportCertDialog *dlg = new QgsAuthImportCertDialog( this,
      QgsAuthImportCertDialog::CaFilter,
      QgsAuthImportCertDialog::FileInput );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, 250 );
  if ( dlg->exec() )
  {
    // clear out any currently defined file certs and their trust settings
    if ( !leCaFile->text().isEmpty() )
    {
      on_btnCaFileClear_clicked();
    }

    const QString& fn = dlg->certFileToImport();
    leCaFile->setText( fn );

    if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "cafile" ), QVariant( fn ) ) )
    {
      authMessageOut( QObject::tr( "Could not store 'CA file path' in authentication database" ),
                      QObject::tr( "Authorities Manager" ),
                      QgsAuthManager::WARNING );
    }
    if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "cafileallowinvalid" ),
         QVariant( dlg->allowInvalidCerts() ) ) )
    {
      authMessageOut( QObject::tr( "Could not store 'CA file allow invalids' setting in authentication database" ),
                      QObject::tr( "Authorities Manager" ),
                      QgsAuthManager::WARNING );
    }

    QgsAuthManager::instance()->rebuildCaCertsCache();

    if ( dlg->certTrustPolicy() != QgsAuthCertUtils::DefaultTrust )
    {
      QList<QSslCertificate> certs( QgsAuthManager::instance()->getExtraFileCAs() );
      Q_FOREACH ( const QSslCertificate& cert, certs )
      {
        if ( !QgsAuthManager::instance()->storeCertTrustPolicy( cert, dlg->certTrustPolicy() ) )
        {
          authMessageOut( QObject::tr( "Could not set trust policy for imported certificates" ),
                          QObject::tr( "Authorities Manager" ),
                          QgsAuthManager::WARNING );
        }
      }
      QgsAuthManager::instance()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }

    QgsAuthManager::instance()->rebuildTrustedCaCertsCache();

    populateFileCaCerts();
    mFileCaSecItem->setExpanded( true );
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::on_btnCaFileClear_clicked()
{
  if ( !QgsAuthManager::instance()->removeAuthSetting( QString( "cafile" ) ) )
  {
    authMessageOut( QObject::tr( "Could not remove 'CA file path' from authentication database" ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
    return;
  }
  if ( !QgsAuthManager::instance()->removeAuthSetting( QString( "cafileallowinvalid" ) ) )
  {
    authMessageOut( QObject::tr( "Could not remove 'CA file allow invalids' setting from authentication database" ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
    return;
  }

  QgsAuthManager::instance()->rebuildCaCertsCache();

  QString fn( leCaFile->text() );
  if ( QFile::exists( fn ) )
  {
    QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromFile( fn ) );

    if ( !certs.isEmpty() )
    {
      if ( !QgsAuthManager::instance()->removeCertTrustPolicies( certs ) )
      {
        messageBar()->pushMessage( tr( "ERROR removing cert(s) trust policy from authentication database" ),
                                   QgsMessageBar::CRITICAL );
        return;
      }
      QgsAuthManager::instance()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }
  }

  QgsAuthManager::instance()->rebuildTrustedCaCertsCache();

  leCaFile->clear();
  populateFileCaCerts();
}

void QgsAuthAuthoritiesEditor::showTrustedCertificateAuthorities()
{
  QgsAuthTrustedCAsDialog *dlg = new QgsAuthTrustedCAsDialog( this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthAuthoritiesEditor::showEvent( QShowEvent * e )
{
  if ( !mDisabled )
  {
    treeWidgetCAs->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar * QgsAuthAuthoritiesEditor::messageBar()
{
  return msgBar;
}

int QgsAuthAuthoritiesEditor::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
