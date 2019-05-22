/***************************************************************************

               ----------------------------------------------------
              date                 : 16.5.2019
              copyright            : (C) 2019 by Matthias Kuhn
              email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemplateprojectsmodel.h"
#include "qgsziputils.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgis.h"
#include "qgsrecentprojectsitemsmodel.h"

#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QPainter>

#include <memory>


QgsTemplateProjectsModel::QgsTemplateProjectsModel( QObject *parent )
  : QStandardItemModel( parent )
{
  const QStringList paths = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation );
  QString templateDirName = QgsSettings().value( QStringLiteral( "qgis/projectTemplateDir" ),
                            QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_templates" ) ).toString();

  for ( const QString &templatePath : paths )
  {
    const QString path = templatePath + QDir::separator() + QStringLiteral( "project_templates" );
    scanDirectory( path );
    mFileSystemWatcher.addPath( path );
  }

  scanDirectory( templateDirName );
  mFileSystemWatcher.addPath( templateDirName );

  connect( &mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsTemplateProjectsModel::scanDirectory );

  setColumnCount( 1 );

  QStandardItem *emptyProjectItem = new QStandardItem();

  emptyProjectItem->setData( tr( "New empty project" ), QgsRecentProjectItemsModel::TitleRole );
  QSize previewSize( 250, 177 );
  QImage image( previewSize, QImage::Format_ARGB32 );
  image.fill( Qt::white );
  QgsProjectPreviewImage previewImage( image );
  emptyProjectItem->setData( previewImage.pixmap(), Qt::DecorationRole );

  appendRow( emptyProjectItem );
}

void QgsTemplateProjectsModel::scanDirectory( const QString &path )
{
  QDir dir = QDir( path );
  const QFileInfoList files = dir.entryInfoList( QStringList() << QStringLiteral( "*.qgs" ) << QStringLiteral( "*.qgz" ) );

  // Remove any template from this directory)
  for ( int i = rowCount() - 1; i >= 0; --i )
  {
    if ( index( i, 0 ).data( QgsRecentProjectItemsModel::NativePathRole ).toString().startsWith( path ) )
    {
      removeRow( i );
    }
  }

  // Refill with templates from this directory
  for ( const QFileInfo &file : files )
  {
    std::unique_ptr<QStandardItem> item = qgis::make_unique<QStandardItem>( file.fileName() ) ;

    const QString fileId = QCryptographicHash::hash( file.filePath().toUtf8(), QCryptographicHash::Sha224 ).toHex();

    QStringList files;
    QDir().mkpath( mTemporaryDir.filePath( fileId ) );

    QgsZipUtils::unzip( file.filePath(), mTemporaryDir.filePath( fileId ), files );

    QString filename( mTemporaryDir.filePath( fileId ) + QDir::separator() + QStringLiteral( "preview.png" ) );

    QgsProjectPreviewImage thumbnail( filename );

    if ( !thumbnail.isNull() )
    {
      item->setData( thumbnail.pixmap(), Qt::DecorationRole );
    }
    item->setData( file.baseName(), QgsRecentProjectItemsModel::TitleRole );
    item->setData( file.filePath(), QgsRecentProjectItemsModel::NativePathRole );

    item->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled ) ;
    appendRow( item.release() );
  }
}
