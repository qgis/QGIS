/***************************************************************************
                          qgsshapefilelayer.cpp  -  description
                             -------------------
    begin                : Thu Aug 1 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /*  $Id$  */
 
#include <iostream>
#include <strstream>
#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qmessagebox.h>
#include "qgsrect.h"
#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgsshapefilelayer.h"
#include "qgsidentifyresults.h"
#include "qgsattributetable.h"
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>

QgsShapeFileLayer::QgsShapeFileLayer(QString vectorLayerPath, QString baseName)
:QgsMapLayer(VECTOR, baseName, vectorLayerPath)
{

// test ogr access to a shapefile
    
	ogrDataSource = OGRSFDriverRegistrar::Open((const char *) dataSource);
	if (ogrDataSource != NULL) {
		//std::cout << "Adding " << dataSource << std::endl;
		ogrLayer = ogrDataSource->GetLayer(0);
		OGREnvelope *ext = new OGREnvelope();
		ogrLayer->GetExtent(ext);
		layerExtent.setXmax(ext->MaxX);
		layerExtent.setXmin(ext->MinX);
		layerExtent.setYmax(ext->MaxY);
		layerExtent.setYmin(ext->MinY);
		// get the feature type
		OGRFeature *feat = ogrLayer->GetNextFeature();
		if(feat){
			OGRGeometry *geom = feat->GetGeometryRef();
			if(geom){
				feature = geom->getGeometryType();
				ogrLayer->ResetReading();
			}else{
				valid = false;
			}
			delete feat;
		}else{
			valid = false;
		}
	} else {
		valid = false;
	}

	//create a boolean vector and set every entry to false

	if(valid)
	{
	    selected=new QValueVector<bool>(ogrLayer->GetFeatureCount(),false);
	}
	else
	{
	    selected=0;
	}
	tabledisplay=0;
	//draw the selected features in yellow
	selectionColor.setRgb(255,255,0);
}

QgsShapeFileLayer::~QgsShapeFileLayer()
{
//delete ogrDataSource;
    if(selected)
    {
	delete selected;
    }
    if(tabledisplay)
    {
	tabledisplay->close();
    }
}

/** No descriptions */
void QgsShapeFileLayer::registerFormats()
{
}

void QgsShapeFileLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	// painter is active (begin has been called
	/* Steps to draw the layer
	   1. get the features in the view extent by SQL query
	   2. read WKB for a feature
	   3. transform
	   4. draw
	 */
	// set pen and fill
	QgsSymbol *sym = symbol();
	QPen pen;
	pen.setColor(sym->color());
	pen.setWidth(sym->lineWidth());
	p->setPen(pen);


	QBrush *brush = new QBrush(sym->fillColor());

	// reset the pointer to read from start of features

	// set the spatial filter
//  std::cout << "Drawing " << dataSource << std::endl;
	OGRGeometry *filter = 0;
	filter = new OGRPolygon();
	std::ostrstream wktExtent;
	wktExtent << "POLYGON ((" << viewExtent->stringRep() << "))" << std::ends;
	char *wktText = wktExtent.str();

	OGRErr result = ((OGRPolygon *) filter)->importFromWkt(&wktText);
	if (result == OGRERR_NONE) {

	        
		ogrLayer->SetSpatialFilter(filter);
		int featureCount = 0;
		while (OGRFeature * fet = ogrLayer->GetNextFeature()) {
		if(!fet){
			std::cout << "get next feature returned null\n";
			}
		//std::cout << "reading feautures\n";
			//fet->DumpReadable(stdout);

		
		//if feature is selected, change the color of the painter
		if((*selected)[fet->GetFID()]==true)
		{
			// must change color of pen since it holds not only color
			// but line width
			pen.setColor(selectionColor);
		    p->setPen(pen);
		    brush->setColor(selectionColor);
		}
		else
		{
			pen.setColor(sym->color());
		    p->setPen(pen);  
		    brush->setColor(sym->fillColor());
		}

			OGRGeometry *geom = fet->GetGeometryRef();
			if(!geom){
				std::cout << "geom pointer is null" << std::endl;
				}
			// get the wkb representation
			unsigned char *feature = new unsigned char[geom->WkbSize()];
			if(!feature){
				std::cout << featureCount << "'the feature is null\n";
				}
			geom->exportToWkb((OGRwkbByteOrder) endian(), feature);


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
			  case WKBPoint:
			         p->setBrush(*brush);
			//	fldDef = fet->GetFieldDefnRef(1);
			//	 fld = fldDef->GetNameRef();
				val = fet->GetFieldAsString(1);
				//std::cout << val << "\n";
				
				  x = (double *) (feature + 5);
				  y = (double *) (feature + 5 + sizeof(double));
				  //std::cout << "transforming point\n";
				  pt = cXf->transform(*x, *y);
				  //std::cout << "drawing marker for feature " << featureCount << "\n";
				  p->drawRect(pt.xToInt(), pt.yToInt(), 5, 5);
				  //std::cout << "marker draw complete\n";
				  break;
			  case WKBLineString:
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
					  pt = cXf->transform(*x, *y);
					  if (idx == 0)
						  p->moveTo(pt.xToInt(), pt.yToInt());
					  else
						  p->lineTo(pt.xToInt(), pt.yToInt());
				  }
				  break;
			  case WKBMultiLineString:

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
						  pt = cXf->transform(*x, *y);
						  if (idx == 0)
							  p->moveTo(pt.xToInt(), pt.yToInt());
						  else
							  p->lineTo(pt.xToInt(), pt.yToInt());

					  }
				  }
				  break;
			  case WKBPolygon:
				  p->setBrush(*brush);
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
						  pt = cXf->transform(*x, *y);
						  pa->setPoint(jdx, pt.xToInt(), pt.yToInt());
					  }
					  // draw the ring
					  p->drawPolygon(*pa);

				  }
				  break;
			  case WKBMultiPolygon:
				  p->setBrush(*brush);
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

							  pt = cXf->transform(*x, *y);
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
			featureCount++;
			delete[]feature;
			//std::cout << "deleting fet\n";
			delete fet;
			//std::cout << "ready to fetch next feature\n";
		}
		//std::cout << "finished reading features\n";
