/***************************************************************************
                          qgsrastercalculator.h  -  description
                          ---------------------
    begin                : September 28th, 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCALCULATOR_H
#define QGSRASTERCALCULATOR_H

#include "qgsfield.h"
#include "qgsrectangle.h"
#include <QString>
#include <QVector>
#include "gdal.h"

class QgsRasterLayer;
class QProgressDialog;


struct ANALYSIS_EXPORT QgsRasterCalculatorEntry
{
  QString ref; //name
  QgsRasterLayer* raster; //pointer to rasterlayer
  int bandNumber; //raster band number
};

/**Raster calculator class*/
class ANALYSIS_EXPORT QgsRasterCalculator
{
  public:
    QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
                         const QgsRectangle& outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries );
    ~QgsRasterCalculator();

    /**Starts the calculation and writes new raster
      @param p progress bar (or 0 if called from non-gui code)
      @return 0 in case of success*/
    int processCalculation( QProgressDialog* p = 0 );

  private:
    //default constructor forbidden. We need formula, output file, output format and output raster resolution obligatory
    QgsRasterCalculator();

    /**Opens the output driver and tests if it supports the creation of a new dataset
      @return NULL on error and the driver handle on success*/
    GDALDriverH openOutputDriver();

    /**Opens the output file and sets the same geotransform and CRS as the input data
      @return the output dataset or NULL in case of error*/
    GDALDatasetH openOutputFile( GDALDriverH outputDriver );

    /**Reads raster pixels from a dataset/band
      @param targetGeotransform transformation parameters of the requested raster array
                                (not necessarily the same as the transform of the source dataset)
      @param xOffset x offset
      @param yOffset y offset
      @param nCols number of columns
      @param nRows number of rows
      @param sourceTransform source transformation
      @param sourceBand source band
      @param rasterBuffer raster buffer
      */
    void readRasterPart( double* targetGeotransform,
                         int xOffset, int yOffset,
                         int nCols, int nRows,
                         double* sourceTransform,
                         GDALRasterBandH sourceBand,
                         float* rasterBuffer );

    /**Compares two geotransformations (six parameter double arrays*/
    bool transformationsEqual( double* t1, double* t2 ) const;

    /**Sets gdal 6 parameters array from mOutputRectangle, mNumOutputColumns, mNumOutputRows
      @param transform double[6] array that receives the GDAL parameters*/
    void outputGeoTransform( double* transform ) const;

    QString mFormulaString;
    QString mOutputFile;
    QString mOutputFormat;

    /**Output raster extent*/
    QgsRectangle mOutputRectangle;
    /**Number of output columns*/
    int mNumOutputColumns;
    /**Number of output rows*/
    int mNumOutputRows;

    /***/
    QVector<QgsRasterCalculatorEntry> mRasterEntries;
};

#endif // QGSRASTERCALCULATOR_H
