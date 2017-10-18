/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSSCALEBARPLUGINGUI_H
#define QGSSCALEBARPLUGINGUI_H

#include "ui_qgsdecorationscalebardialog.h"
#include <QDialog>
#include "qgis_app.h"

class QgsDecorationScaleBar;

class APP_EXPORT QgsDecorationScaleBarDialog : public QDialog, private Ui::QgsDecorationScaleBarDialog
{
    Q_OBJECT

  public:
    QgsDecorationScaleBarDialog( QgsDecorationScaleBar &deco, int units, QWidget *parent = nullptr );
    ~QgsDecorationScaleBarDialog();

  private slots:
    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();
    void apply();

  protected:
    QgsDecorationScaleBar &mDeco;
};

#endif
