/***************************************************************************
  qgspdalsourceselect.h
  -------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPDALSOURCESELECT_H
#define QGSPDALSOURCESELECT_H

#include "ui_qgspdalsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"


/**
 * Dialog to select PDAL supported point cloud sources
 */
class QgsPdalSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsPdalSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPdalSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                         QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private:
    QString mPath;

};

#endif // QGSPDALSOURCESELECT_H
