/***************************************************************************
    qgsoracleprojectstoragedialog.h
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLEPROJECTSTORAGEDIALOG_H
#define QGSORACLEPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "ui_qgsoracleprojectstoragedialog.h"


class QgsOracleProjectStorageDialog : public QDialog, private Ui::QgsOracleProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsOracleProjectStorageDialog( bool saving, QWidget *parent = nullptr );

    QString connectionName() const;
    QString schemaName() const;
    QString projectName() const;

    QString currentProjectUri( bool ownerOnly = false );

  signals:

  private slots:
    void populateOwners();
    void populateProjects();
    void onOK();
    void projectChanged();
    void removeProject();

  private:

    bool mSaving = false;  //!< Whether using this dialog for loading or saving a project
    QAction *mActionRemoveProject = nullptr;
    QStringList mExistingProjects;
};

#endif // QGSORACLEPROJECTSTORAGEDIALOG_H
