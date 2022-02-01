/***************************************************************************
  qgs3dviewsmanager.h
  --------------------------------------
  Date                 : December 2021
  Copyright            : (C) 2021 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DVIEWSMANAGERDIALOG_H
#define QGS3DVIEWSMANAGERDIALOG_H

#include "ui_qgs3dviewsmanagerdialog.h"

#include <QDialog>
#include <QStringListModel>
#include <QDomElement>

class Qgs3DMapCanvasDockWidget;

class Qgs3DViewsManagerDialog : public QDialog, private Ui::Qgs3DViewsManagerDialog
{
    Q_OBJECT

  public:
    explicit Qgs3DViewsManagerDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    void reload();

  private slots:
    void showClicked();
    void hideClicked();
    void duplicateClicked();
    void removeClicked();
    void renameClicked();

    void currentChanged( const QModelIndex &current, const QModelIndex &previous );

    void on3DViewsListChanged();
  private:
    QStringListModel *mListModel = nullptr;

    QString askUserForATitle( QString oldTitle, QString action, bool allowExistingTitle );
};

#endif // QGS3DVIEWSMANAGERDIALOG_H
