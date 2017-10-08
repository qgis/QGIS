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
 *  Class to select destination file, type and CRS for ogr layers
 *  \note not available in Python bindings
 */
class GUI_EXPORT QgsVectorLayerSaveAsDialog : public QDialog, private Ui::QgsVectorLayerSaveAsDialogBase
{
    Q_OBJECT

  public:
    // bitmask of options to be shown
    enum Options
    {
      Symbology = 1,
      AllOptions = ~0
    };

    QgsVectorLayerSaveAsDialog( long srsid, QWidget *parent = nullptr, Qt::WindowFlags fl = 0 );
    QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, int options = AllOptions, QWidget *parent = nullptr, Qt::WindowFlags fl = 0 );
    ~QgsVectorLayerSaveAsDialog();

    QString format() const;
    QString encoding() const;
    QString filename() const;
    QString layername() const;
    QStringList datasourceOptions() const;
    QStringList layerOptions() const;
    long crs() const;
    QgsAttributeList selectedAttributes() const;
    //! Return selected attributes that must be exported with their displayed values instead of their raw values. Added in QGIS 2.16
    QgsAttributeList attributesAsDisplayedValues() const;
    bool addToCanvas() const;

    /**
     * Returns type of symbology export.
        0: No symbology
        1: Feature symbology
        2: Symbol level symbology*/
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

    bool hasFilterExtent() const;
    QgsRectangle filterExtent() const;

    bool onlySelected() const;

    /**
     * Returns the selected flat geometry type for the export.
     * \see automaticGeometryType()
     * \see forceMulti()
     * \see includeZ()
     */
    QgsWkbTypes::Type geometryType() const;

    /**
     * Returns true if geometry type is set to automatic.
     * \see geometryType()
     */
    bool automaticGeometryType() const;

    /**
     * Returns true if force multi geometry type is checked.
     * \see includeZ()
     */
    bool forceMulti() const;

    /**
     * Sets whether the force multi geometry checkbox should be checked.
     */
    void setForceMulti( bool checked );

    /**
     * Returns true if include z dimension is checked.
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
    void leFilename_textChanged( const QString &text );
    void browseFilename_clicked();
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void showHelp();
    void mSymbologyExportComboBox_currentIndexChanged( const QString &text );
    void mGeometryTypeComboBox_currentIndexChanged( int index );
    void accept() override;
    void mSelectAllAttributes_clicked();
    void mDeselectAllAttributes_clicked();
    void mReplaceRawFieldValues_stateChanged( int state );
    void mAttributeTable_itemChanged( QTableWidgetItem *item );

  private:
    void setup();
    QList< QPair< QLabel *, QWidget * > > createControls( const QMap<QString, QgsVectorFileWriter::Option *> &options );

    long mCRS;

    QgsRectangle mLayerExtent;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    bool mAttributeTableItemChangedSlotEnabled;
    bool mReplaceRawFieldValuesStateChangedSlotEnabled;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
};

#endif // QGSVECTORLAYERSAVEASDIALOG_H
