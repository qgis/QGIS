/***************************************************************************
                         qgspointcloudsourceselect.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
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

#include "qgspointcloudsourceselect.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

QgsPointCloudSourceSelect::QgsPointCloudSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open Point Cloud Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mPath = path;
    emit enableButtons( ! mPath.isEmpty() );
  } );
}

void QgsPointCloudSourceSelect::addButtonClicked()
{
  if ( mPath.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "Add Point Cloud Layers" ),
                              tr( "No layers selected." ) );
    return;
  }

  for ( const QString &path : QgsFileWidget::splitFilePaths( mPath ) )
  {
    // auto determine preferred provider for each path

    const QList< QgsProviderRegistry::ProviderCandidateDetails > preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( mPath );
    // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
    if ( preferredProviders.empty() )
      continue;

    emit addPointCloudLayer( path, QFileInfo( path ).baseName(), preferredProviders.at( 0 ).metadata()->key() ) ;
  }
}
