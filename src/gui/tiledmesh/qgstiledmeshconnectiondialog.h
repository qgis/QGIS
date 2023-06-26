/***************************************************************************
    qgstiledmeshconnectiondialog.h
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHCONNECTIONDIALOG_H
#define QGSTILEDMESHCONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgstiledmeshconnectiondialog.h"


class QgsTiledMeshConnectionDialog : public QDialog, public Ui::QgsTiledMeshConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsTiledMeshConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    void accept() override;

  private slots:
    void updateOkButtonState();

};

///@endcond

#endif // QGSTILEDMESHCONNECTIONDIALOG_H
