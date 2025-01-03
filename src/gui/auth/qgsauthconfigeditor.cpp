/***************************************************************************
    qgsauthconfigeditor.cpp
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#include "qgsauthconfigeditor.h"
#include "moc_qgsauthconfigeditor.cpp"
#include "qgsauthconfigurationstoragedb.h"
#include "ui_qgsauthconfigeditor.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlTableModel>

#include "qgssettings.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgsauthguiutils.h"
#include "qgsapplication.h"

QgsAuthConfigEditor::QgsAuthConfigEditor( QWidget *parent, bool showUtilities, bool relayMessages )
  : QWidget( parent )
  , mRelayMessages( relayMessages )
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
    connect( btnAddConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnAddConfig_clicked );
    connect( btnEditConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnEditConfig_clicked );
    connect( btnRemoveConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnRemoveConfig_clicked );

    setShowUtilitiesButton( showUtilities );

    Q_NOWARN_DEPRECATED_PUSH
    const QSqlDatabase connection { QgsApplication::authManager()->authDatabaseConnection() };
    Q_NOWARN_DEPRECATED_POP

    mIsReadOnly = !QgsApplication::authManager()->defaultDbStorage() || QgsApplication::authManager()->defaultDbStorage()->isReadOnly();
    if ( mIsReadOnly )
    {
      mConfigModel = new QSqlTableModel( this, connection );
      btnAddConfig->setEnabled( false );
      btnEditConfig->setEnabled( false );
      btnRemoveConfig->setEnabled( false );
      tableViewConfigs->setEditTriggers( QAbstractItemView::EditTrigger::NoEditTriggers );
    }
    else
    {
      mConfigModel = new QSqlTableModel( this, connection );
    }
    mConfigModel->setTable( QgsApplication::authManager()->methodConfigTableName() );

    mConfigModel->select();

    mConfigModel->setHeaderData( 0, Qt::Horizontal, tr( "ID" ) );
    mConfigModel->setHeaderData( 1, Qt::Horizontal, tr( "Name" ) );
    mConfigModel->setHeaderData( 2, Qt::Horizontal, tr( "URI" ) );
    mConfigModel->setHeaderData( 3, Qt::Horizontal, tr( "Type" ) );
    mConfigModel->setHeaderData( 4, Qt::Horizontal, tr( "Version" ) );
    mConfigModel->setHeaderData( 5, Qt::Horizontal, tr( "Config" ) );

    tableViewConfigs->setModel( mConfigModel );
    tableViewConfigs->resizeColumnsToContents();
    //    tableViewConfigs->resizeColumnToContents( 0 );
    //    tableViewConfigs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    //    tableViewConfigs->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
    //    tableViewConfigs->resizeColumnToContents( 3 );
    tableViewConfigs->hideColumn( 4 );
    tableViewConfigs->hideColumn( 5 );

    // sort by config 'name'
    tableViewConfigs->sortByColumn( 1, Qt::AscendingOrder );
    tableViewConfigs->setSortingEnabled( true );

    connect( tableViewConfigs->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsAuthConfigEditor::selectionChanged );

    if ( mRelayMessages )
    {
      connect( QgsApplication::authManager(), &QgsAuthManager::messageLog, this, &QgsAuthConfigEditor::authMessageLog );
    }

    connect( QgsApplication::authManager(), &QgsAuthManager::authDatabaseChanged, this, &QgsAuthConfigEditor::refreshTableView );

    checkSelection();

    // set up utility actions menu
    mActionImportAuthenticationConfigs = new QAction( tr( "Import Authentication Configurations from File…" ), this );
    mActionExportSelectedAuthenticationConfigs = new QAction( tr( "Export Selected Authentication Configurations to File…" ), this );
    mActionSetMasterPassword = new QAction( QStringLiteral( "Input Master Password…" ), this );
    mActionClearCachedMasterPassword = new QAction( QStringLiteral( "Clear Cached Master Password" ), this );
    mActionResetMasterPassword = new QAction( QStringLiteral( "Reset Master Password…" ), this );
    mActionClearCachedAuthConfigs = new QAction( QStringLiteral( "Clear Cached Authentication Configurations" ), this );
    mActionRemoveAuthConfigs = new QAction( QStringLiteral( "Remove all Authentication Configurations…" ), this );
    mActionEraseAuthDatabase = new QAction( QStringLiteral( "Erase Authentication Database…" ), this );

    connect( mActionExportSelectedAuthenticationConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::exportSelectedAuthenticationConfigs );
    connect( mActionSetMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::setMasterPassword );
    connect( mActionClearCachedMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::clearCachedMasterPassword );
    connect( mActionClearCachedAuthConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::clearCachedAuthenticationConfigs );

    if ( !mIsReadOnly )
    {
      connect( tableViewConfigs, &QAbstractItemView::doubleClicked, this, &QgsAuthConfigEditor::btnEditConfig_clicked );

      connect( mActionImportAuthenticationConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::importAuthenticationConfigs );
      connect( mActionResetMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::resetMasterPassword );
      connect( mActionRemoveAuthConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::removeAuthenticationConfigs );
      connect( mActionEraseAuthDatabase, &QAction::triggered, this, &QgsAuthConfigEditor::eraseAuthenticationDatabase );
    }
    else
    {
      mActionImportAuthenticationConfigs->setEnabled( false );
      mActionSetMasterPassword->setEnabled( false );
      mActionClearCachedMasterPassword->setEnabled( false );
      mActionResetMasterPassword->setEnabled( false );
      mActionClearCachedAuthConfigs->setEnabled( false );
      mActionRemoveAuthConfigs->setEnabled( false );
      mActionEraseAuthDatabase->setEnabled( false );
    }

    mAuthUtilitiesMenu = new QMenu( this );

    if ( !mIsReadOnly )
    {
      mAuthUtilitiesMenu->addAction( mActionSetMasterPassword );
      mAuthUtilitiesMenu->addAction( mActionClearCachedMasterPassword );
      mAuthUtilitiesMenu->addAction( mActionResetMasterPassword );
      mAuthUtilitiesMenu->addSeparator();
    }

    mAuthUtilitiesMenu->addAction( mActionClearCachedAuthConfigs );

    if ( !mIsReadOnly )
      mAuthUtilitiesMenu->addAction( mActionRemoveAuthConfigs );

    mAuthUtilitiesMenu->addSeparator();

    if ( !mIsReadOnly )
      mAuthUtilitiesMenu->addAction( mActionImportAuthenticationConfigs );

    mAuthUtilitiesMenu->addAction( mActionExportSelectedAuthenticationConfigs );
    mAuthUtilitiesMenu->addSeparator();

    if ( !mIsReadOnly )
      mAuthUtilitiesMenu->addAction( mActionEraseAuthDatabase );

    btnAuthUtilities->setMenu( mAuthUtilitiesMenu );
    lblAuthConfigDb->setVisible( false );
  }
}

void QgsAuthConfigEditor::importAuthenticationConfigs()
{
  QgsAuthGuiUtils::importAuthenticationConfigs( messageBar() );
}

void QgsAuthConfigEditor::exportSelectedAuthenticationConfigs()
{
  QgsAuthGuiUtils::exportSelectedAuthenticationConfigs( selectedAuthenticationConfigIds(), messageBar() );
}

void QgsAuthConfigEditor::setMasterPassword()
{
  QgsAuthGuiUtils::setMasterPassword( messageBar() );
}

void QgsAuthConfigEditor::clearCachedMasterPassword()
{
  QgsAuthGuiUtils::clearCachedMasterPassword( messageBar() );
}

void QgsAuthConfigEditor::resetMasterPassword()
{
  QgsAuthGuiUtils::resetMasterPassword( messageBar(), this );
}

void QgsAuthConfigEditor::clearCachedAuthenticationConfigs()
{
  QgsAuthGuiUtils::clearCachedAuthenticationConfigs( messageBar() );
}

void QgsAuthConfigEditor::removeAuthenticationConfigs()
{
  QgsAuthGuiUtils::removeAuthenticationConfigs( messageBar(), this );
}

void QgsAuthConfigEditor::eraseAuthenticationDatabase()
{
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), this );
}

void QgsAuthConfigEditor::authMessageLog( const QString &message, const QString &authtag, Qgis::MessageLevel level )
{
  messageBar()->pushMessage( authtag, message, level );
}

void QgsAuthConfigEditor::toggleTitleVisibility( bool visible )
{
  if ( !mDisabled )
  {
    lblAuthConfigDb->setVisible( visible );
  }
}

QStringList QgsAuthConfigEditor::selectedAuthenticationConfigIds() const
{
  QStringList ids;
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );
  for ( const QModelIndex index : selection )
  {
    ids << index.sibling( index.row(), 0 ).data().toString();
  }
  return ids;
}

void QgsAuthConfigEditor::setShowUtilitiesButton( bool show )
{
  if ( !mDisabled )
  {
    btnAuthUtilities->setVisible( show );
  }
}

void QgsAuthConfigEditor::setRelayMessages( bool relay )
{
  if ( mDisabled )
  {
    return;
  }
  if ( relay == mRelayMessages )
  {
    return;
  }

  if ( mRelayMessages )
  {
    disconnect( QgsApplication::authManager(), &QgsAuthManager::messageLog, this, &QgsAuthConfigEditor::authMessageLog );
    mRelayMessages = relay;
    return;
  }

  connect( QgsApplication::authManager(), &QgsAuthManager::messageLog, this, &QgsAuthConfigEditor::authMessageLog );
  mRelayMessages = relay;
}

void QgsAuthConfigEditor::refreshTableView()
{
  mConfigModel->select();
  tableViewConfigs->reset();
}

void QgsAuthConfigEditor::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  checkSelection();
}

void QgsAuthConfigEditor::checkSelection()
{
  if ( !mIsReadOnly )
  {
    const bool hasselection = tableViewConfigs->selectionModel()->selection().length() > 0;
    btnEditConfig->setEnabled( hasselection );
    btnRemoveConfig->setEnabled( hasselection );
  }
}

void QgsAuthConfigEditor::btnAddConfig_clicked()
{
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::btnEditConfig_clicked()
{
  const QString authcfg = selectedConfigId();

  if ( authcfg.isEmpty() )
    return;

  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this, authcfg );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::btnRemoveConfig_clicked()
{
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return;

  for ( const QModelIndex index : selection )
  {
    const QString name = index.sibling( index.row(), 1 ).data().toString();

    if ( QMessageBox::warning( this, tr( "Remove Configuration" ), tr( "Are you sure you want to remove '%1'?\n\n"
                                                                       "Operation can NOT be undone!" )
                                                                     .arg( name ),
                               QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel )
         == QMessageBox::Ok )
    {
      mConfigModel->removeRow( index.row() );
    }
  }
}

QgsMessageBar *QgsAuthConfigEditor::messageBar()
{
  return mMsgBar;
}

QString QgsAuthConfigEditor::selectedConfigId()
{
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return QString();

  const QModelIndex indx = selection.at( 0 );
  return indx.sibling( indx.row(), 0 ).data().toString();
}
