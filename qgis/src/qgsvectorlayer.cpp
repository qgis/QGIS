/***************************************************************************
                          qgsvectorlayer.cpp 
 This class implements a generic means to display vector layers. The features
 and attributes are read from the data store using a "data provider" plugin.
 QgsVectorLayer can be used with any data store for which an appropriate 
 plugin is available. 
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /*  $Id$ */

#include <iostream>
#include <cfloat>
#include <sstream>
#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include "qgisapp.h"
#include "qgsrect.h"
#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgsvectorlayer.h"
#include "qgsidentifyresults.h"
#include "qgsattributetable.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include <qlistview.h>
#include <qlibrary.h>
#include "qgsrenderer.h"
#include "qgslegenditem.h"
#include "qgsvectorlayerproperties.h"
#include "qgssinglesymrenderer.h"
#include "qgsrenderitem.h"
#include "qgssisydialog.h"
#include "qgsproviderregistry.h"
#include "qgsrect.h"
#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider *create_it(const char *uri);

QgsVectorLayer::QgsVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
    :QgsMapLayer(VECTOR, baseName, vectorLayerPath), providerKey(providerKey), 
    tabledisplay(0), m_renderer(0), m_propertiesDialog(0), m_rendererDialog(0) 
{
  std::cerr << "VECTORLAYERPATH: " << vectorLayerPath << std::endl;
  std::cerr << "BASENAME: " << baseName << std::endl;
// load the plugin
QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
QString ogrlib = pReg->library(providerKey);
//QString ogrlib = libDir + "/libpostgresprovider.so";
const char *cOgrLib = (const char *)ogrlib;
#ifdef TESTPROVIDERLIB
// test code to help debug provider loading problems
	void *handle = dlopen(cOgrLib, RTLD_LAZY);
	if (!handle) {
		std::cout << "Error in dlopen: " << dlerror() << std::endl;

	} else {
		std::cout << "dlopen suceeded" << std::endl;
		dlclose(handle);
	}

#endif
// load the data provider
	QLibrary *myLib = new QLibrary((const char *)ogrlib);
	std::cout << "Library name is " << myLib->library() << std::endl;
	bool loaded = myLib->load();
	if (loaded) {
		std::cout << "Loaded data provider library" << std::endl;
		std::cout << "Attempting to resolve the classFactory function" << std::endl;
		create_it *cf = (create_it *) myLib->resolve("classFactory");
    valid = false; // assume the layer is invalid until we determine otherwise
		if (cf) {
			std::cout << "Getting pointer to a dataProvider object from the library\n";
			dataProvider = cf(vectorLayerPath);
			if (dataProvider) {
				std::cout << "Instantiated the data provider plugin\n";
         
				if(dataProvider->isValid()){
          valid = true;
				// get the extent
				QgsRect *mbr = dataProvider->extent();
        // show the extent
				QString s = mbr->stringRep();
				std::cout << "Extent of layer: " << s << std::endl;
				// store the extent
				layerExtent.setXmax(mbr->xMax());
				layerExtent.setXmin(mbr->xMin());
				layerExtent.setYmax(mbr->yMax());
				layerExtent.setYmin(mbr->yMin());
				// get and store the feature type
				geometryType = dataProvider->geometryType();
        // look at the fields in the layer and set the primary
        // display field using some real fuzzy logic
        setDisplayField();
         }
			} else {
				std::cout << "Unable to instantiate the data provider plugin\n";
        valid = false;
			}
		}
	} else {
    valid = false;
		std::cout << "Failed to load " << "../providers/libproviders.so" << "\n";
	}
	//TODO - fix selection code that formerly used
  //       a boolean vector and set every entry to false

	//draw the selected features in yellow
	selectionColor.setRgb(255, 255, 0);
  
  // Default for the popup menu
  popMenu = 0;
}

QgsVectorLayer::~QgsVectorLayer()
{
	if (tabledisplay) {
		tabledisplay->close();
		delete tabledisplay;
	}
	if(m_renderer)
	{
	    delete m_renderer;
	}
	if(m_propertiesDialog)
	{
	    delete m_propertiesDialog;
	}
	if(m_rendererDialog)
	{
	    delete m_rendererDialog;
	}
}

