/***************************************************************************
                         qgssensorthingssourceselect.h
                         ---------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSSOURCESELECT_H
#define QGSSENSORTHINGSSOURCESELECT_H

#include "qgsabstractdatasourcewidget.h"
#include "ui_qgssensorthingssourceselectbase.h"

class QgsSensorThingsSourceWidget;
class QgsSensorThingsConnectionWidget;

#define SIP_NO_FILE

///@cond PRIVATE
class QgsSensorThingsSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsSensorThingsSourceSelectBase
{
    Q_OBJECT

  public:
    QgsSensorThingsSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
    void addButtonClicked() override;
    void setMapCanvas( QgsMapCanvas *mapCanvas ) override;

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
    void buildFilter();

    void validate();

  private:
    void populateConnectionList();
    void setConnectionListPosition();
#if 0
    void showHelp();
#endif

    QgsSensorThingsConnectionWidget *mConnectionWidget = nullptr;
    QgsSensorThingsSourceWidget *mSourceWidget = nullptr;
    int mBlockChanges = 0;
};
///@endcond PRIVATE

#endif // QGSSENSORTHINGSSOURCESELECT_H
