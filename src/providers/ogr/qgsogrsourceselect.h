/***************************************************************************
                          qgsogrsourceselect.h
 Dialog to select the type and source for ogr vectors, supports
 file, database, directory and protocol sources.
                             -------------------
    ---------------------
    Adapted to source select:

    date                 : Aug 5, 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at itopen dot it

    Original work done by:
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
#ifndef QGSOGRSOURCESELECT_H
#define QGSOGRSOURCESELECT_H

#include "ui_qgsogrsourceselectbase.h"
#include <QDialog>
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

/**
 *  Class for a  dialog to select the type and source for ogr vectors, supports
 *  file, database, directory and protocol sources.
 *  \note not available in Python bindings
 */
class QgsOgrSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsOgrSourceSelectBase
{
    Q_OBJECT

  public:
    QgsOgrSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = 0, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
    ~QgsOgrSourceSelect();
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
    //! Stores the file vector filters */
    QString mVectorFileFilter;
    //! Stores the selected datasources */
    QStringList mDataSources;
    //! Stores the user selected encoding
    QString mEnc;
    //! Stores the datasource type
    QString mDataSourceType;
    //! Embedded dialog (do not call parent's accept) and emit signals
    QgsProviderRegistry::WidgetMode mWidgetMode = QgsProviderRegistry::WidgetMode::None;

  public slots:
    void addButtonClicked() override;

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

    void radioSrcFile_toggled( bool checked );
    void radioSrcDirectory_toggled( bool checked );
    void radioSrcDatabase_toggled( bool checked );
    void radioSrcProtocol_toggled( bool checked );
    void btnNew_clicked();
    void btnEdit_clicked();
    void btnDelete_clicked();
    void cmbDatabaseTypes_currentIndexChanged( const QString &text );
    void cmbConnections_currentIndexChanged( const QString &text );
    void showHelp();

  private:

    QString mVectorPath;

};

#endif // QGSOGRSOURCESELECT_H
