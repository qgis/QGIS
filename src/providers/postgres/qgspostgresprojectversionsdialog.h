/***************************************************************************
    qgspostgresprojectversionsdialog.h
    ---------------------
    begin                : October 2025
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
#ifndef QGSPOSTGRESPROJECTVERSIONSDIALOG_H
#define QGSPOSTGRESPROJECTVERSIONSDIALOG_H

#include "qgspostgresprojectversionsmodel.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QTreeView>

class QgsPostgresProjectVersionsDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsPostgresProjectVersionsDialog( const QString &connectionName, const QString &schema, const QString &project, QWidget *parent = nullptr );

    void accept() override;

  private:
    QgsPostgresProjectVersionsModel *mModel = nullptr;
    QTreeView *mTreeView = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSPOSTGRESPROJECTVERSIONSDIALOG_H
