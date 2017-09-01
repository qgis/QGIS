/***************************************************************************
     qgsgeorefconfigdialog.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOREFCONFIGDIALOG_H
#define QGSGEOREFCONFIGDIALOG_H

#include "ui_qgsgeorefconfigdialogbase.h"

class QgsGeorefConfigDialog : public QDialog, private Ui::QgsGeorefConfigDialogBase
{
    Q_OBJECT
  public:
    explicit QgsGeorefConfigDialog( QWidget *parent = nullptr );
    ~QgsGeorefConfigDialog();

  protected:
    void changeEvent( QEvent *e ) override;

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

  private:
    void readSettings();
    void writeSettings();
};

#endif // QGSGEOREFCONFIGDIALOG_H
