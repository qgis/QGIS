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
  QFileInfo baseName( mRasterPath );
  emit addRasterLayer( mRasterPath, baseName.baseName(), QStringLiteral( "gdal" ) );
}

QGISEXTERN QgsGdalSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsGdalSourceSelect( parent, fl, widgetMode );
}
