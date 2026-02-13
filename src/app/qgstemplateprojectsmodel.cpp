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

#include <memory>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsziputils.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QPainter>
#include <QStandardPaths>
#include <QString>
#include <QUrl>

#include "moc_qgstemplateprojectsmodel.cpp"

using namespace Qt::StringLiterals;

QgsTemplateProjectsModel::QgsTemplateProjectsModel( QObject *parent )
  : QStandardItemModel( parent )
{
  const QStringList paths = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation );
  const QString templateDirName = QgsSettings().value( u"qgis/projectTemplateDir"_s, QString( QgsApplication::qgisSettingsDirPath() + u"project_templates"_s ) ).toString();

  for ( const QString &templatePath : paths )
  {
    const QString path = templatePath + QDir::separator() + u"project_templates"_s;
    addTemplateDirectory( path );
  }

  addTemplateDirectory( templateDirName );

  connect( &mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsTemplateProjectsModel::scanDirectory );

  setColumnCount( 1 );

  QStandardItem *emptyProjectItem = new QStandardItem();
  emptyProjectItem->setData( static_cast<int>( TemplateType::Blank ), static_cast<int>( CustomRole::TypeRole ) );
  emptyProjectItem->setData( tr( "Blank" ), static_cast<int>( CustomRole::TitleRole ) );
  connect( QgsProject::instance(), &QgsProject::crsChanged, this, [emptyProjectItem]() { emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) ); } );
  emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) );
  emptyProjectItem->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
  appendRow( emptyProjectItem );

  emptyProjectItem = new QStandardItem();
  emptyProjectItem->setData( static_cast<int>( TemplateType::Basemap ), static_cast<int>( CustomRole::TypeRole ) );
  emptyProjectItem->setData( tr( "OpenStreetMap Basemap" ), static_cast<int>( CustomRole::TitleRole ) );
  emptyProjectItem->setData( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ).userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) );
  emptyProjectItem->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
  appendRow( emptyProjectItem );
}

QHash<int, QByteArray> QgsTemplateProjectsModel::roleNames() const
{
  QHash<int, QByteArray> roles = QStandardItemModel::roleNames();
  roles[static_cast<int>( CustomRole::TypeRole )] = "Type";
  roles[static_cast<int>( CustomRole::TitleRole )] = "Title";
  roles[static_cast<int>( CustomRole::PathRole )] = "TemplatePath";
  roles[static_cast<int>( CustomRole::NativePathRole )] = "TemplateNativePath"; //#spellok
  roles[static_cast<int>( CustomRole::CrsRole )] = "Crs";
  roles[static_cast<int>( CustomRole::PreviewImagePathRole )] = "PreviewImagePath";
  return roles;
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
  const QFileInfoList files = dir.entryInfoList( QStringList() << u"*.qgs"_s << u"*.qgz"_s );

  // Remove any template from this directory
  for ( int i = rowCount() - 1; i >= 0; --i )
  {
    if ( index( i, 0 ).data( static_cast<int>( CustomRole::NativePathRole ) ).toString().startsWith( path ) )
    {
      removeRow( i );
    }
  }

  // Refill with templates from this directory
  for ( const QFileInfo &file : files )
  {
    auto item = std::make_unique<QStandardItem>( file.fileName() );
    item->setData( static_cast<int>( TemplateType::File ), static_cast<int>( CustomRole::TypeRole ) );

    const QString fileId = QCryptographicHash::hash( file.filePath().toUtf8(), QCryptographicHash::Sha224 ).toHex();

    QStringList files;
    QDir().mkpath( mTemporaryDir.filePath( fileId ) );

    QgsZipUtils::unzip( file.filePath(), mTemporaryDir.filePath( fileId ), files );

    const QString filename( mTemporaryDir.filePath( fileId ) + QDir::separator() + u"preview.png"_s );
    item->setData( QFileInfo::exists( filename ) ? QUrl::fromLocalFile( filename ) : QString(), static_cast<int>( CustomRole::PreviewImagePathRole ) );
    item->setData( file.baseName(), static_cast<int>( CustomRole::TitleRole ) );
    item->setData( file.filePath(), static_cast<int>( CustomRole::NativePathRole ) );

    item->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
    appendRow( item.release() );
  }
}
