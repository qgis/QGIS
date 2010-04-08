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
/* $Id$ */
#ifndef qgsnewspatialitelayerdialog_H
#define qgsnewspatialitelayerdialog_H

#include "ui_qgsnewspatialitelayerdialogbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgis.h"

extern "C"
{
#include <sqlite3.h>
}

class QgsNewSpatialiteLayerDialog: public QDialog, private Ui::QgsNewSpatialiteLayerDialogBase
{
    Q_OBJECT

  public:
    QgsNewSpatialiteLayerDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsNewSpatialiteLayerDialog();
    /**Returns the selected geometry type*/
    QString selectedType() const;
    /**Appends the chosen attribute names and types to at*/
    //void attributes( std::list<std::pair<QString, QString> >& at ) const;
    QList<QStringList> * attributes() const;
    /**Returns the database name */
    QString databaseName() const;
    /**Returns the layer name to be created */
    QString layerName() const;
    /**Returns the geometry column name */
    QString geometryColumn() const;
    /**Returns the selected crs id*/
    QString selectedCrsId() const;
    /**Returns the state of the primary key checkbox*/
    bool includePrimaryKey() const;
    /** Create a new database */
    bool createDb();

  protected slots:
    void on_mAddAttributeButton_clicked();
    void on_mRemoveAttributeButton_clicked();
    void on_mTypeBox_currentIndexChanged( int index );
    void on_pbnFindSRID_clicked();
    void on_leLayerName_textChanged( QString text );
    void createNewDb();

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QPushButton *mOkButton;
    int mCrsId;
    sqlite3 *db;
    bool needNewDb;
};

#endif //qgsnewvectorlayerdialog_H
