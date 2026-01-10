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

#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"
#include "qgspostgresprojectversionsdialog.h"
#include "qgspostgresutils.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

#include <QMenu>
#include <QPushButton>

#include "moc_qgspostgresprojectstoragedialog.cpp"

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

  mVersionsTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mVersionsTreeView->setSelectionMode( QAbstractItemView::SingleSelection );

  mVersionsModel = new QgsPostgresProjectVersionsModel( QString(), this );
  mVersionsTreeView->setModel( mVersionsModel );

  connect( mVersionsModel, &QAbstractTableModel::modelReset, this, [this] {
    mVersionsTreeView->resizeColumnToContents( 0 );
    mVersionsTreeView->setCurrentIndex( mVersionsModel->index( 0, 0, QModelIndex() ) );
  } );

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

  mVersionsModel->setConnection( mCboConnection->currentText() );

  const QString name = mCboConnection->currentText();
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

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
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( u"postgresql"_s );
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

    mVersionsModel->populateVersions( mCboSchema->currentText(), mCboProject->currentText() );
  }
  else
  {
    mVersionsModel->clear();
  }
}

void QgsPostgresProjectStorageDialog::removeProject()
{
  int res = QMessageBox::question( this, tr( "Remove project" ), tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ), QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( u"postgresql"_s );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsPostgresProjectStorageDialog::currentProjectUri( bool schemaOnly )
{
  QgsPostgresProjectUri postUri;

  // either project is empty (schema uri is requested) or nothig from versions is selected - return simple uri
  if ( mCboProject->currentText().isEmpty() || mVersionsModel->rowCount() == 0 )
  {
    postUri.connInfo = QgsPostgresConn::connUri( mCboConnection->currentText() );
    postUri.schemaName = mCboSchema->currentText();
    if ( !schemaOnly )
      postUri.projectName = mCboProject->currentText();
  }
  else
  {
    postUri = mVersionsModel->projectUriForRow( mVersionsTreeView->currentIndex().row() );
  }

  return QgsPostgresProjectStorage::encodeUri( postUri );
}

void QgsPostgresProjectStorageDialog::onSchemaChanged()
{
  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  const QString name = mCboConnection->currentText();
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), false );

  const bool versioningEnabled = QgsPostgresUtils::qgisProjectVersioningEnabled( conn, mCboSchema->currentText() );

  conn->unref();

  QgsSignalBlocker( mEnableProjectVersions )->setChecked( versioningEnabled );
  mEnableProjectVersions->setEnabled( !versioningEnabled );

  mGroupBoxVersions->setEnabled( versioningEnabled );

  populateProjects();
}

void QgsPostgresProjectStorageDialog::setupQgisProjectVersioning()
{
  if ( mEnableProjectVersions->isChecked() )
  {
    QMessageBox::StandardButton result = QgsPostgresProjectStorageDialog::questionAllowProjectVersioning( this, mCboSchema->currentText() );

    if ( result == QMessageBox::StandardButton::Yes )
    {
      QgsTemporaryCursorOverride override( Qt::WaitCursor );

      const QString name = mCboConnection->currentText();
      const QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

      QgsPostgresConn *conn = QgsPostgresConn::connectDb( QgsPostgresConn::connectionInfo( uri, false ), false );
      if ( !conn )
      {
        QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + QgsPostgresConn::connectionInfo( uri, false ) );
        return;
      }

      if ( !QgsPostgresUtils::createProjectsTable( conn, mCboSchema->currentText() ) )
      {
        QMessageBox::critical( this, tr( "Error" ), tr( "Could not create qgis_projects table." ) );
        return;
      }

      if ( !QgsPostgresUtils::enableQgisProjectVersioning( conn, mCboSchema->currentText() ) )
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

QMessageBox::StandardButton QgsPostgresProjectStorageDialog::questionAllowProjectVersioning( QWidget *parent, const QString &schemaName )
{
  return QMessageBox::question( parent, tr( "Enable versioning of QGIS projects" ), tr( "Do you want to enable versioning of QGIS projects in the schema “%1”?\nThis will create a new table in the schema and store older versions of QGIS projects there." ).arg( schemaName ) );
}
