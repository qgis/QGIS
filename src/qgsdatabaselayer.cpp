/***************************************************************************
                    qgsdatabaselayer.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/
/* $Id$ */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <qapplication.h>
#include <qcursor.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qbrush.h>

#include "qgis.h"
#include "qgsrect.h"
#include "qgspoint.h"
#include <libpq++.h>
#include <qmessagebox.h>
#include "qgsdatabaselayer.h"
#include "qgsidentifyresults.h"
#include "qgsattributetable.h"
#include "qgsattributetabledisplay.h"

QgsDatabaseLayer::QgsDatabaseLayer(const char *conninfo, QString table):QgsMapLayer(QgsMapLayer::DATABASE, table, conninfo),
tableName(table)
{
	// create the database layer and get the needed information
	// about it from the database
    qWarning("Incoming table name: " + tableName);
    // get the schema
    schema = tableName.left(tableName.find("."));
    geometryColumn = tableName.mid(tableName.find(" (") +2);
    geometryColumn.truncate(geometryColumn.length()-1);
    tableName = tableName.mid(tableName.find(".")+1, tableName.find(" (") - (tableName.find(".")+1));
 
    qWarning("Geometry column is: " + geometryColumn);
    qWarning("Schema is: " + schema);
    qWarning("Table name is: " + tableName);

	PgDatabase *pd = new PgDatabase(conninfo);
	if (pd->Status() == CONNECTION_OK) {
		// get the geometry column
		QString sql = "select f_geometry_column,type,srid from geometry_columns where f_table_name='" + tableName + "' and f_geometry_column = '" + geometryColumn + "' and f_table_schema = '" + schema + "'";
      qWarning("Getting geometry column: " + sql);

//		reset tableName to include schema
		tableName.prepend(schema+".");

		int result = pd->ExecTuplesOk((const char *) sql);
		if (result) {
			// store the srid 
			 srid = pd->GetValue(0, "srid");
			
			// set the simple type for use with symbology operations
			QString fType = pd->GetValue(0, "type");
			if (fType == "POINT" || fType == "MULTIPOINT")
				feature = QGis::WKBPoint;
			else if (fType == "LINESTRING" || fType == "MULTILINESTRING")
				feature = QGis::WKBLineString;
			else if (fType == "POLYGON" || fType == "MULTIPOLYGON")
				feature = QGis::WKBPolygon;

			// set the extent of the layer
			QString sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
			  "xmin(extent(" + geometryColumn + ")) as xmin,"
			  "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
          qWarning("Getting extents using schema.table: " + sql);
			result = pd->ExecTuplesOk((const char *) sql);

//			if that failed, and databasename == schema retry without 
//			the schema part
			if (result < 1  && schema == pd->DBName() ) {
				qWarning("Failed to get extents using schema.table -- trying without schema");
				tableName.remove(0,(int) tableName.find(".",0)+1);
				sql = "select xmax(extent(" + geometryColumn + 
				  ")) as xmax,"
				  "xmin(extent(" + geometryColumn + ")) as xmin,"
				  "ymax(extent(" + geometryColumn + 
				  ")) as ymax," "ymin(extent(" + geometryColumn + 
				  ")) as ymin" " from " + tableName;
				result = pd->ExecTuplesOk((const char *) sql);
			}

//			if that failed, and databasename == schema retry without 
//			the schema part
		/*	if (result < 1  && schema == pd->DBName() ) {
				tableName.remove(0,(int) tableName.find(".",0)+1);
				sql = "select xmax(extent(" + geometryColumn + 
				  ")) as xmax,"
				  "xmin(extent(" + geometryColumn + ")) as xmin,"
				  "ymax(extent(" + geometryColumn + 
				  ")) as ymax," "ymin(extent(" + geometryColumn + 
				  ")) as ymin" " from " + tableName;
				result = pd->ExecTuplesOk((const char *) sql);
			}
*/
			if (result) {
				//QString vRight = pd->GetValue(0, "right");
				layerExtent.setXmax(QString(pd->GetValue(0, "xmax")).toDouble());
				layerExtent.setXmin(QString(pd->GetValue(0, "xmin")).toDouble());
				layerExtent.setYmax(QString(pd->GetValue(0, "ymax")).toDouble());
				layerExtent.setYmin(QString(pd->GetValue(0, "ymin")).toDouble());
				QString xMsg;
				QTextOStream(&xMsg).precision(18);
				QTextOStream(&xMsg).width(18);
				QTextOStream(&xMsg) << "Set extents to: " << layerExtent.
				  xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
             // qWarning(xMsg);

			} else {
				QString msg = "Unable to access " + tableName;
				qWarning(msg);
//				QMessageBox::warning(this,"Connection Problem",msg); 
				valid = false;
			}

		} else {
			QString msg = "Unable to get geometry information for " + tableName;
			qWarning(msg);
//			QMessageBox::warning(this,"Connection Problem",msg); 
			valid = false;
		}

		delete pd;
	}
}

