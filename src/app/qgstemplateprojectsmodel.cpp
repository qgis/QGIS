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
#include "qgssettingsregistrycore.h"
#include "qgsziputils.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QPainter>
#include <QString>
#include <QUrl>

#include "moc_qgstemplateprojectsmodel.cpp"

using namespace Qt::StringLiterals;

QgsTemplateProjectsModel::QgsTemplateProjectsModel( QObject *parent )
  : QStandardItemModel( parent )
{
  setColumnCount( 1 );

  const QColor canvasColor = QgsSettingsRegistryCore::settingsDefaultCanvasColor->value();

  QStandardItem *emptyProjectItem = new QStandardItem();
  emptyProjectItem->setData( false, static_cast<int>( CustomRole::WritableRole ) );
  emptyProjectItem->setData( canvasColor, static_cast<int>( CustomRole::CanvasColorRole ) );
  emptyProjectItem->setData( static_cast<int>( TemplateType::Blank ), static_cast<int>( CustomRole::TypeRole ) );
  emptyProjectItem->setData( tr( "Blank" ), static_cast<int>( CustomRole::TitleRole ) );
  connect( QgsProject::instance(), &QgsProject::crsChanged, this, [emptyProjectItem]() {
    emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) );
  } );
  emptyProjectItem->setData( QgsProject::instance()->crs().userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) );
  emptyProjectItem->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
  appendRow( emptyProjectItem );

  emptyProjectItem = new QStandardItem();
  emptyProjectItem->setData( false, static_cast<int>( CustomRole::WritableRole ) );
  emptyProjectItem->setData( canvasColor, static_cast<int>( CustomRole::CanvasColorRole ) );
  emptyProjectItem->setData( static_cast<int>( TemplateType::Basemap ), static_cast<int>( CustomRole::TypeRole ) );
  emptyProjectItem->setData( tr( "OpenStreetMap Basemap" ), static_cast<int>( CustomRole::TitleRole ) );
  emptyProjectItem->setData( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ).userFriendlyIdentifier(), static_cast<int>( CustomRole::CrsRole ) );
  emptyProjectItem->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
  appendRow( emptyProjectItem );

  connect( &mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsTemplateProjectsModel::reload );
  reload();
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
  roles[static_cast<int>( CustomRole::WritableRole )] = "Writable";
  roles[static_cast<int>( CustomRole::CanvasColorRole )] = "CanvasColor";
  roles[static_cast<int>( CustomRole::SectionRole )] = "Section";
  return roles;
}

void QgsTemplateProjectsModel::reload()
{
  // Remove file templates only
  for ( int i = rowCount() - 1; i >= 0; --i )
  {
    if ( index( i, 0 ).data( static_cast<int>( CustomRole::TypeRole ) ).toInt() == static_cast<int>( TemplateType::File ) )
    {
      removeRow( i );
    }
  }

  const QStringList watchedDirectories = mFileSystemWatcher.directories();
  if ( !watchedDirectories.isEmpty() )
    mFileSystemWatcher.removePaths( watchedDirectories );

  const QStringList templatePaths = QgsApplication::projectTemplatePaths();

  // Count how many directories have the same name (homonyms)
  QHash<QString, int> directoryNameCount;
  for ( const QString &templatePath : templatePaths )
  {
    directoryNameCount[QDir( templatePath ).dirName()]++;
  }

  // Use default canvas color when preview image is missing
  const QColor canvasColor = QgsSettingsRegistryCore::settingsDefaultCanvasColor->value();

  int row = 0;
  for ( const QString &templatePath : templatePaths )
  {
    const QDir dir( templatePath );
    if ( !dir.exists() )
      continue;

    // Section title is the directory name, unless several directories share the same name, in which case the full path is used
    const QString section = directoryNameCount.value( dir.dirName() ) > 1 ? QDir::toNativeSeparators( templatePath ) : dir.dirName();

    mFileSystemWatcher.addPath( templatePath );

    const QFileInfoList files = dir.entryInfoList( QStringList() << u"*.qgs"_s << u"*.qgz"_s );

    for ( const QFileInfo &file : files )
    {
      auto item = std::make_unique<QStandardItem>( file.fileName() );
      item->setData( file.isWritable(), static_cast<int>( CustomRole::WritableRole ) );
      item->setData( canvasColor, static_cast<int>( CustomRole::CanvasColorRole ) );
      item->setData( static_cast<int>( TemplateType::File ), static_cast<int>( CustomRole::TypeRole ) );

      const QString fileId = QCryptographicHash::hash( file.filePath().toUtf8(), QCryptographicHash::Sha224 ).toHex();

      QStringList unzippedFiles;
      QDir().mkpath( mTemporaryDir.filePath( fileId ) );

      QgsZipUtils::unzip( file.filePath(), mTemporaryDir.filePath( fileId ), unzippedFiles );

      const QString filename( mTemporaryDir.filePath( fileId ) + QDir::separator() + u"preview.png"_s );
      item->setData( QFileInfo::exists( filename ) ? QUrl::fromLocalFile( filename ) : QString(), static_cast<int>( CustomRole::PreviewImagePathRole ) );
      item->setData( file.baseName(), static_cast<int>( CustomRole::TitleRole ) );
      item->setData( file.filePath(), static_cast<int>( CustomRole::NativePathRole ) );
      item->setData( section, static_cast<int>( CustomRole::SectionRole ) );

      item->setFlags( Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled );
      insertRow( row++, item.release() );
    }
  }
}
