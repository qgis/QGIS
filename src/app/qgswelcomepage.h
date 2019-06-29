/***************************************************************************

               ----------------------------------------------------
              date                 : 18.8.2015
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

#ifndef QGSWELCOMEPAGE_H
#define QGSWELCOMEPAGE_H

#include <QWidget>
#include <QTextBrowser>
#include <QStandardItemModel>
#include <QFileSystemWatcher>

#include "qgsrecentprojectsitemsmodel.h"

class QgsVersionInfo;
class QListView;
class QLabel;

class QgsWelcomePage : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsWelcomePage( bool skipVersionCheck = false, QWidget *parent = nullptr );

    ~QgsWelcomePage() override;

    void setRecentProjects( const QList<QgsRecentProjectItemsModel::RecentProjectData> &recentProjects );

  signals:
    void projectRemoved( int row );
    void projectPinned( int row );
    void projectUnpinned( int row );

  private slots:
    void recentProjectItemActivated( const QModelIndex &index );
    void templateProjectItemActivated( const QModelIndex &index );
    void versionInfoReceived();
    void showContextMenuForProjects( QPoint point );
    void showContextMenuForTemplates( QPoint point );

  private:
    void updateRecentProjectsVisibility();

    QgsRecentProjectItemsModel *mRecentProjectsModel = nullptr;
    QTextBrowser *mVersionInformation = nullptr;
    QgsVersionInfo *mVersionInfo = nullptr;
    QListView *mRecentProjectsListView = nullptr;
    QLabel *mRecentProjectsTitle = nullptr;
    QListView *mTemplateProjectsListView = nullptr;
    QStandardItemModel *mTemplateProjectsModel = nullptr;
};

#endif // QGSWELCOMEPAGE_H
