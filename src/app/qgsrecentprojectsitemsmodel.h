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
    /**
     * Custom model roles.
     *
     * \since QGIS 4.0
     */
    enum class CustomRole : int
    {
      TitleRole = Qt::UserRole + 1,
      PathRole,
      NativePathRole,
      ExistsRole,
      CrsRole,
      PinnedRole,
      AnonymisedNativePathRole,
      PreviewImagePathRole,
    };
    Q_ENUM( CustomRole )

    struct RecentProjectData
    {
        bool operator==( const RecentProjectData &other ) const { return other.path == this->path; }
        QString path;
        QString title;
        QString previewImagePath;
        QString crs;
        mutable bool pinned = false;
        mutable bool checkedExists = false;
        mutable bool exists = false;
    };

    explicit QgsRecentProjectItemsModel( QObject *parent = nullptr );

    void setRecentProjects( const QList<RecentProjectData> &recentProjects );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear( bool clearPinned = false );

    Q_INVOKABLE void openProject( int row );

    Q_INVOKABLE void pinProject( int row );

    Q_INVOKABLE void unpinProject( int row );

    Q_INVOKABLE void removeProject( int row );

    Q_INVOKABLE void recheckProject( int row );

  signals:
    void projectRemoved( int row );
    void projectPinned( int row );
    void projectUnpinned( int row );
    void projectsCleared( bool clearPinned );

  private:
    QList<RecentProjectData> mRecentProjects;
};

#endif // QGSRECENTPROJECTITEMSMODEL_H
