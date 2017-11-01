/***************************************************************************
                          qgsgdalsourceselect.h
                             -------------------
    begin                : August 05 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalsourceselect.h"
#include "qgsproviderregistry.h"

QgsGdalSourceSelect::QgsGdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );
  mFileWidget->setDialogTitle( tr( "Open GDAL Supported Raster Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileRasterFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mRasterPath = path;
    emit enableButtons( ! mRasterPath.isEmpty() );
  } );
}

void QgsGdalSourceSelect::addButtonClicked()
{
  Q_FOREACH ( const QString &path, QgsFileWidget::splitFilePaths( mRasterPath ) )
  {
    emit addRasterLayer( path, QFileInfo( path ).baseName(), QStringLiteral( "gdal" ) );
  }
}

QGISEXTERN QgsGdalSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsGdalSourceSelect( parent, fl, widgetMode );
}
