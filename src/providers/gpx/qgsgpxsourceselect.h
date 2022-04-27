/***************************************************************************
  qgsgpxsourceselect.h
  -------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
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
#ifndef QGSGPXSOURCESELECT_H
#define QGSGPXSOURCESELECT_H

#include "ui_qgsgpxsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"


/**
 * \class QgsGpxSourceSelect
 * \brief Dialog to select GPX supported sources
 */
class QgsGpxSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsGpxSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGpxSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private slots:

    void enableRelevantControls();

  private:
    QString mGpxPath;

};

#endif // QGSGPXSOURCESELECT_H
