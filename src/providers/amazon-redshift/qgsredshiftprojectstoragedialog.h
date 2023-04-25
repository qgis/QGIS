/***************************************************************************
   qgsredshiftprojectstoragedialog.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTPROJECTSTORAGEDIALOG_H
#define QGSREDSHIFTPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "ui_qgsredshiftprojectstoragedialog.h"

class QgsRedshiftProjectStorageDialog : public QDialog, private Ui::QgsRedshiftProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsRedshiftProjectStorageDialog( bool saving, QWidget *parent = nullptr );

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
    bool mSaving = false; //!< Whether using this dialog for loading or saving a project
    QAction *mActionRemoveProject = nullptr;

};

#endif // QGSREDSHIFTPROJECTSTORAGEDIALOG_H
