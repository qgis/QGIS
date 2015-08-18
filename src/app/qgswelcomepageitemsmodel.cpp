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

#include "qgswelcomepageitemsmodel.h"

#include <QPixmap>
#include <QFile>

QgsWelcomePageItemsModel::QgsWelcomePageItemsModel( QObject* parent )
    : QAbstractListModel( parent )
{

}

void QgsWelcomePageItemsModel::setRecentProjects( const QList<RecentProjectData>& recentProjects )
{
  beginResetModel();
  mRecentProjects = recentProjects;
  endResetModel();
}


int QgsWelcomePageItemsModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return mRecentProjects.size();
}

QVariant QgsWelcomePageItemsModel::data( const QModelIndex& index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      return mRecentProjects.at( index.row() ).title;
      break;

    case Qt::DecorationRole:
    {
      QPixmap previewImage;
      previewImage.load( mRecentProjects.at( index.row() ).previewImagePath );
      return previewImage;
      break;
    }

    case Qt::ToolTipRole:
      return mRecentProjects.at( index.row() ).path;
      break;

    default:
      return QVariant();
  }
}


Qt::ItemFlags QgsWelcomePageItemsModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  const RecentProjectData& projectData = mRecentProjects.at( index.row() );

  if ( !QFile::exists(( projectData.path ) ) )
    flags &= ~Qt::ItemIsEnabled;

  return flags;

}
