/***************************************************************************
    qgsarcgisvectortileconnectiondialog.h
    ---------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARCGISVECTORTILECONNECTIONDIALOG_H
#define QGSARCGISVECTORTILECONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgsarcgisvectortileconnectiondialog.h"


class QgsArcgisVectorTileConnectionDialog : public QDialog, public Ui::QgsArcgisVectorTileConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsArcgisVectorTileConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    void accept() override;

  private slots:
    void updateOkButtonState();
};

///@endcond

#endif // QGSVECTORTILECONNECTIONDIALOG_H
