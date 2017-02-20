/***************************************************************************
  qgskde.h
  --------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSKDE_H
#define QGSKDE_H

#include "qgsrectangle.h"
#include <QString>

// GDAL includes
#include <gdal.h>
#include <cpl_string.h>
#include <cpl_conv.h>
#include "qgis_analysis.h"

class QgsVectorLayer;
class QProgressDialog;
class QgsFeature;


/**
 * \class QgsKernelDensityEstimation
 * \ingroup analysis
 * Performs Kernel Density Estimation ("heatmap") calculations on a vector layer.
 * @note added in QGIS 3.0
 */
class ANALYSIS_EXPORT QgsKernelDensityEstimation
{
  public:

    //! Kernel shape type
    enum KernelShape
    {
      KernelQuartic = 0, //!< Quartic kernel
      KernelTriangular, //!< Triangular kernel
      KernelUniform, //!< Uniform (flat) kernel
      KernelTriweight, //!< Triweight kernel
      KernelEpanechnikov, //!< Epanechnikov kernel
    };

    //! Output values type
    enum OutputValues
    {
      OutputRaw = 0, //!< Output the raw KDE values
      OutputScaled, //!< Output mathematically correct scaled values
    };

    //! Result of operation
    enum Result
    {
      Success, //!< Operation completed successfully
      DriverError, //!< Could not open the driver for the specified format
      InvalidParameters, //!< Input parameters were not valid
      FileCreationError, //!< Error creating output file
      RasterIoError, //!< Error writing to raster
    };

    //! KDE parameters
    struct Parameters
    {
      //! Vector point layer
      QgsVectorLayer* vectorLayer = nullptr;

      //! Fixed radius, in map units
      double radius;

      //! Field for radius, or empty if using a fixed radius
      QString radiusField;

      //! Field name for weighting field, or empty if not using weights
      QString weightField;

      //! Size of pixel in output file
      double pixelSize;

      //! Kernel shape
      KernelShape shape;

      //! Decay ratio (Triangular kernels only)
      double decayRatio;

      //! Type of output value
      OutputValues outputValues;
    };

    /**
     * Constructor for QgsKernelDensityEstimation. Requires a Parameters object specifying the options to use
     * to generate the surface. The output path and file format are also required.
     */
    QgsKernelDensityEstimation( const Parameters& parameters, const QString& outputFile, const QString& outputFormat );

    /**
     * Runs the KDE calculation across the whole layer at once. Either call this method, or manually
     * call run(), addFeature() and finalise() separately.
     */
    Result run();

    /**
     * Prepares the output file for writing and setups up the surface calculation. This must be called
     * before adding features via addFeature().
     * @see addFeature()
     * @see finalise()
     */
    Result prepare();

    /**
     * Adds a single feature to the KDE surface. prepare() must be called before adding features.
     * @see prepare()
     * @see finalise()
     */
    Result addFeature( const QgsFeature& feature );

    /**
     * Finalises the output file. Must be called after adding all features via addFeature().
     * @see prepare()
     * @see addFeature()
     */
    Result finalise();

  private:

    //! Calculate the value given to a point width a given distance for a specified kernel shape
    double calculateKernelValue( const double distance, const double bandwidth, const KernelShape shape, const OutputValues outputType ) const;
    //! Uniform kernel function
    double uniformKernel( const double distance, const double bandwidth, const OutputValues outputType ) const;
    //! Quartic kernel function
    double quarticKernel( const double distance, const double bandwidth, const OutputValues outputType ) const;
    //! Triweight kernel function
    double triweightKernel( const double distance, const double bandwidth, const OutputValues outputType ) const;
    //! Epanechnikov kernel function
    double epanechnikovKernel( const double distance, const double bandwidth, const OutputValues outputType ) const;
    //! Triangular kernel function
    double triangularKernel( const double distance, const double bandwidth, const OutputValues outputType ) const;

    QgsRectangle calculateBounds() const;

    QgsVectorLayer* mInputLayer = nullptr;

    QString mOutputFile;
    QString mOutputFormat;

    int mRadiusField;
    int mWeightField;
    double mRadius;
    double mPixelSize;
    QgsRectangle mBounds;

    KernelShape mShape;
    double mDecay;
    OutputValues mOutputValues;

    int mBufferSize;

    GDALDatasetH mDatasetH;
    GDALRasterBandH mRasterBandH;

    //! Creates a new raster layer and initializes it to the no data value
    bool createEmptyLayer( GDALDriverH driver, const QgsRectangle& bounds, int rows, int columns ) const;
    int radiusSizeInPixels( double radius ) const;
};


#endif // QGSKDE_H
