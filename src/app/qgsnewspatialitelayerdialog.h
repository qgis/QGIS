/***************************************************************************
                         QgsNewSpatialiteLayerDialog.h  -  description
                             -------------------
    begin                : 2010-03-19
    copyright            : (C) 2010 by Gary Sherman
    email                : gsherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWSPATIALITELAYERDIALOG_H
#define QGSNEWSPATIALITELAYERDIALOG_H

#include "ui_qgsnewspatialitelayerdialogbase.h"
#include "qgsguiutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgshelp.h"

#include "qgis.h"

extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
#include "qgis_app.h"
}

class APP_EXPORT QgsNewSpatialiteLayerDialog : public QDialog, private Ui::QgsNewSpatialiteLayerDialogBase
{
    Q_OBJECT

  public:
    QgsNewSpatialiteLayerDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, const QgsCoordinateReferenceSystem &defaultCrs = QgsCoordinateReferenceSystem() );

  protected slots:
    void mAddAttributeButton_clicked();
    void mRemoveAttributeButton_clicked();
    void mGeometryTypeBox_currentIndexChanged( int index );
    void mTypeBox_currentIndexChanged( int index );
    void pbnFindSRID_clicked();
    void nameChanged( const QString & );
    void selectionChanged();
    void checkOk();

    void buttonBox_accepted();
    void buttonBox_rejected();

  private:
    //! Returns the selected geometry type
    QString selectedType() const;
    //! Returns the selected Z dimension and/or M measurement
    QString selectedZM() const;

    //! Create a new database
    bool createDb();

    bool apply();

    void showHelp();
    void moveFieldsUp();
    void moveFieldsDown();

    static QString quotedIdentifier( QString id );

    QPushButton *mOkButton = nullptr;
    QString mCrsId;
};

#endif // QGSNEWVECTORLAYERDIALOG_H