QString QgsVectorLayer::providerType()
{
  return providerKey;
}
/** 
* sets the preferred display field based on some fuzzy logic
*/
void QgsVectorLayer::setDisplayField(){
// Determine the field index for the feature column of the identify
// dialog. We look for fields containing "name" first and second for
// fields containing "id". If neither are found, the first field
// is used as the node.
	QString idxName;
	QString idxId;
			
   std::vector<QgsField> fields = dataProvider->fields();
   int j = 0;
   for(int j=0; j< fields.size(); j++){
     
			QString fldName = fields[j].getName();
					std::cout << "Checking field " << fldName << std::endl;
					if (fldName.find("name", false) > -1) {
						idxName = fldName;
						break;
					}
          if(fldName.find("descrip", false) > -1){
            idxName = fldName;
            break;
          }
					if (fldName.find("id", false)> -1) {
						idxId = fldName;
						break;
					}
				}
        
				
				if (idxName.length() > 0) {
					fieldIndex = idxName;
				} else {
					if (idxId.length() > 0) {
						fieldIndex = idxId;
					}else{
            fieldIndex = fields[0].getName();
          }
				}
}
/*
* Draw the layer
*/
void QgsVectorLayer::draw_old(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	// set pen and fill
	QgsSymbol *sym = symbol();
	QPen pen;
	pen.setColor(sym->color());
	pen.setWidth(sym->lineWidth());
	p->setPen(pen);


	QBrush *brush = new QBrush(sym->fillColor());
}

void QgsVectorLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
    if (/*1 == 1*/m_renderer) {
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

		// select the records in the extent. The provider sets a spatial filter
		// and sets up the selection set for retrieval
		qWarning("Selecting features based on view extent");
		dataProvider->reset();
		dataProvider->select(viewExtent);
		int featureCount = 0;
		//  QgsFeature *ftest = dataProvider->getFirstFeature();
		qWarning("Starting draw of features");
		QgsFeature *fet;
		unsigned char *feature;
		bool attributesneeded = m_renderer->needsAttributes();

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
		int wkbType;

		while ((fet = dataProvider->getNextFeature(attributesneeded))) {//true is necessary for graduated symbol

			if (fet == 0) {
				std::cout << "get next feature returned null\n";
			} else {
			//	std::cout << "get next feature returned valid feature\n";

				//if feature is selected, change the color of the painter
				//TODO fix this selection code to work with the provider
				//if ((*selected)[(fet->featureId())] == true)
			    if(selected.find(fet->featureId())!=selected.end())
				{
				    // must change color of pen since it holds not only color
				    // but line width
				    if(vectorType()==QGis::Line)
				    pen.setColor(selectionColor);
				    else
				    {
					pen.setColor(Qt::black);
				    }
				    p->setPen(pen);
				    brush->setColor(selectionColor); 

				/* OGRGeometry *geom = fet->GetGeometryRef();
				   if (!geom) {
				   std::cout << "geom pointer is null" << std::endl;
				   } */
				// get the wkb representation
				feature = fet->getGeometry();
				//  if (feature != 0) {
				//    std::cout << featureCount << "'the feature is null\n";

				wkbType = (int) feature[1];
			//	std::cout << "Feature type: " << wkbType << std::endl;
				// read each feature based on its type
				
				switch (wkbType) {
				  case WKBPoint:

				      p->setBrush(*brush);
				      //  fldDef = fet->GetFieldDefnRef(1);
				      //   fld = fldDef->GetNameRef();
				      //NEEDTHIS?  val = fet->GetFieldAsString(1);
				      //std::cout << val << "\n";

				      x = (double *) (feature + 5);
				      y = (double *) (feature + 5 + sizeof(double));
				      //	  std::cout << "transforming point\n";
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
					      pt = cXf->transform(*x, *y);
					      pa->setPoint(jdx, pt.xToInt(), pt.yToInt());
					  }
					  // draw the ring
					  //std::cout << "Drawing the polygon\n";
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

            default:
              std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
              break;
				}
				delete[] feature;
				}
				else
				{
				    //pass the feature to the renderer
				    m_renderer->renderFeature(p, fet, cXf);
				}
				//std::cout << "deleting feature[]\n";
				//      std::cout << geom->getGeometryName() << std::endl;
				featureCount++;
        //std::cout << "Feature count: " << featureCount << std::endl;
				//delete[]feature;
			}
		}
		qApp->processEvents();
	}
	else
	{
	    qWarning("Warning, QgsRenderer is null in QgsVectorLayer::draw()");
	}
}

int QgsVectorLayer::endian()
{
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;

	int *ce = (int *) chkEndian;
	int retVal;
	if (232 == *ce)
		retVal = NDR;
	else
		retVal = XDR;
	delete[]chkEndian;
	return retVal;
}

