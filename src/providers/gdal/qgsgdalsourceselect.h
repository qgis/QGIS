/***************************************************************************
                          qgsgdalsourceselect.h
                             -------------------
    begin                : August 05 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGGDALSOURCESELECT_H
#define QGGDALSOURCESELECT_H

#include "ui_qgsgdalsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"


/**
 * \class QgsGdalSourceSelect
 * \brief Dialog to select GDAL supported rasters
 */
class QgsGdalSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsGdalSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGdalSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private:

    QString mRasterPath;

};

#endif // QGGDALSOURCESELECT_H
