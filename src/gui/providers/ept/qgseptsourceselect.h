/***************************************************************************
                         qgseptsourceselect.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
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
#ifndef QGSEPTSOURCESELECT_H
#define QGSEPTSOURCESELECT_H

///@cond PRIVATE
#include "qgis_sip.h"
#define SIP_NO_FILE

#include "ui_qgspointcloudsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"


/**
 * \class QgsEptSourceSelect
 * \brief Dialog to select EPT point cloud supported sources
 */
class QgsEptSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsPointCloudSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsEptSourceSelect( QWidget *parent = nullptr,
                        Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                        QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private:
    QString mPath;

};

///@endcond
///
#endif // QGSEPTSOURCESELECT_H
