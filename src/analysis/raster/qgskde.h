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

#include "qgsogrutils.h"
#include "qgsrectangle.h"

#include <QString>

// GDAL includes
#include <gdal.h>
#include <cpl_string.h>
#include <cpl_conv.h>
#include "qgis_analysis.h"

class QgsFeatureSource;
class QgsFeature;


/**
 * \class QgsKernelDensityEstimation
 * \ingroup analysis
 * \brief Performs Kernel Density Estimation ("heatmap") calculations on a vector layer.
 */
class ANALYSIS_EXPORT QgsKernelDensityEstimation
{
  public:
    //! Kernel shape type
    enum class KernelShape SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsKernelDensityEstimation, KernelShape ) : int
    {
      Quartic SIP_MONKEYPATCH_COMPAT_NAME( KernelQuartic ) = 0,       //!< Quartic kernel
      Triangular SIP_MONKEYPATCH_COMPAT_NAME( KernelTriangular ),     //!< Triangular kernel
      Uniform SIP_MONKEYPATCH_COMPAT_NAME( KernelUniform ),           //!< Uniform (flat) kernel
      Triweight SIP_MONKEYPATCH_COMPAT_NAME( KernelTriweight ),       //!< Triweight kernel
      Epanechnikov SIP_MONKEYPATCH_COMPAT_NAME( KernelEpanechnikov ), //!< Epanechnikov kernel
    };

    //! Output values type
    enum class OutputValues SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsKernelDensityEstimation, OutputValues ) : int
    {
      Raw SIP_MONKEYPATCH_COMPAT_NAME( OutputRaw ) = 0,   //!< Output the raw KDE values
      Scaled SIP_MONKEYPATCH_COMPAT_NAME( OutputScaled ), //!< Output mathematically correct scaled values
    };

    //! Result of operation
    enum class Result SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsKernelDensityEstimation, Result ) : int
    {
      Success,           //!< Operation completed successfully
      DriverError,       //!< Could not open the driver for the specified format
      InvalidParameters, //!< Input parameters were not valid
      FileCreationError, //!< Error creating output file
      RasterIoError,     //!< Error writing to raster
    };

    //! KDE parameters
    struct Parameters
    {
        //! Point feature source
        QgsFeatureSource *source = nullptr;

        //! Fixed radius, in map units
        double radius;

        //! Field for radius, or empty if using a fixed radius
        QString radiusField;

        //! Field name for weighting field, or empty if not using weights
        QString weightField;

        //! Size of pixel in output file
        double pixelSize;

        //! Kernel shape
        QgsKernelDensityEstimation::KernelShape shape;

        //! Decay ratio (Triangular kernels only)
        double decayRatio;

        //! Type of output value
        QgsKernelDensityEstimation::OutputValues outputValues;
    };

    /**
     * Constructor for QgsKernelDensityEstimation. Requires a Parameters object specifying the options to use
     * to generate the surface. The output path and file format are also required.
     */
    QgsKernelDensityEstimation( const Parameters &parameters, const QString &outputFile, const QString &outputFormat );

    QgsKernelDensityEstimation( const QgsKernelDensityEstimation &other ) = delete;
    QgsKernelDensityEstimation &operator=( const QgsKernelDensityEstimation &other ) = delete;

    /**
     * Runs the KDE calculation across the whole layer at once. Either call this method, or manually
     * call run(), addFeature() and finalise() separately.
     */
    Result run();

    /**
     * Prepares the output file for writing and setups up the surface calculation. This must be called
     * before adding features via addFeature().
     * \see addFeature()
     * \see finalise()
     */
    Result prepare();

    /**
     * Adds a single feature to the KDE surface. prepare() must be called before adding features.
     * \see prepare()
     * \see finalise()
     */
    Result addFeature( const QgsFeature &feature );

    /**
     * Finalises the output file. Must be called after adding all features via addFeature().
     * \see prepare()
     * \see addFeature()
     */
    Result finalise();

  private:
    //! Calculate the value given to a point width a given distance for a specified kernel shape
    [[nodiscard]] double calculateKernelValue( double distance, double bandwidth, KernelShape shape, OutputValues outputType ) const;
    //! Uniform kernel function
    [[nodiscard]] double uniformKernel( double distance, double bandwidth, OutputValues outputType ) const;
    //! Quartic kernel function
    [[nodiscard]] double quarticKernel( double distance, double bandwidth, OutputValues outputType ) const;
    //! Triweight kernel function
    [[nodiscard]] double triweightKernel( double distance, double bandwidth, OutputValues outputType ) const;
    //! Epanechnikov kernel function
    [[nodiscard]] double epanechnikovKernel( double distance, double bandwidth, OutputValues outputType ) const;
    //! Triangular kernel function
    [[nodiscard]] double triangularKernel( double distance, double bandwidth, OutputValues outputType ) const;

    [[nodiscard]] QgsRectangle calculateBounds() const;

    QgsFeatureSource *mSource = nullptr;

    QString mOutputFile;
    QString mOutputFormat;

    int mRadiusField = -1;
    int mWeightField = -1;
    double mRadius;
    double mPixelSize;
    QgsRectangle mBounds;

    KernelShape mShape;
    double mDecay;
    OutputValues mOutputValues;

    int mBufferSize = -1;

    gdal::dataset_unique_ptr mDatasetH;
    GDALRasterBandH mRasterBandH = nullptr;

    //! Creates a new raster layer and initializes it to the no data value
    bool createEmptyLayer( GDALDriverH driver, const QgsRectangle &bounds, int rows, int columns ) const;
    [[nodiscard]] int radiusSizeInPixels( double radius ) const;

#ifdef SIP_RUN
    QgsKernelDensityEstimation( const QgsKernelDensityEstimation &other );
#endif
};

#endif // QGSKDE_H
