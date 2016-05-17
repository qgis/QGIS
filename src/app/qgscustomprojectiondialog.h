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
#include "qgscoordinatereferencesystem.h"

class QDir;

/**
The custom projection widget is used to define the projection family, ellipsoid and paremters needed by proj4 to assemble a customised projection definition. The resulting projection will be store in an sqlite backend.

@author Tim Sutton
*/
class APP_EXPORT QgsCustomProjectionDialog : public QDialog, private Ui::QgsCustomProjectionDialogBase
{
    Q_OBJECT
  public:
    QgsCustomProjectionDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr );
    ~QgsCustomProjectionDialog();

  public slots:
    void on_pbnCalculate_clicked();
    void on_pbnAdd_clicked();
    void on_pbnRemove_clicked();
    void on_pbnCopyCRS_clicked();
    void on_leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_buttonBox_accepted();

  private:

    //helper functions
    void populateList();
    QString quotedValue( QString value );
    bool deleteCRS( const QString& id );
    bool saveCRS( QgsCoordinateReferenceSystem myParameters, const QString& myName, QString myId, bool newEntry );
    void insertProjection( const QString& myProjectionAcronym );

    //These two QMap store the values as they are on the database when loading
    QMap <QString, QString> existingCRSparameters;
    QMap <QString, QString> existingCRSnames;

    //These three list store the value updated with the current modifications
    QStringList customCRSnames;
    QStringList customCRSids;
    QStringList customCRSparameters;

    //vector saving the CRS to be deleted
    QStringList deletedCRSs;

    //Columns in the tree widget
    enum columns { QGIS_CRS_NAME_COLUMN, QGIS_CRS_ID_COLUMN, QGIS_CRS_PARAMETERS_COLUMN };
};


#endif
