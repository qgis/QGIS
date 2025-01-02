/***************************************************************************
    qgsmaptopixelgeometrysimplifier.h
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H
#define QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsgeometrysimplifier.h"
#include <QPolygonF>
#include <memory>

class QgsAbstractGeometry;
class QgsWkbPtr;
class QgsConstWkbPtr;


/**
 * \ingroup core
 * \brief Implementation of GeometrySimplifier using the "MapToPixel" algorithm
 *
 * Simplifies a geometry removing points within of the maximum distance difference that defines the MapToPixel info of a RenderContext request.
 * This class enables simplify the geometries to be rendered in a MapCanvas target to speed up the vector drawing.
 */
class CORE_EXPORT QgsMapToPixelSimplifier : public QgsAbstractGeometrySimplifier
{
  public:

    //! Constructor
    QgsMapToPixelSimplifier( int simplifyFlags, double tolerance, Qgis::VectorSimplificationAlgorithm simplifyAlgorithm = Qgis::VectorSimplificationAlgorithm::Distance );

    //! Applicable simplification flags
    enum SimplifyFlag
    {
      NoFlags          = 0, //!< No simplification can be applied
      SimplifyGeometry = 1, //!< The geometries can be simplified using the current map2pixel context state
      SimplifyEnvelope = 2, //!< The geometries can be fully simplified by its BoundingBox
    };

  private:
    //! Simplify the geometry using the specified tolerance
    static std::unique_ptr<QgsAbstractGeometry> simplifyGeometry( int simplifyFlags, Qgis::VectorSimplificationAlgorithm simplifyAlgorithm, const QgsAbstractGeometry &geometry, double map2pixelTol, bool isaLinearRing );

  protected:
    //! Current simplification flags
    int mSimplifyFlags;

    //! Current algorithm
    Qgis::VectorSimplificationAlgorithm mSimplifyAlgorithm = Qgis::VectorSimplificationAlgorithm::Distance;

    //! Distance tolerance for the simplification
    double mTolerance;

    //! Returns the squared 2D-distance of the vector defined by the two points specified
    static float calculateLengthSquared2D( double x1, double y1, double x2, double y2 );

    //! Returns whether the points belong to the same grid
    static bool equalSnapToGrid( double x1, double y1, double x2, double y2, double gridOriginX, double gridOriginY, float gridInverseSizeXY );

  public:
    //! Gets the simplification hints of the vector layer managed
    int simplifyFlags() const { return mSimplifyFlags; }
    //! Sets the simplification hints of the vector layer managed
    void setSimplifyFlags( int simplifyFlags ) { mSimplifyFlags = simplifyFlags; }

    //! Gets the local simplification algorithm of the vector layer managed
    Qgis::VectorSimplificationAlgorithm simplifyAlgorithm() const { return mSimplifyAlgorithm; }
    //! Sets the local simplification algorithm of the vector layer managed
    void setSimplifyAlgorithm( Qgis::VectorSimplificationAlgorithm simplifyAlgorithm ) { mSimplifyAlgorithm = simplifyAlgorithm; }

    QgsGeometry simplify( const QgsGeometry &geometry ) const override;
    QgsAbstractGeometry *simplify( const QgsAbstractGeometry *geometry ) const override SIP_FACTORY;

    //! Sets the tolerance of the vector layer managed
    void setTolerance( double value ) { mTolerance = value; }

    // MapToPixel simplification helper methods
  public:

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    static bool isGeneralizableByMapBoundingBox( const QgsRectangle &envelope, double map2pixelTol );

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    inline bool isGeneralizableByMapBoundingBox( const QgsRectangle &envelope ) const
    {
      return isGeneralizableByMapBoundingBox( envelope, mTolerance );
    }
};

#endif // QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H
