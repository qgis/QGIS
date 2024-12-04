/***************************************************************************
    qgsgeopackageprojectstoragedialog.h
    ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOPACKAGEPROJECTSTORAGEDIALOG_H
#define QGSGEOPACKAGEPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "ui_qgsgeopackageprojectstoragedialog.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGeoPackageProjectStorageDialog : public QDialog, private Ui::QgsGeoPackageProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsGeoPackageProjectStorageDialog( bool saving, QWidget *parent = nullptr );

    QString connectionName() const;
    QString schemaName() const;
    QString projectName() const;

    QString currentProjectUri();

  signals:

  private slots:
    void populateProjects();
    void onOK();
    void projectChanged();
    void removeProject();

  private:
    bool mSaving = false; //!< Whether using this dialog for loading or saving a project
    QAction *mActionRemoveProject = nullptr;
};

///@endcond
#endif // QGSGEOPACKAGEPROJECTSTORAGEDIALOG_H
