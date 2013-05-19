/***************************************************************************
                         qgserrordialog.h  -  error dialog
                             -------------------
    begin                : October 2012
    copyright            : (C) October 2012 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSERRORDIALOG_H
#define QGSERRORDIALOG_H

#include <QDialog>

#include "ui_qgserrordialogbase.h"
#include "qgisgui.h"
#include "qgserror.h"

class GUI_EXPORT QgsErrorDialog: public QDialog, private Ui::QgsErrorDialogBase
{
    Q_OBJECT
  public:
    QgsErrorDialog( const QgsError & theError, const QString & theTitle, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsErrorDialog();

    /** Show dialog with error
     * @param theError error
     * @param theTitle title
     * @param parent parent object
     * @param fl widget flags
     */
    static void show( const QgsError & theError, const QString & theTitle, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

  public slots:
    void on_mDetailPushButton_clicked();
    void on_mDetailCheckBox_stateChanged( int state );

  private:
    QgsError mError;
};

#endif