QgsDatabaseLayer::~QgsDatabaseLayer()
{
}

QgsRect QgsDatabaseLayer::calculateExtent()
{
	return layerExtent;
}

void QgsDatabaseLayer::draw(QPainter * p, QgsRect * viewExtent, int yTransform)
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
	p->setPen(sym->color());
	//std::cout << "Drawing layer using view extent " << viewExtent->stringRep() << " with a y transform of " << yTransform << std::endl;
	PgCursor pgs(dataSource, "drawCursor");
	QString sql = "select asbinary(" + geometryColumn + ",'" + endianString() + "'";
//	QString sql = "select asbinary(" + geometryColumn;
	sql += ") as features from " + tableName;
	sql += " where " + geometryColumn;
	sql += " && GeometryFromText('BOX3D(" + viewExtent->stringRep();
	sql += ")'::box3d,";
	sql += srid;
	sql += ")";
//  qWarning(sql);
	pgs.Declare((const char *) sql, true);
	//! \todo Check return from Fetch();
	int res = pgs.Fetch();

	//std::cout << "Number of matching records: " << pgs.Tuples() << std::endl;
	for (int idx = 0; idx < pgs.Tuples(); idx++) {
		// allocate memory for the item
		char *feature = new char[pgs.GetLength(idx, 0) + 1];
		memset(feature, '\0', pgs.GetLength(idx, 0) + 1);
		memcpy(feature, pgs.GetValue(idx, 0), pgs.GetLength(idx, 0));
		wkbType = *(int *) (feature + 1);
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
		char *ptr;
		char lsb;

		QPointArray *pa;
		QBrush *brush;
		switch (wkbType) {
		  case QGis::WKBPoint:
			  x = (double *) (feature + 5);
			  y = (double *) (feature + 5 + sizeof(double));
			  p->drawRect((int) *x, yTransform - (int) *y, 15000, 15000);
			  break;
		  case QGis::WKBLineString:
			  // get number of points in the line
			  numPoints = (int) (feature + 1 + sizeof(int));
			  ptr = feature + 1 + 2 * sizeof(int);
			  for (idx = 0; idx < numPoints; idx++) {
				  x = (double *) ptr;
				  ptr += sizeof(double);
				  y = (double *) ptr;
				  ptr += sizeof(double);
				  if (idx == 0)
					  p->moveTo((int) *x, yTransform - (int) *y);
				  else
					  p->lineTo((int) *x, yTransform - (int) *y);

			  }
			  break;
		  case QGis::WKBMultiLineString:
			  numLineStrings = *((int *) (feature + 5));
			  ptr = feature + 9;
			  for (jdx = 0; jdx < numLineStrings; jdx++) {
				  // each of these is a wbklinestring so must handle as such
				  lsb = *ptr;
				  ptr += 5;		// skip type since we know its 2
				  nPoints = (int *) ptr;
				  ptr += sizeof(int);
				  for (idx = 0; idx < *nPoints; idx++) {
					  x = (double *) ptr;
					  ptr += sizeof(double);
					  y = (double *) ptr;
					  ptr += sizeof(double);
					  if (idx == 0)
						  p->moveTo((int) *x, yTransform - (int) *y);
					  else
						  p->lineTo((int) *x, yTransform - (int) *y);

				  }
			  }
			  break;
		  case QGis::WKBPolygon:
			  brush = new QBrush(sym->fillColor());
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
					  pa->setPoint(jdx, (int) *x, yTransform - (int) *y);
				  }
				  // draw the ring
				  p->drawPolygon(*pa);

			  }
			  break;
		  case QGis::WKBMultiPolygon:
			  brush = new QBrush(sym->fillColor());
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
						  pa->setPoint(jdx, (int) *x, yTransform - (int) *y);
					  }
					  // draw the ring
					  p->drawPolygon(*pa);
					  delete pa;
				  }
			  }
			  break;
		}

	}



}
void QgsDatabaseLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	// painter is active (begin has been called
	/* Steps to draw the layer
	   1. get the features in the view extent by SQL query
	   2. read WKB for a feature
	   3. transform
	   4. draw
	 */
	QgsSymbol *sym = symbol();
	QPen pen;
	pen.setColor(sym->color());
	pen.setWidth(sym->lineWidth());
	p->setPen(pen);
	PgCursor pgs(dataSource, "drawCursor");
	QString sql = "select asbinary(" + geometryColumn + ",'" + endianString() + "'";
//	QString sql = "select asbinary(" + geometryColumn;
	sql += ") as features from " + tableName;
	sql += " where " + geometryColumn;
	sql += " && GeometryFromText('BOX3D(" + viewExtent->stringRep();
	sql += ")'::box3d,";
	sql += srid;
	sql += ")";
//  qWarning(sql);
	pgs.Declare((const char *) sql, true);
	int res = pgs.Fetch();
	QString msg;
	QTextOStream(&msg) << "Number of matching records: " << pgs.Tuples() << endl;
