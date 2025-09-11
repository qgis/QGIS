/***************************************************************************
    qgspostgresimportprojectdialog.h
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPOSTGRESIMPORTDIALOG_H
#define QGSPOSTGRESIMPORTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QToolButton>

#include "qgsabstractdatabaseproviderconnection.h"

class QgsPostgresImportProjectDialog : public QDialog
{
  public:
    QgsPostgresImportProjectDialog( std::shared_ptr<QgsAbstractDatabaseProviderConnection> dbConn, const QString targetSchema, QWidget *parent = nullptr );

    /**
     * Returns pairs of project path and name under which it should be saved.
     */
    QList<QPair<QString, QString>> projectsToSave();

  private:
    QString prepareProjectName( const QString &fullFilePath );
    void addProject( const QString &name, const QString &path );
    QString lastUsedDir();
    QString createUniqueProjectName( const QString &projectName );
    QSet<QString> projectNamesInSchema();

    std::shared_ptr<QgsAbstractDatabaseProviderConnection> mDbConnection = nullptr;
    QString mSchemaToImportTo;
    QTableWidget *mFilesTableWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QToolButton *mButtonAdd = nullptr;
    QToolButton *mButtonRemove = nullptr;
    QSet<QString> mExistingProjectNames;
};

#endif // QGSPOSTGRESIMPORTDIALOG_H
