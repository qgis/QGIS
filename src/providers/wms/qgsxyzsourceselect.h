/***************************************************************************
                         qgsxyzsourceselect.h
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

#ifndef QGSXYZSOURCESELECT_H
#define QGSXYZSOURCESELECT_H

#include "qgsabstractdatasourcewidget.h"
#include "ui_qgstilesourceselectbase.h"

class QgsXyzSourceWidget;

/*!
 * \brief   Dialog to create connections to XYZ servers.
 *
 * This dialog allows the user to define and save connection information
 * for XYZ servers.
 *
 * The user can then connect and add layers from the XYZ server to the
 * map canvas.
 */
class QgsXyzSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsTileSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsXyzSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

    //! Determines the layers the user selected
    void addButtonClicked() override;

  private slots:

    //! Opens the create connection dialog to build a new connection
    void btnNew_clicked();
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

    QgsXyzSourceWidget *mSourceWidget = nullptr;
    int mBlockChanges = 0;
};

#endif // QGSXYZSOURCESELECT_H
