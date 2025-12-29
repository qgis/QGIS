/***************************************************************************
                             qgswelcomescreen.h
                             -------------------
    begin                : December 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWELCOMESCREEN_H
#define QGSWELCOMESCREEN_H

#include "qgsnewsfeedmodel.h"
#include "qgsnewsfeedparser.h"
#include "qgsrecentprojectsitemsmodel.h"
#include "qgstemplateprojectsmodel.h"

#include <QDialog>
#include <QQuickWidget>

class QgsWelcomeScreenController : public QObject
{
    Q_OBJECT

  public:
    QgsWelcomeScreenController( QObject *parent = nullptr );

    Q_INVOKABLE void openProject( const QString &path );

    Q_INVOKABLE void createProjectFromTemplate( const QString &path );
};


class QgsWelcomeScreen : public QQuickWidget
{
    Q_OBJECT

  public:
    QgsWelcomeScreen( QWidget *parent = nullptr );
    ~QgsWelcomeScreen() = default;

    /**
     * Applies the recent projects list to the recent proejcts model.
     */
    void setRecentProjects( const QList<QgsRecentProjectItemsModel::RecentProjectData> &recentProjects );

    /**
     * Returns the recent projects model.
     */
    QgsRecentProjectItemsModel *recentProjectsModel();

    /**
     * Returns the template projects model.
     */
    QgsTemplateProjectsModel *templateProjectsModel();

    /**
     * Returns the URL used for the QGIS project news feed.
     */
    static QString newsFeedUrl();

  protected:
    bool eventFilter( QObject *object, QEvent *event ) override;

  public slots:
    void removeProject( int row );
    void pinProject( int row );
    void unpinProject( int row );
    void clearRecentProjects();

  signals:
    void projectRemoved( int row );
    void projectPinned( int row );
    void projectUnpinned( int row );
    void projectsCleared( bool clearPinned );

  private:
    void refreshGeometry();

    QgsWelcomeScreenController *mWelcomeScreenController = nullptr;

    QgsRecentProjectItemsModel *mRecentProjectsModel = nullptr;
    QgsTemplateProjectsModel *mTemplateProjectsModel = nullptr;

    QgsNewsFeedParser *mNewsFeedParser = nullptr;
    QgsNewsFeedProxyModel *mNewsFeedModel = nullptr;
};

#endif // QGSWELCOMESCREEN_H
