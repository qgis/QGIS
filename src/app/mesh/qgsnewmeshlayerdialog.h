/***************************************************************************
  qgsnewmeshlayerdialog.h - QgsNewMeshLayerDialog

 ---------------------
 begin                : 22.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWMESHLAYERDIALOG_H
#define QGSNEWMESHLAYERDIALOG_H

#include "ui_qgsnewmeshlayerdialogbase.h"

#include "qgsguiutils.h"
#include "qgsmeshlayer.h"
#include "qgis_app.h"

/**
 * \brief A Dialog Widget that is used to create and load in the project a new Mesh Layer from scratch or derived from another mesh
 *
 * \since QGIS 3.22
 */
class APP_EXPORT QgsNewMeshLayerDialog : public QDialog, private Ui::QgsNewMeshLayerDialogBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsNewMeshLayerDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    void accept() override;

    //! Sets the default CRS of the widget
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    //! Sets a mesh layer that could be used as base for the new mesh
    void setSourceMeshLayer( QgsMeshLayer *meshLayer, bool fromExistingAsDefault = false );

    //! Returns a pointer to the new created mesh layer
    QgsMeshLayer *newLayer() const;

  private slots:
    void updateDialog();
    void updateSourceMeshframe();
    void onFormatChanged();
    void onFilePathChanged();
    void updateSourceMeshInformation();

  private:
    bool apply();

    std::unique_ptr<QgsMeshLayer> mSourceMeshFromFile;
    bool mSourceMeshFrameReady;
    QMap<QString, QString> mDriverSuffixes;
    QMap<QString, QString> mDriverFileFilters;

    QgsMeshLayer *mNewLayer = nullptr;
};

#endif // QGSNEWMESHLAYERDIALOG_H
