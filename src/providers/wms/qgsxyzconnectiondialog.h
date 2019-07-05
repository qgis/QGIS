/***************************************************************************
    qgsxyzconnectiondialog.h
    ---------------------
    begin                : February 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSXYZCONNECTIONDIALOG_H
#define QGSXYZCONNECTIONDIALOG_H

#include <QDialog>

#include "ui_qgsxyzconnectiondialog.h"


struct QgsXyzConnection;


class QgsXyzConnectionDialog : public QDialog, public Ui::QgsXyzConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsXyzConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QgsXyzConnection &conn );

    QgsXyzConnection connection() const;

    void accept() override;

  private:

    QString mBaseKey;
    QString mCredentialsBaseKey;
};

#endif // QGSXYZCONNECTIONDIALOG_H
