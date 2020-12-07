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
    QgsCustomProjectionDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

  public slots:
    void pbnCalculate_clicked();
    void pbnAdd_clicked();
    void pbnRemove_clicked();
    void pbnCopyCRS_clicked();
    void leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void buttonBox_accepted();

  private slots:

    void updateListFromCurrentItem();
    void validateCurrent();
    void formatChanged();

  private:

    //helper functions
    void populateList();
    bool deleteCrs( const QString &id );
    bool saveCrs( QgsCoordinateReferenceSystem crs, const QString &name, const QString &id, bool newEntry, QgsCoordinateReferenceSystem::Format format );
    void insertProjection( const QString &projectionAcronym );
    void showHelp();
    QString multiLineWktToSingleLine( const QString &wkt );

    //These two QMap store the values as they are on the database when loading
    QMap <QString, QString> mExistingCRSproj;
    QMap <QString, QString> mExistingCRSwkt;
    QMap <QString, QString> mExistingCRSnames;

    struct Definition
    {
      QString name;
      QString id;
      QString wkt;
      QString proj;
    };

    enum Roles
    {
      FormattedWktRole = Qt::UserRole + 1,
    };

    QList< Definition > mDefinitions;

    //vector saving the CRS to be deleted
    QStringList mDeletedCRSs;

    //Columns in the tree widget
    enum Columns { QgisCrsNameColumn, QgisCrsIdColumn, QgisCrsParametersColumn };


};


#endif
