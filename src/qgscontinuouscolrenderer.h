/***************************************************************************
                         qgscontinuouscolrenderer.h  -  description
                             -------------------
    begin                : Nov 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONTINUOUSCOLRENDERER_H
#define QGSCONTINUOUSCOLRENDERER_H

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include <qpainter.h>
#include "qgscoordinatetransform.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include <iostream>
#include "qgsdlgvectorlayerproperties.h"


/**Renderer class which interpolates rgb values linear between the minimum and maximum value of the classification field*/
class QgsContinuousColRenderer: public QgsRenderer
{
 public:
    QgsContinuousColRenderer();
    ~QgsContinuousColRenderer();
    /**Sets the initial symbology configuration for a layer. An instance of the corresponding renderer dialog is created and associated with the layer. Finally, a pixmap for the legend is drawn
     @param layer the vector layer associated with the renderer*/
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    /**Renders the feature using the minimum and maximum value of the classification field*/
    void renderFeature(QPainter* p, QgsFeature* f, QgsCoordinateTransform* t);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Sets the id of the classification field*/
    void setClassificationField(int id);
    /**Sets the item for the minimum value. The item has to be created using the new operator and is automatically deleted when inserting a new item or when the instance is destroyed*/
    void setMinimumItem(QgsRenderItem* it);
    /**Sets the item for the maximum value. The item has to be created using the new operator and is automatically deleted when inserting a new item or when the instance is destroyed*/
    void setMaximumItem(QgsRenderItem* it);
    /**Returns the item for the minimum value*/
    QgsRenderItem* minimumItem();
    /**Returns the item for the maximum value*/
    QgsRenderItem* maximumItem();
    /** Returns true*/
    bool needsAttributes();
 protected:
    /**Number of the classification field (it must be a numerical field)*/
    int m_classificationField;
    /**Item for the minimum value*/
    QgsRenderItem* m_minimumItem;
    /**Item for the maximum value*/
    QgsRenderItem* m_maximumItem;
};

inline QgsContinuousColRenderer::QgsContinuousColRenderer(): m_minimumItem(0), m_maximumItem(0)
{

}

inline int QgsContinuousColRenderer::classificationField() const
{
    return m_classificationField;
}

inline void QgsContinuousColRenderer::setClassificationField(int id)
{
    m_classificationField=id;
}

inline QgsRenderItem* QgsContinuousColRenderer::minimumItem()
{
    return m_minimumItem;
}

inline QgsRenderItem* QgsContinuousColRenderer::maximumItem()
{
    return m_maximumItem;
}

