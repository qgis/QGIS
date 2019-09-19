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
#include "qgshelp.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_app.h"

class QDir;

/**
The custom projection widget is used to define the projection family, ellipsoid and parameters needed by proj4 to assemble a customized projection definition. The resulting projection will be store in an sqlite backend.
*/
class APP_EXPORT QgsCustomProjectionDialog : public QDialog, private Ui::QgsCustomProjectionDialogBase
{
    Q_OBJECT
  public:
    QgsCustomProjectionDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr );

  public slots:
    void pbnCalculate_clicked();
    void pbnAdd_clicked();
    void pbnRemove_clicked();
    void pbnCopyCRS_clicked();
    void leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void buttonBox_accepted();

  private slots:

    void updateListFromCurrentItem();

  private:

    //helper functions
    void populateList();
    bool deleteCrs( const QString &id );
    bool saveCrs( QgsCoordinateReferenceSystem parameters, const QString &name, const QString &id, bool newEntry );
    void insertProjection( const QString &projectionAcronym );
    void showHelp();

    //These two QMap store the values as they are on the database when loading
    QMap <QString, QString> mExistingCRSparameters;
    QMap <QString, QString> mExistingCRSnames;

    //These three list store the value updated with the current modifications
    QStringList mCustomCRSnames;
    QStringList mCustomCRSids;
    QStringList mCustomCRSparameters;

    //vector saving the CRS to be deleted
    QStringList mDeletedCRSs;

    //Columns in the tree widget
    enum Columns { QgisCrsNameColumn, QgisCrsIdColumn, QgisCrsParametersColumn };
};


#endif
