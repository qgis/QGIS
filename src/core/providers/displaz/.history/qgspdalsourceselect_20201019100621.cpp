/***************************************************************************
  qgspdalsourceselect.cpp
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

#include "qgspdalsourceselect.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

QgsPdalSourceSelect::QgsPdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open PDAL Supported PointCloud Dataset(s)" ) );
 // mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mPointCloudPath = path;
    emit enableButtons( ! mPointCloudPath.isEmpty() );
  } );
}

void QgsPdalSourceSelect::addButtonClicked()
{
  if ( mPointCloudPath.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "添加点云图层" ),
                              tr( "无点云数据选中." ) );
    return;
  }

  for ( const QString &path : QgsFileWidget::splitFilePaths( mPointCloudPath ) )
  {
    emit addVectorLayer( path, QFileInfo( path ).baseName(), QStringLiteral( "pdal" ) );
  }
}

 QgsPdalSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsPdalSourceSelect( parent, fl, widgetMode );
}
