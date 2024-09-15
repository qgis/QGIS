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

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgsvectortileconnectiondialog.h"

#include "qgssettingsentryenumflag.h"

class QgsVectorTileConnectionDialog : public QDialog, public Ui::QgsVectorTileConnectionDialog
{
    Q_OBJECT
  public:
    enum class LoadingMode : int
    {
      Url,
      Style
    };
    Q_ENUM( LoadingMode )

    static const QgsSettingsEntryEnumFlag<QgsVectorTileConnectionDialog::LoadingMode> *settingsLastLoadingMode;

    explicit QgsVectorTileConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    void accept() override;

  private slots:
    void updateOkButtonState();
};

///@endcond

#endif // QGSVECTORTILECONNECTIONDIALOG_H
