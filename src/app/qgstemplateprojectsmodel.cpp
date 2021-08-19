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
#include "qgsprojectlistitemdelegate.h"
#include "qgsproject.h"

#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QPainter>

#include <memory>


QgsTemplateProjectsModel::QgsTemplateProjectsModel( QObject *parent )
  : QStandardItemModel( parent )
{
  const QStringList paths = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation );
  const QString templateDirName = QgsSettings().value( QStringLiteral( "qgis/projectTemplateDir" ),
                                  QString( QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_templates" ) ) ).toString();

  for ( const QString &templatePath : paths )
  {
    const QString path = templatePath + QDir::separator() + QStringLiteral( "project_templates" );
    addTemplateDirectory( path );
  }

  addTemplateDirectory( templateDirName );

  connect( &mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsTemplateProjectsModel::scanDirectory );

  setColumnCount( 1 );

  QStandardItem *emptyProjectItem = new QStandardItem();

  emptyProjectItem->setData( tr( "New Empty Project" ), QgsProjectListItemDelegate::TitleRole );
  connect( QgsProject::instance(), &QgsProject::crsChanged, this, [emptyProjectItem]() { emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), QgsProjectListItemDelegate::CrsRole ); } );
  emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), QgsProjectListItemDelegate::CrsRole );
  emptyProjectItem->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled ) ;
  const QSize previewSize( 250, 177 );
  QImage image( previewSize, QImage::Format_ARGB32 );
  const QgsSettings settings;
  const int myRed = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  const int myGreen = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  const int myBlue = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  image.fill( QColor( myRed, myGreen, myBlue ) );
  QPainter painter( &image );
  painter.setOpacity( 0.5 );
  const QRect rect( 20, 20, 210, 137 );
  QPen pen;
  pen.setStyle( Qt::DashLine );
  pen.setColor( Qt::gray );
  painter.setPen( pen );
  painter.drawRect( rect );
  const QgsProjectPreviewImage previewImage( image );
  emptyProjectItem->setData( previewImage.pixmap(), Qt::DecorationRole );

  appendRow( emptyProjectItem );
}

void QgsTemplateProjectsModel::addTemplateDirectory( const QString &path )
{
  if ( QDir().exists( path ) )
  {
    scanDirectory( path );
    mFileSystemWatcher.addPath( path );
  }
}

void QgsTemplateProjectsModel::scanDirectory( const QString &path )
{
  const QDir dir = QDir( path );
  const QFileInfoList files = dir.entryInfoList( QStringList() << QStringLiteral( "*.qgs" ) << QStringLiteral( "*.qgz" ) );

  // Remove any template from this directory
  for ( int i = rowCount() - 1; i >= 0; --i )
  {
    if ( index( i, 0 ).data( QgsProjectListItemDelegate::NativePathRole ).toString().startsWith( path ) )
    {
      removeRow( i );
    }
  }

  // Refill with templates from this directory
  for ( const QFileInfo &file : files )
  {
    std::unique_ptr<QStandardItem> item = std::make_unique<QStandardItem>( file.fileName() ) ;

    const QString fileId = QCryptographicHash::hash( file.filePath().toUtf8(), QCryptographicHash::Sha224 ).toHex();

    QStringList files;
    QDir().mkpath( mTemporaryDir.filePath( fileId ) );

    QgsZipUtils::unzip( file.filePath(), mTemporaryDir.filePath( fileId ), files );

    const QString filename( mTemporaryDir.filePath( fileId ) + QDir::separator() + QStringLiteral( "preview.png" ) );

    const QgsProjectPreviewImage thumbnail( filename );

    if ( !thumbnail.isNull() )
    {
      item->setData( thumbnail.pixmap(), Qt::DecorationRole );
    }
    item->setData( file.baseName(), QgsProjectListItemDelegate::TitleRole );
    item->setData( file.filePath(), QgsProjectListItemDelegate::NativePathRole );

    item->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled ) ;
    appendRow( item.release() );
  }
}
