#ifndef QGSSPIT_H
#define QGSSPIT_H
/***************************************************************************
                        qgsspit.h  -  description
                           -------------------
  begin                : Fri Dec 19 2003
  copyright            : (C) 2003 by Denis Antipov
                       : (C) 2004 by Gary Sherman
  email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vector>
#include <algorithm>

#include <QStringList>
#include <QString>
#include <QItemDelegate>

#include "qgsshapefile.h"
#include "ui_qgsspitbase.h"

class QTableWidgetItem;

extern "C"
{
#include <libpq-fe.h>
}

class QgsSpit : public QDialog, private Ui::QgsSpitBase
{
    Q_OBJECT
  public:
    QgsSpit( QWidget *parent = 0, Qt::WindowFlags fl = 0 );
    ~QgsSpit();
    //! Populate the list of available database connections
    void populateConnectionList();
    //! Connect to the selected database
    void dbConnect();
    //! Return a list of selected tables
    QStringList selectedTables();
    //! Return the connection info
    QString connectionInfo();
    //! Create a new PostgreSQL connection
    void newConnection();
    //! Edit a PostgreSQL connection
    void editConnection();
    //! Remove a PostgreSQL connection
    void removeConnection();
    //! Add file to the queue
    void addFile();
    //! Remove selected file from the queue
    void removeFile();
    //! Remove all files from the queue
    void removeAllFiles();
    //! Use the default SRID (Spatial Reference ID)
    void useDefaultSrid();
    //! Use the default geometry field name (the_geom)
    void useDefaultGeom();
    //! Show brief help
    void helpInfo();
    //! Import shapefiles into PostgreSQL
    void import();

  public slots:

    // In porting from Qt3 to Qt4 it was easier to have these small
    // redirects for the widget signals rather than rename the existing
    // functions (which would be been connected to the widgets using the
    // Qt3 designer signal/slot connection mechanism), in case you were
    // wondering.
    void on_btnConnect_clicked()    { dbConnect();        }
    void on_btnEdit_clicked()       { editConnection();   }
    void on_btnNew_clicked()        { newConnection();    }
    void on_btnRemove_clicked()     { removeConnection(); }
    void on_buttonBox_accepted()    { import();           }
    void on_buttonBox_helpRequested() { helpInfo();       }
    void on_buttonBox_rejected()    { reject();           }
    void on_btnAddFile_clicked()    { addFile();          }
    void on_btnRemoveAll_clicked()  { removeAllFiles();   }
    void on_btnRemoveFile_clicked() { removeFile();       }
    void on_tblShapefiles_itemClicked( QTableWidgetItem* item )
    { tblShapefiles->editItem( item ); }
    // When the user changes the selected connection, update the schema list
    void on_chkUseDefaultSrid_toggled( bool ) { useDefaultSrid(); }
    void on_chkUseDefaultGeom_toggled( bool ) { useDefaultGeom(); }

  private:

    void restoreState();

    // Enum of table columns indexes
    enum ShpTableColumns
    {
      ColFILENAME = 0,
      ColFEATURECLASS = 1, // is editable
      ColFEATURECOUNT = 2,
      ColDBRELATIONNAME = 3, // is editable
      ColDBSCHEMA = 4  // is editable
    };

    PGconn* checkConnection();
    QStringList schema_list;
    QStringList geometry_list;
    int total_features;
    QVector<QgsShapeFile *> fileList;
    int defSrid;
    QString defGeom;
    int defaultSridValue;
    QString defaultGeomValue;
    PGconn *conn;
};

// We want to provide combo boxes in the table of shape files to
// load. Qt4 doesn't provide an 'out-of-the-box' way to do this
// (unlike Qt3), so we have to use the Qt4 delegate technique to
// provide combo boxes for the table, hence this class...

class ShapefileTableDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    ShapefileTableDelegate( QObject *parent, QStringList& schema_list )
        : mSchemaList( schema_list )
    { Q_UNUSED( parent ); }

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void updateSchemaList( QStringList& schema_list, QString currentSchema );

  private:
    QStringList mSchemaList;
    int mCurrentIndex;
};

#endif
