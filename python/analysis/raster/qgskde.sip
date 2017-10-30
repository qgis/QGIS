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
%Docstring
Point feature source
%End

      double radius;
%Docstring
Fixed radius, in map units
%End

      QString radiusField;
%Docstring
Field for radius, or empty if using a fixed radius
%End

      QString weightField;
%Docstring
Field name for weighting field, or empty if not using weights
%End

      double pixelSize;
%Docstring
Size of pixel in output file
%End

      QgsKernelDensityEstimation::KernelShape shape;
%Docstring
Kernel shape
%End

      double decayRatio;
%Docstring
Decay ratio (Triangular kernels only)
%End

      QgsKernelDensityEstimation::OutputValues outputValues;
%Docstring
Type of output value
%End
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
 :rtype: Result
%End

    Result prepare();
%Docstring
 Prepares the output file for writing and setups up the surface calculation. This must be called
 before adding features via addFeature().
.. seealso:: addFeature()
.. seealso:: finalise()
 :rtype: Result
%End

    Result addFeature( const QgsFeature &feature );
%Docstring
 Adds a single feature to the KDE surface. prepare() must be called before adding features.
.. seealso:: prepare()
.. seealso:: finalise()
 :rtype: Result
%End

    Result finalise();
%Docstring
 Finalises the output file. Must be called after adding all features via addFeature().
.. seealso:: prepare()
.. seealso:: addFeature()
 :rtype: Result
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
