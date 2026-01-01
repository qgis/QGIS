/***************************************************************************

               ----------------------------------------------------
              date                 : 17.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrecentprojectsitemsmodel.h"

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsfocuskeeper.h"
#include "qgsgui.h"
#include "qgsmessagelog.h"
#include "qgsnative.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextDocument>
#include <QUrl>

#include "moc_qgsrecentprojectsitemsmodel.cpp"

using namespace Qt::StringLiterals;

QgsRecentProjectItemsModel::QgsRecentProjectItemsModel( QObject *parent )
  : QAbstractListModel( parent )
{
}

void QgsRecentProjectItemsModel::setRecentProjects( const QList<RecentProjectData> &recentProjects )
{
  beginResetModel();
  mRecentProjects = recentProjects;
  endResetModel();
}


int QgsRecentProjectItemsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mRecentProjects.size();
}

QVariant QgsRecentProjectItemsModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case static_cast< int >( CustomRole::TitleRole ):
    {
      return mRecentProjects.at( index.row() ).title != mRecentProjects.at( index.row() ).path ? mRecentProjects.at( index.row() ).title : QFileInfo( mRecentProjects.at( index.row() ).path ).completeBaseName();
    }

    case static_cast< int >( CustomRole::PathRole ):
    {
      return mRecentProjects.at( index.row() ).path;
    }

    case static_cast< int >( CustomRole::NativePathRole ):
    {
      const QString path = mRecentProjects.at( index.row() ).path;
      QString filePath;
      QgsProjectStorage *projectStorage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( mRecentProjects.at( index.row() ).path );
      if ( projectStorage )
      {
        filePath = projectStorage->filePath( path );
      }
      return QDir::toNativeSeparators( !filePath.isEmpty() ? filePath : path );
    }

    case static_cast< int >( CustomRole::ExistsRole ):
    {
      return static_cast<bool>( flags( index ) & Qt::ItemIsEnabled );
    }

    case static_cast< int >( CustomRole::CrsRole ):
    {
      if ( !mRecentProjects.at( index.row() ).crs.isEmpty() )
      {
        const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mRecentProjects.at( index.row() ).crs );
        return u"%1 (%2)"_s.arg( mRecentProjects.at( index.row() ).crs, crs.userFriendlyIdentifier() );
      }
      return QString();
    }

    case static_cast< int >( CustomRole::PinnedRole ):
    {
      return mRecentProjects.at( index.row() ).pinned;
    }

    case static_cast< int >( CustomRole::PreviewImagePathRole ):
    {
      const QString imagePath = mRecentProjects.at( index.row() ).previewImagePath;
      if ( !imagePath.isEmpty() && QFile::exists( imagePath ) )
      {
        return QUrl::fromLocalFile( imagePath );
      }
      return QVariant();
    }

    case Qt::ToolTipRole:
    case static_cast< int >( CustomRole::AnonymisedNativePathRole ):
    {
      QString path = mRecentProjects.at( index.row() ).path;
      QString filePath;
      QgsProjectStorage *projectStorage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( path );
      if ( projectStorage )
      {
        path = QgsDataSourceUri::removePassword( path, true );
        filePath = projectStorage->filePath( path );
      }
      return !filePath.isEmpty() ? filePath : path;
    }

    default:
      return QVariant();
  }
}

QHash<int, QByteArray> QgsRecentProjectItemsModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[Qt::DisplayRole] = "display";
  roles[Qt::DecorationRole] = "decoration";
  roles[static_cast< int >( CustomRole::TitleRole )] = "Title";
  roles[static_cast< int >( CustomRole::PathRole )] = "ProjectPath";
  roles[static_cast< int >( CustomRole::NativePathRole )] = "ProjectNativePath";
  roles[static_cast< int >( CustomRole::ExistsRole )] = "Exists";
  roles[static_cast< int >( CustomRole::CrsRole )] = "Crs";
  roles[static_cast< int >( CustomRole::PinnedRole )] = "Pinned";
  roles[static_cast< int >( CustomRole::AnonymisedNativePathRole )] = "AnonymisedNativePath";
  roles[static_cast< int >( CustomRole::PreviewImagePathRole )] = "PreviewImagePath";
  return roles;
}

Qt::ItemFlags QgsRecentProjectItemsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || !rowCount( index.parent() ) || index.row() >= mRecentProjects.size() )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = QAbstractListModel::flags( index );

  const RecentProjectData &projectData = mRecentProjects.at( index.row() );

  // This check can be slow for network based projects, so only run it the first time
  if ( !projectData.checkedExists )
  {
    QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( projectData.path );
    if ( storage )
    {
      QString path = storage->filePath( projectData.path );
      if ( storage->type() == "geopackage"_L1 && path.isEmpty() )
        projectData.exists = false;
      }
      else
      {
        projectData.exists = true;
      }
    }
    else
    {
      projectData.exists = QFile::exists( ( projectData.path ) );
    }
    projectData.checkedExists = true;
  }

  if ( !projectData.exists )
    flags &= ~Qt::ItemIsEnabled;

  return flags;
}

void QgsRecentProjectItemsModel::openProject( int row )
{
  if ( row < 0 || row >= mRecentProjects.size() )
  {
    return;
  }

  QString path = mRecentProjects.at( row ).path;
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( path );
  if ( storage )
  {
    path = storage->filePath( path );
  }

  if ( !path.isEmpty() )
  {
    const QgsFocusKeeper focusKeeper;
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( path );
  }
}

void QgsRecentProjectItemsModel::pinProject( int row )
{
  if ( row < 0 || row >= mRecentProjects.size() )
  {
    return;
  }

  mRecentProjects.at( row ).pinned = true;
  const QModelIndex idx = index( row, 0 );
  emit dataChanged( idx, idx, QList<int>() << static_cast<int>( QgsRecentProjectItemsModel::CustomRole::PinnedRole ) );

  emit projectPinned( row );
}

void QgsRecentProjectItemsModel::unpinProject( int row )
{
  if ( row < 0 || row >= mRecentProjects.size() )
  {
    return;
  }

  mRecentProjects.at( row ).pinned = false;
  const QModelIndex idx = index( row, 0 );
  emit dataChanged( idx, idx, QList<int>() << static_cast<int>( QgsRecentProjectItemsModel::CustomRole::PinnedRole ) );

  emit projectUnpinned( row );
}

void QgsRecentProjectItemsModel::removeProject( int row )
{
  if ( row < 0 || row >= mRecentProjects.size() )
  {
    return;
  }

  beginRemoveRows( QModelIndex(), row, row );
  mRecentProjects.removeAt( row );
  endRemoveRows();

  emit projectRemoved( row );
}

void QgsRecentProjectItemsModel::clear( bool clearPinned )
{
  beginResetModel();
  if ( clearPinned )
  {
    mRecentProjects.clear();
  }
  else
  {
    mRecentProjects.erase(
      std::remove_if(
        mRecentProjects.begin(),
        mRecentProjects.end(),
        []( const QgsRecentProjectItemsModel::RecentProjectData &recentProject ) { return !recentProject.pinned; }
      ),
      mRecentProjects.end()
    );
  }
  endResetModel();

  emit projectsCleared( clearPinned );
}

void QgsRecentProjectItemsModel::recheckProject( int row )
{
  const RecentProjectData &projectData = mRecentProjects.at( row );
  const bool previousExists = projectData.exists;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( projectData.path );
  if ( storage )
  {
    QString path = storage->filePath( projectData.path );
    if ( storage->type() == "geopackage"_L1 && path.isEmpty() )
    {
      projectData.exists = false;
    }
    else
    {
      projectData.exists = true;
    }
  }
  else
  {
    projectData.exists = QFile::exists( ( projectData.path ) );
  }

  projectData.checkedExists = true;

  if ( projectData.exists != previousExists )
  {
    const QModelIndex idx = index( row, 0 );
    emit dataChanged( idx, idx, QList<int>() << static_cast<int>( QgsRecentProjectItemsModel::CustomRole::ExistsRole ) );
  }
}
