/***************************************************************************
  sasourceselect.h
  Dialogue box for defining vector layers from a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

  The author gratefully acknowledges that portions of this class were copied
  from QgsPgSourceSelect, and so the following copyright holds on the
  original content:
    qgpgsourceselect.h
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SASOURCESELECT_H
#define SASOURCESELECT_H

#include "ui_sasourceselectbase.h"
#include "sadbfilterproxymodel.h"
#include "sadbtablemodel.h"
#include "sqlanyconnection.h"
#include "sqlanystatement.h"

#include "qgisgui.h"
#include "qgscontexthelp.h"

#include <QThread>
#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>

class QPushButton;
class QStringList;
class SaGeomColTypeThread;
class QgisApp;

class SaSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT;

  public:
    SaSourceSelectDelegate( QObject *parent = NULL ) : QItemDelegate( parent )
    {
    }

    /** Used to create an editor for when the user tries to
     * change the contents of a cell */
    QWidget *createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &option,
      const QModelIndex &index ) const
    {
      if ( index.column() == SaDbTableModel::dbtmSql )
      {
        QLineEdit *le = new QLineEdit( parent );
        le->setText( index.data( Qt::DisplayRole ).toString() );
        return le;
      }

      return NULL;
    }

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
    {
      QComboBox *cb = qobject_cast<QComboBox *>( editor );
      if ( cb )
        model->setData( index, cb->currentText() );

      QLineEdit *le = qobject_cast<QLineEdit *>( editor );
      if ( le )
        model->setData( index, le->text() );
    }
};


/*! \class SaSourceSelect
 * \brief Dialog to create connections and add tables from SQL Anywhere.
 *
 * This dialog allows the user to define and save connection information
 * for SQL Anywhere databases.  The user can then connect and add
 * tables from the database to the map canvas.
 */
class SaSourceSelect : public QDialog, private Ui::SaSourceSelectBase
{
    Q_OBJECT

  public:

    //! Constructor
    SaSourceSelect( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~SaSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo();

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    void buildQuery();

    /*! Connects to the database using the stored connection parameters.
    * Once connected, available layers are displayed.
    */
    void on_btnConnect_clicked();
    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void on_btnEdit_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();
    void on_mSearchTableEdit_textChanged( const QString & text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString & text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString & text );
    void setSql( const QModelIndex& index );
    //! Store the selected database
    void on_cmbConnections_activated( int );
    void setLayerType( QString schema, QString table, QString column,
                       QString type, QString srid, QString lineinterp );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString& regexp );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    /**Inserts information about the spatial tables into mTableModel.
     * Accepts ownership of given connection pointer. */
    bool getTableInfo( SqlAnyConnection *conn, bool searchOtherSchemas );

    // queue another query for the thread
    void addSearchGeometryColumn( const QString &schema, const QString &table, const QString &column, const QString &geomtype, const QString &sridstr, const QString &lineinterp );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();

    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( QString schema, QString table, QString column, QString type );

    // return URI for layer
    QString layerURI( const QModelIndex &index );

  private:
    // The column labels
    QStringList mColumnLabels;

    // Our thread for doing long running queries
    SaGeomColTypeThread* mColumnTypeThread;

    // connection information
    QString mConnInfo;
    bool mEstimateMetadata;
    bool mOtherSchemas;
    QStringList mSelectedTables;

    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    SaDbTableModel mTableModel;
    SaDbFilterProxyModel mProxyModel;

    // button for building queries
    QPushButton *mBuildQueryButton;

    // button for adding layers
    QPushButton *mAddButton;
};


// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class SaGeomColTypeThread : public QThread
{
    Q_OBJECT
  public:

    void setConnInfo( QString s, bool estMeta, bool otherSchemas );
    void addGeometryColumn( QString schema, QString table, QString column, QString geomtype, QString sridstr, QString lineinterp );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal. The getLayerTypes()
    // function does the actual work, but use the run() function if you
    // want the work to be done as a separate thread from the calling
    // process.
    virtual void run() { getLayerTypes(); }
    void getLayerTypes();

  signals:
    void setLayerType( QString schema, QString table, QString column,
                       QString type, QString srid, QString interp );

  public slots:
    void stop();


  private:
    QString mConnInfo;
    bool mEstimateMetadata;
    bool mOtherSchemas;
    bool mStopped;
    std::vector<QString> schemas, tables, columns, geomtypes, sridstrs, lineinterps;
};

#endif // SASOURCESELECT_H
