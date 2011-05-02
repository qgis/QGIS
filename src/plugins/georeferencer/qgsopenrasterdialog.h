/***************************************************************************
     qgsopenrasterdialog.h
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
/* $Id$ */

#ifndef QGSOPENRASTERDIALOG_H
#define QGSOPENRASTERDIALOG_H

#include "ui_qgsopenrasterdialogbase.h"

class QgsOpenRasterDialog : public QDialog, private Ui::QgsOpenRasterDialog
{
    Q_OBJECT
  public:
    QgsOpenRasterDialog( QWidget *parent = 0 );
    void getRasterOptions( QString &rasterFileName, QString &modifiedFileName, QString &worldFileName );

  protected:
    void changeEvent( QEvent *e );

  private slots:
    void on_tbnSelectRaster_clicked();
    void on_tbnSelectModifiedRaster_clicked();

    void on_leModifiedRasterFileName_textChanged( const QString name );

  private:
    QString generateModifiedRasterFileName();
    QString guessWorldFileName( const QString rasterFileName );

    QString mWorldFileName;
};

#endif // QGSOPENRASTERDIALOG_H
