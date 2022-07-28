/***************************************************************************
    qgsoracleprojectstoragedialog.cpp
    ---------------------
    begin                : April 2022
    copyright            : (C) 2022 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsoracleprojectstoragedialog.h"

#include "qgsoracleconn.h"
#include "qgsoracleconnpool.h"
#include "qgsoracleproviderconnection.h"
#include "qgsoracleprojectstorage.h"

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

QgsOracleProjectStorageDialog::QgsOracleProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent )
  , mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsOracleProjectStorageDialog::onOK );

  QPushButton *btnManageProjects = new QPushButton( tr( "Manage Projects" ), this );
  QMenu *menuManageProjects = new QMenu( btnManageProjects );
  mActionRemoveProject = menuManageProjects->addAction( tr( "Remove Project" ) );
  connect( mActionRemoveProject, &QAction::triggered, this, &QgsOracleProjectStorageDialog::removeProject );
  btnManageProjects->setMenu( menuManageProjects );
  buttonBox->addButton( btnManageProjects, QDialogButtonBox::ActionRole );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to Oracle" ) );
    mCboProject->setEditable( true );
  }
  else
  {
    setWindowTitle( tr( "Load project from Oracle" ) );
  }

  connect( mCboConnection, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsOracleProjectStorageDialog::populateOwners );

  mLblProjectsNotAllowed->setVisible( false );

  // populate connections
  mCboConnection->addItems( QgsOracleConn::connectionList() );

  // If possible, set the item currently displayed database
  const QString toSelect = QgsOracleConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );
  populateProjects();

  connect( mCboOwner, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsOracleProjectStorageDialog::populateProjects );
  connect( mCboProject, &QComboBox::currentTextChanged, this, &QgsOracleProjectStorageDialog::projectChanged );

  projectChanged();
}

QString QgsOracleProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}

QString QgsOracleProjectStorageDialog::schemaName() const
{
  return mCboOwner->currentText();
}

QString QgsOracleProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}

void QgsOracleProjectStorageDialog::populateOwners()
{
  mCboOwner->clear();
  mCboProject->clear();

  const QString name = mCboConnection->currentText();
  const QgsDataSourceUri uri = QgsOracleConn::connUri( name );

  const bool projectsAllowed = QgsOracleConn::allowProjectsInDatabase( name );
  mLblProjectsNotAllowed->setVisible( !projectsAllowed );
  if ( !projectsAllowed )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  try
  {
    QgsOracleProviderConnection conn( uri.connectionInfo( false ), QVariantMap() );
    const QStringList schemas = conn.schemas();

    QApplication::restoreOverrideCursor();

    for ( const QString &schema : schemas )
      mCboOwner->addItem( schema );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    QApplication::restoreOverrideCursor();
    return;
  }

  projectChanged();
}

void QgsOracleProjectStorageDialog::populateProjects()
{
  mCboProject->clear();
  mExistingProjects.clear();

  const QString uri = currentProjectUri();
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "oracle" ) );
  Q_ASSERT( storage );
  mExistingProjects = storage->listProjects( uri );
  mCboProject->addItems( mExistingProjects );
  projectChanged();
}

void QgsOracleProjectStorageDialog::onOK()
{
  // check that the fields are filled in
  if ( mCboProject->currentText().isEmpty() )
    return;

  if ( mSaving )
  {
    if ( mExistingProjects.contains( mCboProject->currentText() ) )
    {
      const int res = QMessageBox::question( this, tr( "Overwrite project" ),
                                             tr( "A project with the same name already exists. Would you like to overwrite it?" ),
                                             QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
        return;
    }
  }

  accept();
}

void QgsOracleProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mExistingProjects.contains( mCboProject->currentText() ) );
}

void QgsOracleProjectStorageDialog::removeProject()
{
  const int res = QMessageBox::question( this, tr( "Remove project" ),
                                         tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ),
                                         QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "oracle" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsOracleProjectStorageDialog::currentProjectUri( bool ownerOnly )
{
  QgsOracleProjectUri projectUri;
  projectUri.connInfo = QgsOracleConn::connUri( mCboConnection->currentText() );
  projectUri.owner = mCboOwner->currentText();
  if ( !ownerOnly )
    projectUri.projectName = mCboProject->currentText();
  return QgsOracleProjectStorage::encodeUri( projectUri );
}
