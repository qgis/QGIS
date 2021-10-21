/***************************************************************************
 qgsdecorationtilegriddialog.h
 -----------------------------
 Date: 24-Nov-2021
 Copyright: (C) 2021 by Jochen Topf
 Email: jochen@topf.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDECORATIONTILEGRIDDIALOG_H
#define QGSDECORATIONTILEGRIDDIALOG_H

#include "ui_qgsdecorationtilegriddialog.h"
#include <QDialog>
#include "qgis_app.h"

class QgsDecorationTileGrid;

class APP_EXPORT QgsDecorationTileGridDialog : public QDialog, private Ui::QgsDecorationTileGridDialog
{
    Q_OBJECT

  public:
    QgsDecorationTileGridDialog( QgsDecorationTileGrid &decoGrid, QWidget *parent = nullptr );

  private slots:
    void apply();
    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();
    void updateSymbolButtons();
    void updateGridButtons();

  private:
    QgsDecorationTileGrid &mDeco;

    void updateGuiElements();
    void updateDecoFromGui();

};

#endif
