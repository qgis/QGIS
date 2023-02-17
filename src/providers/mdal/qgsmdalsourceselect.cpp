/***************************************************************************
  qgsmdalsourceselect.cpp
  -----------------------
    begin                : July 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>

#include "qgsmdalsourceselect.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

QgsMdalSourceSelect::QgsMdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open MDAL Supported Mesh Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileMeshFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mMeshPath = path;
    emit enableButtons( ! mMeshPath.isEmpty() );
  } );
}

void QgsMdalSourceSelect::addButtonClicked()
{
  if ( mMeshPath.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "Add mesh layer" ),
                              tr( "No layers selected." ) );
    return;
  }

  for ( const QString &path : QgsFileWidget::splitFilePaths( mMeshPath ) )
  {
    emit addMeshLayer( path, QFileInfo( path ).completeBaseName(), QStringLiteral( "mdal" ) );
  }
}