void QgsVectorLayer::identify(QgsRect * r)
{
  	QApplication::setOverrideCursor(Qt::waitCursor);
  dataProvider->select(r);
		int featureCount = 0;
    QgsFeature *fet;
		unsigned char *feature;
    // display features falling within the search radius
		QgsIdentifyResults *ir = 0;
		while ((fet = dataProvider->getNextFeature(true))) {
      featureCount++;
      if(featureCount == 1){
        ir = new QgsIdentifyResults();
      }
      	
				QListViewItem *featureNode = ir->addNode("foo");
        featureNode->setText(0, fieldIndex);
        std::vector<QgsFeatureAttribute> attr = fet->attributeMap();
        for(int i=0; i < attr.size(); i++){
          std::cout << attr[i].fieldName()<< " == " << fieldIndex << std::endl;
          if(attr[i].fieldName().lower() == fieldIndex){
            featureNode->setText(1, attr[i].fieldValue());
          }
					ir->addAttribute(featureNode, attr[i].fieldName(), attr[i].fieldValue());
				}
       
			}

		std::cout << "Feature count on identify: " << featureCount << std::endl;
		if (ir) {
			ir->setTitle(name());
			ir->show();
		}
    QApplication::restoreOverrideCursor();
		if (featureCount == 0) {
			QMessageBox::information(0, tr("No features found"), tr("No features were found in the active layer at the point you clicked"));
		}

}
void QgsVectorLayer::table()
{
	if (tabledisplay) {
		tabledisplay->raise();
	} else {
		// display the attribute table
		QApplication::setOverrideCursor(Qt::waitCursor);
		dataProvider->reset();
		int numFields = dataProvider->fieldCount();
		tabledisplay = new QgsAttributeTableDisplay();
	        QObject:connect(tabledisplay, SIGNAL(deleted()), this, SLOT(invalidateTableDisplay()));
		tabledisplay->table()->setNumRows(dataProvider->featureCount());
		//tabledisplay->table()->setNumCols(numFields);	//+1 for the id-column
		tabledisplay->table()->setNumCols(numFields+1); //+1 for the id-column

		int row = 0;
		// set up the column headers
		QHeader *colHeader = tabledisplay->table()->horizontalHeader();
		colHeader->setLabel(0, "id");	//label for the id-column
    std::vector<QgsField> fields = dataProvider->fields();
    //for (int h = 0; h < numFields; h++) {
    for(int h=1;h<=numFields;h++)
    {
	colHeader->setLabel(h, fields[h-1].getName());
    }
    QgsFeature *fet;
		while ((fet = dataProvider->getNextFeature(true))) {

			//id-field
		tabledisplay->table()->setText(row, 0, QString::number(fet->featureId()));
		tabledisplay->table()->insertFeatureId(fet->featureId(),row);//insert the id into the search tree of qgsattributetable
     std::vector<QgsFeatureAttribute> attr = fet->attributeMap();
        for(int i=0; i < attr.size(); i++){
				// get the field values
				tabledisplay->table()->setText(row, i+1, attr[i].fieldValue());
        }
        row++;
        delete fet;
			}
			
		// reset the pointer to start of fetabledisplayures so
		// subsequent reads will not fail
		dataProvider->reset();
		tabledisplay->table()->setSorting(true);


		tabledisplay->setTitle(tr("Tabledisplaytribute table - ") + name());
		tabledisplay->show();
		tabledisplay->table()->clearSelection();	//deselect the first row

		
		QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
	
		for(std::map<int,bool>::iterator it=selected.begin();it!=selected.end();++it)
		{
			tabledisplay->table()->selectRowWithId(it->first);
			qWarning("selecting row with id " +QString::number(it->first));
		}

		QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));

		//etablish the necessary connections between the table and the shapefilelayer
		QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));
		QObject::connect(tabledisplay->table(), SIGNAL(selectionRemoved()), this, SLOT(removeSelection()));
		QObject::connect(tabledisplay->table(), SIGNAL(repaintRequested()), this, SLOT(triggerRepaint()));
		QApplication::restoreOverrideCursor();
	}
	
}

void QgsVectorLayer::select(int number)
{
    selected[number]=true;
}

