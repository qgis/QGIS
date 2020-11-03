/***************************************************************************
  qgspdalsourceselect.cpp
  -----------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
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

#include "qgspdalsourceselect.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

QgsPdalSourceSelect::QgsPdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open PDAL Supported Point Cloud Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileMeshFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mPath = path;
    emit enableButtons( ! mPath.isEmpty() );
  } );
}

void QgsPdalSourceSelect::addButtonClicked()
{
  if ( mPath.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "Add mesh layer" ),
                              tr( "No layers selected." ) );
    return;
  }

  for ( const QString &path : QgsFileWidget::splitFilePaths( mPath ) )
  {
    emit addMeshLayer( path, QFileInfo( path ).baseName(), QStringLiteral( "mdal" ) );
  }
}
