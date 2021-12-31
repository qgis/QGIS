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

#ifndef QGS3DVIEWSMANAGER_H
#define QGS3DVIEWSMANAGER_H

#include "ui_qgs3dviewsmanager.h"

#include <QDialog>
#include <QStringListModel>
#include <QDomElement>

class Qgs3DMapCanvasDockWidget;

class Qgs3DViewsManager : public QDialog, private Ui::Qgs3DViewsManager
{
    Q_OBJECT

  public:
    explicit Qgs3DViewsManager( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    void reload();

    void set3DMapViewsDom( QMap<QString, QDomElement> &mapViews3DDom );
    void set3DMapViewsWidgets( QMap<QString, Qgs3DMapCanvasDockWidget *> &mapViews3DWidgets );
  private slots:
    void openClicked();
    void duplicateClicked();
    void removeClicked();
    void renameClicked();
  private:
    QStringListModel *mListModel = nullptr;

    QMap<QString, QDomElement> *m3DMapViewsDom = nullptr;
    QMap<QString, Qgs3DMapCanvasDockWidget *> *m3DMapViewsWidgets = nullptr;

    QString askUserForATitle( QString oldTitle, QString action, bool allowExistingTitle );
};

#endif // QGS3DVIEWSMANAGER_H
