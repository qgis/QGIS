/***************************************************************************
    qgspostgresprojectstoragedialog.h
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESPROJECTSTORAGEDIALOG_H
#define QGSPOSTGRESPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "ui_qgspostgresprojectstoragedialog.h"


class QgsPostgresProjectStorageDialog : public QDialog, private Ui::QgsPostgresProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsPostgresProjectStorageDialog( bool saving, QWidget *parent = nullptr );

    QString connectionName() const;
    QString schemaName() const;
    QString projectName() const;

    QString currentProjectUri( bool schemaOnly = false );

  signals:

  private slots:
    void populateSchemas();
    void populateProjects();
    void onOK();
    void projectChanged();
    void removeProject();

  private:

    bool mSaving = false;  //!< Whether using this dialog for loading or saving a project
    QAction *mActionRemoveProject = nullptr;
    QStringList mExistingProjects;
};

#endif // QGSPOSTGRESPROJECTSTORAGEDIALOG_H
