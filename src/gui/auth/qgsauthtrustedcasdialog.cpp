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
#include <QSettings>

#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsAuthTrustedCAsDialog::QgsAuthTrustedCAsDialog( QWidget *parent,
    const QList<QSslCertificate>& trustedCAs )
    : QDialog( parent )
    , mTrustedCAs( trustedCAs )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
    , mRootCaSecItem( nullptr )
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

    setupCaCertsTree();

    connect( treeTrustedCAs->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    connect( treeTrustedCAs, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( handleDoubleClick( QTreeWidgetItem *, int ) ) );


    btnGroupByOrg->setChecked( false );
    QVariant sortbyval = QgsAuthManager::instance()->getAuthSetting( QString( "trustedcasortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    populateCaCertsView();
    checkSelection();
  }
}

QgsAuthTrustedCAsDialog::~QgsAuthTrustedCAsDialog()
{
}

static void setItemBold_( QTreeWidgetItem* item )
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
    ( int )QgsAuthTrustedCAsDialog::Section );
  setItemBold_( mRootCaSecItem );
  mRootCaSecItem->setFlags( Qt::ItemIsEnabled );
  mRootCaSecItem->setExpanded( true );
  treeTrustedCAs->insertTopLevelItem( 0, mRootCaSecItem );
}

static void removeChildren_( QTreeWidgetItem* item )
{
  Q_FOREACH ( QTreeWidgetItem* child, item->takeChildren() )
  {
    delete child;
  }
}

void QgsAuthTrustedCAsDialog::populateCaCertsView()
{
  removeChildren_( mRootCaSecItem );

  if ( mTrustedCAs.isEmpty() )
  {
    mTrustedCAs = QgsAuthManager::instance()->getTrustedCaCerts();
  }

  populateCaCertsSection( mRootCaSecItem, mTrustedCAs, QgsAuthTrustedCAsDialog::CaCert );
}

void QgsAuthTrustedCAsDialog::populateCaCertsSection( QTreeWidgetItem* item, const QList<QSslCertificate>& certs,
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

void QgsAuthTrustedCAsDialog::appendCertsToGroup( const QList<QSslCertificate>& certs,
    QgsAuthTrustedCAsDialog::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeTrustedCAs->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  QMap< QString, QList<QSslCertificate> > orgcerts(
    QgsAuthCertUtils::certsGroupedByOrg( certs ) );

  QMap< QString, QList<QSslCertificate> >::const_iterator it = orgcerts.constBegin();
  for ( ; it != orgcerts.constEnd(); ++it )
  {
    QTreeWidgetItem * grpitem( new QTreeWidgetItem( parent,
                               QStringList() << it.key(),
                               ( int )QgsAuthTrustedCAsDialog::OrgName ) );
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

void QgsAuthTrustedCAsDialog::appendCertsToItem( const QList<QSslCertificate>& certs,
    QgsAuthTrustedCAsDialog::CaType catype,
    QTreeWidgetItem *parent )
{
  if ( certs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeTrustedCAs->currentItem();
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

    QTreeWidgetItem * item( new QTreeWidgetItem( parent, coltxts, ( int )catype ) );

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

void QgsAuthTrustedCAsDialog::showCertInfo( QTreeWidgetItem *item )
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

  QgsAuthCertInfoDialog * dlg = new QgsAuthCertInfoDialog( cert, false, this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthTrustedCAsDialog::selectionChanged( const QItemSelection& selected , const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  checkSelection();
}

void QgsAuthTrustedCAsDialog::checkSelection()
{
  bool iscert = false;
  if ( treeTrustedCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeTrustedCAs->currentItem() );

    switch (( QgsAuthTrustedCAsDialog::CaType )item->type() )
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
  Q_UNUSED( col );
  bool iscert = true;

  switch (( QgsAuthTrustedCAsDialog::CaType )item->type() )
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

void QgsAuthTrustedCAsDialog::on_btnInfoCa_clicked()
{
  if ( treeTrustedCAs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeTrustedCAs->currentItem() );
    handleDoubleClick( item, 0 );
  }
}

void QgsAuthTrustedCAsDialog::on_btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "trustedcasortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Trusted Authorities/Issuers" ),
                    QgsAuthManager::WARNING );
  }
  populateCaCertsView();
}

void QgsAuthTrustedCAsDialog::authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthTrustedCAsDialog::showEvent( QShowEvent * e )
{
  if ( !mDisabled )
  {
    treeTrustedCAs->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar * QgsAuthTrustedCAsDialog::messageBar()
{
  return msgBar;
}

int QgsAuthTrustedCAsDialog::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