inline bool QgsContinuousColRenderer::needsAttributes()
{
  return true;
}
/*inline void QgsContinuousColRenderer::renderFeature(QPainter* p, OGRFeature* f, QgsCoordinateTransform* t, int endian)
{
    if(m_minimumItem&&m_maximumItem)
    {
	//finally draw the feature
	OGRGeometry *geom = f->GetGeometryRef();
	if(!geom){
	    std::cout << "geom pointer is null" << std::endl;
	}
	// get the wkb representation
	unsigned char *feature = new unsigned char[geom->WkbSize()];
	if(!feature)
	{
	    std::cout <<  "'the feature is null\n";
	}
	geom->exportToWkb((OGRwkbByteOrder)endian, feature);


	int wkbType = (int) feature[1];
	//std::cout << "Feature type: " << wkbType << std::endl;
	// read each feature based on its type

	//interpolate the color values******************************************************************************************************
	double fvalue=f->GetFieldAsDouble(m_classificationField);
	double minvalue=m_minimumItem->value().toDouble();
	double maxvalue=m_maximumItem->value().toDouble();
        
	QColor mincolor, maxcolor;

	if(wkbType==wkbLineString||wkbType==wkbMultiLineString)
	{
	    mincolor=m_minimumItem->getSymbol()->pen().color();
	    maxcolor=m_maximumItem->getSymbol()->pen().color();
	}
        else//if(point or polygon)
	{
	    p->setPen(m_minimumItem->getSymbol()->pen());
	    mincolor=m_minimumItem->getSymbol()->fillColor();
	    maxcolor=m_maximumItem->getSymbol()->fillColor();
	}

	int red=int(maxcolor.red()*(fvalue-minvalue)/(maxvalue-minvalue)+mincolor.red()*(maxvalue-fvalue)/(maxvalue-minvalue));
	int green=int(maxcolor.green()*(fvalue-minvalue)/(maxvalue-minvalue)+mincolor.green()*(maxvalue-fvalue)/(maxvalue-minvalue));
	int blue=int(maxcolor.blue()*(fvalue-minvalue)/(maxvalue-minvalue)+mincolor.blue()*(maxvalue-fvalue)/(maxvalue-minvalue));
	
	if(wkbType==wkbLineString||wkbType==wkbMultiLineString)
	{
	  p->setPen(QColor(red,green,blue));  
	}
	else
	{
	    p->setBrush(QColor(red,green,blue));
	}
	//*********************************************************************************************************************************

	double *x;
	double *y;
	int *nPoints;
	int *numRings;
	int *numPolygons;
	int numPoints;
	int numLineStrings;
	int idx, jdx, kdx;
	unsigned char *ptr;
	char lsb;
	QgsPoint pt;
	QPointArray *pa;
	OGRFieldDefn *fldDef;
	QString fld;
	QString val;
	switch (wkbType) { 
	    case wkbPoint://WKBPoint:
		//	fldDef = f->GetFieldDefnRef(1);
		//	 fld = fldDef->GetNameRef();
		val = f->GetFieldAsString(1);
		//std::cout << val << "\n";
				
		x = (double *) (feature + 5);
		y = (double *) (feature + 5 + sizeof(double));
		//std::cout << "transforming point\n";
		pt = t->transform(*x, *y);
		//std::cout << "drawing marker for feature " << featureCount << "\n";
		p->drawRect(pt.xToInt(), pt.yToInt(), 5, 5);
		//std::cout << "marker draw complete\n";
		break;
	    case wkbLineString://WKBLineString:
		// get number of points in the line 
		ptr = feature + 5;
		nPoints = (int *) ptr;
		ptr = feature + 1 + 2 * sizeof(int);
		for (idx = 0; idx < *nPoints; idx++) {
		    x = (double *) ptr;
		    ptr += sizeof(double);
		    y = (double *) ptr;
		    ptr += sizeof(double);
		    // transform the point
		    pt = t->transform(*x, *y);
		    if (idx == 0)
			p->moveTo(pt.xToInt(), pt.yToInt());
		    else
			p->lineTo(pt.xToInt(), pt.yToInt());
		}
		break;
	    case wkbMultiLineString://WKBMultiLineString:

		numLineStrings = (int) (feature[5]);
		ptr = feature + 9;
		for (jdx = 0; jdx < numLineStrings; jdx++) {
		    // each of these is a wbklinestring so must handle as such
		    lsb = *ptr;
		    ptr += 5;	// skip type since we know its 2
		    nPoints = (int *) ptr;
		    ptr += sizeof(int);
		    for (idx = 0; idx < *nPoints; idx++) {
			x = (double *) ptr;
			ptr += sizeof(double);
			y = (double *) ptr;
			ptr += sizeof(double);
			// transform the point
			pt = t->transform(*x, *y);
			if (idx == 0)
			    p->moveTo(pt.xToInt(), pt.yToInt());
			else
			    p->lineTo(pt.xToInt(), pt.yToInt());

		    }
		}
		break;
	    case wkbPolygon: //WKBPolygon:
		// get number of rings in the polygon
		numRings = (int *) (feature + 1 + sizeof(int));
		ptr = feature + 1 + 2 * sizeof(int);
		for (idx = 0; idx < *numRings; idx++) {
		    // get number of points in the ring
		    nPoints = (int *) ptr;
		    ptr += 4;
		    pa = new QPointArray(*nPoints);
		    for (jdx = 0; jdx < *nPoints; jdx++) {
			// add points to a point array for drawing the polygon
			x = (double *) ptr;
			ptr += sizeof(double);
			y = (double *) ptr;
			ptr += sizeof(double);
			pt = t->transform(*x, *y);
			pa->setPoint(jdx, pt.xToInt(), pt.yToInt());
		    }
		    // draw the ring
		    p->drawPolygon(*pa);

		}
		break;
	    case wkbMultiPolygon: //WKBMultiPolygon:
		// get the number of polygons
		ptr = feature + 5;
		numPolygons = (int *) ptr;
		for (kdx = 0; kdx < *numPolygons; kdx++) {
		    //skip the endian and feature type info and
		    // get number of rings in the polygon
		    ptr = feature + 14;
		    numRings = (int *) ptr;
		    ptr += 4;
		    for (idx = 0; idx < *numRings; idx++) {
			// get number of points in the ring
			nPoints = (int *) ptr;
			ptr += 4;
			pa = new QPointArray(*nPoints);
			for (jdx = 0; jdx < *nPoints; jdx++) {
			    // add points to a point array for drawing the polygon
			    x = (double *) ptr;
			    ptr += sizeof(double);
			    y = (double *) ptr;
			    ptr += sizeof(double);
			    // std::cout << "Transforming " << *x << "," << *y << " to ";

			    pt = t->transform(*x, *y);
			    //std::cout << pt.xToInt() << "," << pt.yToInt() << std::endl;
			    pa->setPoint(jdx, pt.xToInt(), pt.yToInt());

			}
			// draw the ring
			p->drawPolygon(*pa);
			delete pa;
		    }
		}
		break;
	}
		
	//std::cout << "deleting feature[]\n";
	//      std::cout << geom->getGeometryName() << std::endl;
	delete[] feature;
    }
    else
    {
	std::cout << "warning, null pointer in QgsContinuousColRenderer::renderFeature" << std::endl << std::flush;
    }
}*/

#endif
