/***************************************************************************
                         qgssinglesymrenderer.cpp  -  description
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
/* $Id$ */
#include "qgssinglesymrenderer.h"
#include "qgsfeature.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qstring.h"
#include "qgssisydialog.h"
#include "qgslegenditem.h"

QgsSingleSymRenderer::QgsSingleSymRenderer()
{

}

QgsSingleSymRenderer::~QgsSingleSymRenderer()
{

}

void QgsSingleSymRenderer::addItem(QgsRenderItem ri)
{
    m_item=ri;
    
}

void QgsSingleSymRenderer::renderFeature(QPainter* p, QgsFeature* f, QgsCoordinateTransform* t)
{
    qWarning("rendere feature");
    p->setPen(m_item.getSymbol()->pen());
    p->setBrush(m_item.getSymbol()->brush());
    unsigned char *feature= f->getGeometry();

				//  if (feature != 0) {
				//    std::cout << featureCount << "'the feature is null\n";

				int wkbType = (int) feature[1];
			//	std::cout << "Feature type: " << wkbType << std::endl;
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
			       
				switch (wkbType) {
				  case QGis::WKBPoint:
					 
					  //  fldDef = fet->GetFieldDefnRef(1);
					  //   fld = fldDef->GetNameRef();
					  //NEEDTHIS?  val = fet->GetFieldAsString(1);
					  //std::cout << val << "\n";

					  x = (double *) (feature + 5);
					  y = (double *) (feature + 5 + sizeof(double));
				//	  std::cout << "transforming point\n";
					  pt = t->transform(*x, *y);
					  //std::cout << "drawing marker for feature " << featureCount << "\n";
					  p->drawRect(pt.xToInt(), pt.yToInt(), 5, 5);
					  //std::cout << "marker draw complete\n";
					  break;
				  case QGis::WKBLineString:
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
				  case QGis::WKBMultiLineString:

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
				  case QGis::WKBPolygon:
					  // get number of rings in the polygon
					  numRings = (int *) (feature + 1 + sizeof(int));
					  //std::cout << "Number of rings: " << *numRings << std::endl;
					  ptr = feature + 1 + 2 * sizeof(int);
					  for (idx = 0; idx < *numRings; idx++) {
						  // get number of points in the ring
						  nPoints = (int *) ptr;
						  //std::cout << "Number of points: " << *nPoints << std::endl;
						  ptr += 4;
						  pa = new QPointArray(*nPoints);
						  for (jdx = 0; jdx < *nPoints; jdx++) {
							  // add points to a point array for drawing the polygon
							  //   std::cout << "Adding points to array\n";
							  x = (double *) ptr;
							  ptr += sizeof(double);
							  y = (double *) ptr;
							  ptr += sizeof(double);
							  pt = t->transform(*x, *y);
							  pa->setPoint(jdx, pt.xToInt(), pt.yToInt());
						  }
						  // draw the ring
						  //std::cout << "Drawing the polygon\n";
						  p->drawPolygon(*pa);

					  }

					  break;
				  case QGis::WKBMultiPolygon:
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
				    default:
					std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
					break;
				}

				//std::cout << "deleting feature[]\n";
				//      std::cout << geom->getGeometryName() << std::endl;
			     
        //std::cout << "Feature count: " << featureCount << std::endl;
				delete[]feature;
}

void QgsSingleSymRenderer::initializeSymbology(QgsVectorLayer* layer, QgsVectorLayerProperties* pr)
{
    bool toproperties=false;//if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if(pr)
    {
	toproperties=true;
    }

    if(layer)
    {
	QgsSymbol sy;
	sy.brush().setStyle(Qt::SolidPattern);
	sy.pen().setStyle(Qt::SolidLine);

	//random fill colors for points and polygons and pen colors for lines
	int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
    
	//font tor the legend text
	//TODO Make the font a user option
	QFont f( "times", 12, QFont::Normal );
	QFontMetrics fm(f);

	
	QPixmap* pixmap;
	if(toproperties)
	{
	    pixmap=pr->getBufferPixmap();
	}
	else
	{
	    pixmap=layer->legendPixmap();
	}
	QString name=layer->name();
	int width=40+fm.width(layer->name());
	int height=(fm.height()+10>35) ? fm.height()+10 : 35;

	pixmap->resize(width,height);
	pixmap->fill();
	QPainter p(pixmap);
	p.setPen(sy.pen());
	
	if(layer->vectorType()==QGis::Line)
	{
	    sy.pen().setColor(QColor(red,green,blue));
	    //paint the pixmap for the legend
	    p.setPen(sy.pen());
	    p.drawLine(10,pixmap->height()-25,25,pixmap->height()-10);
	}
	else
	{
	    sy.brush().setColor(QColor(red,green,blue));
	    sy.pen().setColor(QColor(0,0,0));
	    //paint the pixmap for the legend
	    p.setPen(sy.pen());
	    p.setBrush(sy.brush());
	    if(layer->vectorType()==QGis::Point)
	    {
		p.drawRect(20,pixmap->height()-17,5,5);	
	    }
	    else//polygon
	    {
		p.drawRect(10,pixmap->height()-25,20,15);	
	    }
			
	}

	p.setPen( Qt::black );
	p.setFont( f );
	p.drawText(35,pixmap->height()-10,name);
	QgsRenderItem ri(sy,"", "");
	addItem(ri);

	QgsSiSyDialog* dialog=new QgsSiSyDialog(layer);
	if(toproperties)
	{
	    pr->setBufferDialog(dialog);
	}
	else
	{
	    layer->setRendererDialog(dialog);
	    QgsLegendItem* item;
	    if(item=layer->legendItem())
	    {
		item->setPixmap(0,(*pixmap));
	    }
	}
    }
    else
    {
	qWarning("Warning, null pointer in QgsSingleSymRenderer::initializeSymbology()");
    }
}


