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
#include <QStyledItemDelegate>

class QgsMapCanvas;

class QgsRecentProjectItemsModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    struct RecentProjectData
    {
        bool operator==( const RecentProjectData &other ) const { return other.path == this->path; }
        QString path;
        QString title;
        QString previewImagePath;
        QString crs;
        mutable bool pin = false;
        mutable bool checkedExists = false;
        mutable bool exists = false;
    };

    explicit QgsRecentProjectItemsModel( QObject *parent = nullptr );

    void setRecentProjects( const QList<RecentProjectData> &recentProjects );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    void pinProject( const QModelIndex &index );
    void unpinProject( const QModelIndex &index );
    void removeProject( const QModelIndex &index );
    void recheckProject( const QModelIndex &index );
    void clear( bool clearPinned = false );

  private:
    QList<RecentProjectData> mRecentProjects;
};

#endif // QGSRECENTPROJECTITEMSMODEL_H
