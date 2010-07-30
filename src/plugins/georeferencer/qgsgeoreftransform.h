/***************************************************************************
    qgsgeoreftransform.h - Encapsulates GCP-based parameter estimation and
    reprojection for different transformation models.
     --------------------------------------
    Date                 : 18-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGS_GEOREF_TRANSFORM_H
#define QGS_GEOREF_TRANSFORM_H

#include <gdal_alg.h> // just needed for GDALTransformerFunc, forward?

#include <qgspoint.h>
#include <vector>
#include <stdexcept>

class QgsGeorefTransformInterface
{
  public:
    virtual ~QgsGeorefTransformInterface() { }

    virtual bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords ) = 0;

    /**
     * Returns the minimum number of GCPs required for parameter fitting.
     */
    virtual uint getMinimumGCPCount() const = 0;

    /**
     * Return funtion pointer to the GDALTransformer function.
     * Used by GDALwarp.
     */
    virtual GDALTransformerFunc  GDALTransformer()     const = 0;
    virtual void*                GDALTransformerArgs() const = 0;
};

/**
 * \brief Transform class for different gcp-based transform methods.
 *
 * Select transform type via \ref selectTransformParametrisation.
 * Initialize and update parameters via \ref updateParametersFromGCPs.
 * An initialized instance then provides transform functions and GDALTransformer entry points
 * for warping and coordinate remapping.
 *
 * Delegates to concrete implementations of \ref QgsGeorefInterface. For exception safety,
 * this is preferred over using the subclasses directly.
 */
class QgsGeorefTransform : public QgsGeorefTransformInterface
{
  public:
    // GCP based transform methods implemented by subclasses
    enum TransformParametrisation
    {
      Linear,
      Helmert,
      PolynomialOrder1,
      PolynomialOrder2,
      PolynomialOrder3,
      ThinPlateSpline,
      Projective,
      InvalidTransform = 65535
    };

    QgsGeorefTransform( TransformParametrisation parametrisation );
    QgsGeorefTransform();
    ~QgsGeorefTransform();

    /**
     * Switches the used transform type to the given parametrisation.
     */
    void selectTransformParametrisation( TransformParametrisation parametrisation );

    //! \brief The transform parametrisation currently in use.
    TransformParametrisation transformParametrisation() const;

    /**True for linear, Helmert, first order polynomial*/
    bool providesAccurateInverseTransformation() const;

    //! \returns whether the parameters of this transform have been initialised by \ref updateParametersFromGCPs
    bool parametersInitialized() const;

    /**
     * \brief Fits transformation parameters to the supplied ground control points.
     *
     * \returns true on success, false on failure
     */
    bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords );

    //! \brief Returns the minimum number of GCPs required for parameter fitting.
    uint getMinimumGCPCount() const;

    /**
     * \brief Return funtion pointer to the GDALTransformer function.
     *
     * Used by the transform routines \ref transform, \ref transformRasterToWorld
     * \ref transformWorldToRaster and by the GDAL warping code
     * in \ref QgsImageWarper::warpFile.
     */
    GDALTransformerFunc  GDALTransformer()     const;
    void*                GDALTransformerArgs() const;

    /**
     * \brief Transform from pixel coordinates to georeferenced coordinates.
     *
     * \note Negative y-axis points down in raster CS.
     */
    bool transformRasterToWorld( const QgsPoint &raster, QgsPoint &world ) const;

    /**
     * \brief Transform from referenced coordinates to raster coordinates.
     *
     * \note Negative y-axis points down in raster CS.
     */
    bool transformWorldToRaster( const QgsPoint &world, QgsPoint &raster ) const;

    /**
     * \brief Transforms from raster to world if rasterToWorld is true,
     * \brief or from world to raster when rasterToWorld is false.
     *
     * \note Negative y-axis points down in raster CS.
     */
    bool transform( const QgsPoint &src, QgsPoint &dst, bool rasterToWorld ) const;

    //! \brief Returns origin and scale if this is a linear transform, fails otherwise.
    bool getLinearOriginScale( QgsPoint &origin, double &scaleX, double &scaleY ) const;

    //! \brief Returns origin, scale and rotation for linear and helmert transform, fails otherwise.
    bool getOriginScaleRotation( QgsPoint &origin, double &scaleX, double &scaleY, double& rotation ) const;

  private:
    // shallow copy constructor
    QgsGeorefTransform( const QgsGeorefTransform &other );

    //! Factory function which creates an implementation for the given parametrisation.
    static QgsGeorefTransformInterface *createImplementation( TransformParametrisation parametrisation );

    // convenience wrapper around GDALTransformerFunc
    bool gdal_transform( const QgsPoint &src, QgsPoint &dst, int dstToSrc ) const;

    QgsGeorefTransformInterface *mGeorefTransformImplementation;
    TransformParametrisation     mTransformParametrisation;
    bool                         mParametersInitialized;
};

#endif
