/***************************************************************************
    qgsauthidentitieseditor.cpp
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

#include "qgsauthidentitieseditor.h"
#include "ui_qgsauthidentitieseditor.h"

#include <QMenu>
#include <QMessageBox>

#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgsauthimportidentitydialog.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#include "qgslogger.h"


QgsAuthIdentitiesEditor::QgsAuthIdentitiesEditor( QWidget *parent )
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
    connect( btnAddIdentity, &QToolButton::clicked, this, &QgsAuthIdentitiesEditor::btnAddIdentity_clicked );
    connect( btnRemoveIdentity, &QToolButton::clicked, this, &QgsAuthIdentitiesEditor::btnRemoveIdentity_clicked );
    connect( btnInfoIdentity, &QToolButton::clicked, this, &QgsAuthIdentitiesEditor::btnInfoIdentity_clicked );
    connect( btnGroupByOrg, &QToolButton::toggled, this, &QgsAuthIdentitiesEditor::btnGroupByOrg_toggled );

    connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
             this, &QgsAuthIdentitiesEditor::authMessageOut );

    connect( QgsApplication::authManager(), &QgsAuthManager::authDatabaseChanged,
             this, &QgsAuthIdentitiesEditor::refreshIdentitiesView );

    setupIdentitiesTree();

    connect( treeIdentities->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &QgsAuthIdentitiesEditor::selectionChanged );

    connect( treeIdentities, &QTreeWidget::itemDoubleClicked,
             this, &QgsAuthIdentitiesEditor::handleDoubleClick );

    connect( btnViewRefresh, &QAbstractButton::clicked, this, &QgsAuthIdentitiesEditor::refreshIdentitiesView );

    btnGroupByOrg->setChecked( false );
    const QVariant sortbyval = QgsApplication::authManager()->authSetting( QStringLiteral( "identitiessortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    populateIdentitiesView();
    checkSelection();
  }
}

static void setItemBold_( QTreeWidgetItem *item )
{
  item->setFirstColumnSpanned( true );
  QFont secf( item->font( 0 ) );
  secf.setBold( true );
  item->setFont( 0, secf );
}

void QgsAuthIdentitiesEditor::setupIdentitiesTree()
{
  treeIdentities->setColumnCount( 3 );
  treeIdentities->setHeaderLabels(
    QStringList() << tr( "Common Name" )
    << tr( "Serial #" )
    << tr( "Expiry Date" ) );
  treeIdentities->setColumnWidth( 0, 300 );
  treeIdentities->setColumnWidth( 1, 75 );

  // add root sections
  mRootCertIdentItem = new QTreeWidgetItem(
    treeIdentities,
    QStringList( tr( "Certificate Bundles" ) ),
    static_cast<int>( QgsAuthIdentitiesEditor::Section ) );
  setItemBold_( mRootCertIdentItem );
  mRootCertIdentItem->setFlags( Qt::ItemIsEnabled );
  mRootCertIdentItem->setExpanded( true );
  treeIdentities->insertTopLevelItem( 0, mRootCertIdentItem );
}

static void removeChildren_( QTreeWidgetItem *item )
{
  const auto constTakeChildren = item->takeChildren();
  for ( QTreeWidgetItem *child : constTakeChildren )
  {
    delete child;
  }
}

void QgsAuthIdentitiesEditor::populateIdentitiesView()
{
  removeChildren_( mRootCertIdentItem );

  populateIdentitiesSection( mRootCertIdentItem,
                             QgsApplication::authManager()->certIdentities(),
                             QgsAuthIdentitiesEditor::CertIdentity );
}

void QgsAuthIdentitiesEditor::refreshIdentitiesView()
{
  populateIdentitiesView();
}

void QgsAuthIdentitiesEditor::populateIdentitiesSection( QTreeWidgetItem *item, const QList<QSslCertificate> &certs,
    QgsAuthIdentitiesEditor::IdentityType identype )
{
  if ( btnGroupByOrg->isChecked() )
  {
    appendIdentitiesToGroup( certs, identype, item );
  }
  else
  {
    appendIdentitiesToItem( certs, identype, item );
  }
}

void QgsAuthIdentitiesEditor::appendIdentitiesToGroup( const QList<QSslCertificate> &certs,
    QgsAuthIdentitiesEditor::IdentityType identype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeIdentities->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  const QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem *grpitem( new QTreeWidgetItem( parent,
                              QStringList() << it.key(),
                              static_cast<int>( QgsAuthIdentitiesEditor::OrgName ) ) );
    grpitem->setFirstColumnSpanned( true );
    grpitem->setFlags( Qt::ItemIsEnabled );
    grpitem->setExpanded( true );

    QBrush orgb( grpitem->foreground( 0 ) );
    orgb.setColor( QColor::fromRgb( 90, 90, 90 ) );
    grpitem->setForeground( 0, orgb );
    QFont grpf( grpitem->font( 0 ) );
    grpf.setItalic( true );
    grpitem->setFont( 0, grpf );

    appendIdentitiesToItem( it.value(), identype, grpitem );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthIdentitiesEditor::appendIdentitiesToItem( const QList<QSslCertificate> &certs,
    QgsAuthIdentitiesEditor::IdentityType identype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeIdentities->currentItem();
  }

  const QBrush redb( QgsAuthGuiUtils::redColor() );

  // Columns: Common Name, Serial #, Expiry Date
  const auto constCerts = certs;
  for ( const QSslCertificate &cert : constCerts )
  {
    const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

    QStringList coltxts;
    coltxts << QgsAuthCertUtils::resolvedCertName( cert );
    coltxts << QString( cert.serialNumber() );
    coltxts << cert.expiryDate().toString();

    QTreeWidgetItem *item( new QTreeWidgetItem( parent, coltxts, static_cast<int>( identype ) ) );

    item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificate.svg" ) ) );
    if ( !QgsAuthCertUtils::certIsViable( cert ) )
    {
      item->setForeground( 2, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificateUntrusted.svg" ) ) );
    }

    item->setData( 0, Qt::UserRole, id );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthIdentitiesEditor::showCertInfo( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  const QString digest( item->data( 0, Qt::UserRole ).toString() );

  if ( !QgsApplication::authManager()->existsCertIdentity( digest ) )
  {
    QgsDebugMsg( QStringLiteral( "Certificate identity does not exist in database" ) );
    return;
  }

  const QSslCertificate cert( QgsApplication::authManager()->certIdentity( digest ) );

  QgsAuthCertInfoDialog *dlg = new QgsAuthCertInfoDialog( cert, false, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthIdentitiesEditor::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  checkSelection();
}

void QgsAuthIdentitiesEditor::checkSelection()
{
  bool iscert = false;
  if ( treeIdentities->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeIdentities->currentItem() );

    switch ( ( QgsAuthIdentitiesEditor::IdentityType )item->type() )
    {
      case QgsAuthIdentitiesEditor::CertIdentity:
        iscert = true;
        break;
      default:
        break;
    }
  }

  btnRemoveIdentity->setEnabled( iscert );
  btnInfoIdentity->setEnabled( iscert );
}

void QgsAuthIdentitiesEditor::handleDoubleClick( QTreeWidgetItem *item, int col )
{
  Q_UNUSED( col )
  bool iscert = true;

  switch ( ( QgsAuthIdentitiesEditor::IdentityType )item->type() )
  {
    case QgsAuthIdentitiesEditor::Section:
      iscert = false;
      break;
    case QgsAuthIdentitiesEditor::OrgName:
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

void QgsAuthIdentitiesEditor::btnAddIdentity_clicked()
{
  QgsAuthImportIdentityDialog *dlg = new QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::CertIdentity, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, dlg->height() );
  if ( dlg->exec() )
  {
    if ( dlg->identityType() == QgsAuthImportIdentityDialog::CertIdentity )
    {
      const QPair<QSslCertificate, QSslKey> &bundle( dlg->certBundleToImport() );
      if ( !QgsApplication::authManager()->storeCertIdentity( bundle.first, bundle.second ) )
      {
        messageBar()->pushMessage( tr( "ERROR storing identity bundle in authentication database." ),
                                   Qgis::MessageLevel::Critical );
      }
      populateIdentitiesView();
      mRootCertIdentItem->setExpanded( true );
    }
  }
  dlg->deleteLater();
}

void QgsAuthIdentitiesEditor::btnRemoveIdentity_clicked()
{
  QTreeWidgetItem *item( treeIdentities->currentItem() );

  if ( !item )
  {
    QgsDebugMsg( QStringLiteral( "Current tree widget item not set" ) );
    return;
  }

  const QString digest( item->data( 0, Qt::UserRole ).toString() );

  if ( digest.isEmpty() )
  {
    messageBar()->pushMessage( tr( "Certificate id missing." ),
                               Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsApplication::authManager()->existsCertIdentity( digest ) )
  {
    QgsDebugMsg( QStringLiteral( "Certificate identity does not exist in database" ) );
    return;
  }

  if ( QMessageBox::warning(
         this, tr( "Remove Certificate Identity" ),
         tr( "Are you sure you want to remove the selected "
             "certificate identity from the database?\n\n"
             "Operation can NOT be undone!" ),
         QMessageBox::Ok | QMessageBox::Cancel,
         QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  if ( !QgsApplication::authManager()->removeCertIdentity( digest ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing cert identity from authentication database for id %1:" ).arg( digest ),
                               Qgis::MessageLevel::Critical );
    return;
  }

  item->parent()->removeChild( item );
  delete item;
}

void QgsAuthIdentitiesEditor::btnInfoIdentity_clicked()
{
  if ( treeIdentities->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeIdentities->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthIdentitiesEditor::btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsApplication::authManager()->storeAuthSetting( QStringLiteral( "identitiessortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference." ),
                    QObject::tr( "Authentication Identities" ),
                    QgsAuthManager::WARNING );
  }
  populateIdentitiesView();
}

void QgsAuthIdentitiesEditor::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  const int levelint = static_cast<int>( level );
  messageBar()->pushMessage( authtag, message, ( Qgis::MessageLevel )levelint, 7 );
}

void QgsAuthIdentitiesEditor::showEvent( QShowEvent *e )
{
  if ( !mDisabled )
  {
    treeIdentities->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar *QgsAuthIdentitiesEditor::messageBar()
{
  return msgBar;
}

int QgsAuthIdentitiesEditor::messageTimeout()
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}
