/***************************************************************************
  qgsgpxsourceselect.cpp
  -------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpxsourceselect.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

#include <QMessageBox>


QgsGpxSourceSelect::QgsGpxSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open GPX Dataset" ) );
  mFileWidget->setFilter( QStringLiteral( "%1 (*.gpx *.GPX)" ).arg( tr( "GPX files" ) ) );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mGpxPath = path;
    emit enableButtons( !mGpxPath.isEmpty() );
  } );

  connect( mFileWidget, &QgsFileWidget::fileChanged,
           this, &QgsGpxSourceSelect::enableRelevantControls );
}

void QgsGpxSourceSelect::addButtonClicked()
{
  if ( mGpxPath.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "Add GPX Layer" ),
                              tr( "No layers selected." ) );
    return;
  }

  const QFileInfo fileInfo( mGpxPath );
  if ( !fileInfo.isReadable() )
  {
    QMessageBox::warning( nullptr, tr( "Add GPX Layer" ),
                          tr( "Unable to read the selected file.\n"
                              "Please select a valid file." ) );
    return;
  }

  if ( cbGPXTracks->isChecked() )
    emit addVectorLayer( mGpxPath + "?type=track",
                         fileInfo.baseName() + ", tracks", QStringLiteral( "gpx" ) );
  if ( cbGPXRoutes->isChecked() )
    emit addVectorLayer( mGpxPath + "?type=route",
                         fileInfo.baseName() + ", routes", QStringLiteral( "gpx" ) );
  if ( cbGPXWaypoints->isChecked() )
    emit addVectorLayer( mGpxPath + "?type=waypoint",
                         fileInfo.baseName() + ", waypoints", QStringLiteral( "gpx" ) );
}

void QgsGpxSourceSelect::enableRelevantControls()
{
  const bool enabled = !mFileWidget->filePath().isEmpty();
  cbGPXWaypoints->setEnabled( enabled );
  cbGPXRoutes->setEnabled( enabled );
  cbGPXTracks->setEnabled( enabled );
  cbGPXWaypoints->setChecked( enabled );
  cbGPXRoutes->setChecked( enabled );
  cbGPXTracks->setChecked( enabled );
}
