/***************************************************************************
    qgsgdalcloudconnectiondialog.h
    ---------------------
    Date                 : June 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALCLOUDCONNECTIONDIALOG_H
#define QGSGDALCLOUDCONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>
class QgsGdalCredentialOptionsWidget;

#include "ui_qgsgdalcloudconnectiondialog.h"

class QgsGdalCloudConnectionDialog : public QDialog, public Ui::QgsGdalCloudConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsGdalCloudConnectionDialog( QWidget *parent = nullptr );

    void setVsiHandler( const QString &handler );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    void accept() override;

  private slots:
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();

  private:
    QString mHandler;

    QgsGdalCredentialOptionsWidget *mCredentialsWidget = nullptr;
};

///@endcond PRIVATE
#endif // QGSGDALCLOUDCONNECTIONDIALOG_H