void QgsVectorLayer::select(QgsRect * rect, bool lock)
{
    QApplication::setOverrideCursor(Qt::waitCursor);
    if (tabledisplay) 
    {
	QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
	QObject::disconnect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));	//disconnecting because of performance reason
    }

    if (lock == false) 
    {
	removeSelection();		//only if ctrl-button is not pressed
	if (tabledisplay) 
	{
	    tabledisplay->table()->clearSelection();
	}
    }

    dataProvider->select(rect);
	
    QgsFeature* fet;

    while (fet = dataProvider->getNextFeature(true))
    {
	select(fet->featureId());
	if (tabledisplay) 
	{
	    tabledisplay->table()->selectRowWithId(fet->featureId());
	}
    }

    if (tabledisplay) 
    {
	QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
	QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));	//disconnecting because of performance reason
    }
    triggerRepaint();
    QApplication::restoreOverrideCursor();
}

void QgsVectorLayer::removeSelection()
{
    selected.clear();
}

void QgsVectorLayer::triggerRepaint()
{
	emit repaintRequested();
}

void QgsVectorLayer::invalidateTableDisplay()
{
	tabledisplay = 0;
}

QgsDataProvider* QgsVectorLayer::getDataProvider()
{
    return dataProvider;
}

void QgsVectorLayer::showLayerProperties()
{
    if(m_propertiesDialog)
    {
	m_propertiesDialog->raise();
	m_propertiesDialog->show();
    }
    else
    {
	m_propertiesDialog = new QgsVectorLayerProperties(this);
	m_propertiesDialog->show();
    }
}

QgsRenderer* QgsVectorLayer::renderer()
{
    return m_renderer;
}
	
QDialog* QgsVectorLayer::rendererDialog()
{
    return m_rendererDialog;
}
	
void QgsVectorLayer::setRenderer(QgsRenderer* r)
{
    if(m_renderer)//delete any previous renderer
    {
	delete m_renderer;
    }
    
    m_renderer=r;
}
	
void QgsVectorLayer::setRendererDialog(QDialog* dialog)
{
    if(m_rendererDialog)
    {
	delete m_rendererDialog;
    } 
    m_rendererDialog=dialog;
}

QGis::VectorType QgsVectorLayer::vectorType()
{
    if(dataProvider)
    {
	int type=dataProvider->geometryType();
	switch(type)
	{
	    case QGis::WKBPoint:
		return QGis::Point;
	    case QGis::WKBLineString:
		return QGis::Line;
	    case QGis::WKBPolygon:
		return QGis::Polygon;
	    case QGis::WKBMultiPoint:
		return QGis::Point;
	    case QGis::WKBMultiLineString:
		return QGis::Line;
	    case QGis::WKBMultiPolygon:
		return QGis::Polygon;
	}
    }
    else
    {
	qWarning("warning, pointer to dataProvider is null in QgsVectorLayer::vectorType()");
    }
}

QgsVectorLayerProperties* QgsVectorLayer::propertiesDialog()
{
    return m_propertiesDialog;
}
void QgsVectorLayer::initContextMenu(QgisApp *app){
  popMenu = new QPopupMenu();
  popMenu->insertItem(tr("&Zoom to extent of selected layer"), app, SLOT(zoomToLayerExtent()));
  popMenu->insertItem(tr("&Open attribute table"), app, SLOT(attributeTable()));
  popMenu->insertSeparator();
  popMenu->insertItem(tr("&Properties"), app, SLOT(layerProperties()));
  popMenu->insertSeparator();
  popMenu->insertItem(tr("&Remove"), app, SLOT(removeLayer()));
}
//
QPopupMenu *QgsVectorLayer::contextMenu(){
  return popMenu;
}

