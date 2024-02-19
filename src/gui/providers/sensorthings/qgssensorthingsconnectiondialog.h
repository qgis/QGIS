/***************************************************************************
    qgssensorthingsconnectiondialog.h
    ---------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSCONNECTIONDIALOG_H
#define QGSSENSORTHINGSCONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgssensorthingsconnectiondialog.h"


struct QgsSensorThingsConnection;

class QgsSensorThingsConnectionWidget;

class QgsSensorThingsConnectionDialog : public QDialog, public Ui::QgsSensorThingsConnectionDialog
{
    Q_OBJECT
  public:
    explicit QgsSensorThingsConnectionDialog( QWidget *parent = nullptr );

    void setConnection( const QString &name, const QString &uri );

    QString connectionUri() const;
    QString connectionName() const;

    QgsSensorThingsConnectionWidget *connectionWidget() { return mConnectionWidget; }

    void accept() override;

  private slots:
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();

  private:
    QString mBaseKey;
    QString mCredentialsBaseKey;

    QgsSensorThingsConnectionWidget *mConnectionWidget = nullptr;
};

///@endcond PRIVATE
#endif // QGSSENSORTHINGSCONNECTIONDIALOG_H
