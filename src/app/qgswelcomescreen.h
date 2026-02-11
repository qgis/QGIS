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
#include "qgsversioninfo.h"

#include <QDialog>
#include <QQuickWidget>

class QgsWelcomeScreen;


class QgsWelcomeScreenController : public QObject
{
    Q_OBJECT

  public:
    QgsWelcomeScreenController( QgsWelcomeScreen *welcomeScreen = nullptr );

    Q_INVOKABLE void openProject( const QString &path );

    Q_INVOKABLE void createBlankProject();
    Q_INVOKABLE void createProjectFromBasemap();
    Q_INVOKABLE void createProjectFromTemplate( const QString &path );

    Q_INVOKABLE void clearRecentProjects();

    Q_INVOKABLE void removeTemplateProject( int row );

    Q_INVOKABLE void showPluginManager();

    Q_INVOKABLE void hideScene();

  signals:
    void newVersionAvailable( const QString &versionString );
    void pluginUpdatesAvailable( const QStringList &plugins );

  private:
    QgsWelcomeScreen *mWelcomeScreen = nullptr;
};


class QgsWelcomeScreen : public QQuickWidget
{
    Q_OBJECT

  public:
    QgsWelcomeScreen( bool skipVersionCheck = false, QWidget *parent = nullptr );
    ~QgsWelcomeScreen() override = default;

    /**
     * Shows the welcome screen.
     */
    void showScene();

    /**
     * Hides the welcome screen.
     */
    void hideScene();

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

    /**
     * Register object types needed by the QML scene.
     */
    static void registerTypes();

  protected:
    bool eventFilter( QObject *object, QEvent *event ) override;

  public slots:
    void clearRecentProjects();
    void removeTemplateProject( int row );
    void pluginUpdatesAvailableReceived( const QStringList &plugins );

  signals:
    void projectRemoved( int row );
    void projectPinned( int row );
    void projectUnpinned( int row );
    void projectsCleared( bool clearPinned );

  private slots:
    void versionInfoReceived();

  private:
    void refreshGeometry();

    QgsWelcomeScreenController *mWelcomeScreenController = nullptr;

    QgsRecentProjectItemsModel *mRecentProjectsModel = nullptr;
    QgsTemplateProjectsModel *mTemplateProjectsModel = nullptr;

    QgsNewsFeedParser *mNewsFeedParser = nullptr;
    QgsNewsFeedProxyModel *mNewsFeedModel = nullptr;

    QgsVersionInfo *mVersionInfo = nullptr;

    int mOriginalWidth = 0;
    int mOriginalHeight = 0;
};

#endif // QGSWELCOMESCREEN_H
