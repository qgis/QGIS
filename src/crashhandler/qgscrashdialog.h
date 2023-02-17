/***************************************************************************
                qgscrashdialog.h - QgsCrashDialog

 ---------------------
 begin                : 11.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCRASHDIALOG_H
#define QGSCRASHDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QPushButton>

#include "qgscrashreport.h"

#include "ui_qgscrashdialog.h"

/**
 * A dialog to show a nicer crash dialog to the user.
 */
class QgsCrashDialog : public QDialog, private Ui::QgsCrashDialog
{
    Q_OBJECT
  public:

    /**
     * A dialog to show a nicer crash dialog to the user.
     */
    QgsCrashDialog( QWidget *parent = nullptr );

    void setBugReport( const QString &reportData );
    void setReloadArgs( const QString &reloadArgs );

    void setPythonFault( const QgsCrashReport::PythonFault &fault );

  private slots:
    void showReportWidget();
    void createBugReport();
    void reloadQGIS();
    void userFeedbackText_textChanged();

  private:
    static QStringList splitCommand( const QString &command );

    QString mReportData;
    QString mReloadArgs;
    QgsCrashReport::PythonFault mPythonFault;
};

#endif // QGSCRASHDIALOG_H
