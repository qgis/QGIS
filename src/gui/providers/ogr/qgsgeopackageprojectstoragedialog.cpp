/***************************************************************************
    qgsgeopackageprojectstoragedialog.cpp
    ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeopackageprojectstoragedialog.h"
///@cond PRIVATE

#include "qgsgeopackageprojectstorage.h"
#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsogrdbconnection.h"
#include "qgsvectorfilewriter.h"
#include "qgis.h"

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QFileInfo>
#include <QFile>

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
  mFileWidget->lineEdit()->hide();
  mFileWidget->setFilter( QgsVectorFileWriter::filterForDriver( QStringLiteral( "GPKG" ) ) );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    const QString fileName{ QFileInfo( path ).fileName() };
    if ( mCboConnection->findData( path ) == -1 )
    {
      // the call to filePath standardizes the path and prevents
      // an error when opening the file on windows
      mCboConnection->addItem( fileName, QFileInfo( path ).filePath() );
      mCboConnection->setItemData( mCboConnection->findText( fileName ), path, Qt::ItemDataRole::ToolTipRole );
    }
    mCboConnection->setCurrentIndex( mCboConnection->findText( fileName ) );
  } );

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
  const auto &connList { QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ) };
  for ( const auto &connName : connList )
  {
    const QgsOgrDbConnection conn { connName, QStringLiteral( "GPKG" ) };
    mCboConnection->addItem( connName, conn.path() );
    mCboConnection->setItemData( mCboConnection->findText( connName ), conn.path(), Qt::ItemDataRole::ToolTipRole );
  }

  connect( mCboProject, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGeoPackageProjectStorageDialog::projectChanged );
  connect( mCboProject, qOverload< const QString & >( &QComboBox::currentTextChanged ), this, [ = ]( const QString & )
  {
    mCboProject->setItemData( mCboProject->currentIndex(), false );
  } );
  connect( mCboConnection, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGeoPackageProjectStorageDialog::populateProjects );

  // If possible, set the item currently displayed database
  const QString toSelect = QgsOgrDbConnection::selectedConnection( QStringLiteral( "GPKG" ) );
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );

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

  const QString uri = currentProjectUri();
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "geopackage" ) );
  Q_ASSERT( storage );
  const auto projects { storage->listProjects( uri ) };
  for ( const auto &projectName : projects )
  {
    // Set data to true for existing projects
    mCboProject->addItem( projectName, true );
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
    if ( mCboProject->currentData().toBool() )
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

void QgsGeoPackageProjectStorageDialog::projectChanged()
{
  mActionRemoveProject->setEnabled( mCboProject->count() != 0 && mCboProject->findText( mCboProject->currentText() ) != -1 );
}

void QgsGeoPackageProjectStorageDialog::removeProject()
{
  const int res = QMessageBox::question( this, tr( "Remove project" ),
                                         tr( "Do you really want to remove the project \"%1\"?" ).arg( mCboProject->currentText() ),
                                         QMessageBox::Yes | QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "geopackage" ) );
  Q_ASSERT( storage );
  storage->removeProject( currentProjectUri() );
  populateProjects();
}

QString QgsGeoPackageProjectStorageDialog::currentProjectUri( )
{
  QgsGeoPackageProjectUri gpkgUri;
  gpkgUri.database = mCboConnection->currentData().toString();
  gpkgUri.projectName = mCboProject->currentText();
  return QgsGeoPackageProjectStorage::encodeUri( gpkgUri );
}

///@endcond
