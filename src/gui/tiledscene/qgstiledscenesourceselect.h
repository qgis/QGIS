/***************************************************************************
                         qgstiledscenesourceselect.h
                         ---------------------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENESOURCESELECT_H
#define QGSTILEDSCENESOURCESELECT_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsabstractdatasourcewidget.h"
#include "ui_qgstiledscenesourceselectbase.h"

/*!
 * \brief Dialog to create connections to tiled scene servers.
 *
 * This dialog allows the user to define and save connection information
 * for tiled scene servers.
 *
 * The user can then connect and add layers from the tiled scene server
 * to the map canvas.
 */
class QgsTiledSceneSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsTiledSceneSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsTiledSceneSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

    //! Determines the layers the user selected
    void addButtonClicked() override;

  private slots:

    //! Opens a dialog to edit an existing connection
    void btnEdit_clicked();
    //! Deletes the selected connection
    void btnDelete_clicked();
    //! Saves connections to the file
    void btnSave_clicked();
    //! Loads connections from the file
    void btnLoad_clicked();
    //! Stores the selected datasource whenerver it is changed
    void cmbConnections_currentTextChanged( const QString &text );

    void showHelp();

  private:
    //! Opens the create connection dialog to build a new connection
    void newConnection( QString provider );
    void populateConnectionList();
    void setConnectionListPosition();
};

///@endcond

#endif // QGSTILEDSCENESOURCESELECT_H
