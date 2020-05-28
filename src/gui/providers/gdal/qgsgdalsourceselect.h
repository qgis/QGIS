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
#include "qgis_gui.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

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

    //! Returns whether the protocol is a cloud type
    bool isProtocolCloudType();

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;
    //! Sets protocol-related widget visibility
    void setProtocolWidgetsVisibility();

    void radioSrcFile_toggled( bool checked );
    void radioSrcProtocol_toggled( bool checked );
    void cmbProtocolTypes_currentIndexChanged( const QString &text );

  private:

    QString mRasterPath;

};

///@endcond
#endif // QGGDALSOURCESELECT_H