//      std::cout << featureCount << " features in ogr layer within the extent" << std::endl;
		OGRGeometry *filt = ogrLayer->GetSpatialFilter();
		//filt->dumpReadable(stdout);
		ogrLayer->ResetReading();
	}


}

int QgsShapeFileLayer::endian()
{
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;
	int *ce = (int *) chkEndian;
	if (232 == *ce)
		return NDR;
	else
		return XDR;
}

void QgsShapeFileLayer::identify(QgsRect * r)
{
	OGRGeometry *filter = 0;
	filter = new OGRPolygon();
	std::ostrstream wktExtent;
	wktExtent << "POLYGON ((" << r->stringRep() << "))" << std::ends;
	char *wktText = wktExtent.str();

	OGRErr result = ((OGRPolygon *) filter)->importFromWkt(&wktText);
	if (result == OGRERR_NONE) {

		ogrLayer->SetSpatialFilter(filter);
		int featureCount = 0;
		// just id the first feature for now
		//while (OGRFeature * fet = ogrLayer->GetNextFeature()) {
		//}

		OGRFeature *fet = ogrLayer->GetNextFeature();
		if (fet) {
			// found feature - show it in the identify box
			QgsIdentifyResults *ir = new QgsIdentifyResults();
			// just show one result - modify this later
			int numFields = fet->GetFieldCount();
			for (int i = 0; i < numFields; i++) {
				// get the field definition
				OGRFieldDefn *fldDef = fet->GetFieldDefnRef(i);
				QString fld = fldDef->GetNameRef();
				OGRFieldType fldType = fldDef->GetType();
				QString val;
				//if(fldType ==  16604 )    // geometry
				val = "(geometry column)";
				// else
				val = fet->GetFieldAsString(i);
				ir->addAttribute(fld, val);
			}
			ir->setTitle(name());
			ir->show();

		} else {
			QMessageBox::information(0, "No features found", "No features were found in the active layer at the point you clicked");
		}
		ogrLayer->ResetReading();	
	}

}
void QgsShapeFileLayer::table()
{
    if(tabledisplay)
    {
	tabledisplay->raise();
    }
    else
    {
	// display the attribute table
	QApplication::setOverrideCursor(Qt::waitCursor);
	ogrLayer->SetSpatialFilter(0);
	OGRFeature *fet = ogrLayer->GetNextFeature();
	int numFields = fet->GetFieldCount();
	tabledisplay = new QgsAttributeTableDisplay();
        QObject:connect(tabledisplay,SIGNAL(deleted()),this,SLOT(invalidateTableDisplay()));
	tabledisplay->table()->setNumRows(ogrLayer->GetFeatureCount(true));
	tabledisplay->table()->setNumCols(numFields+1);//+1 for the id-column

	int row = 0;
	// set up the column headers
	QHeader *colHeader = tabledisplay->table()->horizontalHeader();
	colHeader->setLabel(0,"id");//label for the id-column
	for (int h = 1; h < numFields+1; h++) {
	    OGRFieldDefn *fldDef = fet->GetFieldDefnRef(h-1);
		QString fld = fldDef->GetNameRef();
		colHeader->setLabel(h, fld);
	}
	while (fet) {
	    
	    //id-field
	    tabledisplay->table()->setText(row,0,QString::number(fet->GetFID()));
	    tabledisplay->table()->insertFeatureId(fet->GetFID());//insert the id into the search tree of qgsattributetable
	    for (int i = 1; i < numFields+1; i++) {
			// get the field values
			QString val;
			//if(fldType ==  16604 )    // geometry
			val = "(geometry column)";
			// else
			val = fet->GetFieldAsString(i-1);
			
			tabledisplay->table()->setText(row, i, val);

		}
		row++;
		delete fet;
		fet = ogrLayer->GetNextFeature();

	}
	// reset the pointer to start of fetabledisplayures so
	// subsequent reads will not fail
	ogrLayer->ResetReading();
	tabledisplay->table()->setSorting(true);


	tabledisplay->setTitle("Tabledisplaytribute table - " + name());
	tabledisplay->show();
	tabledisplay->table()->clearSelection();//deselect the first row

	//select the rows of the already selected features
	QObject::disconnect(tabledisplay->table(),SIGNAL(selectionChanged()),tabledisplay->table(),SLOT(handleChangedSelections()));
	for(int i=0;i<ogrLayer->GetFeatureCount();i++)
	{
	    if((*selected)[i]==true)
	    {
		tabledisplay->table()->selectRow(i);
	    }
	}
	QObject::connect(tabledisplay->table(),SIGNAL(selectionChanged()),tabledisplay->table(),SLOT(handleChangedSelections()));

	//etablish the necessary connections between the table and the shapefilelayer
	QObject::connect(tabledisplay->table(),SIGNAL(selected(int)),this,SLOT(select(int)));
	QObject::connect(tabledisplay->table(),SIGNAL(selectionRemoved()),this,SLOT(removeSelection()));
	QObject::connect(tabledisplay->table(),SIGNAL(repaintRequested()),this,SLOT(triggerRepaint()));	
	QApplication::restoreOverrideCursor();
    }
}

