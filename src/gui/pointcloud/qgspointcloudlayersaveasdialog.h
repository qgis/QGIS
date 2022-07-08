/***************************************************************************
                          qgspointcloudlayersaveasdialog.h
 Dialog to select destination, type and crs to save as pointcloud layers
                             -------------------
    begin                : July 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTCLOUDLAYERSAVEASDIALOG_H
#define QGSPOINTCLOUDLAYERSAVEASDIALOG_H

#include "ui_qgspointcloudlayersaveasdialogbase.h"
#include <QDialog>
#include "qgshelp.h"
#include "qgsfields.h"
#include "qgsvectorfilewriter.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsPointCloudLayer;

/**
 * \ingroup gui
 * \brief Class to select destination file, type and CRS for ogr layers
 * \note not available in Python bindings
 * \since QGIS 1.0
 */
class GUI_EXPORT QgsPointCloudLayerSaveAsDialog : public QDialog, private Ui::QgsPointCloudLayerSaveAsDialogBase
{
    Q_OBJECT

  public:

    /**
     * Construct a new QgsPointCloudLayerSaveAsDialog
     */
    QgsPointCloudLayerSaveAsDialog( QgsPointCloudLayer *layer, QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

    /**
     * The format in which the export should be written.
     * \see QgsVectorFileWriter::filterForDriver()
     */
    QString format() const;

    /**
     * Returns the target filename.
     */
    QString filename() const;

    /**
     * Returns the target layer name
     */
    QString layername() const;

    /**
     * Returns the CRS chosen for export
     * \since QGIS 3.14
     */
    QgsCoordinateReferenceSystem crsObject() const;

    /**
     * Returns a list of attributes which are selected for saving.
     */
    QStringList selectedAttributes() const;

    /**
     * Returns TRUE if the "add to canvas" checkbox is checked.
     *
     * \see setAddToCanvas()
     */
    bool addToCanvas() const;

    /**
     * Sets whether the  "add to canvas" checkbox should be \a checked.
     *
     * \see addToCanvas()
     * \since QGIS 3.6
     */
    void setAddToCanvas( bool checked );

    /**
     * Sets a map \a canvas to associate with the dialog.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Determines if filtering the export by an extent is activated.
     * \see filterExtent()
     */
    bool hasFilterExtent() const;

    /**
     * Determines the extent to be exported.
     * \see hasFilterExtent()
     */
    QgsRectangle filterExtent() const;

    //! Returns creation action
    QgsVectorFileWriter::ActionOnExistingFile creationActionOnExistingFile() const;

  private slots:

    void mFormatComboBox_currentIndexChanged( int idx );
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void showHelp();
    void accept() override;
    void mSelectAllAttributes_clicked();
    void mDeselectAllAttributes_clicked();

  private:

    enum class ColumnIndex : int
    {
      Name = 0,
      ExportName = 1,
      Type = 2,
      ExportAsDisplayedValue = 3
    };

    void setup();

    QgsCoordinateReferenceSystem mSelectedCrs;

    QgsRectangle mLayerExtent;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsPointCloudLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
};

#endif // QGSPOINTCLOUDLAYERSAVEASDIALOG_H
