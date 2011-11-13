/***************************************************************************
                          qgsninecellfilter.h  -  description
                             -------------------
    begin                : August 6th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNINECELLFILTER_H
#define QGSNINECELLFILTER_H

#include <QString>
#include "gdal.h"

class QProgressDialog;

/**Base class for raster analysis methods that work with a 3x3 cell filter and calculate the value of each cell based on
the cell value and the eight neighbour cells. Common examples are slope and aspect calculation in DEMs. Subclasses only implement
the method that calculates the new value from the nine values. Everything else (reading file, writing file) is done by this subclass*/

class ANALYSIS_EXPORT QgsNineCellFilter
{
  public:
    /**Constructor that takes input file, output file and output format (GDAL string)*/
    QgsNineCellFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    virtual ~QgsNineCellFilter();
    /**Starts the calculation, reads from mInputFile and stores the result in mOutputFile
      @param p progress dialog that receives update and that is checked for abort. 0 if no progress bar is needed.
      @return 0 in case of success*/
    int processRaster( QProgressDialog* p );

  private:
    //default constructor forbidden. We need input file, output file and format obligatory
    QgsNineCellFilter();

    /**Opens the input file and returns the dataset handle and the number of pixels in x-/y- direction*/
    GDALDatasetH openInputFile( int& nCellsX, int& nCellsY );
    /**Opens the output driver and tests if it supports the creation of a new dataset
      @return NULL on error and the driver handle on success*/
    GDALDriverH openOutputDriver();
    /**Opens the output file and sets the same geotransform and CRS as the input data
      @return the output dataset or NULL in case of error*/
    GDALDatasetH openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver );

  protected:
    /**Calculates output value from nine input values. The input values and the output value can be equal to the
      nodata value if not present or outside of the border. Must be implemented by subclasses*/
    virtual float processNineCellWindow( float* x11, float* x21, float* x31,
                                         float* x12, float* x22, float* x32,
                                         float* x13, float* x23, float* x33 ) = 0;

    QString mInputFile;
    QString mOutputFile;
    QString mOutputFormat;

    double mCellSizeX;
    double mCellSizeY;
    /**The nodata value of the input layer*/
    float mInputNodataValue;
    /**The nodata value of the output layer*/
    float mOutputNodataValue;
};

#endif // QGSNINECELLFILTER_H
