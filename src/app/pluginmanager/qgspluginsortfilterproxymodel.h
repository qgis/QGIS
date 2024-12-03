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
#include <QString>

const int PLUGIN_BASE_NAME_ROLE = Qt::UserRole + 1;
const int PLUGIN_DESCRIPTION_ROLE = Qt::UserRole + 2;  // for filtering
const int PLUGIN_AUTHOR_ROLE = Qt::UserRole + 3;       // for filtering
const int PLUGIN_TAGS_ROLE = Qt::UserRole + 4;         // for filtering
const int PLUGIN_ERROR_ROLE = Qt::UserRole + 5;        // for filtering
const int PLUGIN_STATUS_ROLE = Qt::UserRole + 6;       // for filtering and sorting
const int PLUGIN_DOWNLOADS_ROLE = Qt::UserRole + 7;    // for sorting
const int PLUGIN_VOTE_ROLE = Qt::UserRole + 8;         // for sorting
const int PLUGIN_ISDEPRECATED_ROLE = Qt::UserRole + 9; // for styling
const int PLUGIN_STATUSEXP_ROLE = Qt::UserRole + 10;   // for filtering and sorting
const int PLUGIN_CREATE_DATE = Qt::UserRole + 11;      // for sorting
const int PLUGIN_UPDATE_DATE = Qt::UserRole + 12;      // for sorting
const int SPACER_ROLE = Qt::UserRole + 20;             // for sorting


/**
 * \brief Proxy model for filtering and sorting items in Plugin Manager
*/
class QgsPluginSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    explicit QgsPluginSortFilterProxyModel( QObject *parent = nullptr );

    //! (Re)configure the status filter
    void setAcceptedStatuses( const QStringList &statuses );

    //! (Re)configure the spacer filter
    void setAcceptedSpacers( const QString &spacers = QString() );

    //! Returns the number of item with status filter matching (no other filters are considered)
    int countWithCurrentStatus();

  public slots:
    void sortPluginsByName();
    void sortPluginsByDownloads();
    void sortPluginsByVote();
    void sortPluginsByStatus();
    void sortPluginsByDateCreated();
    void sortPluginsByDateUpdated();

  protected:
    //! Filter by status: this method is used in both filterAcceptsRow and countWithCurrentStatus.
    bool filterByStatus( QModelIndex &index ) const;

    //! Filter by phrase: this method is used in filterAcceptsRow.
    bool filterByPhrase( QModelIndex &index ) const;

    //! The main filter method
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    //! The sort method overwritten in order to always display deprecated plugins last.
    bool lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const override;

  private:
    QStringList mAcceptedStatuses;
    QString mAcceptedSpacers;
};

// clazy:excludeall=qstring-allocations

#endif
