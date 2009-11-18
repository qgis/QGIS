/***************************************************************************
                          qgsopenvectorlayerdialog.h
 Dialog to select the type and source for ogr vectors, supports
 file, database, directory and protocol sources.
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id:$ */
#ifndef QGSOPENVECTORLAYERDIALOG_H
#define QGSOPENVECTORLAYERDIALOG_H

#include <ui_qgsopenvectorlayerdialogbase.h>
#include <QDialog>
#include "qgscontexthelp.h"

/**
 *  Class for a  dialog to select the type and source for ogr vectors, supports
 *  file, database, directory and protocol sources.
 */
class QgsOpenVectorLayerDialog : public QDialog, private Ui::QgsOpenVectorLayerDialogBase
{
    Q_OBJECT

  public:
    QgsOpenVectorLayerDialog( QWidget* parent = 0,  Qt::WFlags fl = 0 );
    ~QgsOpenVectorLayerDialog();
    //! Opens a dialog to select a file datasource*/
    QStringList openFile();
    //! Opens a dialog to select a directory datasource*/
    QString openDirectory();
    //! Returns a list of selected datasources*/
    QStringList dataSources();
    //! Returns the encoding selected for user*/
    QString encoding();
    //! Returns the connection type
    QString dataSourceType();
  private:
    //! Shows a dialog remembering the last directory and filter selected */
    void openFilesRememberingFilter( QString const &filterName,
                                     QString const &filters, QStringList & selectedFiles, QString &title );
    //! Stores the file vector filters */
    QString mVectorFileFilter;
    //! Stores the selected datasources */
    QStringList mDataSources;
    //! Stores the user selected encoding
    QString mEnc;
    //! Stores the datasource type
    QString mDataSourceType;

  private slots:
    //! Opens the create connection dialog to build a new connection
    void addNewConnection();
    //! Opens a dialog to edit an existing connection
    void editConnection();
    //! Deletes the selected connection
    void deleteConnection();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! Sets the actual position in connection list
    void setConnectionListPosition();
    //! Sets the actual position in types connection list
    void setConnectionTypeListPosition();
    //! Sets the selected connection type
    void setSelectedConnectionType();
    //! Sets the selected connection
    void setSelectedConnection();

    void on_buttonBox_accepted();
    void on_buttonSelectSrc_clicked();
    void on_radioSrcFile_toggled( bool checked );
    void on_radioSrcDirectory_toggled( bool checked );
    void on_radioSrcDatabase_toggled( bool checked );
    void on_radioSrcProtocol_toggled( bool checked );
    void on_btnNew_clicked();
    void on_btnEdit_clicked();
    void on_btnDelete_clicked();
    void on_cmbDatabaseTypes_currentIndexChanged( const QString & text );
    void on_cmbConnections_currentIndexChanged( const QString & text );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
};

#endif // QGSOPENVECTORDIALOG_H
