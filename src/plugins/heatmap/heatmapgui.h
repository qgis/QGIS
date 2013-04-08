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
    HeatmapGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~HeatmapGui();

    // Buffer unit type
    // Should have been private, made public to be used in heatmap.cpp
    enum mBufferType
    {
      Meters,
      MapUnits
    };

    /** Returns whether to apply weighted heat */
    bool weighted();

    /** Returns whether the radius is static or based on a field */
    bool variableRadius();

    /** Returns the fixed radius value */
    double radius();

    /** Return the radius Unit (meters/map units) */
    int radiusUnit();

    /** Return the selected kernel shape */
    int kernelShape();

    /** Return the decay ratio */
    double decayRatio();

    /** Return the attribute field for variable radius */
    int radiusField();

    /** Returns the attrinute field for weighted heat */
    int weightField();

    /** Returns the output filename/path */
    QString outputFilename();

    /** Returns the GDAL Format for output raster */
    QString outputFormat();

    /** Returns the input Vector layer */
    QgsVectorLayer* inputVectorLayer();

    /** Returns the no of rows for the raster */
    int rows() { return mRows; }

    /** Returns the no of columns in the raster */
    int columns() { return mColumns; }

    /** Returns the cell size X value */
    double cellSizeX() { return mXcellsize; }

    /** Returns the cell size Y valuue */
    double cellSizeY() { return mYcellsize; }

    /** Return the BBox */
    QgsRectangle bbox() { return mBBox; }

  private:
    QMap<QString, QString> mExtensionMap;

    // bbox of layer for lineedit changes
    QgsRectangle mBBox;
    double mXcellsize, mYcellsize;
    int mRows, mColumns;

    /** Function to check wether all constrains are satisfied and enable the OK button */
    void enableOrDisableOkButton();

    /** Populate the attribute fields from selected vector for radius&weight */
    void populateFields();

    /** Set the mBBox value - mainly used for updation purpose */
    void updateBBox();

    /** Update the LineEdits cellsize and row&col values  */
    void updateSize();

    /** Convert Maters value to the corresponding map units based on Layer projection */
    double mapUnitsOf( double meters, QgsCoordinateReferenceSystem layerCrs );

  private slots:
    void on_mButtonBox_accepted();
    void on_mButtonBox_rejected();
    void on_mButtonBox_helpRequested();
    void on_mBrowseButton_clicked();
    void on_mOutputRasterLineEdit_editingFinished();
    void on_advancedGroupBox_toggled( bool enabled );
    void on_rowLineEdit_editingFinished();
    void on_columnLineEdit_editingFinished();
    void on_cellXLineEdit_editingFinished();
    void on_cellYLineEdit_editingFinished();
    void on_radiusFieldCombo_currentIndexChanged( int index );
    void on_radiusFieldUnitCombo_currentIndexChanged( int index );
    void on_mRadiusUnitCombo_currentIndexChanged( int index );
    void on_mInputVectorCombo_currentIndexChanged( int index );
    void on_mBufferLineEdit_editingFinished();
    void on_kernelShapeCombo_currentIndexChanged( int index );

    /** Estimate a reasonable starting value for the radius field */
    double estimateRadius();

    inline double max( double a, double b )
    { return a > b ? a : b; }
};

#endif