//  qWarning(msg);
//  std::cout << "Using following transform parameters:\n" << cXf->showParameters() << std::endl;
	for (int idx = 0; idx < pgs.Tuples(); idx++) {
		// allocate memory for the item
		char *feature = new char[pgs.GetLength(idx, 0) + 1];
		memset(feature, '\0', pgs.GetLength(idx, 0) + 1);
		memcpy(feature, pgs.GetValue(idx, 0), pgs.GetLength(idx, 0));
//      this is platform specific 
		wkbType = *((int *) (feature + 1));
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
		char *ptr;
		char lsb;
		QgsPoint pt;
		QPointArray *pa;
		QBrush *brush;
		switch (wkbType) {
		  case QGis::WKBPoint:

			  x = (double *) (feature + 5);
			  y = (double *) (feature + 5 + sizeof(double));
			  pt = cXf->transform(*x, *y);
			  p->drawRect(pt.xToInt(), pt.yToInt(), 5, 5);
			  break;
		  case QGis::WKBLineString:

			  // get number of points in the line
			 lsb = *ptr;
			 	nPoints = (int *) (feature + 1 + sizeof(int));
				  ptr = feature + 1 + 2 * sizeof(int);	
				 // ptr += sizeof(int);
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
		  case QGis::WKBMultiLineString:

			  numLineStrings = *((int *) (feature + 5));
			  ptr = feature + 9;
			  for (jdx = 0; jdx < numLineStrings; jdx++) {
				  // each of these is a wbklinestring so must handle as such
				  lsb = *ptr;
				  ptr += 5;		// skip type since we know its 2
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
		  case QGis::WKBPolygon:
			  brush = new QBrush(sym->fillColor());
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
		  case QGis::WKBMultiPolygon:
			  brush = new QBrush(sym->fillColor());
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

	}



}

void QgsDatabaseLayer::identify(QgsRect * r)
{
// create a search filter  for identifying records

	QString sql = "select * from " + tableName;
	sql += " where " + geometryColumn;
	sql += " && GeometryFromText('BOX3D(" + r->stringRep();
	sql += ")'::box3d,";
	sql += srid;
	sql += ")";
	//qWarning(sql);
	// select the features
	PgCursor pgs(dataSource, "identifyCursor");

	pgs.Declare((const char *) sql, false);
	int res = pgs.Fetch();
	QString msg;
	QTextOStream(&msg) << "Number of matching records: " << pgs.Tuples() << endl;
//  qWarning(msg);
//  std::cout << "Using following transform parameters:\n" << cXf->showParameters() << std::endl;
	// create the results window
	if (pgs.Tuples() > 0) {
		QgsIdentifyResults *ir = new QgsIdentifyResults();
		// just show one result - modify this later
		int numFields = pgs.Fields();
		for (int i = 0; i < numFields; i++) {
			QString fld = pgs.FieldName(i);
			int fldType = pgs.FieldType(i);
			QString val;
			if (fldType == 16604)	// geometry
				val = "(geometry column)";
			else
				val = pgs.GetValue(0, i);
			ir->addAttribute(fld, val);
		}
		ir->setTitle(name());
		ir->show();

	} else {
		QMessageBox::information(0, "No features found", "No features were found in the active layer at the point you clicked");
	}

}
void QgsDatabaseLayer::table()
{
	// display the attribute table
	QString sql = "select * from " + tableName;
	//qWarning(sql);
	// select the features
	PgCursor pgs(dataSource, "attributeCursor");

	pgs.Declare((const char *) sql, false);
	int res = pgs.Fetch();
	QString msg;
	QTextOStream(&msg) << "Number of matching records: " << pgs.Tuples() << endl;
	// create the results window
	if (pgs.Tuples() > 0) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		QgsAttributeTableDisplay *at = new QgsAttributeTableDisplay();
		at->table()->setNumRows(pgs.Tuples());
		at->table()->setNumCols(pgs.Fields());
		// set the column headers
		QHeader *colHeader = at->table()->horizontalHeader();
		for (int h = 0; h < pgs.Fields(); h++) {
			colHeader->setLabel(h, pgs.FieldName(h));
		}
		// add the data to the rows
		for (int ir = 0; ir < pgs.Tuples(); ir++) {
			for (int ic = 0; ic < pgs.Fields(); ic++) {
				int fldType = pgs.FieldType(ic);
				QString val;
				if (fldType == 16604)	// geometry -- naughty -- shouldnt code to a value
					val = "(geometry column)";
				else
					val = pgs.GetValue(ir, ic);
				at->table()->setText(ir, ic, val);
			}

		}
		at->table()->setSorting(true);

		QApplication::restoreOverrideCursor();
		at->setTitle("Attribute table - " + name());
		at->show();
	}
}


int QgsDatabaseLayer::endian()
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

QString QgsDatabaseLayer::endianString()
{
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;
	int *ce = (int *) chkEndian;
	if (232 == *ce)
		return QString("NDR");
	else
		return QString("XDR");
}
