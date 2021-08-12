/***************************************************************************
    qgsauthtrustedcasdialog.cpp
    ---------------------
    begin                : May 9, 2015
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

#include "qgsauthtrustedcasdialog.h"
#include "ui_qgsauthtrustedcasdialog.h"

#include <QPushButton>

#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsAuthTrustedCAsDialog::QgsAuthTrustedCAsDialog( QWidget *parent,
    const QList<QSslCertificate> &trustedCAs )
  : QDialog( parent )
  , mTrustedCAs( trustedCAs )
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
    connect( btnInfoCa, &QToolButton::clicked, this, &QgsAuthTrustedCAsDialog::btnInfoCa_clicked );
    connect( btnGroupByOrg, &QToolButton::toggled, this, &QgsAuthTrustedCAsDialog::btnGroupByOrg_toggled );

    connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
             this, &QgsAuthTrustedCAsDialog::authMessageOut );

    setupCaCertsTree();

    connect( treeTrustedCAs->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &QgsAuthTrustedCAsDialog::selectionChanged );

    connect( treeTrustedCAs, &QTreeWidget::itemDoubleClicked,
             this, &QgsAuthTrustedCAsDialog::handleDoubleClick );


    btnGroupByOrg->setChecked( false );
    const QVariant sortbyval = QgsApplication::authManager()->authSetting( QStringLiteral( "trustedcasortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    populateCaCertsView();
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

void QgsAuthTrustedCAsDialog::setupCaCertsTree()
{
  treeTrustedCAs->setColumnCount( 3 );
  treeTrustedCAs->setHeaderLabels(
    QStringList() << tr( "Common Name" )
    << tr( "Serial #" )
    << tr( "Expiry Date" ) );
  treeTrustedCAs->setColumnWidth( 0, 300 );
  treeTrustedCAs->setColumnWidth( 1, 75 );

  // add root section
  mRootCaSecItem = new QTreeWidgetItem(
    treeTrustedCAs,
    QStringList( tr( "Authorities/Issuers" ) ),
    static_cast<int>( QgsAuthTrustedCAsDialog::Section ) );
  setItemBold_( mRootCaSecItem );
  mRootCaSecItem->setFlags( Qt::ItemIsEnabled );
  mRootCaSecItem->setExpanded( true );
  treeTrustedCAs->insertTopLevelItem( 0, mRootCaSecItem );
}

static void removeChildren_( QTreeWidgetItem *item )
{
  const auto constTakeChildren = item->takeChildren();
  for ( QTreeWidgetItem *child : constTakeChildren )
  {
    delete child;
  }
}

void QgsAuthTrustedCAsDialog::populateCaCertsView()
{
  removeChildren_( mRootCaSecItem );

  if ( mTrustedCAs.isEmpty() )
  {
    mTrustedCAs = QgsApplication::authManager()->trustedCaCerts();
  }

  populateCaCertsSection( mRootCaSecItem, mTrustedCAs, QgsAuthTrustedCAsDialog::CaCert );
}

void QgsAuthTrustedCAsDialog::populateCaCertsSection( QTreeWidgetItem *item, const QList<QSslCertificate> &certs,
    QgsAuthTrustedCAsDialog::CaType catype )
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

void QgsAuthTrustedCAsDialog::appendCertsToGroup( const QList<QSslCertificate> &certs,
    QgsAuthTrustedCAsDialog::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeTrustedCAs->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  const QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem *grpitem( new QTreeWidgetItem( parent,
                              QStringList() << it.key(),
                              static_cast<int>( QgsAuthTrustedCAsDialog::OrgName ) ) );
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

void QgsAuthTrustedCAsDialog::appendCertsToItem( const QList<QSslCertificate> &certs,
    QgsAuthTrustedCAsDialog::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.empty() )
    return;

  if ( !parent )
  {
    parent = treeTrustedCAs->currentItem();
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

    QTreeWidgetItem *item( new QTreeWidgetItem( parent, coltxts, static_cast<int>( catype ) ) );

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

void QgsAuthTrustedCAsDialog::showCertInfo( QTreeWidgetItem *item )
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

  QgsAuthCertInfoDialog *dlg = new QgsAuthCertInfoDialog( cert, false, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthTrustedCAsDialog::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  checkSelection();
}

void QgsAuthTrustedCAsDialog::checkSelection()
{
  bool iscert = false;
  if ( treeTrustedCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeTrustedCAs->currentItem() );

    switch ( ( QgsAuthTrustedCAsDialog::CaType )item->type() )
    {
      case QgsAuthTrustedCAsDialog::CaCert:
        iscert = true;
        break;
      default:
        break;
    }
  }

  btnInfoCa->setEnabled( iscert );
}

void QgsAuthTrustedCAsDialog::handleDoubleClick( QTreeWidgetItem *item, int col )
{
  Q_UNUSED( col )
  bool iscert = true;

  switch ( ( QgsAuthTrustedCAsDialog::CaType )item->type() )
  {
    case QgsAuthTrustedCAsDialog::Section:
      iscert = false;
      break;
    case QgsAuthTrustedCAsDialog::OrgName:
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

void QgsAuthTrustedCAsDialog::btnInfoCa_clicked()
{
  if ( treeTrustedCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem *item( treeTrustedCAs->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthTrustedCAsDialog::btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsApplication::authManager()->storeAuthSetting( QStringLiteral( "trustedcasortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Trusted Authorities/Issuers" ),
                    QgsAuthManager::WARNING );
  }
  populateCaCertsView();
}

void QgsAuthTrustedCAsDialog::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  const int levelint = static_cast<int>( level );
  messageBar()->pushMessage( authtag, message, ( Qgis::MessageLevel )levelint, 7 );
}

void QgsAuthTrustedCAsDialog::showEvent( QShowEvent *e )
{
  if ( !mDisabled )
  {
    treeTrustedCAs->setFocus();
  }
  QDialog::showEvent( e );
}

QgsMessageBar *QgsAuthTrustedCAsDialog::messageBar()
{
  return msgBar;
}

int QgsAuthTrustedCAsDialog::messageTimeout()
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}
