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
#include <qimage.h>
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
	std::cout << "Layer extent: " << layerExtent.stringRep() << std::endl;
	
	// clip raster extent to view extent
	QgsRect rasterExtent = viewExtent->intersect(&layerExtent);
	std::cout << "viewXMin: " << viewExtent->xMin() <<std::endl;
	std::cout << "viewYMax: " << viewExtent->yMax() <<std::endl;
	std::cout << "viewXMax: " << viewExtent->xMax() <<std::endl;
	std::cout << "viewYMin: " << viewExtent->yMin() <<std::endl;
	std::cout << "rasterXMin: " << rasterExtent.xMin() <<std::endl;
	std::cout << "rasterYMax: " << rasterExtent.yMax() <<std::endl;
	std::cout << "rasterXMax: " << rasterExtent.xMax() <<std::endl;
	std::cout << "rasterYMin: " << rasterExtent.yMin() <<std::endl;
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
	
	std::cout << "rXOff: " << rXOff <<std::endl;
	std::cout << "rYOff: " << rYOff <<std::endl;
	
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
	
	std::cout << "xMin: " << layerExtent.xMin() <<std::endl;
	std::cout << "yMax: " << layerExtent.yMax() <<std::endl;
	std::cout << "xMax: " << layerExtent.xMax() <<std::endl;
	std::cout << "yMin: " << layerExtent.yMin() <<std::endl;
	std::cout << "lXSize: " << lXSize <<std::endl;
	std::cout << "lYSize: " << lYSize <<std::endl;
	
	// if there is more than one raster band they can be for red, green, blue, etc.
	// so this loop doesn't make much sense right now
	// only handling the case of 1 raster band that uses a palette
	// to index the rgb color values
	// this works for GeoTIFFs from:
	// http://cugir.mannlib.cornell.edu/browse_map/quad_map.html
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		
		//std::cout << "gdalBand->GetOverviewCount(): " << gdalBand->GetOverviewCount() <<std::endl;
		
		//int nXSize = gdalBand->GetXSize() - nXOff;
		//int nYSize = gdalBand->GetYSize() - nYOff;
		// make sure we don't exceed size of raster
		rXSize = rXSize <? gdalBand->GetXSize();
		rYSize = rYSize <? gdalBand->GetYSize();		
		std::cout << "rXSize: " << rXSize <<std::endl;
		std::cout << "rYSize: " << rYSize <<std::endl;
		
		// read entire clipped area of raster
		// treat scandata as a pseudo-multidimensional array
		// RasterIO() takes care of scaling down image
		uint *scandata = (uint*) CPLMalloc(sizeof(uint)*lXSize * sizeof(uint)*lYSize);
		CPLErr result = gdalBand->RasterIO( 
				GF_Read, rXOff, rYOff, rXSize, rYSize, scandata, lXSize, lYSize, GDT_UInt32, 0, 0 );
			
		GDALColorTable *colorTable = gdalBand->GetColorTable();
			
		// print each point in scandata using color looked up in color table
		for (int y = 0; y < lYSize; y++) {
			for (int x =0; x < lXSize; x++) {
				const GDALColorEntry *colorEntry = GDALGetColorEntry(colorTable, scandata[y*lXSize + x]);
				p->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
				p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
			}
		}
			
		CPLFree(scandata);
	}
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}
