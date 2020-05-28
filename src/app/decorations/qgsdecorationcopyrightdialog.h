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

#ifndef QGSCOPYRIGHTLABELPLUGINGUI_H
#define QGSCOPYRIGHTLABELPLUGINGUI_H

#include "ui_qgsdecorationcopyrightdialog.h"
#include "qgis_app.h"

class QColor;
class QFont;

class QgsDecorationCopyright;

class APP_EXPORT QgsDecorationCopyrightDialog : public QDialog, private Ui::QgsDecorationCopyrightDialog
{
    Q_OBJECT

  public:
    QgsDecorationCopyrightDialog( QgsDecorationCopyright &deco, QWidget *parent = nullptr );

  private slots:
    void buttonBox_accepted();
    void buttonBox_rejected();
    void mInsertExpressionButton_clicked();
    void showHelp();
    void apply();

  protected:
    QgsDecorationCopyright &mDeco;
};

#endif
