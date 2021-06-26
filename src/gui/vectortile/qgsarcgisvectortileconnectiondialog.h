/***************************************************************************
    qgsarcgisvectortileconnectiondialog.h
    ---------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
