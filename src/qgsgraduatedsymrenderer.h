/***************************************************************************
                         qgsgraduatedsymrenderer.h  -  description
                             -------------------
    begin                : Oct 2003
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

#ifndef QGSGRADUATEDSYMRENDERER_H
#define QGSGRADUATEDSYMRENDERER_H

#include "qgsrenderer.h"
#include "qgsrangerenderitem.h"
#include <list>
#include <iostream>
#include "qgspoint.h"
#include "qpainter.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"

/**This class contains the information for graduate symbol rendering*/
class QgsGraduatedSymRenderer: public QgsRenderer
{
 public:
    QgsGraduatedSymRenderer();
    ~QgsGraduatedSymRenderer();
    /**Adds a new item
    \param ri a pointer to the QgsRangeRenderItem to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when the instance is destroyed*/
    void addItem(QgsRangeRenderItem* ri);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all items*/
    void removeItems();
    /**Renders an OGRFeature
     \param p a painter (usually the one from the current map canvas)
     \param f a pointer to a feature to render
     \param t the transform object containing the information how to transform the map coordinates to screen coordinates*/
    void renderFeature(QPainter* p, QgsFeature* f, QgsCoordinateTransform* t);
    /**Sets the number of the classicifation field
    \param field the number of the field to classify*/
    void setClassificationField(int field);
    void initializeSymbology(QgsVectorLayer* layer);
    /**Returns the list with the render items*/
    std::list<QgsRangeRenderItem*>& items();
    /** Returns true*/
    bool needsAttributes();
 protected:
    /**Name of the classification field (it must be a numerical field)*/
    int m_classificationField;
    /**List holding the render items for the individual classes*/
    std::list<QgsRangeRenderItem*> m_items;
    
};

inline QgsGraduatedSymRenderer::QgsGraduatedSymRenderer()
{

}

inline void QgsGraduatedSymRenderer::addItem(QgsRangeRenderItem* ri)
{
    m_items.push_back(ri); 
}

inline int QgsGraduatedSymRenderer::classificationField() const
{
    return m_classificationField;
}

inline void QgsGraduatedSymRenderer::setClassificationField(int field)
{
    m_classificationField=field;
}

inline std::list<QgsRangeRenderItem*>& QgsGraduatedSymRenderer::items()
{
    return m_items;
}

inline bool QgsGraduatedSymRenderer::needsAttributes()
{
    return true;
}
/*inline void QgsGraduatedSymRenderer::renderFeature(QPainter* p, OGRFeature* f, QgsCoordinateTransform* t, int endian)
{
    //first find out the value for the classification attribute
    double value=f->GetFieldAsDouble(m_classificationField);
    //std::cout << "classification field: " << m_classificationField << std::endl << std::flush;
    //std::cout << "value: " << value << std::endl << std::flush;

    std::list<QgsRangeRenderItem*>::iterator it;
    //first find the first render item which contains the feature
    for(it=m_items.begin();it!=m_items.end();++it)
    {
	//std::cout << "lower value: " << (*it)->value().toDouble()  << std::endl << std::flush;
	//std::cout << "upper value: " << (*it)->upper_value().toDouble() << std::endl << std::flush;

	if(value>=(*it)->value().toDouble()&&value<=(*it)->upper_value().toDouble())
	{
	    //std::cout << "Wert gefunden" << std::endl << std::flush;
	    break;
	}
    }

    if(it==m_items.end())//value is contained in no item
    {
	//std::cout << "Wert ist in keiner Klasse enthalten" << std::endl << std::flush;
	return;
    }
    else
    {
	//set the qpen and qpainter to the right values
	p->setPen((*it)->getSymbol()->pen());
	p->setBrush((*it)->getSymbol()->brush());
    }

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
}*/



#endif
