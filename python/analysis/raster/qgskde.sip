/**
 * \class QgsKernelDensityEstimation
 * \ingroup analysis
 * Performs Kernel Density Estimation ("heatmap") calculations on a vector layer.
 * @note added in QGIS 3.0
 */
class QgsKernelDensityEstimation
{
%TypeHeaderCode
#include <qgskde.h>
%End

  public:

    //! Kernel shape type
    enum KernelShape
    {
      KernelQuartic, //!< Quartic kernel
      KernelTriangular, //!< Triangular kernel
      KernelUniform, //!< Uniform (flat) kernel
      KernelTriweight, //!< Triweight kernel
      KernelEpanechnikov, //!< Epanechnikov kernel
    };

    //! Output values type
    enum OutputValues
    {
      OutputRaw, //!< Output the raw KDE values
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
      QgsVectorLayer* vectorLayer;

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

};
