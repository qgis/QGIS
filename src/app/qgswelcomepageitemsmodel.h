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

class QgsWelcomePageItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsWelcomePageItemDelegate( QObject * parent = 0 );
    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
};

class QgsWelcomePageItemsModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    enum Role
    {
      TitleRole = Qt::UserRole + 1,
      PathRole = Qt::UserRole + 2
    };

    struct RecentProjectData
    {
      bool operator==( const RecentProjectData& other ) { return other.path == this->path; }
      QString path;
      QString title;
      QString previewImagePath;
    };

    QgsWelcomePageItemsModel( QObject* parent = 0 );

    void setRecentProjects( const QList<RecentProjectData>& recentProjects );

    int rowCount( const QModelIndex& parent ) const override;
    QVariant data( const QModelIndex& index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex& index ) const override;

  private:
    QList<RecentProjectData> mRecentProjects;
};

#endif // QGSWELCOMEPAGEITEMSMODEL_H
