/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef HeatmapGUI_H
#define HeatmapGUI_H

#include <QDialog>
#include <ui_heatmapguibase.h>

#include "heatmap.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
/**
@author Arunmozhi
*/
class HeatmapGui : public QDialog, private Ui::HeatmapGuiBase
{
    Q_OBJECT
  public:
    HeatmapGui( QWidget* parent, Qt::WindowFlags fl, QMap<QString, QVariant>* temporarySettings );
    ~HeatmapGui();

    // Buffer unit type
    // Should have been private, made public to be used in heatmap.cpp
    enum mBufferType
    {
      Meters,
      MapUnits
    };

    /** Returns whether to apply weighted heat */
    bool weighted() const;

    /** Returns whether the radius is static or based on a field */
    bool variableRadius() const;

    /** Returns the fixed radius value */
    double radius() const;

    /** Return the radius Unit (meters/map units) */
    int radiusUnit() const;

    /** Return the selected kernel shape */
    Heatmap::KernelShape kernelShape() const;

    /** Return the selected output values setting */
    Heatmap::OutputValues outputValues() const;

    /** Return the decay ratio */
    double decayRatio() const;

    /** Return the attribute field for variable radius */
    int radiusField() const;

    /** Returns the attrinute field for weighted heat */
    int weightField() const;

    /** Returns state of the add to canvas checkbox*/
    bool addToCanvas() const;

    /** Returns the output filename/path */
    QString outputFilename() const;

    /** Returns the GDAL Format for output raster */
    QString outputFormat() const;

    /** Returns the input Vector layer */
    QgsVectorLayer* inputVectorLayer() const;

    /** Returns the no of rows for the raster */
    int rows() const { return mRows; }

    /** Returns the no of columns in the raster */
    int columns() const { return mColumns; }

    /** Returns the cell size X value */
    double cellSizeX() const { return mXcellsize; }

    /** Returns the cell size Y valuue */
    double cellSizeY() const { return mYcellsize; }

    /** Return the BBox */
    QgsRectangle bbox() const { return mBBox; }

  private:
    QMap<QString, QString> mExtensionMap;

    QMap<QString, QVariant> *mHeatmapSessionSettings;

    // bbox of layer for lineedit changes
    QgsRectangle mBBox;
    double mXcellsize, mYcellsize;
    int mRows, mColumns;

    /** Restores control values */
    void restoreSettings( bool usingLastInputLayer );

    /** Saves control values */
    void saveSettings();

    /** Blocks/unblocks signals for controls */
    void blockAllSignals( bool b );

    /** Function to check wether all constrains are satisfied and enable the OK button */
    void enableOrDisableOkButton();

    /** Set the mBBox value - mainly used for updation purpose */
    void updateBBox();

    /** Update the LineEdits cellsize and row&col values  */
    void updateSize();

    /** Convert Maters value to the corresponding map units based on Layer projection */
    double mapUnitsOf( double meters, QgsCoordinateReferenceSystem layerCrs ) const;

    /** Estimate a reasonable starting value for the radius field */
    double estimateRadius();

    inline double max( double a, double b )
    { return a > b ? a : b; }

  private slots:
    void on_mButtonBox_accepted();
    void on_mButtonBox_rejected();
    void on_mButtonBox_helpRequested();
    void on_mBrowseButton_clicked();
    void on_mOutputRasterLineEdit_editingFinished();
    void on_mAdvancedGroupBox_toggled( bool enabled );
    void on_mRowsSpinBox_valueChanged();
    void on_mColumnsSpinBox_valueChanged();
    void on_mCellXLineEdit_editingFinished();
    void on_mCellYLineEdit_editingFinished();
    void on_mRadiusFieldCombo_currentIndexChanged( int index );
    void on_mRadiusFieldUnitCombo_currentIndexChanged( int index );
    void on_mBufferUnitCombo_currentIndexChanged( int index );
    void on_mInputLayerCombo_currentIndexChanged( int index );
    void on_mBufferSizeLineEdit_editingFinished();
    void on_mKernelShapeCombo_currentIndexChanged( int index );
};

#endif
