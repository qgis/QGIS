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
#include <QSslConfiguration>

#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthimportcertdialog.h"
#include "qgsauthmanager.h"
#include "qgsauthtrustedcasdialog.h"
#include "qgslogger.h"
#include "qgsvariantutils.h"

QgsAuthAuthoritiesEditor::QgsAuthAuthoritiesEditor( QWidget *parent )
  : QWidget( parent )
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
    connect( btnAddCa, &QToolButton::clicked, this, &QgsAuthAuthoritiesEditor::btnAddCa_clicked );
    connect( btnRemoveCa, &QToolButton::clicked, this, &QgsAuthAuthoritiesEditor::btnRemoveCa_clicked );
    connect( btnInfoCa, &QToolButton::clicked, this, &QgsAuthAuthoritiesEditor::btnInfoCa_clicked );
    connect( btnGroupByOrg, &QToolButton::toggled, this, &QgsAuthAuthoritiesEditor::btnGroupByOrg_toggled );
    connect( btnCaFile, &QToolButton::clicked, this, &QgsAuthAuthoritiesEditor::btnCaFile_clicked );
    connect( btnCaFileClear, &QToolButton::clicked, this, &QgsAuthAuthoritiesEditor::btnCaFileClear_clicked );

    connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
             this, &QgsAuthAuthoritiesEditor::authMessageOut );

    connect( QgsApplication::authManager(), &QgsAuthManager::authDatabaseChanged,
             this, &QgsAuthAuthoritiesEditor::refreshCaCertsView );

    setupCaCertsTree();

    connect( treeWidgetCAs->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &QgsAuthAuthoritiesEditor::selectionChanged );

    connect( treeWidgetCAs, &QTreeWidget::itemDoubleClicked,
             this, &QgsAuthAuthoritiesEditor::handleDoubleClick );

    connect( btnViewRefresh, &QAbstractButton::clicked, this, &QgsAuthAuthoritiesEditor::refreshCaCertsView );

    const QVariant cafileval = QgsApplication::authManager()->authSetting( QStringLiteral( "cafile" ) );
    if ( !QgsVariantUtils::isNull( cafileval ) )
    {
      leCaFile->setText( cafileval.toString() );
    }

    btnGroupByOrg->setChecked( false );
    const QVariant sortbyval = QgsApplication::authManager()->authSetting( QStringLiteral( "casortby" ), QVariant( false ) );
    if ( !QgsVariantUtils::isNull( sortbyval ) )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    mDefaultTrustPolicy = QgsApplication::authManager()->defaultCertTrustPolicy();
    populateCaCertsView();
    checkSelection();

    populateUtilitiesMenu();
  }
}

static void setItemBold_( QTreeWidgetItem *item )
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
    static_cast<int>( QgsAuthAuthoritiesEditor::Section ) );
  setItemBold_( mDbCaSecItem );
  mDbCaSecItem->setFlags( Qt::ItemIsEnabled );
  mDbCaSecItem->setExpanded( true );
  treeWidgetCAs->insertTopLevelItem( 0, mDbCaSecItem );

  mFileCaSecItem = new QTreeWidgetItem(
    treeWidgetCAs,
    QStringList( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::FromFile ) ),
    static_cast<int>( QgsAuthAuthoritiesEditor::Section ) );
  setItemBold_( mFileCaSecItem );
  mFileCaSecItem->setFlags( Qt::ItemIsEnabled );
  mFileCaSecItem->setExpanded( true );
  treeWidgetCAs->insertTopLevelItem( 0, mFileCaSecItem );

  mRootCaSecItem = new QTreeWidgetItem(
    treeWidgetCAs,
    QStringList( QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::SystemRoot ) ),
    static_cast<int>( QgsAuthAuthoritiesEditor::Section ) );
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
//  QgsApplication::authManager()->rebuildCaCertsCache();
  populateCaCertsView();
}

static void removeChildren_( QTreeWidgetItem *item )
{
  const auto constTakeChildren = item->takeChildren();
  for ( QTreeWidgetItem *child : constTakeChildren )
  {
    delete child;
  }
}

