/***************************************************************************
    qgsdamengprojectstoragedialog.cpp
    ---------------------
    begin                : April 2018
    copyright            : ( C ) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdamengprojectstoragedialog.h"

#include "qgsdamengconn.h"
#include "qgsdamengconnpool.h"
#include "qgsdamengprojectstorage.h"

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

QgsDamengProjectStorageDialog::QgsDamengProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent )
  , mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDamengProjectStorageDialog::onOK );

  QPushButton *btnManageProjects = new QPushButton( tr( "Manage Projects" ), this );
  QMenu *menuManageProjects = new QMenu( btnManageProjects );
  mActionRemoveProject = menuManageProjects->addAction( tr( "Remove Project" ) );
  connect( mActionRemoveProject, &QAction::triggered, this, &QgsDamengProjectStorageDialog::removeProject );
  btnManageProjects->setMenu( menuManageProjects );
  buttonBox->addButton( btnManageProjects, QDialogButtonBox::ActionRole );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to Dameng" ) );
    mCboProject->setEditable( true );
  }
  else
  {
    setWindowTitle( tr( "Load project from Dameng" ) );
  }

  connect( mCboConnection, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsDamengProjectStorageDialog::populateSchemas );

  mLblProjectsNotAllowed->setVisible( false );

  // populate connections
  mCboConnection->addItems( QgsDamengConn::connectionList() );

  // If possible, set the item currently displayed database
  QString toSelect = QgsDamengConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );
  populateProjects();

  connect( mCboSchema, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsDamengProjectStorageDialog::populateProjects );
  connect( mCboProject, &QComboBox::currentTextChanged, this, &QgsDamengProjectStorageDialog::projectChanged );

  projectChanged();
}

QString QgsDamengProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}

QString QgsDamengProjectStorageDialog::schemaName() const
{
  return mCboSchema->currentText();
}

QString QgsDamengProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}

void QgsDamengProjectStorageDialog::populateSchemas()
{
  mCboSchema->clear();
  mCboProject->clear();

  QString name = mCboConnection->currentText();
  QgsDataSourceUri uri = QgsDamengConn::connUri( name );

  bool projectsAllowed = QgsDamengConn::allowProjectsInDatabase( name );
  mLblProjectsNotAllowed->setVisible( !projectsAllowed );
  if ( !projectsAllowed )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + uri.connectionInfo( false ) );
    return;
  }

  QList<QgsDamengSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsDamengConnPool::instance()->releaseConnection( conn );

  QApplication::restoreOverrideCursor();

  if ( !ok )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    return;
  }

  for ( const QgsDamengSchemaProperty &schema : std::as_const( schemas ) )
  {
    mCboSchema->addItem( schema.name );
  }

  projectChanged();
}

void QgsDamengProjectStorageDialog::populateProjects()
{
  mCboProject->clear();
  mExistingProjects.clear();

  QString uri = currentProjectUri();
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "dameng" ) );
  Q_ASSERT( storage );
  mExistingProjects = storage->listProjects( uri );
  mCboProject->addItems( mExistingProjects );
  projectChanged();
}

void QgsDamengProjectStorageDialog::onOK()
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

void QgsDamengProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mExistingProjects.contains( mCboProject->currentText() ) );
}

void QgsDamengProjectStorageDialog::removeProject()
{
  int res = QMessageBox::question( this, tr( "Remove project" ), tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ), QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "dameng" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsDamengProjectStorageDialog::currentProjectUri( bool schemaOnly )
{
  QgsDamengProjectUri postUri;
  postUri.connInfo = QgsDamengConn::connUri( mCboConnection->currentText() );
  postUri.schemaName = mCboSchema->currentText();
  if ( !schemaOnly )
    postUri.projectName = mCboProject->currentText();
  return QgsDamengProjectStorage::encodeUri( postUri );
}
