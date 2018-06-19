/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgskde.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsKernelDensityEstimation
{
%Docstring
Performs Kernel Density Estimation ("heatmap") calculations on a vector layer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgskde.h"
%End
  public:

    enum KernelShape
    {
      KernelQuartic,
      KernelTriangular,
      KernelUniform,
      KernelTriweight,
      KernelEpanechnikov,
    };

    enum OutputValues
    {
      OutputRaw,
      OutputScaled,
    };

    enum Result
    {
      Success,
      DriverError,
      InvalidParameters,
      FileCreationError,
      RasterIoError,
    };

    struct Parameters
    {
      QgsFeatureSource *source;

      double radius;

      QString radiusField;

      QString weightField;

      double pixelSize;

      QgsKernelDensityEstimation::KernelShape shape;

      double decayRatio;

      QgsKernelDensityEstimation::OutputValues outputValues;
    };

    QgsKernelDensityEstimation( const Parameters &parameters, const QString &outputFile, const QString &outputFormat );
%Docstring
Constructor for QgsKernelDensityEstimation. Requires a Parameters object specifying the options to use
to generate the surface. The output path and file format are also required.
%End


    Result run();
%Docstring
Runs the KDE calculation across the whole layer at once. Either call this method, or manually
call run(), addFeature() and finalise() separately.
%End

    Result prepare();
%Docstring
Prepares the output file for writing and setups up the surface calculation. This must be called
before adding features via addFeature().

.. seealso:: :py:func:`addFeature`

.. seealso:: :py:func:`finalise`
%End

    Result addFeature( const QgsFeature &feature );
%Docstring
Adds a single feature to the KDE surface. prepare() must be called before adding features.

.. seealso:: :py:func:`prepare`

.. seealso:: :py:func:`finalise`
%End

    Result finalise();
%Docstring
Finalises the output file. Must be called after adding all features via addFeature().

.. seealso:: :py:func:`prepare`

.. seealso:: :py:func:`addFeature`
%End

  private:
    QgsKernelDensityEstimation( const QgsKernelDensityEstimation &other );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgskde.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
