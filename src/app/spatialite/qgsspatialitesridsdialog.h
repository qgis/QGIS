/***************************************************************************
             QgsSpatialiteSridsDialog.h  -  Dialog for selecting an
                  SRID from a Spatialite spatial_ref_sys table
                             -------------------
    begin                : 2010-04-03
    copyright            : (C) 2010 by Gary Sherman
    email                : gsherman@geoapt.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QgsSpatialiteSridsDialog_H
#define QgsSpatialiteSridsDialog_H
#include "ui_qgsspatialitesridsdialogbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgis.h"

extern "C"
{
#include <sqlite3.h>
}

class QgsSpatialiteSridsDialog: public QDialog, private Ui::QgsSpatialiteSridsDialogBase
{
    Q_OBJECT
  public:
    QgsSpatialiteSridsDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsSpatialiteSridsDialog();
    bool load( QString dbName );
    QString selectedSrid();
  public slots:
    void on_pbnFilter_clicked();
  private:
    sqlite3 *db;
    QString mDbName;
};
#endif //QgsSpatialiteSridsDialog_H
