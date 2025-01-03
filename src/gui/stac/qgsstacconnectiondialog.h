/***************************************************************************
    qgsstacconnectiondialog.h
    ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACCONNECTIONDIALOG_H
#define QGSSTACCONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgsstacconnectiondialog.h"


class QgsStacConnectionDialog : public QDialog, public Ui::QgsStacConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsStacConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    void accept() override;

  private slots:
    void updateOkButtonState();
};

///@endcond

#endif // QGSSTACCONNECTIONDIALOG_H
