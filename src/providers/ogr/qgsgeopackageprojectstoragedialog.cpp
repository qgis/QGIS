/***************************************************************************
    qgsgeopackageprojectstoragedialog.cpp
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
#include "qgsgeopackageprojectstoragedialog.h"
#include "qgsgeopackageprojectstorage.h"
#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsogrdbconnection.h"
#include "qgis.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

QgsGeoPackageProjectStorageDialog::QgsGeoPackageProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent )
  , mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsGeoPackageProjectStorageDialog::onOK );

  QPushButton *btnManageProjects = new QPushButton( tr( "Manage Projects" ), this );
  QMenu *menuManageProjects = new QMenu( btnManageProjects );
  mActionRemoveProject = menuManageProjects->addAction( tr( "Remove Project" ) );
  connect( mActionRemoveProject, &QAction::triggered, this, &QgsGeoPackageProjectStorageDialog::removeProject );
  btnManageProjects->setMenu( menuManageProjects );
  buttonBox->addButton( btnManageProjects, QDialogButtonBox::ActionRole );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to GeoPackage" ) );
    mCboProject->setEditable( true );
  }
  else
  {
    setWindowTitle( tr( "Load project from GeoPackage" ) );
  }

  // populate connections
  mCboConnection->addItems( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ) );

  // If possible, set the item currently displayed database
  QString toSelect = QgsOgrDbConnection::selectedConnection(QStringLiteral( "GPKG" ));
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );

  connect( mCboProject, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGeoPackageProjectStorageDialog::projectChanged );

  projectChanged();
}

QString QgsGeoPackageProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}


QString QgsGeoPackageProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}


void QgsGeoPackageProjectStorageDialog::populateProjects()
{
  mCboProject->clear();

  QString uri = currentProjectUri();
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "geopackage" ) );
  Q_ASSERT( storage );
  const auto projects { storage->listProjects( uri ) };
  for (const auto &projectName: projects )
  {
    // Set data to true for existing projects
    mCboProject->addItem( projectName, true);
  }
  projectChanged();
}

void QgsGeoPackageProjectStorageDialog::onOK()
{
  // check that the fields are filled in
  if ( mCboProject->currentText().isEmpty() )
    return;

  if ( mSaving )
  {
    // Check if this is an overwrite of an existing project
    if ( mCboProject->currentData( ).toBool() )
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

void QgsGeoPackageProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mCboProject->findText( mCboProject->currentText() ) != -1 );
}

void QgsGeoPackageProjectStorageDialog::removeProject()
{
  int res = QMessageBox::question( this, tr( "Remove project" ),
                                   tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ),
                                   QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "GeoPackage" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsGeoPackageProjectStorageDialog::currentProjectUri( )
{
  QgsGeoPackageProjectUri gpkgUri;
  // find path in connections
  QgsOgrDbConnection conn( mCboConnection->currentText(), QStringLiteral( "GPKG") );
  gpkgUri.database = conn.path();
  gpkgUri.projectName = mCboProject->currentText();
  return QgsGeoPackageProjectStorage::encodeUri( gpkgUri );
}
