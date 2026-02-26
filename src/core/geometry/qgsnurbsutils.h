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
     * Finds the NURBS curve containing the vertex identified by \a vid.
     *
     * \param geom the geometry to search in
     * \param vid the vertex identifier to search for
     * \param localIndex will be set to the control point index within the found NURBS curve
     * \returns the NURBS curve containing the vertex, or NULLPTR if the vertex is not part of a NURBS curve
     * \note Not available in Python bindings
     */
    static const QgsNurbsCurve *findNurbsCurveForVertex(
      const QgsAbstractGeometry *geom,
      const QgsVertexId &vid,
      int &localIndex ) SIP_SKIP;

    /**
     * Finds the NURBS curve containing the vertex identified by \a vid.
     *
     * \param geom the geometry to search in
     * \param vid the vertex identifier to search for
     * \param localIndex will be set to the control point index within the found NURBS curve
     * \returns the NURBS curve containing the vertex, or NULLPTR if the vertex is not part of a NURBS curve
     */
    static QgsNurbsCurve *findNurbsCurveForVertex(
      QgsAbstractGeometry *geom,
      const QgsVertexId &vid,
      int &localIndex SIP_OUT );
};

#endif // QGSNURBSUTILS_H
