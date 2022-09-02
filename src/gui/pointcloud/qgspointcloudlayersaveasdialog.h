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
#include "qgspointcloudlayerexporter.h"

#define SIP_NO_FILE

class QgsPointCloudLayer;

/**
 * \ingroup gui
 * \brief Class to select destination file, type and CRS for ogr layers
 * \note not available in Python bindings
 * \since QGIS 3.28
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
    QgsPointCloudLayerExporter::ExportFormat exportFormat() const;

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
     */
    QgsCoordinateReferenceSystem crsObject() const;

    /**
     * Sets a map \a canvas to associate with the dialog.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

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
     */
    void setAddToCanvas( bool checked );

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

    /**
     * Determines if points will be spatially filtered by a layer's features.
     */
    bool hasFilterLayer() const;

    /**
     * Returns the layer responsible for spatially filtering points.
     */
    QgsMapLayer *filterLayer() const;

    /**
     * Determines if only the selected features from the filterLayer will be used for spatial filtering.
     */
    bool filterLayerSelectedOnly() const;

    /**
     * Determines if attributes will be exported as fields.
     * \see attributes()
     */
    bool hasAttributes() const;

    /**
     * Returns a list of attributes which are selected for saving.
     */
    QStringList attributes() const;

    /**
     * Determines if filtering by Z values is activated.
     * \see zRange()
     */
    bool hasZRange() const;

    /**
     * Determines the Z range of points to be exported.
     * \see hasZRange()
     */
    QgsDoubleRange zRange() const;

    /**
     * Determines if limiting the number of exported points is enabled.
     * \see pointsLimit()
     */
    bool hasPointsLimit() const;

    /**
     * Determines the limit to the total number of points.
     * \see hasPointsLimit()
     */
    int pointsLimit() const;

    //! Returns creation action
    QgsVectorFileWriter::ActionOnExistingFile creationActionOnExistingFile() const;

  private slots:

    void mFormatComboBox_currentIndexChanged( int idx );
    void mFilterGeometryGroupBoxCheckToggled( bool checked );
    void mMinimumZSpinBoxValueChanged( const double value );
    void mMaximumZSpinBoxValueChanged( const double value );
    void mFilterGeometryLayerChanged( QgsMapLayer *layer );
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void showHelp();
    void accept() override;
    void mSelectAllAttributes_clicked();
    void mDeselectAllAttributes_clicked();

  private:

    void setup();

    /**
     * Gets the translated name for the specified \a format
     */
    static QString getTranslatedNameForFormat( QgsPointCloudLayerExporter::ExportFormat format );

    /**
     * Gets the extensions filter for the specified \a format
     */
    static QString getFilterForFormat( QgsPointCloudLayerExporter::ExportFormat format );

    QgsCoordinateReferenceSystem mSelectedCrs;

    QgsRectangle mLayerExtent;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsPointCloudLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
    QString mDefaultOutputLayerNameFromInputLayerName;
    QString mLastUsedFilename;
    bool mWasAddToCanvasForced = false;
};

#endif // QGSPOINTCLOUDLAYERSAVEASDIALOG_H
