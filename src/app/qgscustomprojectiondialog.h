/***************************************************************************
                          qgscustomprojectiondialog.h

                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Tim Sutton
    email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCUSTOMCRSDIALOG_H
#define QGSCUSTOMCRSDIALOG_H

#include "ui_qgscustomprojectiondialogbase.h"
#include "qgscontexthelp.h"

class QDir;

/**
The custom projection widget is used to define the projection family, ellipsoid and paremters needed by proj4 to assemble a customised projection definition. The resulting projection will be store in an sqlite backend.

@author Tim Sutton
*/
class QgsCustomProjectionDialog : public QDialog, private Ui::QgsCustomProjectionDialogBase
{
    Q_OBJECT
  public:
    QgsCustomProjectionDialog( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~QgsCustomProjectionDialog();
    //a recursive function to make a directory and its ancestors
  public slots:
    void on_pbnCalculate_clicked();
    void on_pbnDelete_clicked();
    //
    // Database navigation controles
    //
    long getRecordCount();
    void on_pbnFirst_clicked();
    void on_pbnPrevious_clicked();
    void on_pbnNext_clicked();
    void on_pbnLast_clicked();
    void on_pbnNew_clicked();
    void on_pbnSave_clicked();

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    //
    // Control population
    //
    /* These two methods will be deprecated
    void getProjList();
    void getEllipsoidList();
    */
    QString getProjectionFamilyName( QString theProjectionFamilyAcronym );
    QString getEllipsoidName( QString theEllipsoidAcronym );
    QString getProjectionFamilyAcronym( QString theProjectionFamilyName );
    QString getEllipsoidAcronym( QString theEllipsoidName );
  private:
    QString getProjFromParameters();
    QString getEllipseFromParameters();

    QString mCurrentRecordId;
    long mCurrentRecordLong;
    //the record previous to starting an insert operation
    //so that we can return to it if the record insert is aborted
    long mLastRecordLong;
    long mRecordCountLong;
    QString quotedValue( QString value );
};

#endif
