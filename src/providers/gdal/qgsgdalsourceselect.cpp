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
  mQgsFileWidget->setFilter( QgsProviderRegistry::instance()->fileRasterFilters() );
  mQgsFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mQgsFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mRasterPath = path;
    emit enableButtons( ! mRasterPath.isEmpty() );
  } );
}

QgsGdalSourceSelect::~QgsGdalSourceSelect()
{

}

void QgsGdalSourceSelect::addButtonClicked()
{
  // Check if multiple files where selected
  if ( mRasterPath.startsWith( '"' ) )
  {
    for ( QString path : mRasterPath.split( QRegExp( "\"\\s+\"" ), QString::SkipEmptyParts ) )
    {
      QString cleanPath( path.remove( '"' ) );
      emit addRasterLayer( cleanPath, QFileInfo( cleanPath ).baseName(), QStringLiteral( "gdal" ) );
    }
  }
  else
  {
    emit addRasterLayer( mRasterPath, QFileInfo( mRasterPath ).baseName(), QStringLiteral( "gdal" ) );
  }
}

QGISEXTERN QgsGdalSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsGdalSourceSelect( parent, fl, widgetMode );
}
