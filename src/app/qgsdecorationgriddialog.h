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
#include "qgis_app.h"

class QgsDecorationGrid;
class QgsLineSymbol;
class QgsMarkerSymbol;

class APP_EXPORT QgsDecorationGridDialog : public QDialog, private Ui::QgsDecorationGridDialog
{
    Q_OBJECT

  public:
    QgsDecorationGridDialog( QgsDecorationGrid &decoGrid, QWidget *parent = nullptr );
    ~QgsDecorationGridDialog() override;

  private slots:
    void apply();
    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();
    void mGridTypeComboBox_currentIndexChanged( int index );
    void mPbtnUpdateFromExtents_clicked();
    void mPbtnUpdateFromLayer_clicked();

    // from composer map
    /* void on_mLineColorButton_clicked(); */
    void annotationFontChanged();

  private:
    QgsDecorationGrid &mDeco;

    void updateGuiElements();
    void updateDecoFromGui();
    void updateInterval( bool force = false );

};

#endif
