/***************************************************************************
    qgsauthserverseditor.cpp
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

#include "qgsauthserverseditor.h"
#include "ui_qgsauthserverseditor.h"
#include "qgsauthsslimportdialog.h"

#include <QMenu>
#include <QMessageBox>
#include <QSettings>

#include "qgsapplication.h"
#include "qgsauthcertificateinfo.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#include "qgslogger.h"

QgsAuthServersEditor::QgsAuthServersEditor( QWidget *parent )
    : QWidget( parent )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
    , mRootSslConfigItem( nullptr )
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
             this, SLOT( refreshSslConfigsView() ) );

    setupSslConfigsTree();

    connect( treeServerConfigs->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    connect( treeServerConfigs, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( handleDoubleClick( QTreeWidgetItem *, int ) ) );

    connect( btnViewRefresh, SIGNAL( clicked() ), this, SLOT( refreshSslConfigsView() ) );

    btnGroupByOrg->setChecked( false );
    QVariant sortbyval = QgsAuthManager::instance()->getAuthSetting( QString( "serverssortby" ), QVariant( false ) );
    if ( !sortbyval.isNull() )
      btnGroupByOrg->setChecked( sortbyval.toBool() );

    populateSslConfigsView();
    checkSelection();
  }
}

QgsAuthServersEditor::~QgsAuthServersEditor()
{
}

static void setItemBold_( QTreeWidgetItem* item )
{
  item->setFirstColumnSpanned( true );
  QFont secf( item->font( 0 ) );
  secf.setBold( true );
  item->setFont( 0, secf );
}


void QgsAuthServersEditor::setupSslConfigsTree()
{
  treeServerConfigs->setColumnCount( 3 );
  treeServerConfigs->setHeaderLabels(
    QStringList() << tr( "Common Name" )
    << tr( "Host" )
    << tr( "Expiry Date" ) );
  treeServerConfigs->setColumnWidth( 0, 275 );
  treeServerConfigs->setColumnWidth( 1, 200 );

  // add root sections
  mRootSslConfigItem = new QTreeWidgetItem(
    treeServerConfigs,
    QStringList( tr( "SSL Server Configurations" ) ),
    ( int )QgsAuthServersEditor::Section );
  setItemBold_( mRootSslConfigItem );
  mRootSslConfigItem->setFlags( Qt::ItemIsEnabled );
  mRootSslConfigItem->setExpanded( true );
  treeServerConfigs->insertTopLevelItem( 0, mRootSslConfigItem );
}

static void removeChildren_( QTreeWidgetItem* item )
{
  Q_FOREACH ( QTreeWidgetItem* child, item->takeChildren() )
  {
    delete child;
  }
}

void QgsAuthServersEditor::populateSslConfigsView()
{
  removeChildren_( mRootSslConfigItem );

  populateSslConfigsSection( mRootSslConfigItem,
                             QgsAuthManager::instance()->getSslCertCustomConfigs(),
                             QgsAuthServersEditor::ServerConfig );
}

void QgsAuthServersEditor::refreshSslConfigsView()
{
  populateSslConfigsView();
}

void QgsAuthServersEditor::populateSslConfigsSection( QTreeWidgetItem *item,
    const QList<QgsAuthConfigSslServer>& configs,
    QgsAuthServersEditor::ConfigType conftype )
{
  if ( btnGroupByOrg->isChecked() )
  {
    appendSslConfigsToGroup( configs, conftype, item );
  }
  else
  {
    appendSslConfigsToItem( configs, conftype, item );
  }
}

void QgsAuthServersEditor::appendSslConfigsToGroup( const QList<QgsAuthConfigSslServer>& configs,
    QgsAuthServersEditor::ConfigType conftype,
    QTreeWidgetItem *parent )
{
  if ( configs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeServerConfigs->currentItem();
  }

  // TODO: find all organizational name, sort and make subsections
  QMap< QString, QList<QgsAuthConfigSslServer> > orgconfigs(
    QgsAuthCertUtils::sslConfigsGroupedByOrg( configs ) );

  QMap< QString, QList<QgsAuthConfigSslServer> >::const_iterator it = orgconfigs.constBegin();
  for ( ; it != orgconfigs.constEnd(); ++it )
  {
    QTreeWidgetItem * grpitem( new QTreeWidgetItem( parent,
                               QStringList() << it.key(),
                               ( int )QgsAuthServersEditor::OrgName ) );
    grpitem->setFirstColumnSpanned( true );
    grpitem->setFlags( Qt::ItemIsEnabled );
    grpitem->setExpanded( true );

    QBrush orgb( grpitem->foreground( 0 ) );
    orgb.setColor( QColor::fromRgb( 90, 90, 90 ) );
    grpitem->setForeground( 0, orgb );
    QFont grpf( grpitem->font( 0 ) );
    grpf.setItalic( true );
    grpitem->setFont( 0, grpf );

    appendSslConfigsToItem( it.value(), conftype, grpitem );
  }

  parent->sortChildren( 0, Qt::AscendingOrder );
}

void QgsAuthServersEditor::appendSslConfigsToItem( const QList<QgsAuthConfigSslServer>& configs,
    QgsAuthServersEditor::ConfigType conftype,
    QTreeWidgetItem *parent )
{
  if ( configs.size() < 1 )
    return;

  if ( !parent )
  {
    parent = treeServerConfigs->currentItem();
  }

  QBrush redb( QgsAuthGuiUtils::redColor() );

  // Columns: Common Name, Host, Expiry Date
  Q_FOREACH ( const QgsAuthConfigSslServer& config, configs )
  {
    QSslCertificate cert( config.sslCertificate() );
    QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

    QStringList coltxts;
    coltxts << QgsAuthCertUtils::resolvedCertName( cert );
    coltxts << QString( config.sslHostPort() );
    coltxts << cert.expiryDate().toString();

    QTreeWidgetItem * item( new QTreeWidgetItem( parent, coltxts, ( int )conftype ) );

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

void QgsAuthServersEditor::selectionChanged( const QItemSelection& selected , const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  checkSelection();
}

void QgsAuthServersEditor::checkSelection()
{
  bool isconfig = false;
  if ( treeServerConfigs->selectionModel()->selection().length() > 0 )
  {
    QTreeWidgetItem* item( treeServerConfigs->currentItem() );

    switch (( QgsAuthServersEditor::ConfigType )item->type() )
    {
      case QgsAuthServersEditor::ServerConfig :
        isconfig = true;
        break;
      default:
        break;
    }
  }

  btnRemoveServer->setEnabled( isconfig );
  btnEditServer->setEnabled( isconfig );
}

void QgsAuthServersEditor::handleDoubleClick( QTreeWidgetItem *item, int col )
{
  Q_UNUSED( col );
  bool isconfig = true;

  switch (( QgsAuthServersEditor::ConfigType )item->type() )
  {
    case QgsAuthServersEditor::Section:
      isconfig = false;
      break;
    case QgsAuthServersEditor::OrgName:
      isconfig = false;
      break;
    default:
      break;
  }

  if ( isconfig )
  {
    on_btnEditServer_clicked();
  }
}

void QgsAuthServersEditor::on_btnAddServer_clicked()
{
  QgsAuthSslImportDialog *dlg = new QgsAuthSslImportDialog( this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 580, 512 );
  if ( dlg->exec() )
  {
    refreshSslConfigsView();
  }
  dlg->deleteLater();
}

void QgsAuthServersEditor::on_btnRemoveServer_clicked()
{
  QTreeWidgetItem* item( treeServerConfigs->currentItem() );

  if ( !item )
  {
    QgsDebugMsg( "Current tree widget item not set" );
    return;
  }

  QString digest( item->data( 0, Qt::UserRole ).toString() );
  QString hostport( item->text( 1 ) );

  if ( digest.isEmpty() )
  {
    messageBar()->pushMessage( tr( "SSL custom config id missing" ),
                               QgsMessageBar::WARNING );
    return;
  }
  if ( hostport.isEmpty() )
  {
    messageBar()->pushMessage( tr( "SSL custom config host:port missing" ),
                               QgsMessageBar::WARNING );
    return;
  }

  if ( !QgsAuthManager::instance()->existsSslCertCustomConfig( digest, hostport ) )
  {
    QgsDebugMsg( QString( "SSL custom config does not exist in database for host:port, id %1:" )
                 .arg( hostport, digest ) );
    return;
  }

  if ( QMessageBox::warning(
         this, tr( "Remove SSL Custom Config" ),
         tr( "Are you sure you want to remove the selected "
             "SSL custom config from the database?\n\n"
             "Operation can NOT be undone!" ),
         QMessageBox::Ok | QMessageBox::Cancel,
         QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  if ( !QgsAuthManager::instance()->removeSslCertCustomConfig( digest, hostport ) )
  {
    messageBar()->pushMessage( tr( "ERROR removing SSL custom config from authentication database for host:port, id %1:" )
                               .arg( hostport, digest ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  item->parent()->removeChild( item );
  delete item;
}

void QgsAuthServersEditor::on_btnEditServer_clicked()
{
  QTreeWidgetItem* item( treeServerConfigs->currentItem() );

  if ( !item )
  {
    QgsDebugMsg( "Current tree widget item not set" );
    return;
  }

  QString digest( item->data( 0, Qt::UserRole ).toString() );
  QString hostport( item->text( 1 ) );

  if ( digest.isEmpty() )
  {
    messageBar()->pushMessage( tr( "SSL custom config id missing" ),
                               QgsMessageBar::WARNING );
    return;
  }
  if ( hostport.isEmpty() )
  {
    messageBar()->pushMessage( tr( "SSL custom config host:port missing" ),
                               QgsMessageBar::WARNING );
    return;
  }

  if ( !QgsAuthManager::instance()->existsSslCertCustomConfig( digest, hostport ) )
  {
    QgsDebugMsg( "SSL custom config does not exist in database" );
    return;
  }

  QgsAuthConfigSslServer config( QgsAuthManager::instance()->getSslCertCustomConfig( digest, hostport ) );
  QSslCertificate cert( config.sslCertificate() );

  QgsAuthSslConfigDialog * dlg = new QgsAuthSslConfigDialog( this, cert, hostport );
  dlg->sslCustomConfigWidget()->setConfigCheckable( false );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 500, 500 );
  if ( dlg->exec() )
  {
    refreshSslConfigsView();
  }
  dlg->deleteLater();
}

void QgsAuthServersEditor::on_btnGroupByOrg_toggled( bool checked )
{
  if ( !QgsAuthManager::instance()->storeAuthSetting( QString( "serverssortby" ), QVariant( checked ) ) )
  {
    authMessageOut( QObject::tr( "Could not store sort by preference" ),
                    QObject::tr( "Authentication SSL Configs" ),
                    QgsAuthManager::WARNING );
  }
  populateSslConfigsView();
}

void QgsAuthServersEditor::authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthServersEditor::showEvent( QShowEvent *e )
{
  if ( !mDisabled )
  {
    treeServerConfigs->setFocus();
  }
  QWidget::showEvent( e );
}

QgsMessageBar * QgsAuthServersEditor::messageBar()
{
  return msgBar;
}

int QgsAuthServersEditor::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
