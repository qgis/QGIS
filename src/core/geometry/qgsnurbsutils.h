/***************************************************************************
                         qgsnurbsutils.h
                         ---------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNURBSUTILS_H
#define QGSNURBSUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsAbstractGeometry;
class QgsNurbsCurve;
struct QgsVertexId;

/**
 * \ingroup core
 * \class QgsNurbsUtils
 * \brief Utility functions for working with NURBS curves.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsNurbsUtils
{
  public:

    /**
     * Returns TRUE if the \a geom contains a NURBS curve (recursively).
     */
    static bool containsNurbsCurve( const QgsAbstractGeometry *geom );

    /**
     * Extracts the first NURBS curve found in the \a geom (recursively).
     * Returns NULLPTR if no NURBS curve is found.
     */
    static const QgsNurbsCurve *extractNurbsCurve( const QgsAbstractGeometry *geom );

    /**
     * Returns TRUE if the NURBS curve is a Poly-Bézier.
     *
     * A Poly-Bézier has degree 3 and control points count = 3*segments + 1.
     */
    static bool isPolyBezier( const QgsNurbsCurve *nurbs );

    /**
     * Finds the NURBS curve containing the vertex identified by \a vid.
     *
     * Returns the NURBS curve and sets \a localIndex to the control point index
     * within that curve. Returns NULLPTR if the vertex is not part of a NURBS curve.
     */
    static const QgsNurbsCurve *findNurbsCurveForVertex(
      const QgsAbstractGeometry *geom,
      const QgsVertexId &vid,
      int &localIndex SIP_OUT );

    /**
     * Mutable version of findNurbsCurveForVertex().
     *
     * Finds the NURBS curve containing the vertex identified by \a vid.
     * Returns the NURBS curve and sets \a localIndex to the control point index
     * within that curve. Returns NULLPTR if the vertex is not part of a NURBS curve.
     */
    static QgsNurbsCurve *findMutableNurbsCurveForVertex(
      QgsAbstractGeometry *geom,
      const QgsVertexId &vid,
      int &localIndex SIP_OUT );
};

#endif // QGSNURBSUTILS_H
