/***************************************************************************
    qgsrendererv2geometrysimplifier.cpp
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

#include "limits.h"
#include "qgsrendererv2geometrysimplifier.h"
#include "QtCore/qrect.h"

/** Returns the squared 2D-distance of the vector defined by the two points specified */
float QgsFeatureRendererSimplifier::lengthGeneralizedSquared2D(double x1, double y1, double x2, double y2)
{
	float vx = (float)(x2-x1);
	float vy = (float)(y2-y1);

	return vx*vx + vy*vy;
}

/** Returns the MapTolerance of the current View for transforms between map coordinates and device coordinates */
float QgsFeatureRendererSimplifier::calculateViewPixelTolerance(QgsRenderContext& context, QRectF& boundingRect)
{
	const QgsMapToPixel& mtp = context.mapToPixel();
	const QgsCoordinateTransform* ct = context.coordinateTransform();

	double mapUnitsPerPixel = mtp.mapUnitsPerPixel();
	double mapUnitsFactor = 1;

	// Calculate one aprox factor of the size of the BBOX from the source CoordinateSystem to the target CoordinateSystem.
	if (ct && !((QgsCoordinateTransform*)ct)->isShortCircuited())
	{
		QgsRectangle sourceRect(boundingRect.left(), boundingRect.bottom(), boundingRect.right(), boundingRect.top());
		QgsRectangle targetRect = ct->transform(sourceRect);

		QgsPoint minimumSrcPoint( sourceRect.xMinimum(), sourceRect.yMinimum() );
		QgsPoint maximumSrcPoint( sourceRect.xMaximum(), sourceRect.yMaximum() );
		QgsPoint minimumDstPoint( targetRect.xMinimum(), targetRect.yMinimum() );
		QgsPoint maximumDstPoint( targetRect.xMaximum(), targetRect.yMaximum() );

		double sourceHypothenuse = sqrt( lengthGeneralizedSquared2D( minimumSrcPoint.x(), minimumSrcPoint.y(), maximumSrcPoint.x(), maximumSrcPoint.y() ) );
		double targetHypothenuse = sqrt( lengthGeneralizedSquared2D( minimumDstPoint.x(), minimumDstPoint.y(), maximumDstPoint.x(), maximumDstPoint.y() ) );

		if (targetHypothenuse!=0) 
		mapUnitsFactor = sourceHypothenuse/targetHypothenuse;
	}
	return (float)( mapUnitsPerPixel * mapUnitsFactor );
}

/** Returns the view-valid number of points of the specified geometry */
unsigned int QgsFeatureRendererSimplifier::calculateGeneralizedPointCount(QgsRenderContext& context, float map2pixelTol, QGis::WkbType wkbType, const unsigned char* wkb, unsigned int numPoints, QRectF& boundingRect, bool& generalizedByBoundingBox)
{
	const unsigned char* wkb2 = wkb;

	double xmin =  std::numeric_limits<double>::max();
	double ymin =  std::numeric_limits<double>::max();
	double xmax = -std::numeric_limits<double>::max();
	double ymax = -std::numeric_limits<double>::max();
	double x,y, lastX=0,lastY=0;

	int sizeOfDoubleX = sizeof(double);
	int sizeOfDoubleY = QGis::wkbDimensions(wkbType)==3 /*hasZValue*/ ? 2*sizeof(double) : sizeof(double);

	// Calculate the full BoundingBox of the current point stream.
	for ( unsigned int jdx = 0; jdx < numPoints; ++jdx )
	{
		x = *(( double * ) wkb ); wkb += sizeOfDoubleX;
		y = *(( double * ) wkb ); wkb += sizeOfDoubleY;

		if (xmin>x) xmin = x;
		if (ymin>y) ymin = y;
		if (xmax<x) xmax = x;
		if (ymax<y) ymax = y;
	}
	boundingRect.setCoords(xmin,ymin,xmax,ymax);
	wkb = wkb2;

	// MapTolerance of the current View for transforms between map coordinates and device coordinates.
	float mappixelTol = map2pixelTol * QgsFeatureRendererSimplifier::calculateViewPixelTolerance( context, boundingRect );

	// Can simplify the geometry using the full BBOX ?
	if ((generalizedByBoundingBox = ((xmax-xmin)<=mappixelTol && (ymax-ymin)<=mappixelTol)))
	{
		return QGis::flatType(wkbType)==QGis::WKBLineString ? 2 : 5;
	}

	unsigned int simplifiedPointCount = 0;
	mappixelTol *= mappixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

	// Force equal the first and last point of the closed geometry.
	if (QGis::flatType(wkbType)==QGis::WKBPolygon)
	{
		simplifiedPointCount++;
		numPoints--;
	}

	// Calculate the view-valid number of points of the specified geometry.
	for ( unsigned int jdx = 0; jdx < numPoints; ++jdx )
	{
		x = *(( double * ) wkb ); wkb += sizeOfDoubleX;
		y = *(( double * ) wkb ); wkb += sizeOfDoubleY;

		if (jdx==0 || lengthGeneralizedSquared2D(lastX,lastY,x,y)>mappixelTol)
		{
			simplifiedPointCount++;
			lastX = x;
			lastY = y;
		}
	}
	wkb = wkb2;

	// Returns the view-valid PointCount.
	if (QGis::flatType(wkbType)==QGis::WKBLineString && simplifiedPointCount<=1)
	{
		generalizedByBoundingBox = true;
		simplifiedPointCount = 2;
	}
	else
	if (QGis::flatType(wkbType)==QGis::WKBPolygon && simplifiedPointCount<=3)
	{
		generalizedByBoundingBox = true;
		simplifiedPointCount = 5;
	}
	return simplifiedPointCount;
}