QgsRect QgsVectorLayer::bBoxOfSelected()
{
    QgsRect rect(DBL_MAX,DBL_MAX,-DBL_MAX,-DBL_MAX);
    dataProvider->reset();

    QgsFeature *fet;
    unsigned char *feature;

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
    int wkbType;

    while ((fet = dataProvider->getNextFeature(false)))
    {
	if(selected.find(fet->featureId())!=selected.end())
	{
	    feature = fet->getGeometry();
	    wkbType = (int) feature[1];
	    
	    
	    switch (wkbType) 
	    {
		case WKBPoint:
		    x = (double *) (feature + 5);
		    y = (double *) (feature + 5 + sizeof(double));
		    if(*x<rect.xMin())
		    {
			rect.setXmin(*x);
		    }
		    if(*x>rect.xMax())
		    {
			rect.setXmax(*x);
		    }
		    if(*y<rect.yMin())
		    {
			rect.setYmin(*y);
		    }
		    if(*y>rect.yMax())
		    {
			rect.setYmax(*y);
		    }
		    break;

		case WKBLineString:
		    // get number of points in the line
		    ptr = feature + 5;
		    nPoints = (int *) ptr;
		    ptr = feature + 1 + 2 * sizeof(int);
		    for (idx = 0; idx < *nPoints; idx++) 
		    {
			x = (double *) ptr;
			ptr += sizeof(double);
			y = (double *) ptr;
			ptr += sizeof(double);
			if(*x<rect.xMin())
			{
			    rect.setXmin(*x);
			}
			if(*x>rect.xMax())
			{
			    rect.setXmax(*x);
			}
			if(*y<rect.yMin())
			{
			    rect.setYmin(*y);
			}
			if(*y>rect.yMax())
			{
			    rect.setYmax(*y);
			}
		    }
		    break;

		case WKBMultiLineString:
		    numLineStrings = (int) (feature[5]);
		    ptr = feature + 9;
		    for (jdx = 0; jdx < numLineStrings; jdx++) 
		    {
			// each of these is a wbklinestring so must handle as such
			lsb = *ptr;
			ptr += 5;	// skip type since we know its 2
			nPoints = (int *) ptr;
			ptr += sizeof(int);
			for (idx = 0; idx < *nPoints; idx++) 
			{
			    x = (double *) ptr;
			    ptr += sizeof(double);
			    y = (double *) ptr;
			    ptr += sizeof(double);
			    if(*x<rect.xMin())
			    {
				rect.setXmin(*x);
			    }
			    if(*x>rect.xMax())
			    {
				rect.setXmax(*x);
			    }
			    if(*y<rect.yMin())
			    {
				rect.setYmin(*y);
			    }
			    if(*y>rect.yMax())
			    {
				rect.setYmax(*y);
			    }
			}
		    }
		    break;

		case WKBPolygon:
                    // get number of rings in the polygon
		    numRings = (int *) (feature + 1 + sizeof(int));
		    ptr = feature + 1 + 2 * sizeof(int);
		    for (idx = 0; idx < *numRings; idx++) 
		    {
			// get number of points in the ring
			nPoints = (int *) ptr;
			ptr += 4;
			for (jdx = 0; jdx < *nPoints; jdx++) 
			{
			    // add points to a point array for drawing the polygon
			    x = (double *) ptr;
			    ptr += sizeof(double);
			    y = (double *) ptr;
			    ptr += sizeof(double);
			    if(*x<rect.xMin())
			    {
				rect.setXmin(*x);
			    }
			    if(*x>rect.xMax())
			    {
				rect.setXmax(*x);
			    }
			    if(*y<rect.yMin())
			    {
				rect.setYmin(*y);
			    }
			    if(*y>rect.yMax())
			    {
				rect.setYmax(*y);
			    }
			}
		    }
		    break;

		case WKBMultiPolygon:
		    // get the number of polygons
		    ptr = feature + 5;
		    numPolygons = (int *) ptr;
		    for (kdx = 0; kdx < *numPolygons; kdx++) 
		    {
			//skip the endian and feature type info and
			// get number of rings in the polygon
			ptr = feature + 14;
			numRings = (int *) ptr;
			ptr += 4;
			for (idx = 0; idx < *numRings; idx++) 
			{
			    // get number of points in the ring
			    nPoints = (int *) ptr;
			    ptr += 4;
			    for (jdx = 0; jdx < *nPoints; jdx++) 
			    {
				// add points to a point array for drawing the polygon
				x = (double *) ptr;
				ptr += sizeof(double);
				y = (double *) ptr;
				ptr += sizeof(double);
				if(*x<rect.xMin())
				{
				    rect.setXmin(*x);
				}
				if(*x>rect.xMax())
				{
				    rect.setXmax(*x);
				}
				if(*y<rect.yMin())
				{
				    rect.setYmin(*y);
				}
				if(*y>rect.yMax())
				{
				    rect.setYmax(*y);
				}
			    }
			}
		    }
		    break;

		    default:
			std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
			break;
		    
	    }
	    delete[] feature;
	}
    }
    return rect;
}

void QgsVectorLayer::setLayerProperties(QgsVectorLayerProperties* properties)
{
    if(m_propertiesDialog)
    {
	delete m_propertiesDialog;
    }
    m_propertiesDialog=properties;
}
