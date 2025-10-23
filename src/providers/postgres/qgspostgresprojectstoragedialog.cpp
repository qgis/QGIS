/***************************************************************************
    qgspostgresprojectstoragedialog.cpp
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgresprojectstoragedialog.h"
#include "moc_qgspostgresprojectstoragedialog.cpp"

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"
#include "qgspostgresutils.h"

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsguiutils.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

QgsPostgresProjectStorageDialog::QgsPostgresProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent )
  , mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsPostgresProjectStorageDialog::onOK );

  QPushButton *btnManageProjects = new QPushButton( tr( "Manage Projects" ), this );
  QMenu *menuManageProjects = new QMenu( btnManageProjects );
  mActionRemoveProject = menuManageProjects->addAction( tr( "Remove Project" ) );
  connect( mActionRemoveProject, &QAction::triggered, this, &QgsPostgresProjectStorageDialog::removeProject );
  btnManageProjects->setMenu( menuManageProjects );
  buttonBox->addButton( btnManageProjects, QDialogButtonBox::ActionRole );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to PostgreSQL" ) );
    mCboProject->setEditable( true );
    mGroupBoxVersions->setVisible( false );
  }
  else
  {
    setWindowTitle( tr( "Load project from PostgreSQL" ) );
    mLabelProjectVersions->setVisible( false );
    mEnableProjectVersions->setVisible( false );

    mGroupBoxVersions->setCollapsed( true );

    mVersionsWidget->setColumnCount( 3 );
    mVersionsWidget->setHorizontalHeaderLabels( QStringList() << tr( "Modified Time" ) << tr( "Modified User" ) << tr( "Comment" ) );
    mVersionsWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
    mVersionsWidget->setSelectionMode( QAbstractItemView::SingleSelection );
    mVersionsWidget->horizontalHeader()->setStretchLastSection( true );
  }

  connect( mCboConnection, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateSchemas );

  connect( mCboSchema, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::onSchemaChanged );

  connect( mEnableProjectVersions, &QCheckBox::clicked, this, &QgsPostgresProjectStorageDialog::setupQgisProjectVersioning );

  mLblProjectsNotAllowed->setVisible( false );

  // populate connections
  mCboConnection->addItems( QgsPostgresConn::connectionList() );

  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );
  populateProjects();

  connect( mCboSchema, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateProjects );
  connect( mCboProject, &QComboBox::currentTextChanged, this, &QgsPostgresProjectStorageDialog::projectChanged );

  projectChanged();
}

QString QgsPostgresProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}

QString QgsPostgresProjectStorageDialog::schemaName() const
{
  return mCboSchema->currentText();
}

QString QgsPostgresProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}

void QgsPostgresProjectStorageDialog::populateSchemas()
{
  mCboSchema->clear();
  mCboProject->clear();

  QString name = mCboConnection->currentText();
  QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

  bool projectsAllowed = QgsPostgresConn::allowProjectsInDatabase( name );
  mLblProjectsNotAllowed->setVisible( !projectsAllowed );
  if ( !projectsAllowed )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( uri, false ) );
  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + QgsPostgresConn::connectionInfo( uri, false ) );
    return;
  }

  QList<QgsPostgresSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsPostgresConnPool::instance()->releaseConnection( conn );

  QApplication::restoreOverrideCursor();

  if ( !ok )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    return;
  }

  for ( const QgsPostgresSchemaProperty &schema : std::as_const( schemas ) )
  {
    mCboSchema->addItem( schema.name );
  }

  projectChanged();
}

void QgsPostgresProjectStorageDialog::populateProjects()
{
  mCboProject->clear();
  mExistingProjects.clear();

  QString uri = currentProjectUri();
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "postgresql" ) );
  Q_ASSERT( storage );
  mExistingProjects = storage->listProjects( uri );
  mCboProject->addItems( mExistingProjects );
  projectChanged();
}

void QgsPostgresProjectStorageDialog::onOK()
{
  // check that the fields are filled in
  if ( mCboProject->currentText().isEmpty() )
    return;

  if ( mSaving )
  {
    if ( mExistingProjects.contains( mCboProject->currentText() ) )
    {
      int res = QMessageBox::question( this, tr( "Overwrite project" ), tr( "A project with the same name already exists. Would you like to overwrite it?" ), QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
        return;
    }
  }

  accept();
}

void QgsPostgresProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mExistingProjects.contains( mCboProject->currentText() ) );

  if ( !mCboProject->currentText().isEmpty() )
  {
    QgsTemporaryCursorOverride override( Qt::WaitCursor );

    QString name = mCboConnection->currentText();
    QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

    QgsPostgresConn *conn = QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), false );
    if ( !conn )
    {
      QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + QgsPostgresConn::connectionInfo( uri, false ) );
      return;
    }

    bool versioningActive = QgsPostgresUtils::qgisProjectVersioningActive( conn, mCboSchema->currentText() );

    if ( versioningActive )
    {
      const QString sqlVersions = QStringLiteral( "SELECT date_saved, (metadata->>'last_modified_user'), comment "
                                                  "FROM %1.qgis_projects_versions WHERE name = %2 ORDER BY (metadata->>'last_modified_time')::TIMESTAMP DESC" )
                                    .arg( QgsPostgresConn::quotedIdentifier( mCboSchema->currentText() ), QgsPostgresConn::quotedValue( mCboProject->currentText() ) );
      QgsPostgresResult resultVersions( conn->PQexec( sqlVersions ) );

      mVersionsWidget->clearContents();
      mVersionsWidget->setRowCount( resultVersions.PQntuples() );

      for ( int i = 0; i < resultVersions.PQntuples(); ++i )
      {
        mVersionsWidget->setItem( i, 0, new QTableWidgetItem( resultVersions.PQgetvalue( i, 0 ) ) );
        mVersionsWidget->setItem( i, 1, new QTableWidgetItem( resultVersions.PQgetvalue( i, 1 ) ) );
        mVersionsWidget->setItem( i, 2, new QTableWidgetItem( resultVersions.PQgetvalue( i, 2 ) ) );
      }

      mVersionsWidget->resizeColumnsToContents();
    }
  }
}

void QgsPostgresProjectStorageDialog::removeProject()
{
  int res = QMessageBox::question( this, tr( "Remove project" ), tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ), QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "postgresql" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsPostgresProjectStorageDialog::currentProjectUri( bool schemaOnly )
{
  QgsPostgresProjectUri postUri;
  postUri.connInfo = QgsPostgresConn::connUri( mCboConnection->currentText() );
  postUri.schemaName = mCboSchema->currentText();
  if ( !schemaOnly )
    postUri.projectName = mCboProject->currentText();

  if ( !mVersionsWidget->selectedItems().isEmpty() )
  {
    postUri.isVersion = true;
    int selectedRow = mVersionsWidget->selectedItems().first()->row();
    postUri.dateSaved = mVersionsWidget->item( selectedRow, 0 )->data( Qt::DisplayRole ).toString();
  }

  return QgsPostgresProjectStorage::encodeUri( postUri );
}

void QgsPostgresProjectStorageDialog::onSchemaChanged()
{
  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  QString name = mCboConnection->currentText();
  QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), false );
  if ( !conn )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + QgsPostgresConn::connectionInfo( uri, false ) );
    return;
  }

  bool versioning = QgsPostgresUtils::qgisProjectVersioningActive( conn, mCboSchema->currentText() );

  QgsSignalBlocker( mEnableProjectVersions )->setChecked( versioning );
  mEnableProjectVersions->setEnabled( !versioning );

  mGroupBoxVersions->setEnabled( versioning );
}

void QgsPostgresProjectStorageDialog::setupQgisProjectVersioning()
{
  if ( mEnableProjectVersions->isChecked() )
  {
    QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Enable versioning of QGIS projects" ), tr( "Do you want to enable versioning of QGIS projects in this schema?\nThis will create new table in the schema and store older versions of QGIS projects there." ) );

    if ( result == QMessageBox::StandardButton::Yes )
    {
      QgsTemporaryCursorOverride override( Qt::WaitCursor );

      QString name = mCboConnection->currentText();
      QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

      QgsPostgresConn *conn = QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), false );
      if ( !conn )
      {
        QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + QgsPostgresConn::connectionInfo( uri, false ) );
        return;
      }

      if ( !QgsPostgresUtils::projectsTableExists( conn, mCboSchema->currentText() ) )
      {
        if ( !QgsPostgresUtils::createProjectsTable( conn, mCboSchema->currentText() ) )
        {
          QMessageBox::critical( this, tr( "Error" ), tr( "Could not create qgis_projects table." ) );
          return;
        }
      }

      bool success = QgsPostgresUtils::setupQgisProjectVersioning( conn, mCboSchema->currentText() );

      if ( !success )
      {
        QMessageBox::critical( this, tr( "Error" ), tr( "Could not setup QGIS project versioning." ) );
        return;
      }

      mEnableProjectVersions->setEnabled( false );
    }
    else
    {
      QgsSignalBlocker( mEnableProjectVersions )->setChecked( false );
    }
  }
}
