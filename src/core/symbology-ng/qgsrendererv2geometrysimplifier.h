/***************************************************************************
    qgsrendererv2geometrysimplifier.h
    ---------------------
    begin                : October 2013
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

#ifndef QGSRENDER_GEOMETRY_SIMPLIFIER_H
#define QGSRENDER_GEOMETRY_SIMPLIFIER_H

#include "qgsrendercontext.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"

/** \ingroup core
 * Provides geometry simplification methods for optimize the drawing of features.
 * This helper class calculate a simplified/generalized geometry from one source 
 * feature using a tolerance of the current map2pixel value of the RenderContext.
 * Output geometry is drawed with undetectable visual changes.
 **/
class CORE_EXPORT QgsFeatureRendererSimplifier
{
private:

	/** Returns the squared 2D-distance of the vector defined by the two points specified */
	static float LengthGeneralizedSquared2D(double x1, double y1, double x2, double y2);

	/** Returns the MapTolerance of the current View for transforms between map coordinates and device coordinates */
	static float CalculateViewPixelTolerance(QgsRenderContext& context, QRectF& boundingRect);

	/** Returns the view-valid number of points of the specified geometry */
	static unsigned int CalculateGeneralizedPointCount(QgsRenderContext& context, QGis::WkbType wkbType, const unsigned char* wkb, unsigned int numPoints, QRectF& boundingRect, bool& generalizedByBoundingBox);

public:

	/** Fill the view-valid simplified points to the specified geometry */
	static unsigned int SimplifyGeometry(QgsRenderContext& context, QGis::WkbType wkbType, const unsigned char* wkb, unsigned int numPoints, QVector<QPointF>& outputPoints, bool& generalizedByBoundingBox);
};

#endif
