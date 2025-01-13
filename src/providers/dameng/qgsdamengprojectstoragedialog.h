/***************************************************************************
    qgsdamengprojectstoragedialog.h
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGPROJECTSTORAGEDIALOG_H
#define QGSDAMENGPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "./ui_include/ui_qgsdamengprojectstoragedialog.h"


class QgsDamengProjectStorageDialog : public QDialog, private Ui::QgsDamengProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsDamengProjectStorageDialog( bool saving, QWidget *parent = nullptr );

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

#endif // QGSDAMENGPROJECTSTORAGEDIALOG_H
