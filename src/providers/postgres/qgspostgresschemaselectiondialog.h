/***************************************************************************
    qgspostgresschemaselectiondialog.h
    ---------------------
    begin                : January 2025
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
#ifndef QGSPOSTGRESSCHEMASELECTIONDIALOG_H
#define QGSPOSTGRESSCHEMASELECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>

#include "qgspostgresprojectstorage.h"
#include "qgsdatasourceuri.h"

class QgsPostgresSchemaSelectionDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit QgsPostgresSchemaSelectionDialog( QgsDataSourceUri dataSourceUri, QWidget *parent = nullptr );

    QString schema();

  private slots:
    void populateSchemas();


  private:
    QDialogButtonBox *mButtonBox;
    QComboBox *mCboSchema = nullptr;
    QgsDataSourceUri mDataSourceUri;
};

#endif // QGSPOSTGRESSCHEMASELECTIONDIALOG_H
