/***************************************************************************
    qgsvectortileconnectiondialog.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILECONNECTIONDIALOG_H
#define QGSVECTORTILECONNECTIONDIALOG_H

#include <QDialog>

#include "ui_qgsvectortileconnectiondialog.h"


struct QgsVectorTileConnection;


class QgsVectorTileConnectionDialog : public QDialog, public Ui::QgsVectorTileConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsVectorTileConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QgsVectorTileConnection &conn );

    QgsVectorTileConnection connection() const;

    void accept() override;

  private:

    QString mBaseKey;
    QString mCredentialsBaseKey;
};

#endif // QGSVECTORTILECONNECTIONDIALOG_H
