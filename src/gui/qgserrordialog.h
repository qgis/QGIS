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
#include "qgsguiutils.h"
#include "qgserror.h"
#include "qgis_gui.h"
#include "qgis.h"

/**
 * \ingroup gui
 * \class QgsErrorDialog
 */
class GUI_EXPORT QgsErrorDialog: public QDialog, private Ui::QgsErrorDialogBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsErrorDialog
     */
    QgsErrorDialog( const QgsError &error, const QString &title, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Show dialog with error
     * \param error error
     * \param title title
     * \param parent parent object
     * \param fl widget flags
     */
    static void show( const QgsError &error, const QString &title, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  private slots:
    void mDetailPushButton_clicked();
    void mDetailCheckBox_stateChanged( int state );

  private:
    QgsError mError;
};

#endif
