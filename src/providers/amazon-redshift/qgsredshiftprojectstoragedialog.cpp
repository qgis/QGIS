/***************************************************************************
   qgsredshiftprojectstoragedialog.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftprojectstoragedialog.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsredshiftconn.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshiftprojectstorage.h"

QgsRedshiftProjectStorageDialog::QgsRedshiftProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent ), mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsRedshiftProjectStorageDialog::onOK );

  QPushButton *btnManageProjects = new QPushButton( tr( "Manage Projects" ), this );
  QMenu *menuManageProjects = new QMenu( btnManageProjects );
  mActionRemoveProject = menuManageProjects->addAction( tr( "Remove Project" ) );
  connect( mActionRemoveProject, &QAction::triggered, this, &QgsRedshiftProjectStorageDialog::removeProject );
  btnManageProjects->setMenu( menuManageProjects );
  buttonBox->addButton( btnManageProjects, QDialogButtonBox::ActionRole );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to Redshift" ) );
    mCboProject->setEditable( true );
  }
  else
  {
    setWindowTitle( tr( "Load project from Redshift" ) );
  }

  connect( mCboConnection, qOverload< int >( &QComboBox::currentIndexChanged ), this,
           &QgsRedshiftProjectStorageDialog::populateSchemas );

  mLblProjectsNotAllowed->setVisible( false );

  // Populate connections.
  mCboConnection->addItems( QgsRedshiftConn::connectionList() );

  // If possible, set the item to currently displayed database.
  QString toSelect = QgsRedshiftConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );
  populateProjects();

  connect( mCboSchema, qOverload< int >( &QComboBox::currentIndexChanged ), this,
           &QgsRedshiftProjectStorageDialog::populateProjects );
  connect( mCboProject, qOverload< int >( &QComboBox::currentIndexChanged ), this,
           &QgsRedshiftProjectStorageDialog::projectChanged );

  projectChanged();
}

QString QgsRedshiftProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}

QString QgsRedshiftProjectStorageDialog::schemaName() const
{
  return mCboSchema->currentText();
}

QString QgsRedshiftProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}

void QgsRedshiftProjectStorageDialog::populateSchemas()
{
  mCboSchema->clear();
  mCboProject->clear();

  QString name = mCboConnection->currentText();
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( name );

  bool projectsAllowed = QgsRedshiftConn::allowProjectsInDatabase( name );
  mLblProjectsNotAllowed->setVisible( !projectsAllowed );
  if ( !projectsAllowed )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsRedshiftConn *conn = QgsRedshiftConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + uri.connectionInfo( false ) );
    return;
  }

  QList<QgsRedshiftSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  QApplication::restoreOverrideCursor();

  if ( !ok )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    return;
  }

  for ( const QgsRedshiftSchemaProperty &schema : std::as_const( schemas ) )
  {
    mCboSchema->addItem( schema.name );
  }

  projectChanged();
}

void QgsRedshiftProjectStorageDialog::populateProjects()
{
  mCboProject->clear();

  QString uri = currentProjectUri();
  QgsProjectStorage *storage =
    QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "redshift" ) );
  Q_ASSERT( storage );
  mCboProject->addItems( storage->listProjects( uri ) );
  projectChanged();
}

void QgsRedshiftProjectStorageDialog::onOK()
{
  // Check that the fields are filled in.
  if ( mCboProject->currentText().isEmpty() )
    return;

  if ( mSaving )
  {
    if ( mCboProject->findText( mCboProject->currentText() ) != -1 )
    {
      int res = QMessageBox::question( this, tr( "Overwrite project" ),
                                       tr( "A project with the same name already "
                                           "exists. Would you like to overwrite it?" ),
                                       QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
        return;
    }
  }

  accept();
}

void QgsRedshiftProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 &&
                                    mCboProject->findText( mCboProject->currentText() ) != -1 );
}

void QgsRedshiftProjectStorageDialog::removeProject()
{
  int res =
    QMessageBox::question( this, tr( "Remove project" ),
                           tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ),
                           QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage =
    QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "redshift" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsRedshiftProjectStorageDialog::currentProjectUri( bool schemaOnly )
{
  QgsRedshiftProjectUri postUri;
  postUri.connInfo = QgsRedshiftConn::connUri( mCboConnection->currentText() );
  postUri.schemaName = mCboSchema->currentText();
  if ( !schemaOnly )
    postUri.projectName = mCboProject->currentText();
  return QgsRedshiftProjectStorage::encodeUri( postUri );
}
