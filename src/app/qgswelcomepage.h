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

#ifndef QGSWELCOMEDIALOG_H
#define QGSWELCOMEDIALOG_H

#include <QWidget>
#include <QLabel>

#include "qgswelcomepageitemsmodel.h"

class QgsWelcomePage : public QWidget
{
    Q_OBJECT

  public:
    QgsWelcomePage( QWidget* parent = 0 );

    void setRecentProjects( const QList<QgsWelcomePageItemsModel::RecentProjectData>& recentProjects );

  private slots:
    void itemActivated( const QModelIndex& index );
    void versionInfoReceived();
    void whatsNewLinkClicked( const QUrl& url );

  private:
    QgsWelcomePageItemsModel* mModel;
    QLabel* mVersionInformation;
};

#endif // QGSWELCOMEDIALOG_H
