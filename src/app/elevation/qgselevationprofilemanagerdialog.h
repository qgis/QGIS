/***************************************************************************
                         qgselevationprofilemanagerdialog.h
                         -----------------------
    begin                : July 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILEMANAGERDIALOG_H
#define QGSELEVATIONPROFILEMANAGERDIALOG_H

#include <QItemDelegate>
#include <QSortFilterProxyModel>

#include "ui_qgselevationprofilemanagerbase.h"

class QListWidgetItem;
class QgsElevationProfile;
class QgsElevationProfileManager;
class QgsElevationProfileManagerModel;
class QgsElevationProfileManagerProxyModel;

/**
 * A dialog that allows management of elevation profiles within a project.
*/
class QgsElevationProfileManagerDialog : public QDialog, private Ui::QgsElevationProfileManagerBase
{
    Q_OBJECT
  public:
    QgsElevationProfileManagerDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

  public slots:
    //! Raise, unminimize and activate this window
    void activate();

  private:
    bool uniqueProfileTitle( QWidget *parent, QString &title, const QString &currentTitle );

    QgsElevationProfileManagerModel *mModel = nullptr;
    QgsElevationProfileManagerProxyModel *mProxyModel = nullptr;

#ifdef Q_OS_MAC
    void showEvent( QShowEvent *event );
    void changeEvent( QEvent * );

    QAction *mWindowAction = nullptr;
#endif

  private slots:
    //! Slot to update buttons state when selecting layouts
    void toggleButtons();

    void removeClicked();
    void showClicked();
    void duplicateClicked();
    void renameClicked();
    void itemDoubleClicked( const QModelIndex &index );
};

#endif // QGSELEVATIONPROFILEMANAGERDIALOG_H
