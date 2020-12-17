/***************************************************************************
  QgsDisplazsourceselect.h
  -------------------
    begin                : July 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGDISPLAZSOURCESELECT_H
#define QGDISPLAZSOURCESELECT_H

#include "src\ui\ui_qgsmdalsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"


/**
 * \class QgsDisplazSourceSelect
 * \brief Dialog to select PDAL supported mesh sources
 */
class QgsDisplazSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsMdalSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsDisplazSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private:
    QString mPointCloudPath;

};

#endif // QGDISPLAZSOURCESELECT_H
