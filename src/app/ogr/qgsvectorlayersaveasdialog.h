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

#include <ui_qgsvectorlayersaveasdialogbase.h>
#include <QDialog>
#include "qgscontexthelp.h"
#include "qgsvectorfilewriter.h"

/**
 *  Class to select destination file, type and CRS for ogr layers
 */
class APP_EXPORT QgsVectorLayerSaveAsDialog : public QDialog, private Ui::QgsVectorLayerSaveAsDialogBase
{
    Q_OBJECT

  public:
    // bitmask of options to be shown
    enum Options
    {
      Symbology = 1,
      AllOptions = ~0
    };

    QgsVectorLayerSaveAsDialog( long srsid, QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr );
    QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, int options = AllOptions, QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr );
    ~QgsVectorLayerSaveAsDialog();

    QString format() const;
    QString encoding() const;
    QString filename() const;
    QStringList datasourceOptions() const;
    QStringList layerOptions() const;
    long crs() const;
    /** @deprecated since 2.16. Now always return true since there is no longer any checkbox */
    bool attributeSelection() const;
    QgsAttributeList selectedAttributes() const;
    /** Return selected attributes that must be exported with their displayed values instead of their raw values. Added in QGIS 2.16 */
    QgsAttributeList attributesAsDisplayedValues() const;
    bool addToCanvas() const;
    /** Returns type of symbology export.
        0: No symbology
        1: Feature symbology
        2: Symbol level symbology*/
    int symbologyExport() const;
    double scaleDenominator() const;

    //! setup canvas extent - for the use in extent group box
    void setCanvasExtent( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs );

    bool hasFilterExtent() const;
    QgsRectangle filterExtent() const;

    bool onlySelected() const;

    /** Returns the selected flat geometry type for the export.
     * @see automaticGeometryType()
     * @see forceMulti()
     * @see includeZ()
     */
    QgsWKBTypes::Type geometryType() const;

    /** Returns true if geometry type is set to automatic.
     * @see geometryType()
     */
    bool automaticGeometryType() const;

    /** Returns true if force multi geometry type is checked.
     * @see includeZ()
     */
    bool forceMulti() const;

    /** Sets whether the force multi geometry checkbox should be checked.
     */
    void setForceMulti( bool checked );

    /** Returns true if include z dimension is checked.
     * @see forceMulti()
     */
    bool includeZ() const;

    /** Sets whether the include z dimension checkbox should be checked.
     */
    void setIncludeZ( bool checked );

  private slots:

    void on_mFormatComboBox_currentIndexChanged( int idx );
    void on_leFilename_textChanged( const QString& text );
    void on_browseFilename_clicked();
    void on_mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem& crs );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_mSymbologyExportComboBox_currentIndexChanged( const QString& text );
    void on_mGeometryTypeComboBox_currentIndexChanged( int index );
    void accept() override;
    void on_mSelectAllAttributes_clicked();
    void on_mDeselectAllAttributes_clicked();
    void on_mReplaceRawFieldValues_stateChanged( int state );
    void on_mAttributeTable_itemChanged( QTableWidgetItem * item );

  private:
    void setup();
    QList< QPair< QLabel*, QWidget* > > createControls( const QMap<QString, QgsVectorFileWriter::Option*>& options );

    long mCRS;

    QgsRectangle mLayerExtent;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsVectorLayer *mLayer;
    bool mAttributeTableItemChangedSlotEnabled;
    bool mReplaceRawFieldValuesStateChangedSlotEnabled;
};

#endif // QGSVECTORLAYERSAVEASDIALOG_H
