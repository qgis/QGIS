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

#include <QDialog>

#include "qgswelcomepageitemsmodel.h"

class QgsWelcomeDialog : public QDialog
{
    Q_OBJECT

  public:
    QgsWelcomeDialog();

    void setRecentProjects( const QList<QgsWelcomePageItemsModel::RecentProjectData>& recentProjects );

  private slots:
    void itemDoubleClicked(const QModelIndex& index );

  private:
    QgsWelcomePageItemsModel* mModel;

  public slots:
    void done( int result );
};

#endif // QGSWELCOMEDIALOG_H
