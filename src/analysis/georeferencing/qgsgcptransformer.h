/***************************************************************************
    qgsgcptransformer.h
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

#ifndef QGSGCPTRANSFORMER_H
#define QGSGCPTRANSFORMER_H

#include <gdal_alg.h>
#include "qgspoint.h"
#include "qgis_analysis.h"
#include "qgis_sip.h"

/**
 * \ingroup analysis
 * An interface for Ground Control Points (GCP) based transformations.
 *
 * QgsGcpTransformerInterface implementations are able to transform point locations
 * based on a transformation method and a list of GCPs.
 *
 * \since QGIS 3.20
*/
class ANALYSIS_EXPORT QgsGcpTransformerInterface SIP_ABSTRACT
{
  public:

    /**
     * Available transformation methods.
     */
    enum class TransformMethod : int
    {
      Linear, //!< Linear transform
      Helmert, //!< Helmert transform
      PolynomialOrder1, //!< Polynomial order 1
      PolynomialOrder2, //!< Polyonmial order 2
      PolynomialOrder3, //!< Polynomial order
      ThinPlateSpline, //!< Thin plate splines
      Projective, //!< Projective
      InvalidTransform = 65535 //!< Invalid transform
    };

    QgsGcpTransformerInterface() = default;

    virtual ~QgsGcpTransformerInterface() = default;

    //! QgsGcpTransformerInterface cannot be copied
    QgsGcpTransformerInterface( const QgsGcpTransformerInterface &other ) = delete;

    //! QgsGcpTransformerInterface cannot be copied
    QgsGcpTransformerInterface &operator=( const QgsGcpTransformerInterface &other ) = delete;

    /**
     * Fits transformation parameters using the specified Ground Control Points (GCPs) lists of map coordinates and layer coordinates.
     *
     * \returns TRUE on success, FALSE on failure
     */
    virtual bool updateParametersFromGcps( const QVector<QgsPointXY> &mapCoordinates, const QVector<QgsPointXY> &layerCoordinates ) = 0;

    /**
     * Returns the minimum number of Ground Control Points (GCPs) required for parameter fitting.
     */
    virtual int minimumGcpCount() const = 0;

    /**
     * Returns the transformation method.
     */
    virtual TransformMethod method() const = 0;

    /**
     * Returns a translated string representing the specified transform \a method.
     */
    static QString methodToString( TransformMethod method );

    /**
     * Creates a new QgsGcpTransformerInterface subclass representing the specified transform \a method.
     *
     * Caller takes ownership of the returned object.
     */
    static QgsGcpTransformerInterface *create( TransformMethod method ) SIP_FACTORY;

    /**
     * Creates a new QgsGcpTransformerInterface subclass representing the specified transform \a method, initialized
     * using the given lists of map and layer coordinates.
     *
     * If the parameters cannot be fit to a transform NULLPTR will be returned.
     *
     * Caller takes ownership of the returned object.
     */
    static QgsGcpTransformerInterface *createFromParameters( TransformMethod method, const QVector<QgsPointXY> &mapCoordinates, const QVector<QgsPointXY> &layerCoordinates ) SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Returns function pointer to the GDALTransformer function.
     */
    virtual GDALTransformerFunc GDALTransformer() const = 0;

    /**
     * Returns pointer to the GDALTransformer arguments.
     */
    virtual void *GDALTransformerArgs() const = 0;
#endif

  private:

#ifdef SIP_RUN
    QgsGcpTransformerInterface( const QgsGcpTransformerInterface &other )
#endif
};

/**
 * A simple transform which is parametrized by a translation and anistotropic scale.
 * \ingroup analysis
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class ANALYSIS_EXPORT QgsLinearGeorefTransform : public QgsGcpTransformerInterface SIP_SKIP
{
  public:
    QgsLinearGeorefTransform() = default;

    /**
     * Returns the origin and scale for the transform.
     */
    bool getOriginScale( QgsPointXY &origin, double &scaleX, double &scaleY ) const;

    bool updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &layerCoords ) override;
    int minimumGcpCount() const override;
    GDALTransformerFunc GDALTransformer() const override;
    void *GDALTransformerArgs() const override;
    TransformMethod method() const override;

  private:
    struct LinearParameters
    {
      QgsPointXY origin;
      double scaleX, scaleY;
    } mParameters;

    static int linearTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                double *x, double *y, double *z, int *panSuccess );

};

/**
 * 2-dimensional helmert transform, parametrised by isotropic scale, rotation angle and translation.
 * \ingroup analysis
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class ANALYSIS_EXPORT QgsHelmertGeorefTransform : public QgsGcpTransformerInterface SIP_SKIP
{
  public:
    QgsHelmertGeorefTransform() = default;

    /**
     * Returns the origin, scale and rotation for the transform.
     */
    bool getOriginScaleRotation( QgsPointXY &origin, double &scale, double &rotation ) const;

    bool updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &layerCoords ) override;
    int minimumGcpCount() const override;
    GDALTransformerFunc GDALTransformer() const override;
    void *GDALTransformerArgs() const override;
    TransformMethod method() const override;

  private:

    struct HelmertParameters
    {
      QgsPointXY origin;
      double scale;
      double angle;
    };
    HelmertParameters mHelmertParameters;

    static int helmert_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                  double *x, double *y, double *z, int *panSuccess );

};

/**
 * Interface to gdal thin plate splines and 1st/2nd/3rd order polynomials.
 * \ingroup analysis
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class ANALYSIS_EXPORT QgsGDALGeorefTransform : public QgsGcpTransformerInterface SIP_SKIP
{
  public:
    QgsGDALGeorefTransform( bool useTPS, unsigned int polynomialOrder );
    ~QgsGDALGeorefTransform() override;

    bool updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &layerCoords ) override;
    int minimumGcpCount() const override;
    GDALTransformerFunc GDALTransformer() const override;
    void *GDALTransformerArgs() const override;
    TransformMethod method() const override;

  private:
    void destroyGdalArgs();

    const int mPolynomialOrder;
    const bool mIsTPSTransform;

    GDALTransformerFunc mGDALTransformer;
    void *mGDALTransformerArgs = nullptr;

};

/**
 * A planar projective transform, expressed by a homography.
 *
 * Implements model fitting which minimizes algebraic error using total least squares.
 *
 * \ingroup analysis
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class ANALYSIS_EXPORT QgsProjectiveGeorefTransform : public QgsGcpTransformerInterface SIP_SKIP
{
  public:
    QgsProjectiveGeorefTransform();

    bool updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &layerCoords ) override;
    int minimumGcpCount() const override;
    GDALTransformerFunc GDALTransformer() const override;
    void *GDALTransformerArgs() const override;
    TransformMethod method() const override;

  private:
    struct ProjectiveParameters
    {
      double H[9];        // Homography
      double Hinv[9];     // Inverted homography
      bool hasInverse;  // Could the inverted homography be calculated?
    } mParameters;

    static int projectiveTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                    double *x, double *y, double *z, int *panSuccess );

};

#endif //QGSGCPTRANSFORMER_H
