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

class QgsGdalCredentialOptionsWidget;

/**
 * \class QgsGdalSourceSelect
 * \brief Dialog to select GDAL supported rasters
 */
class QgsGdalSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsGdalSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGdalSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;
    bool configureFromUri( const QString &uri ) override;
    //! Sets protocol-related widget visibility
    void setProtocolWidgetsVisibility();

    void radioSrcFile_toggled( bool checked );
    void radioSrcOgcApi_toggled( bool checked );
    void radioSrcProtocol_toggled( bool checked );
    void cmbProtocolTypes_currentIndexChanged( const QString &text );

  private slots:
    void showHelp();
    void updateProtocolOptions();
    void credentialOptionsChanged();

  private:
    void computeDataSources();
    void clearOpenOptions();
    void fillOpenOptions();

    std::vector<QWidget *> mOpenOptionsWidgets;
    QgsGdalCredentialOptionsWidget *mCredentialsWidget = nullptr;

    QString mRasterPath;
    QStringList mDataSources;
    bool mIsOgcApi = false;
    QVariantMap mCredentialOptions;
};

///@endcond
#endif // QGGDALSOURCESELECT_H
