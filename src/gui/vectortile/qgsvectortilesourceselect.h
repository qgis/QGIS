/***************************************************************************
                         qgsvectortilesourceselect.h
                         ---------------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILESOURCESELECT_H
#define QGSVECTORTILESOURCESELECT_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsabstractdatasourcewidget.h"
#include "ui_qgsvectortilesourceselectbase.h"

/*!
 * \brief   Dialog to create connections to vector tile servers.
 *
 * This dialog allows the user to define and save connection information
 * for vector tile servers.
 *
 * The user can then connect and add layers from the vector tile server
 * to the map canvas.
 */
class QgsVectorTileSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsVectorTileSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsVectorTileSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

    //! Determines the layers the user selected
    void addButtonClicked() override;

  private slots:

    //! Opens the create connection dialog to build a new connection
    void btnNew_clicked();
    void newArcgisVectorTileServerConnection();
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

  private:
    void populateConnectionList();
    void setConnectionListPosition();
    void showHelp();
};

///@endcond

#endif // QGSVECTORTILESOURCESELECT_H
