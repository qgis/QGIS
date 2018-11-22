/***************************************************************************
                          qgsdecorationlayoutextentdialog.h
                              -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDECORATIONLAYOUTEXTENTDIALOG_H
#define QGSDECORATIONLAYOUTEXTENTDIALOG_H

#include "ui_qgsdecorationlayoutextentdialog.h"
#include <QDialog>
#include "qgis_app.h"
#include "qgstextrenderer.h"
#include "qgshelp.h"
#include <memory>

class QgsDecorationLayoutExtent;
class QgsFillSymbol;

class APP_EXPORT QgsDecorationLayoutExtentDialog : public QDialog, private Ui::QgsDecorationLayoutExtentDialog
{
    Q_OBJECT

  public:
    QgsDecorationLayoutExtentDialog( QgsDecorationLayoutExtent &decoration, QWidget *parent = nullptr );

  private slots:
    void apply();
    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();


  private:
    QgsDecorationLayoutExtent &mDeco;

    void updateGuiElements();
    void updateDecoFromGui();

};

#endif // QGSDECORATIONLAYOUTEXTENTDIALOG_H
