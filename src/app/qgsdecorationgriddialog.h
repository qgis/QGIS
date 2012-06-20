/***************************************************************************
                         qgsdecorationgriddialog.h
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONGRIDDIALOG_H
#define QGSDECORATIONGRIDDIALOG_H

#include "ui_qgsdecorationgriddialog.h"
#include <QDialog>

class QgsDecorationGrid;
class QgsLineSymbolV2;
class QgsMarkerSymbolV2;

/**
@author Etienne Tourigny
*/
class QgsDecorationGridDialog : public QDialog, private Ui::QgsDecorationGridDialog
{
    Q_OBJECT

  public:
    QgsDecorationGridDialog( QgsDecorationGrid& decoGrid, QWidget* parent = 0 );
    ~QgsDecorationGridDialog();

  private slots:
    void apply();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();
    void on_mLineSymbolButton_clicked();
    void on_mMarkerSymbolButton_clicked();

    // from composer map
    /* void on_mLineColorButton_clicked(); */
    void on_mAnnotationFontButton_clicked();

  private:
    QgsDecorationGrid& mDeco;    
    QgsLineSymbolV2* mLineSymbol;
    QgsMarkerSymbolV2* mMarkerSymbol;

    void updateGuiElements();
    void updateDecoFromGui();

};

#endif
