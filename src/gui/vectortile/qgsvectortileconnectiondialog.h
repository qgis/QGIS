/***************************************************************************
    qgsvectortileconnectiondialog.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILECONNECTIONDIALOG_H
#define QGSVECTORTILECONNECTIONDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgsvectortileconnectiondialog.h"


class QgsVectorTileConnectionDialog : public QDialog, public Ui::QgsVectorTileConnectionDialog
{
    Q_OBJECT
  public:
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
