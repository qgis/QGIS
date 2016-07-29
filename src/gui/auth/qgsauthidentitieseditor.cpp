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
#include <QSettings>

#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgsauthimportidentitydialog.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#include "qgslogger.h"


QgsAuthIdentitiesEditor::QgsAuthIdentitiesEditor( QWidget *parent )
    : QWidget( parent )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
    , mRootCertIdentItem( nullptr )
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
             this, SLOT( refreshIdentitiesView() ) );

    setupIdentitiesTree();

    connect( treeIdentities->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    connect( treeIdentities, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( handleDoubleClick( QTreeWidgetItem *, int ) ) );

    connect( btnViewRefresh, SIGNAL( clicked() ), this, SLOT( refreshIdentitiesView() ) );

    btnGroupByOrg->setChecked( false );
    QVariant sortbyval = QgsAuthManager::instance()->getAuthSetting( QString( "identitiessortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    populateIdentitiesView();
    checkSelection();
  }
}

QgsAuthIdentitiesEditor::~QgsAuthIdentitiesEditor()
{
}

static void setItemBold_( QTreeWidgetItem* item )
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
    ( int )QgsAuthIdentitiesEditor::Section );
  setItemBold_( mRootCertIdentItem );
  mRootCertIdentItem->setFlags( Qt::ItemIsEnabled );
  mRootCertIdentItem->setExpanded( true );
  treeIdentities->insertTopLevelItem( 0, mRootCertIdentItem );
}

static void removeChildren_( QTreeWidgetItem* item )
{
  Q_FOREACH ( QTreeWidgetItem* child, item->takeChildren() )
  {
    delete child;
  }
}

void QgsAuthIdentitiesEditor::populateIdentitiesView()
{
  removeChildren_( mRootCertIdentItem );

  populateIdentitiesSection( mRootCertIdentItem,
                             QgsAuthManager::instance()->getCertIdentities(),
                             QgsAuthIdentitiesEditor::CertIdentity );
}

void QgsAuthIdentitiesEditor::refreshIdentitiesView()
{
  populateIdentitiesView();
}

void QgsAuthIdentitiesEditor::populateIdentitiesSection( QTreeWidgetItem *item, const QList<QSslCertificate>& certs,
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

void QgsAuthIdentitiesEditor::appendIdentitiesToGroup( const QList<QSslCertificate>& certs,
    QgsAuthIdentitiesEditor::IdentityType identype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeIdentities->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem * grpitem( new QTreeWidgetItem( parent,
                               QStringList() << it.key(),
                               ( int )QgsAuthIdentitiesEditor::OrgName ) );
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

void QgsAuthIdentitiesEditor::appendIdentitiesToItem( const QList<QSslCertificate>& certs,
    QgsAuthIdentitiesEditor::IdentityType identype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeIdentities->currentItem();
  }

  QBrush redb( QgsAuthGuiUtils::redColor() );

  // Columns: Common Name, Serial #, Expiry Date
  Q_FOREACH ( const QSslCertificate& cert, certs )
  {
    QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

    QStringList coltxts;
    coltxts << QgsAuthCertUtils::resolvedCertName( cert );
    coltxts << QString( cert.serialNumber() );
    coltxts << cert.expiryDate().toString();

    QTreeWidgetItem * item( new QTreeWidgetItem( parent, coltxts, ( int )identype ) );

    item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificate.svg" ) );
    if ( !cert.isValid() )
    {
      item->setForeground( 2, redb );
      item->setIcon( 0, QgsApplication::getThemeIcon( "/mIconCertificateUntrusted.svg" ) );
    }

    item->setData( 0, Qt::UserRole, id );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthIdentitiesEditor::showCertInfo( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  QString digest( item->data( 0, Qt::UserRole ).toString() );

  if ( !QgsAuthManager::instance()->existsCertIdentity( digest ) )
  {
    QgsDebugMsg( "Certificate identity does not exist in database" );
    return;
  }

  QSslCertificate cert( QgsAuthManager::instance()->getCertIdentity( digest ) );

  QgsAuthCertInfoDialog * dlg = new QgsAuthCertInfoDialog( cert, false, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthIdentitiesEditor::selectionChanged( const QItemSelection& selected , const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  checkSelection();
}

void QgsAuthIdentitiesEditor::checkSelection()
{
  bool iscert = false;
  if ( treeIdentities->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeIdentities->currentItem() );

    switch (( QgsAuthIdentitiesEditor::IdentityType )item->type() )
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
  Q_UNUSED( col );
  bool iscert = true;

  switch (( QgsAuthIdentitiesEditor::IdentityType )item->type() )
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

void QgsAuthIdentitiesEditor::on_btnAddIdentity_clicked()
{
  QgsAuthImportIdentityDialog *dlg = new QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::CertIdentity, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 400, dlg->height() );
  if ( dlg->exec() )
  {
    if ( dlg->identityType() == QgsAuthImportIdentityDialog::CertIdentity )
    {
      const QPair<QSslCertificate, QSslKey>& bundle( dlg->certBundleToImport() );
      if ( !QgsAuthManager::instance()->storeCertIdentity( bundle.first, bundle.second ) )
      {
        messageBar()->pushMessage( tr( "ERROR storing identity bundle in authentication database" ),
                                   QgsMessageBar::CRITICAL );
      }
      populateIdentitiesView();
      mRootCertIdentItem->setExpanded( true );
    }
  }
  dlg->deleteLater();
}

void QgsAuthIdentitiesEditor::on_btnRemoveIdentity_clicked()
{
  QTreeWidgetItem* item( treeIdentities->currentItem() );

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

  if ( !QgsAuthManager::instance()->existsCertIdentity( digest ) )
  {
    QgsDebugMsg( "Certificate identity does not exist in database" );
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

  if ( !QgsAuthManager::instance()->removeCertIdentity( digest ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing cert identity from authentication database for id %1:" ).arg( digest ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  item->parent()->removeChild( item );
  delete item;
}

void QgsAuthIdentitiesEditor::on_btnInfoIdentity_clicked()
{
  if ( treeIdentities->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeIdentities->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthIdentitiesEditor::on_btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "identitiessortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Authentication Identities" ),
                    QgsAuthManager::WARNING );
  }
  populateIdentitiesView();
}

void QgsAuthIdentitiesEditor::authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthIdentitiesEditor::showEvent( QShowEvent * e )
{
  if ( !mDisabled )
  {
    treeIdentities->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar * QgsAuthIdentitiesEditor::messageBar()
{
  return msgBar;
}

int QgsAuthIdentitiesEditor::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