void QgsAuthAuthoritiesEditor::populateDatabaseCaCerts()
{
  removeChildren_( mDbCaSecItem );

  const bool expanded = mDbCaSecItem->isExpanded();
  populateCaCertsSection( mDbCaSecItem,
                          QgsApplication::authManager()->databaseCAs(),
                          QgsAuthAuthoritiesEditor::DbCaCert );
  mDbCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateFileCaCerts()
{
  removeChildren_( mFileCaSecItem );

  const bool expanded = mFileCaSecItem->isExpanded();
  populateCaCertsSection( mFileCaSecItem,
                          QgsApplication::authManager()->extraFileCAs(),
                          QgsAuthAuthoritiesEditor::FileCaCert );
  mFileCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateRootCaCerts()
{
  removeChildren_( mRootCaSecItem );

  const bool expanded = mRootCaSecItem->isExpanded();
  populateCaCertsSection( mRootCaSecItem,
                          QgsApplication::authManager()->systemRootCAs(),
                          QgsAuthAuthoritiesEditor::RootCaCert );
  mRootCaSecItem->setExpanded( expanded );
}

void QgsAuthAuthoritiesEditor::populateCaCertsSection( QTreeWidgetItem *item, const QList<QSslCertificate> &certs,
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

void QgsAuthAuthoritiesEditor::appendCertsToGroup( const QList<QSslCertificate> &certs,
    QgsAuthAuthoritiesEditor::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeWidgetCAs->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  const QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem *grpitem( new QTreeWidgetItem( parent,
                              QStringList() << it.key(),
                              static_cast<int>( QgsAuthAuthoritiesEditor::OrgName ) ) );
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

void QgsAuthAuthoritiesEditor::appendCertsToItem( const QList<QSslCertificate> &certs,
    QgsAuthAuthoritiesEditor::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeWidgetCAs->currentItem();
  }

  const QBrush greenb( QgsAuthGuiUtils::greenColor() );
  const QBrush redb( QgsAuthGuiUtils::redColor() );

  const QStringList trustedids = mCertTrustCache.value( QgsAuthCertUtils::Trusted );
  const QStringList untrustedids = mCertTrustCache.value( QgsAuthCertUtils::Untrusted );

  // Columns: Common Name, Serial #, Expiry Date
  const auto constCerts = certs;
  for ( const QSslCertificate &cert : constCerts )
  {
    const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

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
    else if ( untrustedids.contains( id )
              || cert.isBlacklisted()
              || cert.isNull()
              || cert.expiryDate() <= QDateTime::currentDateTime()
              || cert.effectiveDate() > QDateTime::currentDateTime() )
    {
      policy = QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Untrusted );
    }
    coltxts << policy;

    QTreeWidgetItem *item( new QTreeWidgetItem( parent, coltxts, static_cast<int>( catype ) ) );

    item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificate.svg" ) ) );
    if ( cert.isBlacklisted()
         || cert.isNull()
         || cert.expiryDate() <= QDateTime::currentDateTime()
         || cert.effectiveDate() > QDateTime::currentDateTime() )
    {
      item->setForeground( 2, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificateUntrusted.svg" ) ) );
    }

    if ( trustedids.contains( id ) )
    {
      item->setForeground( 3, greenb );
      if ( !cert.isBlacklisted()
           && !cert.isNull()
           && cert.expiryDate() > QDateTime::currentDateTime()
           && cert.effectiveDate() <= QDateTime::currentDateTime() )
      {
        item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificateTrusted.svg" ) ) );
      }
    }
    else if ( untrustedids.contains( id ) )
    {
      item->setForeground( 3, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificateUntrusted.svg" ) ) );
    }
    else if ( mDefaultTrustPolicy == QgsAuthCertUtils::Untrusted )
    {
      item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificateUntrusted.svg" ) ) );
    }

    item->setData( 0, Qt::UserRole, id );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthAuthoritiesEditor::updateCertTrustPolicyCache()
{
  mCertTrustCache = QgsApplication::authManager()->certTrustCache();
}

void QgsAuthAuthoritiesEditor::populateUtilitiesMenu()
{
  mActionDefaultTrustPolicy = new QAction( QStringLiteral( "Change default trust policy" ), this );
  connect( mActionDefaultTrustPolicy, &QAction::triggered, this, &QgsAuthAuthoritiesEditor::editDefaultTrustPolicy );

  mActionShowTrustedCAs = new QAction( QStringLiteral( "Show trusted authorities/issuers" ), this );
  connect( mActionShowTrustedCAs, &QAction::triggered, this, &QgsAuthAuthoritiesEditor::showTrustedCertificateAuthorities );

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

  const QString digest( item->data( 0, Qt::UserRole ).toString() );

  const QMap<QString, QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> > cacertscache(
    QgsApplication::authManager()->caCertsCache() );

  if ( !cacertscache.contains( digest ) )
  {
    QgsDebugMsg( QStringLiteral( "Certificate Authority not in CA certs cache" ) );
    return;
  }

  const QSslCertificate cert( cacertscache.value( digest ).second );

  QgsAuthCertInfoDialog *dlg = new QgsAuthCertInfoDialog( cert, true, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  if ( dlg->trustCacheRebuilt() )
  {
    // QgsApplication::authManager()->rebuildTrustedCaCertsCache() already called in dlg
    populateCaCertsView();
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  checkSelection();
}

void QgsAuthAuthoritiesEditor::checkSelection()
{
  bool iscert = false;
  bool isdbcert = false;
  if ( treeWidgetCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeWidgetCAs->currentItem() );

    switch ( ( QgsAuthAuthoritiesEditor::CaType )item->type() )
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
  Q_UNUSED( col )
  bool iscert = true;

  switch ( ( QgsAuthAuthoritiesEditor::CaType )item->type() )
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

void QgsAuthAuthoritiesEditor::btnAddCa_clicked()
{
  QgsAuthImportCertDialog *dlg = new QgsAuthImportCertDialog( this, QgsAuthImportCertDialog::CaFilter );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, 450 );
  if ( dlg->exec() )
  {
    const QList<QSslCertificate> &certs( dlg->certificatesToImport() );
    if ( !QgsApplication::authManager()->storeCertAuthorities( certs ) )
    {
      messageBar()->pushMessage( tr( "ERROR storing CA(s) in authentication database" ),
                                 Qgis::MessageLevel::Critical );
    }

    QgsApplication::authManager()->rebuildCaCertsCache();

    if ( dlg->certTrustPolicy() != QgsAuthCertUtils::DefaultTrust )
    {
      const auto constCerts = certs;
      for ( const QSslCertificate &cert : constCerts )
      {
        if ( !QgsApplication::authManager()->storeCertTrustPolicy( cert, dlg->certTrustPolicy() ) )
        {
          authMessageOut( QObject::tr( "Could not set trust policy for imported certificates" ),
                          QObject::tr( "Authorities Manager" ),
                          QgsAuthManager::WARNING );
        }
      }
      QgsApplication::authManager()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }

    QgsApplication::authManager()->rebuildTrustedCaCertsCache();
    populateDatabaseCaCerts();
    mDbCaSecItem->setExpanded( true );
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::btnRemoveCa_clicked()
{
  QTreeWidgetItem *item( treeWidgetCAs->currentItem() );

  if ( !item )
  {
    QgsDebugMsg( QStringLiteral( "Current tree widget item not set" ) );
    return;
  }

  const QString digest( item->data( 0, Qt::UserRole ).toString() );

  if ( digest.isEmpty() )
  {
    messageBar()->pushMessage( tr( "Certificate id missing" ),
                               Qgis::MessageLevel::Warning );
    return;
  }

  const QMap<QString, QSslCertificate> mappedcerts(
    QgsApplication::authManager()->mappedDatabaseCAs() );

  if ( !mappedcerts.contains( digest ) )
  {
    QgsDebugMsg( QStringLiteral( "Certificate Authority not in mapped database CAs" ) );
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

  const QSslCertificate cert( mappedcerts.value( digest ) );

  if ( cert.isNull() )
  {
    messageBar()->pushMessage( tr( "Certificate could not be found in database for id %1:" ).arg( digest ),
                               Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsApplication::authManager()->removeCertAuthority( cert ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing CA from authentication database for id %1:" ).arg( digest ),
                               Qgis::MessageLevel::Critical );
    return;
  }

  if ( !QgsApplication::authManager()->removeCertTrustPolicy( cert ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing cert trust policy from authentication database for id %1:" ).arg( digest ),
                               Qgis::MessageLevel::Critical );
    return;
  }

  QgsApplication::authManager()->rebuildCaCertsCache();
  QgsApplication::authManager()->rebuildTrustedCaCertsCache();
  updateCertTrustPolicyCache();

  item->parent()->removeChild( item );
  delete item;

//  populateDatabaseCaCerts();
  mDbCaSecItem->setExpanded( true );
}

void QgsAuthAuthoritiesEditor::btnInfoCa_clicked()
{
  if ( treeWidgetCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeWidgetCAs->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthAuthoritiesEditor::btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsApplication::authManager()->storeAuthSetting( QStringLiteral( "casortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
  }
  populateCaCertsView();
}

void QgsAuthAuthoritiesEditor::editDefaultTrustPolicy()
{
  QDialog *dlg = new QDialog( this );
  dlg->setWindowTitle( tr( "Default Trust Policy" ) );
  QVBoxLayout *layout = new QVBoxLayout( dlg );

  QHBoxLayout *hlayout = new QHBoxLayout();

  QLabel *lblwarn = new QLabel( dlg );
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
    cmbpolicy->addItem( policies.at( i ).second, QVariant( static_cast<int>( policies.at( i ).first ) ) );
  }

  const int idx = cmbpolicy->findData( QVariant( static_cast<int>( mDefaultTrustPolicy ) ) );
  cmbpolicy->setCurrentIndex( idx == -1 ? 0 : idx );
  hlayout2->addWidget( cmbpolicy );

  layout->addLayout( hlayout2 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Close | QDialogButtonBox::Ok,
      Qt::Horizontal, dlg );
  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );

  layout->addWidget( buttonBox );

  connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, dlg, &QWidget::close );

  dlg->setLayout( layout );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, 200 );
  dlg->setMinimumSize( 400, 200 );
  dlg->setMaximumSize( 500, 300 );
  if ( dlg->exec() )
  {
    const QgsAuthCertUtils::CertTrustPolicy trustpolicy(
      ( QgsAuthCertUtils::CertTrustPolicy )cmbpolicy->currentData().toInt() );
    if ( mDefaultTrustPolicy != trustpolicy )
    {
      defaultTrustPolicyChanged( trustpolicy );
    }
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::defaultTrustPolicyChanged( QgsAuthCertUtils::CertTrustPolicy trustpolicy )
{
  if ( !QgsApplication::authManager()->setDefaultCertTrustPolicy( trustpolicy ) )
  {
    authMessageOut( QObject::tr( "Could not store default trust policy." ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::CRITICAL );
  }
  mDefaultTrustPolicy = trustpolicy;
  QgsApplication::authManager()->rebuildCertTrustCache();
  QgsApplication::authManager()->rebuildTrustedCaCertsCache();
  populateCaCertsView();
}

void QgsAuthAuthoritiesEditor::btnCaFile_clicked()
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
      btnCaFileClear_clicked();
    }

    const QString &fn = dlg->certFileToImport();
    leCaFile->setText( fn );

    if ( !QgsApplication::authManager()->storeAuthSetting( QStringLiteral( "cafile" ), QVariant( fn ) ) )
    {
      authMessageOut( QObject::tr( "Could not store 'CA file path' in authentication database." ),
                      QObject::tr( "Authorities Manager" ),
                      QgsAuthManager::WARNING );
    }
    if ( !QgsApplication::authManager()->storeAuthSetting( QStringLiteral( "cafileallowinvalid" ),
         QVariant( dlg->allowInvalidCerts() ) ) )
    {
      authMessageOut( QObject::tr( "Could not store 'CA file allow invalids' setting in authentication database." ),
                      QObject::tr( "Authorities Manager" ),
                      QgsAuthManager::WARNING );
    }

    QgsApplication::authManager()->rebuildCaCertsCache();

    if ( dlg->certTrustPolicy() != QgsAuthCertUtils::DefaultTrust )
    {
      const QList<QSslCertificate> certs( QgsApplication::authManager()->extraFileCAs() );
      const auto constCerts = certs;
      for ( const QSslCertificate &cert : constCerts )
      {
        if ( !QgsApplication::authManager()->storeCertTrustPolicy( cert, dlg->certTrustPolicy() ) )
        {
          authMessageOut( QObject::tr( "Could not set trust policy for imported certificates." ),
                          QObject::tr( "Authorities Manager" ),
                          QgsAuthManager::WARNING );
        }
      }
      QgsApplication::authManager()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }

    QgsApplication::authManager()->rebuildTrustedCaCertsCache();

    populateFileCaCerts();
    mFileCaSecItem->setExpanded( true );
  }
  dlg->deleteLater();
}

void QgsAuthAuthoritiesEditor::btnCaFileClear_clicked()
{
  if ( !QgsApplication::authManager()->removeAuthSetting( QStringLiteral( "cafile" ) ) )
  {
    authMessageOut( QObject::tr( "Could not remove 'CA file path' from authentication database." ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
    return;
  }
  if ( !QgsApplication::authManager()->removeAuthSetting( QStringLiteral( "cafileallowinvalid" ) ) )
  {
    authMessageOut( QObject::tr( "Could not remove 'CA file allow invalids' setting from authentication database." ),
                    QObject::tr( "Authorities Manager" ),
                    QgsAuthManager::WARNING );
    return;
  }

  QgsApplication::authManager()->rebuildCaCertsCache();

  const QString fn( leCaFile->text() );
  if ( QFile::exists( fn ) )
  {
    const QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromFile( fn ) );

    if ( !certs.isEmpty() )
    {
      if ( !QgsApplication::authManager()->removeCertTrustPolicies( certs ) )
      {
        messageBar()->pushMessage( tr( "ERROR removing cert(s) trust policy from authentication database." ),
                                   Qgis::MessageLevel::Critical );
        return;
      }
      QgsApplication::authManager()->rebuildCertTrustCache();
      updateCertTrustPolicyCache();
    }
  }

  QgsApplication::authManager()->rebuildTrustedCaCertsCache();

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

void QgsAuthAuthoritiesEditor::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  const int levelint = static_cast<int>( level );
  messageBar()->pushMessage( authtag, message, ( Qgis::MessageLevel )levelint, 7 );
}

void QgsAuthAuthoritiesEditor::showEvent( QShowEvent *e )
{
  if ( !mDisabled )
  {
    treeWidgetCAs->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar *QgsAuthAuthoritiesEditor::messageBar()
{
  return msgBar;
}

int QgsAuthAuthoritiesEditor::messageTimeout()
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}