void QgsShapeFileLayer::select(int number)
{
    (*selected)[number]=true;
}

void QgsShapeFileLayer::select(QgsRect* rect, bool lock)
{
  
    if(tabledisplay)
    {
	QObject::disconnect(tabledisplay->table(),SIGNAL(selectionChanged()),tabledisplay->table(),SLOT(handleChangedSelections()));
	QObject::disconnect(tabledisplay->table(),SIGNAL(selected(int)),this,SLOT(select(int)));//disconnecting because of performance reason
    }
    
    if(lock==false)
    {
	removeSelection();//only if ctrl-button is not pressed
	if(tabledisplay)
	{
	    tabledisplay->table()->clearSelection();
	}
    }
    
    OGRGeometry *filter = 0;
    filter = new OGRPolygon();
    std::ostrstream wktExtent;
    wktExtent << "POLYGON ((" << rect->stringRep() << "))" << std::ends;
    char *wktText = wktExtent.str();
    
    OGRErr result = ((OGRPolygon *) filter)->importFromWkt(&wktText);
    if (result == OGRERR_NONE)
    {
	ogrLayer->SetSpatialFilter(filter);
	int featureCount = 0;
	while (OGRFeature * fet = ogrLayer->GetNextFeature())
	{
	    if(fet)
	    {
		select(fet->GetFID());
		if(tabledisplay)
		{
		    tabledisplay->table()->selectRowWithId(fet->GetFID());
		    (*selected)[fet->GetFID()]=true;
		}
	    }
	}
	ogrLayer->ResetReading();
    }
    
    if(tabledisplay)
    {
	QObject::connect(tabledisplay->table(),SIGNAL(selectionChanged()),tabledisplay->table(),SLOT(handleChangedSelections()));
	QObject::connect(tabledisplay->table(),SIGNAL(selected(int)),this,SLOT(select(int)));//disconnecting because of performance reason
    }
    triggerRepaint();
}
    


/*
OGRGeometry *filter = 0;
	filter = new OGRPolygon();
	std::ostrstream wktExtent;
	wktExtent << "POLYGON ((" << r->stringRep() << "))" << std::ends;
	char *wktText = wktExtent.str();

	OGRErr result = ((OGRPolygon *) filter)->importFromWkt(&wktText);
	if (result == OGRERR_NONE) {

		ogrLayer->SetSpatialFilter(filter);
		int featureCount = 0;
		// just id the first feature for now
		//while (OGRFeature * fet = ogrLayer->GetNextFeature()) {
		//}

		OGRFeature *fet = ogrLayer->GetNextFeature();
		if (fet) {
			// found feature - show it in the identify box
			QgsIdentifyResults *ir = new QgsIdentifyResults();
			// just show one result - modify this later
			int numFields = fet->GetFieldCount();
			for (int i = 0; i < numFields; i++) {
				// get the field definition
				OGRFieldDefn *fldDef = fet->GetFieldDefnRef(i);
				QString fld = fldDef->GetNameRef();
				OGRFieldType fldType = fldDef->GetType();
				QString val;
				//if(fldType ==  16604 )    // geometry
				val = "(geometry column)";
				// else
				val = fet->GetFieldAsString(i);
				ir->addAttribute(fld, val);
			}
			ir->setTitle(name());
			ir->show();
			ogrLayer->ResetReading();//remove this, if it is not a success

		} else {
			QMessageBox::information(0, "No features found", "No features were found in the active layer at the point you clicked");
		}
	}
*/

void QgsShapeFileLayer::removeSelection()
{
    for(int i=0;i<(int)selected->size();i++)
    {
	(*selected)[i]=false;
    }
}

void QgsShapeFileLayer::triggerRepaint()
{
    emit repaintRequested();
}

void QgsShapeFileLayer::invalidateTableDisplay()
{
    tabledisplay=0;
}
