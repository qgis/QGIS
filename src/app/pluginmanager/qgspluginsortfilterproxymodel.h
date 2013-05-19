/***************************************************************************
    qgspluginsortfilterproxymodel.h
     --------------------------------------
    Date                 : 20-May-2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLUGINSORTFILTERPROXYMODEL_H
#define QGSPLUGINSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
// #include <QString>
// #include <QMap>

const int PLUGIN_BASE_NAME_ROLE = Qt::UserRole + 1;
const int PLUGIN_DESCRIPTION_ROLE = Qt::UserRole + 2;  // for filtering
const int PLUGIN_AUTHOR_ROLE = Qt::UserRole + 3;       // for filtering
const int PLUGIN_TAGS_ROLE = Qt::UserRole + 4;         // for filtering
const int PLUGIN_ERROR_ROLE = Qt::UserRole + 6;        // for filtering
const int PLUGIN_STATUS_ROLE = Qt::UserRole + 5;       // for filtering and sorting
const int PLUGIN_DOWNLOADS_ROLE = Qt::UserRole + 7;    // for sorting
const int PLUGIN_VOTE_ROLE = Qt::UserRole + 8;         // for sorting
const int PLUGIN_REPOSITORY_ROLE = Qt::UserRole + 9;   // for sorting


/*!
 * \brief Proxy model for filtering and sorting items in Plugin Manager
*/
class QgsPluginSortFilterProxyModel : public QSortFilterProxyModel
{
     Q_OBJECT

  public:
    QgsPluginSortFilterProxyModel(QObject *parent = 0);

    //! (Re)configire the status filter
    void setAcceptedStatuses( QStringList statuses );

    //! Return number of item with status filter matching (no other filters are considered)
    int countWithCurrentStatus(  );

  public slots:
    void sortPluginsByName( );
    void sortPluginsByDownloads( );
    void sortPluginsByVote( );
    void sortPluginsByStatus( );
    void sortPluginsByRepository( );

 protected:
     //! Filter by status: this method is used in both filterAcceptsRow and countWithCurrentStatus.
     bool filterByStatus( QModelIndex &index ) const;
     //! The main filter method
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

 private:
     QStringList mAcceptedStatuses;
};

#endif