/** Fill the view-valid simplified points to the specified geometry */
unsigned int QgsFeatureRendererSimplifier::simplifyGeometry(QgsRenderContext& context, float map2pixelTol, QGis::WkbType wkbType, const unsigned char* wkb, unsigned int numPoints, QVector<QPointF>& outputPoints, bool& generalizedByBoundingBox)
{
	QGis::WkbType flatType = QGis::flatType(wkbType);

	int sizeOfDoubleX = sizeof(double);
	int sizeOfDoubleY = QGis::wkbDimensions(wkbType)==3 /*hasZValue*/ ? 2*sizeof(double) : sizeof(double);

	generalizedByBoundingBox = false;
	double x,y;

	// Fill if possible, the view-valid simplified points to the specified geometry.
	if (numPoints>2 && flatType!=QGis::WKBPoint && flatType!=QGis::WKBMultiPoint)
	{
		double lastX=0,lastY=0;

		// Calculate the view-valid number of points of the geometry.
		QRectF boundingRect;
		unsigned int simplifiedPointCount = calculateGeneralizedPointCount(context, map2pixelTol, wkbType, wkb, numPoints, boundingRect, generalizedByBoundingBox);

		outputPoints.resize(simplifiedPointCount);
		QPointF* ptr = outputPoints.data();

		// Can simplify the geometry using the full BBOX ?
		if (generalizedByBoundingBox)
		{
			if (flatType==QGis::WKBLineString)
			{
			   *ptr = boundingRect.topLeft(); ptr++;
			   *ptr = boundingRect.bottomRight();
			}
			else
			if (flatType==QGis::WKBPolygon)
			{
			   *ptr = boundingRect.topLeft(); ptr++;
			   *ptr = boundingRect.topRight(); ptr++;
			   *ptr = boundingRect.bottomRight(); ptr++;
			   *ptr = boundingRect.bottomLeft(); ptr++;
			   *ptr = boundingRect.topLeft();
			}
			return simplifiedPointCount;
		}

		// MapTolerance of the current View for transforms between map coordinates and device coordinates.
		float mappixelTol = map2pixelTol * QgsFeatureRendererSimplifier::calculateViewPixelTolerance( context, boundingRect );
		mappixelTol *= mappixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

		// Force equal the first and last point of the closed geometry.
		bool isaPolygon = QGis::flatType(wkbType)==QGis::WKBPolygon;
		if (isaPolygon) numPoints--;
		
		// Fill the view-valid points of the specified geometry.
		for ( unsigned int jdx = 0; jdx < numPoints; ++jdx)
		{
			x = *(( double * ) wkb ); wkb += sizeOfDoubleX;
			y = *(( double * ) wkb ); wkb += sizeOfDoubleY;

			if (jdx==0 || lengthGeneralizedSquared2D(lastX,lastY,x,y)>mappixelTol)
			{
			   *ptr = QPointF(x, y); ++ptr;
				lastX = x; 
				lastY = y;
			}
		}
		if (isaPolygon)
		{
			*ptr = *outputPoints.data();
		}
		return simplifiedPointCount;
	}
	else
	{
		outputPoints.resize(numPoints);
		QPointF* ptr = outputPoints.data();

		for ( unsigned int jdx = 0; jdx < numPoints; ++jdx, ++ptr )
		{
			x = *(( double * ) wkb ); wkb += sizeOfDoubleX;
			y = *(( double * ) wkb ); wkb += sizeOfDoubleY;

			*ptr = QPointF( x, y );
		}
		return numPoints;
	}
}
