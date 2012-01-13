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
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgis.h"

extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
}

class QgsNewSpatialiteLayerDialog: public QDialog, private Ui::QgsNewSpatialiteLayerDialogBase
{
    Q_OBJECT

  public:
    QgsNewSpatialiteLayerDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsNewSpatialiteLayerDialog();

  protected slots:
    void on_mAddAttributeButton_clicked();
    void on_mRemoveAttributeButton_clicked();
    void on_mTypeBox_currentIndexChanged( int index );
    void on_pbnFindSRID_clicked();
    void on_leLayerName_textChanged( QString text );
    void on_toolButtonNewDatabase_clicked();
    void nameChanged( QString );
    void selectionChanged();

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

  private:
    /**Returns the selected geometry type*/
    QString selectedType() const;

    /** Create a new database */
    bool createDb();

    bool apply();

    static QString quotedIdentifier( QString id );
    static QString quotedValue( QString value );

    QPushButton *mOkButton;
    int mCrsId;
};

#endif // QGSNEWVECTORLAYERDIALOG_H
