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

#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include <QString>
#include <QVector>
#include "gdal.h"
#include "qgis_analysis.h"
#include "qgsogrutils.h"
#include "qgsrastercalcnode.h"

class QgsRasterLayer;
class QgsFeedback;

/**
 * \ingroup analysis
 * \class QgsRasterCalculatorEntry
 * Represents an individual raster layer/band number entry within a raster calculation.
 * \since QGIS 2.18
*/
class ANALYSIS_EXPORT QgsRasterCalculatorEntry
{

  public:

    /**
     * Creates a list of raster entries from the current project.
     *
     * If there is more than one layer with the same data source
     * only one of them is added to the list, duplicate names are
     * also handled by appending an _n integer to the base name.
     *
     * \return the list of raster entries form the current project
     * \since QGIS 3.6
     */
    static QVector<QgsRasterCalculatorEntry> rasterEntries();

    /**
     * Name of entry.
     */
    QString ref;

    /**
     * Raster layer associated with entry.
     */
    QgsRasterLayer *raster = nullptr;

    /**
     * Band number for entry. Numbering for bands usually starts at 1 for the first band, not 0.
     */
    int bandNumber = 1;
};

/**
 * \ingroup analysis
 * Performs raster layer calculations.
*/
class ANALYSIS_EXPORT QgsRasterCalculator
{
  public:

    //! Result of the calculation
    enum Result
    {
      Success = 0, //!< Calculation successful
      CreateOutputError = 1, //!< Error creating output data file
      InputLayerError = 2, //!< Error reading input layer
      Canceled = 3, //!< User canceled calculation
      ParserError = 4, //!< Error parsing formula
      MemoryError = 5, //!< Error allocating memory for result
      BandError = 6, //!< Invalid band number for input
    };


    /**
     * QgsRasterCalculator constructor.
     * \param formulaString formula for raster calculation
     * \param outputFile output file path
     * \param outputFormat output file format
     * \param outputExtent output extent. CRS for output is taken from first entry in rasterEntries.
     * \param nOutputColumns number of columns in output raster
     * \param nOutputRows number of rows in output raster
     * \param rasterEntries list of referenced raster layers
     * \param transformContext coordinate transformation context
     * \since QGIS 3.8
     */
    QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
                         const QgsRectangle &outputExtent, int nOutputColumns, int nOutputRows,
                         const QVector<QgsRasterCalculatorEntry> &rasterEntries,
                         const QgsCoordinateTransformContext &transformContext );

    /**
     * QgsRasterCalculator constructor.
     * \param formulaString formula for raster calculation
     * \param outputFile output file path
     * \param outputFormat output file format
     * \param outputExtent output extent, CRS is specified by outputCrs parameter
     * \param outputCrs destination CRS for output raster
     * \param nOutputColumns number of columns in output raster
     * \param nOutputRows number of rows in output raster
     * \param rasterEntries list of referenced raster layers
     * \param transformContext coordinate transformation context
     * \since QGIS 3.8
     */
    QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
                         const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &outputCrs,
                         int nOutputColumns, int nOutputRows,
                         const QVector<QgsRasterCalculatorEntry> &rasterEntries,
                         const QgsCoordinateTransformContext &transformContext );


    /**
    * QgsRasterCalculator constructor.
    * \param formulaString formula for raster calculation
    * \param outputFile output file path
    * \param outputFormat output file format
    * \param outputExtent output extent. CRS for output is taken from first entry in rasterEntries.
    * \param nOutputColumns number of columns in output raster
    * \param nOutputRows number of rows in output raster
    * \param rasterEntries list of referenced raster layers
    * \deprecated since QGIS 3.8, use the version with transformContext instead
    */
    Q_DECL_DEPRECATED QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
                                           const QgsRectangle &outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries ) SIP_DEPRECATED;

    /**
     * QgsRasterCalculator constructor.
     * \param formulaString formula for raster calculation
     * \param outputFile output file path
     * \param outputFormat output file format
     * \param outputExtent output extent, CRS is specified by outputCrs parameter
     * \param outputCrs destination CRS for output raster
     * \param nOutputColumns number of columns in output raster
     * \param nOutputRows number of rows in output raster
     * \param rasterEntries list of referenced raster layers
     * \deprecated since QGIS 3.8, use the version with transformContext instead
     * \since QGIS 2.10
     */
    Q_DECL_DEPRECATED QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
                                           const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &outputCrs, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries ) SIP_DEPRECATED;

    /**
     * Starts the calculation and writes a new raster.
     *
     * The optional \a feedback argument can be used for progress reporting and cancellation support.
     *
     * \returns QgsRasterCalculator::Success in case of success. If an error is encountered then
     * a description of the error can be obtained by calling lastError().
    */
    Result processCalculation( QgsFeedback *feedback = nullptr );

    /**
     * Returns a description of the last error encountered.
     * \since QGIS 3.4
     */
    QString lastError() const;

  private:
    //default constructor forbidden. We need formula, output file, output format and output raster resolution obligatory
    QgsRasterCalculator() = delete;

    /**
     * Opens the output driver and tests if it supports the creation of a new dataset
      \returns nullptr on error and the driver handle on success*/
    GDALDriverH openOutputDriver();

    /**
     * Opens the output file and sets the same geotransform and CRS as the input data
      \returns the output dataset or nullptr in case of error*/
    gdal::dataset_unique_ptr openOutputFile( GDALDriverH outputDriver );

    /**
     * Sets gdal 6 parameters array from mOutputRectangle, mNumOutputColumns, mNumOutputRows
      \param transform double[6] array that receives the GDAL parameters*/
    void outputGeoTransform( double *transform ) const;

    //! Execute calculations on GPU
    Result processCalculationGPU( std::unique_ptr< QgsRasterCalcNode > calcNode, QgsFeedback *feedback = nullptr );

    QString mFormulaString;
    QString mOutputFile;
    QString mOutputFormat;

    //! Output raster extent
    QgsRectangle mOutputRectangle;
    QgsCoordinateReferenceSystem mOutputCrs;

    //! Number of output columns
    int mNumOutputColumns = 0;
    //! Number of output rows
    int mNumOutputRows = 0;

    QString mLastError;

    /***/
    QVector<QgsRasterCalculatorEntry> mRasterEntries;

    QgsCoordinateTransformContext mTransformContext;
};

#endif // QGSRASTERCALCULATOR_H
