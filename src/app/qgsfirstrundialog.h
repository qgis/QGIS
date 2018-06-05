/***************************************************************************
  qgsfirstrundialog.h - qgsfirstrundialog

 ---------------------
 begin                : 11.12.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIRSTRUNDIALOG_H
#define QGSFIRSTRUNDIALOG_H

#include <QWidget>
#include <QDialog>
#include "qgis_app.h"

#include "ui_qgsfirstrundialog.h"

class APP_EXPORT QgsFirstRunDialog : public QDialog, private Ui::QgsFirstRunDialog
{
    Q_OBJECT
  public:
    QgsFirstRunDialog( QWidget *parent = nullptr );

    bool migrateSettings();

  signals:

  public slots:
};

#endif // QGSFIRSTRUNDIALOG_H
