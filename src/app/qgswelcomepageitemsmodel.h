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

#ifndef QGSWELCOMEPAGEITEMSMODEL_H
#define QGSWELCOMEPAGEITEMSMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class QgsWelcomePageItemsModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    struct RecentProjectData
    {
      bool operator==( const RecentProjectData& other ) { return other.path == this->path; }
      QString path;
      QString title;
      QString previewImagePath;
    };

    QgsWelcomePageItemsModel( QObject* parent = 0 );

    void setRecentProjects( const QList<RecentProjectData>& recentProjects );

    int rowCount( const QModelIndex& parent ) const;
    QVariant data( const QModelIndex& index, int role ) const;
    Qt::ItemFlags flags( const QModelIndex& index ) const;

  private:
    QList<RecentProjectData> mRecentProjects;
};

#endif // QGSWELCOMEPAGEITEMSMODEL_H
