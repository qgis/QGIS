/***************************************************************************
					qgsrasterlayer.cpp -  description
							 -------------------
	begin                : Sat Jun 22 2002
	copyright            : (C) 2002 by Gary E.Sherman
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
/* $Id$ */
#include <qpainter.h>
#include <stdio.h>

#include "qgspoint.h"
#include "qgsrect.h"
#include "qgsrasterlayer.h"
#include "gdal_priv.h"

QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
:QgsMapLayer(RASTER, baseName, path)
{
	//std::cout << "QgsRasterLayer::QgsRasterLayer()" << std::endl;
	
	GDALAllRegister();
	gdalDataset = (GDALDataset *) GDALOpen( path, GA_ReadOnly );
	if ( gdalDataset == NULL ) {
		valid = FALSE;
		return;
	}
	//std::cout << "Raster Count: " << gdalDataset->GetRasterCount() << std::endl;
	
	if( gdalDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
	{
		printf( "Origin = (%.6f,%.6f)\n",
				adfGeoTransform[0], adfGeoTransform[3] );
		
		printf( "Pixel Size = (%.6f,%.6f)\n",
				adfGeoTransform[1], adfGeoTransform[5] );
	}
	
	double XMax = adfGeoTransform[0] + gdalDataset->GetRasterXSize() * adfGeoTransform[1] +  
				  gdalDataset->GetRasterYSize() * adfGeoTransform[2];
	double YMin  = adfGeoTransform[3] + gdalDataset->GetRasterXSize() * adfGeoTransform[4] + 
				   gdalDataset->GetRasterYSize() * adfGeoTransform[5];
	
	layerExtent.setXmax(XMax);
	layerExtent.setXmin(adfGeoTransform[0]);
	layerExtent.setYmax(adfGeoTransform[3]);
	layerExtent.setYmin(YMin);
}

QgsRasterLayer::~QgsRasterLayer()
{
	GDALClose(gdalDataset);
}

void QgsRasterLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	//std::cout << "QgsRasterLayer::draw()" << std::endl;
	//std::cout << "gdalDataset->GetRasterCount(): " << gdalDataset->GetRasterCount() << std::endl;
	//std::cout << "Layer extent: " << layerExtent.stringRep() << std::endl;
	
	// clip raster extent to view extent
	QgsRect rasterExtent = viewExtent->intersect(&layerExtent);
	if (rasterExtent.isEmpty()) {
		// nothing to do
		return;
	}
	
	// calculate raster pixel offsets from origin to clipped rect
	// we're only interested in positive offsets where the origin of the raster
	// is northwest of the origin of the view
	int rXOff = static_cast<int>((viewExtent->xMin() - layerExtent.xMin()) / fabs(adfGeoTransform[1]));
	rXOff = rXOff >? 0;
	int rYOff = static_cast<int>((layerExtent.yMax() - viewExtent->yMax()) / fabs(adfGeoTransform[5]));
	rYOff = rYOff >? 0;
	
	// get dimensions of clipped raster image in raster pixel space
	double rXmin = (rasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
	double rXmax = (rasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
	double rYmin = (rasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
	double rYmax = (rasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];
	int rXSize = abs(static_cast<int>(rXmax - rXmin));
	int rYSize = abs(static_cast<int>(rYmax - rYmin));
	
	// get dimensions of clipped raster image in device coordinate space
	QgsPoint topLeft = cXf->transform(rasterExtent.xMin(), rasterExtent.yMax());
	QgsPoint bottomRight = cXf->transform(rasterExtent.xMax(), rasterExtent.yMin());
	int lXSize = bottomRight.xToInt() - topLeft.xToInt();
	int lYSize = bottomRight.yToInt() - topLeft.yToInt();
	
	// loop through raster bands
	// a band can have color values that correspond to colors in a palette
	// or it can contain the red, green or blue value for an rgb image
	// HLS, CMYK, or RGB alpha bands are silently ignored for now
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		
		//std::cout << "gdalBand->GetOverviewCount(): " << gdalBand->GetOverviewCount() <<std::endl;
		
		// make sure we don't exceed size of raster
		rXSize = rXSize <? gdalBand->GetXSize();
		rYSize = rYSize <? gdalBand->GetYSize();		
		
		// read entire clipped area of raster band
		// treat scandata as a pseudo-multidimensional array
		// RasterIO() takes care of scaling down image
		uint *scandata = (uint*) CPLMalloc(sizeof(uint)*lXSize * sizeof(uint)*lYSize);
		CPLErr result = gdalBand->RasterIO( 
				GF_Read, rXOff, rYOff, rXSize, rYSize, scandata, lXSize, lYSize, GDT_UInt32, 0, 0 );
							
		QString colorInterp = GDALGetColorInterpretationName(gdalBand->GetColorInterpretation());
		if ( colorInterp == "Palette") {
			// print each point in scandata using color looked up in color table
			GDALColorTable *colorTable = gdalBand->GetColorTable();
			
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {
					const GDALColorEntry *colorEntry = GDALGetColorEntry(colorTable, scandata[y*lXSize + x]);
					p->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
					p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
				}
			}			
		} else if ( colorInterp == "Red" ) {
			// print each point in scandata as the red part of an rgb value
			// this assumes that the red band will always be first
			// is that necessarily the case?
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {					
					p->setPen(QColor(scandata[y*lXSize + x], 0, 0));
					p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
				}
			}			
		} else if ( colorInterp == "Green" ) {
			// print each point in scandata as the green part of an rgb value
			p->setRasterOp(Qt::XorROP);
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {					
					p->setPen(QColor(0, scandata[y*lXSize + x], 0));
					p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
				}
			}
			p->setRasterOp(Qt::CopyROP);
		} else if ( colorInterp == "Blue" ) {
			// print each point in scandata as the blue part of an rgb value
			p->setRasterOp(Qt::XorROP);
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {					
					p->setPen(QColor(0, 0, scandata[y*lXSize + x]));
					p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
				}
			}
			p->setRasterOp(Qt::CopyROP);
		} else if ( colorInterp == "Gray" ) {
                       //ensure we are not still xoring
                        p->setRasterOp(Qt::CopyROP);  
			// print each point in scandata with equal parts R, G ,B o make it show as gray
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {	
                                        int myGrayValInt=scandata[y*lXSize + x];				
					p->setPen(QColor(myGrayValInt, myGrayValInt, myGrayValInt));
					p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
				}
			}			                      
		} else {
			// do nothing
		}
		
		CPLFree(scandata);
	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
