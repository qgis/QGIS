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

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

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
  }
  else
  {
    setWindowTitle( tr( "Load project from PostgreSQL" ) );
  }

  connect( mCboConnection, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateSchemas );

  mLblProjectsNotAllowed->setVisible( false );

  // populate connections
  mCboConnection->addItems( QgsPostgresConn::connectionList() );

  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );
  populateProjects();

  connect( mCboSchema, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateProjects );
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

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + uri.connectionInfo( false ) );
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
      int res = QMessageBox::question( this, tr( "Overwrite project" ),
                                       tr( "A project with the same name already exists. Would you like to overwrite it?" ),
                                       QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
        return;
    }
  }

  accept();
}

void QgsPostgresProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mExistingProjects.contains( mCboProject->currentText() ) );
}

void QgsPostgresProjectStorageDialog::removeProject()
{
  int res = QMessageBox::question( this, tr( "Remove project" ),
                                   tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ),
                                   QMessageBox::Yes | QMessageBox::No );
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
  return QgsPostgresProjectStorage::encodeUri( postUri );
}
