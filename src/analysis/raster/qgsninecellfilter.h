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
#include "qgis_analysis.h"
#include "qgsogrutils.h"

class QgsFeedback;

/**
 * \ingroup analysis
 * Base class for raster analysis methods that work with a 3x3 cell filter and calculate the value of each cell based on
the cell value and the eight neighbour cells. Common examples are slope and aspect calculation in DEMs. Subclasses only implement
the method that calculates the new value from the nine values. Everything else (reading file, writing file) is done by this subclass*/

class ANALYSIS_EXPORT QgsNineCellFilter
{
  public:
    //! Constructor that takes input file, output file and output format (GDAL string)
    QgsNineCellFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );
    virtual ~QgsNineCellFilter() = default;

    /**
     * Starts the calculation, reads from mInputFile and stores the result in mOutputFile
     * \param feedback feedback object that receives update and that is checked for cancellation.
     * \returns 0 in case of success
     */
    int processRaster( QgsFeedback *feedback = nullptr );

    double cellSizeX() const { return mCellSizeX; }
    void setCellSizeX( double size ) { mCellSizeX = size; }
    double cellSizeY() const { return mCellSizeY; }
    void setCellSizeY( double size ) { mCellSizeY = size; }

    double zFactor() const { return mZFactor; }
    void setZFactor( double factor ) { mZFactor = factor; }

    double inputNodataValue() const { return mInputNodataValue; }
    void setInputNodataValue( double value ) { mInputNodataValue = value; }
    double outputNodataValue() const { return mOutputNodataValue; }
    void setOutputNodataValue( double value ) { mOutputNodataValue = value; }

    /**
     * Calculates output value from nine input values. The input values and the output
     * value can be equal to the nodata value if not present or outside of the border.
     * Must be implemented by subclasses.
     *
     * First index of the input cell is the row, second index is the column
     *
     * \param x11 surrounding cell top left
     * \param x21 surrounding cell central left
     * \param x31 surrounding cell bottom left
     * \param x12 surrounding cell top central
     * \param x22 the central cell for which the value will be calculated
     * \param x32 surrounding cell bottom central
     * \param x13 surrounding cell top right
     * \param x23 surrounding cell central right
     * \param x33 surrounding cell bottom right
     * \return the calculated cell value for the central cell x22
     */
    virtual float processNineCellWindow( float *x11, float *x21, float *x31,
                                         float *x12, float *x22, float *x32,
                                         float *x13, float *x23, float *x33 ) = 0;

  private:
    //default constructor forbidden. We need input file, output file and format obligatory
    QgsNineCellFilter() = delete;

    //! Opens the input file and returns the dataset handle and the number of pixels in x-/y- direction
    gdal::dataset_unique_ptr openInputFile( int &nCellsX, int &nCellsY );

    /**
     * Opens the output driver and tests if it supports the creation of a new dataset
      \returns nullptr on error and the driver handle on success*/
    GDALDriverH openOutputDriver();

    /**
     * Opens the output file and sets the same geotransform and CRS as the input data
      \returns the output dataset or nullptr in case of error*/
    gdal::dataset_unique_ptr openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver );

    /**
     * \brief processRasterCPU executes the computation on the CPU
     * \param feedback instance of QgsFeedback, to allow for progress monitoring and cancellation
     * \return an opaque integer for error codes: 0 in case of success
     */
    int processRasterCPU( QgsFeedback *feedback = nullptr );

#ifdef HAVE_OPENCL

    /**
     * \brief processRasterGPU executes the computation on the GPU
     * \param source path to the OpenCL source file
     * \param feedback instance of QgsFeedback, to allow for progress monitoring and cancellation
     * \return an opaque integer for error codes: 0 in case of success
     */
    int processRasterGPU( const QString &source, QgsFeedback *feedback = nullptr );

    /**
     * \brief addExtraRasterParams allow derived classes to add parameters needed
     *        by OpenCL program
     * \param params vector of parameters passed to OpenCL algorithm
     */
    virtual void addExtraRasterParams( std::vector<float> &params )
    {
      Q_UNUSED( params );
    }

    virtual const QString openClProgramBaseName() const
    {
      return QString();
    }

#endif

  protected:

    QString mInputFile;
    QString mOutputFile;
    QString mOutputFormat;

    double mCellSizeX = -1.0;
    double mCellSizeY = -1.0;
    //! The nodata value of the input layer
    float mInputNodataValue = -1.0;
    //! The nodata value of the output layer
    float mOutputNodataValue = -1.0;
    //! Scale factor for z-value if x-/y- units are different to z-units (111120 for degree->meters and 370400 for degree->feet)
    double mZFactor = 1.0;
};

#endif // QGSNINECELLFILTER_H
