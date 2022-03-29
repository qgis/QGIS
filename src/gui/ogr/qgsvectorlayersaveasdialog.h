/***************************************************************************
                          qgsvectorlayersaveasdialog.h
 Dialog to select destination, type and crs to save as ogr layers
                             -------------------
    begin                : Mon Mar 22 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERSAVEASDIALOG_H
#define QGSVECTORLAYERSAVEASDIALOG_H

#include "ui_qgsvectorlayersaveasdialogbase.h"
#include <QDialog>
#include "qgshelp.h"
#include "qgsfields.h"
#include "qgsvectorfilewriter.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief Class to select destination file, type and CRS for ogr layers
 * \note not available in Python bindings
 * \since QGIS 1.0
 */
class GUI_EXPORT QgsVectorLayerSaveAsDialog : public QDialog, private Ui::QgsVectorLayerSaveAsDialogBase
{
    Q_OBJECT

  public:

    //! Bitmask of options to be shown
    enum Option
    {
      Symbology = 1, //!< Show symbology options
      DestinationCrs = 1 << 2, //!< Show destination CRS (reprojection) option
      Fields = 1 << 3, //!< Show field customization group
      AddToCanvas = 1 << 4, //!< Show add to map option
      SelectedOnly = 1 << 5, //!< Show selected features only option
      GeometryType = 1 << 6, //!< Show geometry group
      Extent = 1 << 7, //!< Show extent group
      Metadata = 1 << 8, //!< Show metadata options
      AllOptions = ~0 //!< Show all options
    };
    Q_DECLARE_FLAGS( Options, Option )

    /**
     * Construct a new QgsVectorLayerSaveAsDialog
     *
     * \deprecated since QGIS 3.14 - will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED QgsVectorLayerSaveAsDialog( long srsid, QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

    /**
     * Construct a new QgsVectorLayerSaveAsDialog
     */
    QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, Options options = AllOptions, QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

    /**
     * The format in which the export should be written.
     * \see QgsVectorFileWriter::filterForDriver()
     */
    QString format() const;

    /**
     * The encoding of the target file.
     */
    QString encoding() const;

    /**
     * Returns the target filename.
     */
    QString filename() const;

    /**
     * Returns the target layer name
     */
    QString layername() const;

    /**
     * Returns a list of additional data source options which are passed to OGR.
     * Refer to the OGR documentation for the target format for available options.
     */
    QStringList datasourceOptions() const;

    /**
     * Returns a list of additional layer options which are passed to OGR.
     * Refer to the OGR documentation for the target format for available options.
     */
    QStringList layerOptions() const;

    /**
     * Returns the internal CRS ID.
     * \see QgsCoordinateReferenceSystem::srsid()
     * \deprecated since QGIS 3.14 - will be removed in QGIS 4.0. Use crsObject() instead.
     */
    Q_DECL_DEPRECATED long crs() const;

    /**
     * Returns the CRS chosen for export
     * \since QGIS 3.14
     */
    QgsCoordinateReferenceSystem crsObject() const;

    /**
     * Returns a list of attributes which are selected for saving.
     */
    QgsAttributeList selectedAttributes() const;
    //! Returns selected attributes that must be exported with their displayed values instead of their raw values. Added in QGIS 2.16
    QgsAttributeList attributesAsDisplayedValues() const;

    /**
     * Returns a list of export names for attributes
     */
    QStringList attributesExportNames() const;

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
     * Returns type of symbology export.
     * 0: No symbology
     * 1: Feature symbology
     * 2: Symbol level symbology
     */
    int symbologyExport() const;

    /**
     * Returns the specified map scale.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    double scale() const;

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

    /**
     * Sets whether only selected features will be saved.
     */
    void setOnlySelected( bool onlySelected );

    /**
     * Returns whether only selected features will be saved.
     */
    bool onlySelected() const;

    /**
     * Returns TRUE if the persist metadata (copy source metadata to destination layer) option is checked.
     *
     * \since QGIS 3.20
     */
    bool persistMetadata() const;

    /**
     * Returns the selected flat geometry type for the export.
     * \see automaticGeometryType()
     * \see forceMulti()
     * \see includeZ()
     */
    QgsWkbTypes::Type geometryType() const;

    /**
     * Returns TRUE if geometry type is set to automatic.
     * \see geometryType()
     */
    bool automaticGeometryType() const;

    /**
     * Returns TRUE if force multi geometry type is checked.
     * \see includeZ()
     */
    bool forceMulti() const;

    /**
     * Sets whether the force multi geometry checkbox should be checked.
     */
    void setForceMulti( bool checked );

    /**
     * Returns TRUE if include z dimension is checked.
     * \see forceMulti()
     */
    bool includeZ() const;

    /**
     * Sets whether the include z dimension checkbox should be checked.
     */
    void setIncludeZ( bool checked );

    //! Returns creation action
    QgsVectorFileWriter::ActionOnExistingFile creationActionOnExistingFile() const;

  private slots:

    void mFormatComboBox_currentIndexChanged( int idx );
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void showHelp();
    void mSymbologyExportComboBox_currentIndexChanged( const QString &text );
    void mGeometryTypeComboBox_currentIndexChanged( int index );
    void accept() override;
    void mSelectAllAttributes_clicked();
    void mDeselectAllAttributes_clicked();
    void mUseAliasesForExportedName_stateChanged( int state );
    void mReplaceRawFieldValues_stateChanged( int state );
    void mAttributeTable_itemChanged( QTableWidgetItem *item );

  private:

    enum class ColumnIndex : int
    {
      Name = 0,
      ExportName = 1,
      Type = 2,
      ExportAsDisplayedValue = 3
    };

    void setup();
    QList< QPair< QLabel *, QWidget * > > createControls( const QMap<QString, QgsVectorFileWriter::Option *> &options );

    QgsCoordinateReferenceSystem mSelectedCrs;

    QgsRectangle mLayerExtent;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
    Options mOptions = AllOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorLayerSaveAsDialog::Options )


#endif // QGSVECTORLAYERSAVEASDIALOG_H
